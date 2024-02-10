/*
 * Main kernel entry. What TODO? (further research required)
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#include "ARM64/Machine.h"
#include "MemoryLayout.h"

#include "LibKern/Time.h"
#include "LibKern/Console.h"

#include "Memory/PageDef.h"
#include "Memory/BootMem.h"
#include "Memory/Physical.h"

/* Kernel & user page table addresses. Defined in Kernel/kernel.ld */
extern uint64_t _kernel_pgtbl;
extern uint64_t _user_pgtbl;
extern uint64_t _K_l2_pgtbl;
extern uint64_t _U_l2_pgtbl;

extern uint64_t kstart;
extern uint64_t kend;

uint64_t *kernel_pgtbl = &_kernel_pgtbl;
uint64_t *user_pgtbl = &_user_pgtbl;

void kmain(void)
{
        const uint8_t *kernelStart = (uint8_t*) &kstart;
        const uint8_t *kernelEnd = (uint8_t*) &kend;

        const uint8_t *ramStart = (uint8_t*) RAM_START;
        const uint8_t *ramEnd = (uint8_t*) RAM_END;

        /**
         * Need an addt. early mm before a nice buddy pmm can be initialized.
         * It needs to have a FIXED size such as 16MiB or smth.
         * A simple First-Fit alloc should do the trick /w bit-mapped pages.
         *
         * TODO: Implement a new mm called "BootMem" and then init buddy pmm.
         **/

        if ((ramEnd - ramStart) < BM_ARENA_SIZE * PAGE_SIZE) {
                klog("[kmain] Not enough RAM available to boot :(\n");
                /* TODO: Panic here */
        }

        /* 0. Initialize BootMem */
        klog("[kmain] Initializing early memory manager...\n");

        uint64_t pageCount = bootmem_init(kernelEnd);

        klog("[kmain] Pages available: %lu (%lu KiB) in bootmem\n",
                pageCount, (pageCount * PAGE_SIZE) / 1024
        );

        /* 1. Initialize PMM */
        klog("[kmain] Initializing physical memory manager...\n");

        uint64_t blockCount = init_allocator(
                kernelEnd + pageCount * PAGE_SIZE,
                ramEnd
        );

        klog("[kmain] 2 MiB blocks available: %lu (%lu MiB) in pmm\n",
                blockCount, blockCount * 2
        );
    
        /* X. Do something weird */
        klog("[kmain] imma just sleep\n");
        for(;;) {
                klog("[kmain] Zzz..\n");
                ksleep(5000);
        }
}
