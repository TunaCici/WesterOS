/*
 * Physical memory manager using Binary-Buddy System
 *
 * The algorithm is described in D.S. Hirschberg:
 * “A class of dynamic memory allocation algorithms”
 *
 * URL: https://doi.org/10.1145/362375.362392
 *
 * Influenced from:
 * URL: https://www.kernel.org/doc/gorman/html/understand/understand009.html
 * URL: https://people.engr.tamu.edu/bettati/Courses/313/2014A/Labs/mp1_fibo.pdf
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#include "LibKern/Time.h"
#include "LibKern/Console.h"

#include "Memory/PageDef.h"
#include "Memory/BootMem.h"
#include "Memory/Physical.h"

/* Main buddy data structure */
static volatile free_area_t buddyPmm[MAX_ORDER] = {0};

uint8_t* __buddy(const uint8_t* reference, const unsigned int order)
{
        uint8_t *retValue = 0;

        klog("XOR with 0x%x\n", PAGE_SIZE << order);
        klog("reference: 0x%lx\n", (uint64_t) reference);

        retValue = (uint8_t*) ((uint64_t) reference ^ (PAGE_SIZE << order));

        return retValue;
}

uint64_t init_allocator(const uint8_t *startAddr, const uint8_t *endAddr)
{
        uint64_t retValue = 0; /* number of pages that are free */
        
        void *alignedStart = (void*) PALIGN(startAddr);
        void *alignedEnd = (void*) PALIGN(endAddr - PAGE_SIZE + 1);

        klog("[Physical] startdAddr: 0x%p\n", startAddr);
        klog("[Physical] endAddr: 0x%p\n", endAddr);

        /* Align to MAX_ORDER block */
        alignedStart = (void*) CUSTOM_ALIGN(alignedStart, (PAGE_SIZE << (MAX_ORDER - 1)));

        klog("[Physical] alignedStart: 0x%p\n", alignedStart);

        const uint32_t moveBy = SIZEOF_BLOCK(MAX_ORDER - 1) / 8; /* pointer arithmetics */
        
        /* Init 2^(MAX_ORDER - 1) blocks */
        uint32_t maxOrderBlockCount = 0;
        buddyPmm[MAX_ORDER - 1].listHead.next = alignedStart;
        for (list_head_t *i = alignedStart; i < (list_head_t*) alignedEnd; i += moveBy) {
                if ((i + moveBy) < (list_head_t*) alignedEnd) {
                        i->next = (i + moveBy);
                        maxOrderBlockCount++;
                } 
        }
        buddyPmm[MAX_ORDER - 1].map = 0;

        /* Init the rest of the blocks */
        for (uint32_t i = 0; i < MAX_ORDER - 1; i++) {
                uint32_t bitmapSize = 0; /* Bytes */

                bitmapSize = maxOrderBlockCount << (MAX_ORDER - 1 - i); /* Bit */
                bitmapSize = (bitmapSize + 7) & ~7; /* Align to uint8_t */
                bitmapSize = bitmapSize / 8; /* To bytes */

                uint32_t reqPages = (bitmapSize + PAGE_SIZE) / PAGE_SIZE;

                buddyPmm[i].listHead.next = 0;
                buddyPmm[i].map = (uint8_t*) bootmem_alloc(reqPages);

                if (buddyPmm[i].map) {
                        klog("[Physical] Alloc %u pages for 2^%u map OK (0x%p)\n",
                                reqPages, i, buddyPmm[i].map
                        );
                } else {
                        klog("[Physical] Alloc %u pages for 2^%u map FAIL\n",
                                reqPages, i
                        );
                }
        }

#ifdef DEBUG
        /* DEBUG: Traverse buddyPmm[MAX_ORDER - 1] */
        list_head_t *iter = buddyPmm[MAX_ORDER - 1].listHead.next;
        while (iter && iter->next) {
                klog("Address of iter: 0x%p, value of iter->next: 0x%p\n", iter, iter->next);
                iter = iter->next;
                ksleep(2000);
        }
#endif

        klog("[Physical] Number of 2 MiB blocks: %lu\n", maxOrderBlockCount);

        list_head_t *first = buddyPmm[MAX_ORDER - 1].listHead.next;

        /* Basically split a buddy into two */
        list_head_t *buddy1 = first;
        list_head_t *buddy2 = (list_head_t*) __buddy((uint8_t*) buddy1, 8);

        klog("Buddy1: 0x%p, Buddy2: 0x%p\n", buddy1, buddy2);

        return retValue;
}
