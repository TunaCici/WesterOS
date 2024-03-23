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

#include <stdint.h>

#include "ARM64/Machine.h"
#include "LibKern/Console.h"

#include "Memory/PageDef.h"
#include "Memory/Virtual.h"

static void* l0_kernel_pgtbl[ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));
static void* l1_kernel_pgtbl[ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));
static void* l2_kernel_pgtbl[ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));
static void* l3_kernel_pgtbl[ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));

void init_kernel_pgtbl(void)
{
        uint64_t ttbr1 = 0;

        /* Identity mapping for the kernel */
        for (uint32_t i = 0; i < ENTRY_SIZE; i++) {
                l0_kernel_pgtbl[i] = (void *) (i * L0_BLOCK_SIZE);
                l1_kernel_pgtbl[i] = (void *) (i * L1_BLOCK_SIZE); 
                l2_kernel_pgtbl[i] = (void *) (i * L2_BLOCK_SIZE); 
                l3_kernel_pgtbl[i] = (void *) (i * L3_BLOCK_SIZE); 
        }

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
        /* TODO */
        uint64_t tcr_el1 = 0;

        MSR("TCR_EL1", tcr_el1);
        isb();
}

void init_mair(void)
{
        /* TODO */
        uint64_t mair_el1 = 0;

        MSR("MAIR_EL1", mair_el1);
        isb();
}


