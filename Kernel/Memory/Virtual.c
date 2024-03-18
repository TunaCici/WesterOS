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

inline void init_ttbr(void *ttbr0, void *ttbr1)
{
        /* TODO */ 

        MRS("TTBR0_EL1", ttbr0);
        MRS("TTBR1_EL1", ttbr1);
}

inline void init_tcr(void)
{
        /* TODO */
        uint64_t tcr_el1 = 0;

        MRS("TCR_EL1", tcr_el1);
}

inline void init_mair(void)
{
        /* TODO */
        uint64_t mair_el1 = 0;

        MRS("MAIR_EL1", mair_el1);
}
