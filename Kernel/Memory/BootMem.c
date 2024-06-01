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

static volatile void *baseAddr = 0;
static volatile uint8_t map[BM_MAP_SIZE] = {0};

uint8_t __calc_fitting(const uint32_t startIdx, const uint32_t numPages)
{
        if (BM_ARENA_SIZE_PAGE < startIdx + numPages) {
                return 0;
        }

        for (uint32_t i = 0; i < numPages; i++) {
                if (BM_MAP_GET(map, startIdx + i) == 1) {
                        /* it doesn't fit :( */
                        return 0;
                }
        }

        return 1;
}

void __mark_used(const uint32_t startIdx, const uint16_t endIdx)
{
        for (uint32_t i = startIdx; i < endIdx; i++) {
                BM_MAP_SET(map, i);
        }
}

uint32_t bootmem_init(const void *startAddr)
{
        uint32_t retValue = 0; /* Pages available */

        baseAddr = (void*) PALIGN(startAddr);

        /* Initialize the bitmap */
        for (uint32_t i = 0; i < BM_MAP_SIZE; i++) {
                map[i] = 0;
        }

        retValue = BM_ARENA_SIZE_BYTE;

        return retValue;
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

                        return (void*) BM_IDX_TO_ADDR(i, baseAddr);
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
