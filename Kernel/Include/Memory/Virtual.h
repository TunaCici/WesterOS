/*
 * Virtual memory manager for the ARMv8-A architecture
 *
 * References:
 * https://developer.arm.com/documentation/den0024/a/The-Memory-Management-Unit
 * https://github.com/ARM-software/u-boot/blob/master/arch/arm/include/asm/armv8/mmu.h
 * https://opensource.apple.com/source/xnu/xnu-6153.61.1/osfmk/arm64/proc_reg.h.auto.html
 * https://lowenware.com/blog/aarch64-mmu-programming/
 *
 * Author: Tuna CICI
 */

#ifndef VIRTUAL_H
#define VIRTUAL_H

#include <stdint.h>

#include "Memory/PageDef.h"

#define ENTRY_SIZE 512

/*
 * 4KB granule size:
 *
 * Level 0 Translation Table Entry
 *
 *  63 62 61 60  59 58   52 51  48 47                  12 11    2 1 0
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 * |NS|  AP |XN|PXN|ignored| zero | L1TableOutputAddress |ignored|1|V|
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 *
 * Level 1 Translation Table Entry
 *
 *  63 62 61 60  59 58   52 51  48 47                  12 11    2 1 0
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 * |NS|  AP |XN|PXN|ignored| zero | L2TableOutputAddress |ignored|1|V|
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 *
 * Level 1 Translation Block Entry
 *
 *  63 59 58  55 54  53   52 51  48 47                  30 29  12 11 10 9  8 7  6  5 4     2 1 0
 * +-----+------+--+---+----+------+----------------------+------+--+--+----+----+--+-------+-+-+
 * | ign |sw use|XN|PXN|HINT| zero | OutputAddress[47:30] | zero |nG|AF| SH | AP |NS|AttrIdx|0|V|
 * +-----+------+--+---+----+------+----------------------+------+--+--+----+----+--+-------+-+-+
 *
 * Level 2 Translation Table Entry
 *
 *  63 62 61 60  59 58   52 51  48 47                  12 11    2 1 0
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 * |NS|  AP |XN|PXN|ignored| zero | L3TableOutputAddress |ignored|1|V|
 * +--+-----+--+---+-------+------+----------------------+-------+-+-+
 *
 * Level 2 Translation Block Entry
 *
 *  63 59 58  55 54  53   52 51  48 47                  21 20  12 11 10 9  8 7  6  5 4     2 1 0
 * +-----+------+--+---+----+------+----------------------+------+--+--+----+----+--+-------+-+-+
 * | ign |sw use|XN|PXN|HINT| zero | OutputAddress[47:21] | zero |nG|AF| SH | AP |NS|AttrIdx|0|V|
 * +-----+------+--+---+----+------+----------------------+------+--+--+----+----+--+-------+-+-+
 *
 * Level 3 Translation Page entries
 *
 *  63 59 58  55 54  53   52 51  48 47                  12 11 10 9  8 7  6  5 4     2 1 0
 * +-----+------+--+---+----+------+----------------------+--+--+----+----+--+-------+-+-+
 * | ign |sw use|XN|PXN|HINT| zero | OutputAddress[47:12] |nG|AF| SH | AP |NS|AttrIdx|1|V|
 * +-----+------+--+---+----+------+----------------------+--+--+----+----+--+-------+-+-+
 *
 * where:
 *   XN:      eXecute Never bit
 *   PXN:     Privilege eXecute Never bit
 *   HINT:    16 entry continuguous output hint
 *   nG:      notGlobal bit
 *   AF:      Access Flag bit
 *   SH:      Shareability field
 *   AP:      access protection
 *   NS:      Non-Secure bit
 *   AttrIdx: Memory Attribute Index
 *   V:       Valid entry bit
 */

/* 4K L0 */
#define ARM_TT_L0_SIZE          0x0000008000000000ULL /* size of area covered by a tte */
#define ARM_TT_L0_OFFMASK       0x0000007FFFFFFFFFULL /* offset within an L0 entry */
#define ARM_TT_L0_SHIFT         39ULL                 /* page descriptor shift */
#define ARM_TT_L0_INDEX_MASK    0x0000FF8000000000ULL /* mask for getting index in L0 table from VA */

/* 4K L1 */
#define ARM_TT_L1_SIZE          0x0000000040000000ULL /* size of area covered by a tte */
#define ARM_TT_L1_OFFMASK       0x000000003FFFFFFFULL /* offset within an L1 entry */
#define ARM_TT_L1_SHIFT         30ULL                 /* page descriptor shift */
#define ARM_TT_L1_INDEX_MASK    0x0000007FC0000000ULL /* mask for getting index into L1 table from VA */

/* 4K L2 */
#define ARM_TT_L2_SIZE          0x0000000000200000ULL /* size of area covered by a tte */
#define ARM_TT_L2_OFFMASK       0x00000000001FFFFFULL /* offset within an L2 entry */
#define ARM_TT_L2_SHIFT         21ULL                 /* page descriptor shift */
#define ARM_TT_L2_INDEX_MASK    0x000000003FE00000ULL /* mask for getting index in L2 table from VA */

/* 4K L3 */
#define ARM_TT_L3_SIZE          0x0000000000001000ULL /* size of area covered by a tte */
#define ARM_TT_L3_OFFMASK       0x0000000000000FFFULL /* offset within L3 PTE */
#define ARM_TT_L3_SHIFT         12ULL                 /* page descriptor shift */
#define ARM_TT_L3_INDEX_MASK    0x00000000001FF000ULL /* mask for page descriptor index */

#define ARM_TE_VALID_SHIFT      0ULL
#define ARM_TE_VALID_WIDTH      1ULL
#define ARM_TE_VALID_MASK       0x0000000000000001ULL

#define ARM_TE_TYPE_SHIFT       1ULL
#define ARM_TE_TYPE_WIDTH       1ULL
#define ARM_TE_TYPE_MASK        0x0000000000000002ULL

#define ARM_TT_NEXT_SHIFT       12ULL
#define ARM_TT_NEXT_WIDTH       36ULL
#define ARM_TT_NEXT_MASK        0x0000FFFFFFFFF000ULL

#define ARM_TT_PXN_SHIFT        59ULL
#define ARM_TT_PXN_WIDTH        1ULL
#define ARM_TT_PXN_MASK         0x0800000000000000ULL

#define ARM_TT_XN_SHIFT         60ULL
#define ARM_TT_XN_WIDTH         1ULL
#define ARM_TT_XN_MASK          0x1000000000000000ULL

#define ARM_TT_AP_SHIFT         61ULL
#define ARM_TT_AP_WIDTH         2ULL
#define ARM_TT_AP_MASK          0x6000000000000000ULL

#define ARM_TT_NS_SHIFT         63ULL
#define ARM_TT_NS_WIDTH         1ULL
#define ARM_TT_NS_MASK          0x8000000000000000ULL

#define ARM_TB_AIDX_SHIFT       2ULL
#define ARM_TB_AIDX_WIDTH       3ULL
#define ARM_TB_AIDX_MASK        0x000000000000001CULL

#define ARM_TB_NS_SHIFT         5ULL
#define ARM_TB_NS_WIDTH         1ULL
#define ARM_TB_NS_MASK          0x0000000000000020ULL

#define ARM_TB_AP_SHIFT         6ULL
#define ARM_TB_AP_WIDTH         2ULL
#define ARM_TB_AP_MASK          0x00000000000000C0ULL

#define ARM_TB_SH_SHIFT         8ULL
#define ARM_TB_SH_WIDTH         2ULL
#define ARM_TB_SH_MASK          0x0000000000000300ULL

#define ARM_TB_AF_SHIFT         10ULL
#define ARM_TB_AF_WIDTH         1ULL
#define ARM_TB_AF_MASK          0x0000000000000400ULL

#define ARM_TB_NG_SHIFT         11ULL
#define ARM_TB_NG_WIDTH         1ULL
#define ARM_TB_NG_MASK          0x0000000000000800ULL

#define ARM_TB_L1NEXT_SHIFT     30ULL
#define ARM_TB_L1NEXT_WIDTH     18ULL
#define ARM_TB_L1NEXT_MASK      0x0000FFFFC0000000ULL

#define ARM_TB_L2NEXT_SHIFT     21ULL
#define ARM_TB_L2NEXT_WIDTH     27ULL
#define ARM_TB_L2NEXT_MASK      0x0000FFFFFFE00000ULL

#define ARM_TB_HINT_SHIFT       52ULL
#define ARM_TB_HINT_WIDTH       1ULL
#define ARM_TB_HINT_MASK        0x0010000000000000ULL

#define ARM_TB_PXN_SHIFT        53ULL
#define ARM_TB_PXN_WIDTH        1ULL
#define ARM_TB_PXN_MASK         0x0020000000000000ULL

#define ARM_TB_XN_SHIFT         54ULL
#define ARM_TB_XN_WIDTH         1ULL
#define ARM_TB_XN_MASK          0x0040000000000000ULL

/* some sugar for getting pointers to page tables and entries */
#define L0_TABLE_INDEX(va) (((va) & ARM_TT_L0_INDEX_MASK) >> ARM_TT_L0_SHIFT)
#define L1_TABLE_INDEX(va) (((va) & ARM_TT_L1_INDEX_MASK) >> ARM_TT_L1_SHIFT)
#define L2_TABLE_INDEX(va) (((va) & ARM_TT_L2_INDEX_MASK) >> ARM_TT_L2_SHIFT)
#define L3_TABLE_INDEX(va) (((va) & ARM_TT_L3_INDEX_MASK) >> ARM_TT_L3_SHIFT)

#define TABLE_DESC_NEXT(tbl) (((tbl) & ARM_TT_NEXT_MASK) >> ARM_TT_NEXT_SHIFT)
#define TABLE_DESC_VALID(tbl) (((tbl & RM_TT_VALID_MASK) >> ARM_TT_VALID_SHIFT))
#define TABLE_DESC_TYPE(tbl) (((tbl) & ARM_TT_TYPE_MASK) >> ARM_TT_TYPE_SHIFT)

/*
 * Translation Control Register (TCR)
 *
 * Current:
 *
 *  63  39   38   37 36   34 32    30 29 28 27 26 25 24   23 22 21  16    14 13 12 11 10 9   8    7   5  0
 * +------+----+----+--+-+-----+-+---+-----+-----+-----+----+--+------+-+---+-----+-----+-----+----+-+----+
 * | zero |TBI1|TBI0|AS|z| IPS |z|TG1| SH1 |ORGN1|IRGN1|EPD1|A1| T1SZ |z|TG0| SH0 |ORGN0|IRGN0|EPD0|z|T0SZ|
 * +------+----+----+--+-+-----+-+---+-----+-----+-----+----+--+------+-+---+-----+-----+-----+----+-+----+
 *
 * TBI1:  Top Byte Ignored for TTBR1 region
 * TBI0:  Top Byte Ignored for TTBR0 region
 * AS:    ASID Size
 * IPS:   Physical Address Size limit
 * TG1:   Granule Size for TTBR1 region
 * SH1:   Shareability for TTBR1 region
 * ORGN1: Outer Cacheability for TTBR1 region
 * IRGN1: Inner Cacheability for TTBR1 region
 * EPD1:  Translation table walk disable for TTBR1
 * A1:    ASID selection from TTBR1 enable
 * T1SZ:  Virtual address size for TTBR1
 * TG0:   Granule Size for TTBR0 region
 * SH0:   Shareability for TTBR0 region
 * ORGN0: Outer Cacheability for TTBR0 region
 * IRGN0: Inner Cacheability for TTBR0 region
 * EPD0:  Translation table walk disable for TTBR0
 * T0SZ:  Virtual address size for TTBR0
 */

/* TCR flags */
#define TCR_T0SZ_SHIFT          0ULL
#define TCR_T0SZ                16ULL // 2^(64 - T0SZ) IA bits for TTBR0

#define TCR_EPD0_SHIFT          7ULL
#define TCR_EPD0_DISABLE        ~(1ULL << TCR_EPD0_SHIFT)

#define TCR_TG0_GRANULE_SHIFT   14ULL
#define TCR_TG0_GRANULE_WIDTH   2ULL
#define TCR_TG0_GRANULE_CLEAR   (~(((1ULL << TCR_TG0_GRANULE_WIDTH) - 1) << TCR_TG0_GRANULE_SHIFT)) 
#define TCR_TG0_GRANULE_4KB     (0ULL << TCR_TG0_GRANULE_SHIFT)
#define TCR_TG0_GRANULE_64KB    (1ULL << TCR_TG0_GRANULE_SHIFT)
#define TCR_TG0_GRANULE_16KB    (2ULL << TCR_TG0_GRANULE_SHIFT)           

#define TCR_T1SZ_SHIFT          16ULL
#define TCR_T1SZ                16ULL // 2^(64 - T1SZ) IA bits for TTBR1

#define TCR_A1_SHIFT            22ULL
#define TCR_A1_TTBR0            ~(1ULL << TCR_A1_SHIFT)
#define TCR_A1_TTBR1            (1ULL << TCR_A1_SHIFT)

#define TCR_EPD1_SHIFT          23ULL
#define TCR_EPD1_DISABLE        ~(1ULL << TCR_EPD1_SHIFT)

#define TCR_TG1_GRANULE_SHIFT   30ULL
#define TCR_TG1_GRANULE_WIDTH   2ULL
#define TCR_TG1_GRANULE_CLEAR   (~(((1ULL << TCR_TG1_GRANULE_WIDTH) - 1) << TCR_TG1_GRANULE_SHIFT)) 
#define TCR_TG1_GRANULE_16KB    (1ULL << TCR_TG1_GRANULE_SHIFT)
#define TCR_TG1_GRANULE_4KB     (2ULL << TCR_TG1_GRANULE_SHIFT)
#define TCR_TG1_GRANULE_64KB    (3ULL << TCR_TG1_GRANULE_SHIFT)

#define TCR_IPS_SHIFT           32ULL
#define TCR_IPS_WIDTH           3ULL
#define TCR_IPS_CLEAR           (~(((1ULL << TCR_IPS_WIDTH) - 1) << TCR_IPS_SHIFT)) 

#define TCR_HDPN0_SHIFT         41ULL
#define TCR_HPDN0_ENABLE        ~(1ULL << TCR_HDPN0_SHIFT)

#define TCR_HDPN1_SHIFT         42ULL
#define TCR_HPDN1_ENABLE        ~(1ULL << TCR_HDPN1_SHIFT)

#define TCR_DS_SHIFT            59ULL
#define TCR_DS_48BITS           ~(1ULL << TCR_DS_SHIFT)
#define TCR_DS_52BITS           (1ULL << TCR_DS_SHIFT)

void init_kernel_pgtbl(void);
void init_tcr(void);
void init_mair(void);

#endif /* VIRTUAL_H */