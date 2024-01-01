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
#include "Memory/PageDef.h"
#include "Memory/Physical.h"

#include "LibKern/Console.h"

/* Main buddy data structure */
static volatile free_area_t buddyPmm[MAX_ORDER] = {0};

static volatile uint8_t *buddyMap = 0;
static volatile uint32_t buddyMapSize = 0; /* bytes */

int ___count_free_pages(const unsigned int order) {
        int retValue = 0;

        return retValue; 
}

uint8_t* __buddy(const uint8_t* reference, const unsigned int order) {
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

        /* Calculate the required buddyMapSize in bytes */
        uint64_t numPages = (alignedEnd - alignedStart) / PAGE_SIZE;
        buddyMapSize = (numPages / 2) / 8; /* 1 bit for a buddy pair */

        klog("[Physical] Required no. of bytes for 'buddyMap': %u\n",
                buddyMapSize
        );
        klog("[Physical] Required pages: %u\n", 
                (buddyMapSize + PAGE_SIZE) / PAGE_SIZE
        );

        /* TODO: Use bootmem_alloc() here instead doing 'manual' allocation? */ 
        buddyMap = alignedStart;

        klog("[Physical] buddyMap.start: 0x%p\n", buddyMap);
        klog("[Physical] buddyMap.end: 0x%p\n", buddyMap + buddyMapSize);

        /* Skip over buddyMap (in dire need of a "memory map") */
        /* AND align to MAX_ORDER block. i don't know if this is ideal */
        alignedStart += ((buddyMapSize + PAGE_SIZE) / PAGE_SIZE) * PAGE_SIZE;
        alignedStart = (void*) CUSTOM_ALIGN(alignedStart, (PAGE_SIZE << (MAX_ORDER - 1)));

        klog("[Physical] alignedStart: 0x%p\n", alignedStart);

        /* Explicitly mark all of buddyMap as 'free' */
        for (uint32_t i = 0; i < buddyMapSize; i++) {
                buddyMap[i] = 0x00;
        }

        const uint32_t blockSize = SIZEOF_BLOCK(MAX_ORDER - 1);
        const uint32_t moveBy = blockSize / 8; /* pointer arithmetics */
        
        /* Create the first freelist of 2^(MAX_ORDER - 1) blocks */
        buddyPmm[MAX_ORDER - 1].listHead.next = alignedStart;
        for (list_head_t *i = alignedStart; i < (list_head_t*) alignedEnd; i += moveBy) {
                if ((i + moveBy) < (list_head_t*) alignedEnd) {
                        i->next = (i + moveBy);
                        retValue++;
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

        klog("[Physical] Number of 2 MiB blocks: %lu\n", retValue);

        list_head_t *first = buddyPmm[MAX_ORDER - 1].listHead.next;

        /* Basically split a buddy into two */
        list_head_t *buddy1 = first;
        list_head_t *buddy2 = (list_head_t*) __buddy((uint8_t*) buddy1, 8);

        klog("Buddy1: 0x%p, Buddy2: 0x%p\n", buddy1, buddy2);

        return retValue;
}
