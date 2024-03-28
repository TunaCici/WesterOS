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
 * Level 3 Page table entries
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
#define ARM_TT_L1_OFFMASK       0x000000003fffffffULL /* offset within an L1 entry */
#define ARM_TT_L1_SHIFT         30ULL                 /* page descriptor shift */
#define ARM_TT_L1_INDEX_MASK    0x0000007fc0000000ULL /* mask for getting index into L1 table from VA */

/* 4K L2 */
#define ARM_TT_L2_SIZE          0x0000000000200000ULL /* size of area covered by a tte */
#define ARM_TT_L2_OFFMASK       0x00000000001fffffULL /* offset within an L2 entry */
#define ARM_TT_L2_SHIFT         21ULL                 /* page descriptor shift */
#define ARM_TT_L2_INDEX_MASK    0x000000003fe00000ULL /* mask for getting index in L2 table from VA */

/* 4K L3 */
#define ARM_TT_L3_SIZE          0x0000000000001000ULL /* size of area covered by a tte */
#define ARM_TT_L3_OFFMASK       0x0000000000000fffULL /* offset within L3 PTE */
#define ARM_TT_L3_SHIFT         12ULL                 /* page descriptor shift */
#define ARM_TT_L3_INDEX_MASK    0x00000000001ff000ULL /* mask for page descriptor index */

/* some sugar for getting pointers to page tables and entries */
#define L1_TABLE_INDEX(va) (((va) & ARM_TT_L1_INDEX_MASK) >> ARM_TT_L1_SHIFT)
#define L2_TABLE_INDEX(va) (((va) & ARM_TT_L2_INDEX_MASK) >> ARM_TT_L2_SHIFT)
#define L3_TABLE_INDEX(va) (((va) & ARM_TT_L3_INDEX_MASK) >> ARM_TT_L3_SHIFT)

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