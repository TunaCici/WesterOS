/*
 * Early boot memory manager using First-Fit allocation.
 *
 * Allows the kernel to have basic dyn. memory before PMM is fully initialized.
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#include "LibKern/Console.h"

#include "Memory/PageDef.h"
#include "Memory/BootMem.h"

static volatile uint64_t base_addr = 0x0;
static volatile uint8_t map[BM_MAP_SIZE] = {0};

uint8_t __calc_fitting(const uint32_t start_idx, const uint32_t num_pages)
{
        if (BM_ARENA_SIZE_PAGE < start_idx + num_pages) {
                return 0;
        }

        for (uint32_t i = 0; i < num_pages; i++) {
                if (BM_MAP_GET(map, start_idx + i) == 1) {
                        /* it doesn't fit :( */
                        return 0;
                }
        }

        return 1;
}

void __mark_used(const uint32_t start_idx, const uint32_t end_idx)
{
        for (uint32_t i = start_idx; i < end_idx; i++) {
                BM_MAP_SET(map, i);
        }
}

uint32_t bootmem_init(const uint64_t base)
{
        base_addr = PALIGN(base);

        /* Initialize the bitmap */
        for (uint32_t i = 0; i < BM_MAP_SIZE; i++) {
                map[i] = 0;
        }

        return BM_ARENA_SIZE_BYTE;
}

void* bootmem_alloc(uint32_t size)
{
        if (BM_ARENA_SIZE_BYTE < size) {
                return (void*) 0;
        }

        if (size < PAGE_SIZE) {
                size = PAGE_SIZE;
        }
        
        uint32_t req_pages = ((size + PAGE_SIZE - 1) / PAGE_SIZE);

        for (uint32_t i = 0; i < BM_ARENA_SIZE_PAGE; i++) {
                /* Skip used areas */
                if (BM_MAP_GET(map, i)) {
                        continue;
                }

                uint8_t fits = __calc_fitting(i, req_pages);

                if (fits) {
                        __mark_used(i, i + req_pages);
 
                        return (void*) BM_IDX_TO_ADDR(i, base_addr);
                }
        }

        return (void*) 0;
}

/* START DEBUG ONLY */

void bootmem_klog_map(void) 
{
        for (uint32_t i = 0; i < BM_MAP_SIZE; i++) {
                KLOG("[bootmem] map[%u]: 0x%x\n", i, map[i]);
        }
}

/* END DEBUG ONLY */
