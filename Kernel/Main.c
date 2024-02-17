/*
 * Kernel entry
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#include "ARM64/Machine.h"
#include "MemoryLayout.h"

#include "LibKern/String.h"
#include "LibKern/Time.h"
#include "LibKern/Console.h"
#include "LibKern/DeviceTree.h"

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
        volatile const uint8_t *kernel_start = (uint8_t*) &kstart;
        volatile const uint8_t *kernel_end = (uint8_t*) &kend;

        uint64_t mem_start = 0x0;
        uint64_t mem_end = 0x0;

        /* 0. Get HW Information */
        if (dtb_init((void*) DTB_START) != 0) {
                klog("[kmain] Couldn't initialize DTB!\n");
                wfi();
        }

        uint8_t res = dtb_mem_info((void*) DTB_START, &mem_start, &mem_end);
        if (res) {
                klog("[kmain] Failed to get memory info from dtb: %u\n", res);
                wfi();
        }

        if ((mem_end - mem_start) < BM_ARENA_SIZE * PAGE_SIZE) {
                klog("[kmain] Not enough memory available to boot :(\n");
                klog("[kmain] ---- Detected memory size: %lu MiB\n",
                        (mem_end - mem_start) / (1024 * 1024));
                klog("[kmain] ---- Required minimum size: %u MiB\n",
                        (BM_ARENA_SIZE * PAGE_SIZE) / (1024 * 1024));
                wfi();
        }

        /* 1. Initialize BootMem */
        klog("[kmain] Initializing early memory manager...\n");

        uint64_t pageCount = bootmem_init(kernel_end);

        klog("[kmain] Pages available: %lu (%lu KiB) in bootmem\n",
                pageCount, (pageCount * PAGE_SIZE) / 1024
        );

        /* 2. Initialize PMM */
        klog("[kmain] Initializing physical memory manager...\n");

        uint64_t blockCount = init_allocator(
                kernel_end + pageCount * PAGE_SIZE,
                mem_end
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
