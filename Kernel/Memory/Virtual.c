/*
 * Virtual memory manager for the ARMv8-A architecture
 *
 * References:
 * https://developer.arm.com/documentation/ddi0487/ka 
 * https://developer.arm.com/documentation/den0024/a/The-Memory-Management-Unit
 * https://www.ndss-symposium.org/wp-content/uploads/2017/09/ndss2017_05B-5_Cho_paper.pdf
 * https://github.com/ARM-software/u-boot/blob/master/arch/arm/include/asm/armv8/mmu.h
 * https://lowenware.com/blog/aarch64-mmu-programming/
 * https://armv8-ref.codingbelief.com/en/
 *
 * See below on how overcome ASID's limit of 256 diff tasks in TLB
 * https://stackoverflow.com/questions/17590146
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#include "ARM64/Machine.h"
#include "LibKern/Console.h"

#include "Memory/PageDef.h"
#include "Memory/Virtual.h"

/* Kernel page table */
static uint64_t l0_kernel_pgtbl[ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));
static uint64_t l1_kernel_pgtbl[ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));

extern void *kernel_vma_base;

/* Common */
#define ENTRY_VALID(entry) ((entry) | ARM_TE_VALID_MASK)
#define EBTRY_INVALID(entry) ((entry) & ~ARM_TE_VALID_MASK)

#define ENTRY_TABLE(entry) ((entry) | ARM_TE_TYPE_MASK)
#define ENTRY_BLOCK(entry) ((entry) & ~ARM_TE_TYPE_MASK)
#define ENTRY_PAGE(entry) TYPE_BLOCK(entry)

/* Table */
#define TBL_SET_NEXT(tbl, next) \
        (((uint64_t)(tbl) & ~ARM_TT_NEXT_MASK) | \
                (((uint64_t)(next) << ARM_TT_NEXT_SHIFT)))
#define TBL_SET_PXN(tbl, pxn) \
        (((uint64_t)(tbl) & ~ARM_TT_PXN_MASK) | \
                (((uint64_t)(pxn) << ARM_TT_PXN_SHIFT)))
#define TBL_SET_XN(tbl, xn) \
        (((uint64_t)(tbl) & ~ARM_TT_XN_MASK) | \
                (((uint64_t)(xn) << ARM_TT_XN_SHIFT)))
#define TBL_SET_AP(tbl, ap) \
        (((uint64_t)(tbl) & ~ARM_TT_AP_MASK) | \
                (((uint64_t)(ap) << ARM_TT_AP_SHIFT)))
#define TBL_SET_NS(tbl, ns) \
        (((uint64_t)(tbl) & ~ARM_TT_NS_MASK) | \
                (((uint64_t)(ns) << ARM_TT_NS_SHIFT)))

/* Block */
#define BLK_SET_AIDX(blk, aidx) \
        (((uint64_t)(blk) & ~ARM_TB_AIDX_MASK) | \
                (((uint64_t)(aidx) << ARM_TB_AIDX_SHIFT)))
#define BLK_SET_NS(blk, ns) \
        (((uint64_t)(blk) & ~ARM_TB_NS_MASK) | \
                (((uint64_t)(ns) << ARM_TB_NS_SHIFT)))
#define BLK_SET_AP(blk, ap) \
        (((uint64_t)(blk) & ~ARM_TB_AP_MASK) | \
                (((uint64_t)(ap) << ARM_TB_AP_SHIFT)))
#define BLK_SET_SH(blk, sh) \
        (((uint64_t)(blk) & ~ARM_TB_SH_MASK) | \
                (((uint64_t)(sh) << ARM_TB_SH_SHIFT)))
#define BLK_SET_AF(blk, af) \
        (((uint64_t)(blk) & ~ARM_TB_AF_MASK) | \
                (((uint64_t)(af) << ARM_TB_AF_SHIFT)))
#define BLK_SET_NG(blk, ng) \
        (((uint64_t)(blk) & ~ARM_TB_NG_MASK) | \
                (((uint64_t)(ng) << ARM_TB_NG_SHIFT)))
#define BLK_SET_L1_NEXT(blk, next) \
        (((uint64_t)(blk) & ~ARM_TB_L1NEXT_MASK) | \
                (((uint64_t)(next) << ARM_TB_L1NEXT_SHIFT)))
#define BLK_SET_L2_NEXT(blk, next) \
        (((uint64_t)(blk) & ~ARM_TB_L2NEXT_MASK) | \
                (((uint64_t)(next) << ARM_TB_L2NEXT_SHIFT)))
#define BLK_SET_HINT(blk, hint) \
        (((uint64_t)(blk) & ~ARM_TB_HINT_MASK) | \
                (((uint64_t)(hint) << ARM_TB_HINT_SHIFT)))
#define BLK_SET_PXN(blk, pxn) \
        (((uint64_t)(blk) & ~ARM_TB_PXN_MASK) | \
                (((uint64_t)(pxn) << ARM_TB_PXN_SHIFT)))             
#define BLK_SET_XN(blk, xn) \
        (((uint64_t)(blk) & ~ARM_TB_XN_MASK) | \
                (((uint64_t)(xn) << ARM_TB_XN_SHIFT)))   

void init_kernel_pgtbl(void)
{
        uint64_t ttbr1 = 0;
        /* Level 1 */
        for (int i = 0; i < ENTRY_SIZE; i++) {
                uint64_t blk = 0;

                blk = ENTRY_VALID(blk);
                blk = ENTRY_BLOCK(blk);

                blk = BLK_SET_AIDX(blk, 0);
                blk = BLK_SET_NS(blk, 0);
                blk = BLK_SET_AP(blk, 0);
                blk = BLK_SET_SH(blk, 0);
                blk = BLK_SET_AF(blk, 0);
                blk = BLK_SET_NG(blk, 0);

                blk = BLK_SET_L1_NEXT(blk, 0x0ULL);

                blk = BLK_SET_HINT(blk, 0);
                blk = BLK_SET_PXN(blk, 0);
                blk = BLK_SET_XN(blk, 0);

                l1_kernel_pgtbl[i] = blk;
        }

        /* Level 0 */
        {
                uint64_t tbl = 0;

                tbl = ENTRY_VALID(tbl);
                tbl = ENTRY_TABLE(tbl);

                tbl = TBL_SET_NEXT(tbl, 0x0ULL);

                tbl = TBL_SET_PXN(tbl, 0);
                tbl = TBL_SET_XN(tbl, 0);
                tbl = TBL_SET_AP(tbl, 0);
                tbl = TBL_SET_NS(tbl, 0);

                l0_kernel_pgtbl[0] = tbl;
        }

        for (int i = 1; i < ENTRY_SIZE; i++) {
                l0_kernel_pgtbl[i] = 0x0ULL;
        }

        klog("[vmm] l0_kernel_pgtbl @ 0x%lx\n", l0_kernel_pgtbl[0]);
        klog("[vmm] l1_kernel_pgtbl @ 0x%lx\n", l1_kernel_pgtbl[0]);

        ttbr1 = (uint64_t) l0_kernel_pgtbl;

        MSR("TTBR1_EL1", ttbr1);
        isb();
}

void init_tcr(void)
{
        uint64_t tcr_el1 = 0;
        uint64_t reg = 0;

        /* Read TCR_EL1 */
        MRS("TCR_EL1", tcr_el1);
        isb();

        /* DS: max. output addr (OA) and virtual addr (VA) size set to 48-bit */
        tcr_el1 &= TCR_DS_48BITS;

        /* IPS: effective output addr (OA) set to ID_AA64MMFR0_EL1.PARange */
        MRS("ID_AA64MMFR0_EL1", reg);
        tcr_el1 &= TCR_IPS_CLEAR;
        tcr_el1 |= (GET_PARange(reg) << TCR_IPS_SHIFT);

        /* T1SZ: input address (IA) size offset of mem region for TTBR1_EL1 */
        /* T0SZ: input address (IA) size offset of mem region for TTBR0_EL1 */
        tcr_el1 |= TCR_T1SZ << TCR_T1SZ_SHIFT;
        tcr_el1 |= TCR_T0SZ << TCR_T0SZ_SHIFT;

        /* HPDN1: enable hierarchical permissions for TTBR1_EL1 */
        /* HPDN0: enable hierarchical permissions for TTBR0_EL1 */
        tcr_el1 &= TCR_HPDN1_ENABLE;
        tcr_el1 &= TCR_HPDN0_ENABLE;

        /* A1: TTBR0 decides the ASID value (?) */
        tcr_el1 &= TCR_A1_TTBR0;

        /* EPD1: perform table walk on TTBR1 after TLB miss (?) */
        /* EPD0: perform table walk on TTBR0 after TLB miss (?) */
        tcr_el1 &= TCR_EPD1_DISABLE;
        tcr_el1 &= TCR_EPD0_DISABLE;

        /* TG1: granule size for TTBR1 region */
        /* TG0: granule size for TTBR0 region */
        tcr_el1 &= TCR_TG1_GRANULE_CLEAR;
        tcr_el1 &= TCR_TG0_GRANULE_CLEAR;
#if PAGE_SIZE == 4096
        tcr_el1 |= TCR_TG1_GRANULE_4KB;
        tcr_el1 |= TCR_TG0_GRANULE_4KB;
#elif PAGE_SIZE == 16392
        tcr_el1 |= TCR_TG1_GRANULE_16KB;
#elif PAGE_SIZE == 65568
        tcr_el1 |= TCR_TG1_GRANULE_64KB;
#else
        return; // we fucked up
#endif

        /* Save TCR_EL1 */
        MSR("TCR_EL1", tcr_el1);
        isb();

        /* DEBUG TCR_EL1 */
        MRS("TCR_EL1", tcr_el1);
        isb();
        klog("[vmm] TCR_EL1: 0x%lx\n", tcr_el1);

}

void init_mair(void)
{
        /* TODO */
        uint64_t mair_el1 = 0;

        MSR("MAIR_EL1", mair_el1);
        isb();
}


