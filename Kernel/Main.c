/*
 * Kernel entry
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#include "ARM64/Machine.h"

#include "Boot.h"
#include "MemoryLayout.h"

#include "LibKern/String.h"
#include "LibKern/Time.h"
#include "LibKern/Console.h"
#include "LibKern/DeviceTree.h"

#include "Memory/PageDef.h"
#include "Memory/BootMem.h"
#include "Memory/Physical.h"
#include "Memory/NBBS.h"
#include "Memory/Virtual.h"

/*
 * Kernel entry.
 *
 * Parameters:
 *      l0_pgtbl: ARM64 level 0 page table
 *      l1_pgtbl: ARM64 level 1 page table
 *      vector_tbl: ARM64 vector table base address
 *      dtb: Device Tree Blob base address
 *      dtb_size: Device Tree Blob size in bytes
 */
void kmain(boot_sysinfo* boot_params)
{
        volatile void *important_ahh = &boot_params;

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

        /* X. Setup vector tables */
        if (boot_params->vector_base) {
                MSR("VBAR_EL1", boot_params->vector_base);
                isb();
        } else {
                klog("[kmain] NULL vector table is given!\n");
        }

        /* 1. Init BootMem */
        klog("[kmain] Initializing early memory manager...\n");

        uint64_t pageCount = bootmem_init(
                (const void*) (boot_params->k_phy_base + boot_params->k_size));

        klog("[kmain] Pages available: %lu (%lu KiB) in bootmem\n",
                pageCount, (pageCount * PAGE_SIZE) / 1024
        );

        /* 2. Init NBBS */
        klog("[kmain] Initializing NBBS...\n");

        if (nb_init(mem_start, mem_end - mem_start)) {
                klog("[kmain] Failed to initialize NBBS ;(\n");
        } else {
                klog("[kmain] Initialized NBBS!\n");
        }

        /* 2. Init PMM */
        // klog("[kmain] Initializing physical memory manager...\n");
        // 
        // uint64_t blockCount = init_allocator(
        //         (const void *) boot_params->k_phy_base + boot_params->k_size + pageCount * PAGE_SIZE,
        //         (const void *) mem_end
        // );

        // klog("[kmain] 2 MiB blocks available: %lu (%lu MiB) in pmm\n",
        //         blockCount, blockCount * 2
        // );

        /* 3. Init Kernel Page Tables & Enable MMU */

        /* X. Do something weird */
        klog("[kmain] imma just sleep\n");
        for(;;) {
                klog("[kmain] Zzz..\n");
                ksleep(5000);
        }
}
