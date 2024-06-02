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

        if ((mem_end - mem_start) < BM_ARENA_SIZE_BYTE) {
                klog("[kmain] Not enough memory available to boot :(\n");
                klog("[kmain] ---- Detected memory size: %lu MiB\n",
                        (mem_end - mem_start) / (1024 * 1024));
                klog("[kmain] ---- Required minimum size: %u MiB\n",
                        (BM_ARENA_SIZE_BYTE) / (1024 * 1024));
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

        uint64_t bootmem_size = bootmem_init(
                (boot_params->k_phy_base + boot_params->k_size));

        klog("[kmain] Available size in bootmem: %lu MiB\n",
                bootmem_size / 1024 / 1024);

        /* 2. Init PMM */
        klog("[kmain] Initializing physical memory manager...\n");

        if (nb_init(mem_start, mem_end - mem_start)) {
                klog("[kmain] Failed to initialize NBBS ;(\n");
                wfi();
        }

        klog("[kmain] Available size in PMM: %lu MiB\n",
                nb_stat_total_memory() / 1024 / 1024);

        /* 3. Init Kernel Page Tables & Enable MMU */

        /* X. Do something weird */
        klog("[kmain] imma just sleep\n");
        for(;;) {
                klog("[kmain] Zzz..\n");
                ksleep(5000);
        }
}
