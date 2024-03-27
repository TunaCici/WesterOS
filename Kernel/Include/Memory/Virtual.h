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

#define L0_BLOCK_SIZE (1UL << 39) // 512 GiB
#define L1_BLOCK_SIZE (1UL << 30) // 1 GiB
#define L2_BLOCK_SIZE (1UL << 21) // 2 MiB
#define L3_BLOCK_SIZE (1UL << 12) // 4 KiB

/* Hardware page table definition */
#define PTE_TYPE_MASK           (3 << 0)
#define PTE_TYPE_FAULT          (0 << 0)
#define PTE_TYPE_TABLE          (3 << 0)
#define PTE_TYPE_PAGE           (3 << 0)
#define PTE_TYPE_BLOCK          (1 << 0)
#define PTE_TYPE_VALID          (1 << 0)

#define PTE_TABLE_PXN           (1UL << 59)
#define PTE_TABLE_XN            (1UL << 60)
#define PTE_TABLE_AP            (1UL << 61)
#define PTE_TABLE_NS            (1UL << 63)

/* Block */
#define PTE_BLOCK_MEMTYPE(x)    ((x) << 2)
#define PTE_BLOCK_NS            (1 << 5)
#define PTE_BLOCK_NON_SHARE     (0 << 8)
#define PTE_BLOCK_OUTER_SHARE   (2 << 8)
#define PTE_BLOCK_INNER_SHARE   (3 << 8)
#define PTE_BLOCK_AF            (1 << 10)
#define PTE_BLOCK_NG            (1 << 11)
#define PTE_BLOCK_PXN           (UL(1) << 53)
#define PTE_BLOCK_UXN           (UL(1) << 54)

/* AttrIndx[2:0] */
#define PMD_ATTRINDX(t)         ((t) << 2)
#define PMD_ATTRINDX_MASK       (7 << 2)
#define PMD_ATTRMASK            (PTE_BLOCK_PXN          | \
                                 PTE_BLOCK_UXN          | \
                                 PMD_ATTRINDX_MASK      | \
                                 PTE_TYPE_VALID)

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