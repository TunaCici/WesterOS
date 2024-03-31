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

static void *l0_kernel_pgtbl[ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));
static void *l1_kernel_pgtbl[ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));
static void *l2_kernel_pgtbl[ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));
static void *l3_kernel_pgtbl[ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));

#define VALIDATE_ENTRY(entry) ((entry) | ARM_TE_VALID_MASK)
#define INVALIDATE_ENTRY(entry) ((entry) & ~ARM_TE_VALID_MASK)

#define TBL_PXN_ENABLE(tbl) ((tbl) | ARM_TT_PXN_MASK)
#define TBL_PXN_DISABLE(tbl) ((tbl) & ~ARM_TT_PXN_MASK)

#define TBL_XN_ENABLE(tbl) ((tbl) | ARM_TT_XN_MASK)
#define TBL_XN_DISABLE(tbl) ((tbl) & ~ARM_TT_XN_MASK)

#define TBL_SET_NEXT(tbl, next_tbl) \
        ((void *)(((uint64_t)(tbl) & ~ARM_TT_NEXT_MASK) | \
                (((uint64_t)(next_tbl) << ARM_TT_NEXT_SHIFT))))
#define TBL_CLR_NEXT(tbl, next_tbl) \
        ((void *)(((uint64_t)(tbl) & ~ARM_TT_NEXT_MASK)))

#define TBL_SET_AP(tbl, ap) \
        ((void *)(((uint64_t)(tbl) & ~ARM_TT_AP_MASK) | \
                (((uint64_t)(next_tbl) << ARM_TT_AP_SHIFT))))
#define TBL_CLR_AP(tbl, ap) \
        ((void *)(((uint64_t)(tbl) & ~ARM_TT_AP_MASK)))

void init_kernel_pgtbl(void)
{
        uint64_t ttbr1 = 0;

        void *entry = (void*) 0x40480000000004A3ULL;
        void *next = (void*) (ARM_TT_NEXT_MASK >> ARM_TT_NEXT_SHIFT);
        
        entry = TBL_SET_NEXT(entry, next);
        entry = TBL_CLR_AP(entry, next);

        klog("[vmm] entry: 0x%p\n", entry);

        klog("[vmm] l0_kernel_pgtbl @ 0x%p\n", l0_kernel_pgtbl);
        klog("[vmm] l1_kernel_pgtbl @ 0x%p\n", l1_kernel_pgtbl);
        klog("[vmm] l2_kernel_pgtbl @ 0x%p\n", l2_kernel_pgtbl);
        klog("[vmm] l3_kernel_pgtbl @ 0x%p\n", l3_kernel_pgtbl);

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


