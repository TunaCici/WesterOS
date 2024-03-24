/*
 * Virtual memory manager for the ARMv8-A architecture
 *
 * References:
 * https://developer.arm.com/documentation/den0024/a/The-Memory-Management-Unit
 * https://github.com/ARM-software/u-boot/blob/master/arch/arm/include/asm/armv8/mmu.h
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

/* TCR flags */
#define TCR_T1SZ                (16) // 2^(64 - T1SZ) bits for TTBR1
#define TCR_T0SZ                (16) // 2^(64 - T0SZ) bits for TTBR0
#define TCR_IRGN_NC             (0 << 8)
#define TCR_IRGN_WBWA           (1 << 8)
#define TCR_IRGN_WT             (2 << 8)
#define TCR_IRGN_WBNWA          (3 << 8)
#define TCR_IRGN_MASK           (3 << 8)
#define TCR_ORGN_NC             (0 << 10)
#define TCR_ORGN_WBWA           (1 << 10)
#define TCR_ORGN_WT             (2 << 10)
#define TCR_ORGN_WBNWA          (3 << 10)
#define TCR_ORGN_MASK           (3 << 10)
#define TCR_SHARED_NON          (0 << 12)
#define TCR_SHARED_OUTER        (2 << 12)
#define TCR_SHARED_INNER        (3 << 12)
#define TCR_TG0_4K              (0 << 14)
#define TCR_TG0_64K             (1 << 14)
#define TCR_TG0_16K             (2 << 14)
#define TCR_EPD1_DISABLE        (1 << 23)
#define TCR_DS_48BITS           ~(1UL << 59)
#define TCR_DS_52BITS           (1UL << 59)
#define TCR_T1SZ_SHIFT          (0)
#define TCR_T0SZ_SHIFT          (16)
#define TCR_IPS_SHIFT           (32)

#define TCR_EL1_RSVD            (1 << 31)
#define TCR_EL2_RSVD            (1 << 31 | 1 << 23)
#define TCR_EL3_RSVD            (1 << 31 | 1 << 23)

void init_kernel_pgtbl(void);
void init_tcr(void);
void init_mair(void);

#endif /* VIRTUAL_H */