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
#include "ARM64/Memory.h"

#include "LibKern/Console.h"

#include "Memory/PageDef.h"
#include "Memory/Virtual.h"

void init_mair(void)
{
        /* TODO */
        uint64_t mair_el1 = 0;

        MSR("MAIR_EL1", mair_el1);
        isb();
}
