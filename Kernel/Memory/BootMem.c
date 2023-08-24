/*
 * Early boot memory manager using First-Fit allocation.
 *
 * Allows the kernel to have basic dyn. memory before PMM is fully initialized.
 *
 * Author: Tuna CICI
 */

#include <stdint-gcc.h>
#include <stdint.h>

#include "Memory/PageDef.h"
#include "Memory/BootMem.h"

#include "LibKern/Console.h"

volatile static uint8_t map[BM_MAP_SIZE] = {1};
volatile static page_t pageList[BM_ARENA_SIZE] = {0};

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
        for (uint64_t i = 0; i < BM_ARENA_SIZE; i++) {
                klog("[bootmem] pageList[%u].addr: 0x%x\n", &i, &pageList[i].addr);
        }

        return retValue;
}

void* bootmem_alloc(const uint32_t numPages)
{
        void *retAddr = 0;

        for (uint64_t i = 0; i < BM_ARENA_SIZE; i++) {
                uint64_t myBit = BM_MAP_GET(map, i);

                klog("[bootmem] bit %u of map is %u\n", &i, &myBit);
        }

        return retAddr;
}

uint8_t bootmem_free(void *targetAddr, const uint32_t numPages)
{
        return 0;
}

/* START DEBUG ONLY */

void bootmem_klog_map(void) 
{
        for (uint64_t i = 0; i < BM_MAP_SIZE; i++) {
                klog("[bootmem] map[%u]: 0x%x\n", &i, map[i]);
        }
}

/* END DEBUG ONLY */
