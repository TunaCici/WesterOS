/*
 * Early boot memory manager using Free Lists.
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

static page_t pageList[BM_ARENA_SIZE] = {0};
static uint8_t map[BM_MAP_SIZE] = {0};

uint16_t init_bootmem(const uint8_t *startAddr)
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
                klog("[bootmem] pageList[%u].addr: %x\n", &i, &pageList[i].addr);
        }

        return retValue;
}

void* alloc_bootmem(const uint32_t numPages)
{
        return 0;
}

void* free_bootmem(void *targetAddr, const uint32_t numPages)
{
        return 0;
}