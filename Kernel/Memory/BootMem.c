/*
 * Early boot memory manager using First-Fit allocation.
 *
 * Allows the kernel to have basic dyn. memory before PMM is fully initialized.
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#include "Memory/PageDef.h"
#include "Memory/BootMem.h"

#include "LibKern/Console.h"

static volatile uint8_t map[BM_MAP_SIZE] = {1};
static volatile page_t pageList[BM_ARENA_SIZE] = {0};

uint16_t bootmem_init(const uint8_t *startAddr)
{
        uint16_t retValue = 0; /* Pages available */
        uint8_t *alignedStart = 0;

        alignedStart = (uint8_t*) PALIGN(startAddr);
        
        /* Initialize the pageList */
        for (uint16_t i = 0; i < BM_ARENA_SIZE; i++) {
                pageList[i].addr = alignedStart;
                pageList[i].flags = 0;

                alignedStart += PAGE_SIZE;
                retValue++;
        }

        /* Initialize the bitmap */
        for (uint16_t i = 0; i < BM_MAP_SIZE; i++) {
                map[i] = 0;
        }

        /* Test it */
        for (uint16_t i = 0; i < BM_ARENA_SIZE; i++) {
                // klog("[bootmem] pageList[%u].addr: 0x%x\n", &i, &pageList[i].addr);
        }

        return retValue;
}

uint8_t bootmem_calc_fitting(const uint32_t startIdx, const uint32_t numPages)
{
        if (BM_ARENA_SIZE < startIdx + numPages) {
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

void bootmem_mark_used(const uint16_t startIdx, const uint16_t endIdx)
{
        for (uint16_t i = startIdx; i < endIdx; i++) {
                BM_MAP_SET(map, i);
        }
}

void* bootmem_alloc(const uint16_t numPages)
{
        void *retAddr = 0;

        for (uint16_t i = 0; i < BM_ARENA_SIZE; i++) {
                if (BM_MAP_GET(map, i)) {
                        continue;
                }

                uint8_t fits = bootmem_calc_fitting(i, numPages);

                if (fits) {
                        bootmem_mark_used(i, i + numPages);

                        retAddr = pageList[i].addr;
                        
                        break;
                }
        }

        return retAddr;
}

/* START DEBUG ONLY */

void bootmem_klog_map(void) 
{
        for (uint16_t i = 0; i < BM_MAP_SIZE; i++) {
                klog("[bootmem] map[%u]: 0x%x\n", i, map[i]);
        }
}

/* END DEBUG ONLY */
