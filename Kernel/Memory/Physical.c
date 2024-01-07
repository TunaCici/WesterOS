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
static volatile void *baseAddr = 0;

void __clear_page_area(uint8_t *startAddr, uint32_t count)
{
        uint8_t *iter = startAddr;
        for (uint32_t i = 0; i < count; i += 8) {
                *iter = 0x00;
                iter++;
        }
}

/* Assumes the memory pool is contiguous */
uint64_t __addr_to_idx(const uint8_t *targetAddr, const uint32_t blockSize)
{
        return (targetAddr - (uint8_t*) baseAddr) / blockSize;
}

list_head_t* __buddy(const uint8_t* reference, const unsigned int order)
{
        list_head_t *retValue = 0;

        retValue = (list_head_t*) ((uint64_t) reference ^ (PAGE_SIZE << order));

        return retValue;
}

void __append_to_order(const unsigned int order, list_head_t *targetBlock)
{
        if (!targetBlock || MAX_ORDER <= order) {
                return;
        }

        /* Append to head */
        if (buddyPmm[order].listHead.next == 0) {
                buddyPmm[order].listHead.next = targetBlock;
                targetBlock->prev = 0;
                return;
        }

        /* Or to the end (TODO: appending to head seems more efficient) */
        list_head_t *iter = buddyPmm[order].listHead.next;
        while(iter->next) {
                iter = iter->next;
        }
        iter->next = targetBlock;

        targetBlock->next = 0;
        targetBlock->prev = iter;
}

void __remove_from_order(const unsigned int order, list_head_t *targetBlock)
{
        if (!targetBlock || MAX_ORDER <= order) {
                return;
        }

        /* Remove from head */
        if (!targetBlock->prev) {
                buddyPmm[order].listHead.next = 0;
                return;
        }

        /* Or somewhere else */
        targetBlock->prev->next = targetBlock->next;
        if (targetBlock->next) {
                targetBlock->next->prev = targetBlock->prev;
        }
        targetBlock->next = 0;
        targetBlock->prev = 0;
}

uint64_t init_allocator(const uint8_t *startAddr, const uint8_t *endAddr)
{
        uint64_t retValue = 0; /* number of available MAX_ORDER - 1 blocks */
        
        void *alignedStart = (void*) PALIGN(startAddr);
        void *alignedEnd = (void*) PALIGN(endAddr - PAGE_SIZE + 1);

        /* Align to MAX_ORDER block */
        alignedStart = (void*) CUSTOM_ALIGN(alignedStart, (PAGE_SIZE << (MAX_ORDER - 1)));
        baseAddr = alignedStart;

        const uint32_t moveBy = SIZEOF_BLOCK(MAX_ORDER - 1) / 8; /* pointer arithmetics */
        
        /* Initialize 2^(MAX_ORDER - 1) blocks */
        buddyPmm[MAX_ORDER - 1].listHead.next = alignedStart;
        buddyPmm[MAX_ORDER - 1].listHead.prev = 0;
        buddyPmm[MAX_ORDER - 1].map = 0;

        /* Add blocks to freeList */
        uint32_t maxOrderBlockCount = 1;
        list_head_t *prevBlock = 0;
        for (list_head_t *i = alignedStart; i < (list_head_t*) alignedEnd; i += moveBy) {
                if ((i + moveBy) < (list_head_t*) alignedEnd) {
                        i->next = (i + moveBy);
                        i->prev = prevBlock;

                        maxOrderBlockCount++;
                        prevBlock = i;
                } 
        }

        /* Initialize the rest of the blocks */
        for (uint32_t i = 0; i < MAX_ORDER - 1; i++) {
                uint32_t bitmapSize = 0; /* Bytes */

                bitmapSize = maxOrderBlockCount << (MAX_ORDER - 1 - i); /* Bit */
                bitmapSize = (bitmapSize + 7) & ~7; /* Align to uint8_t */
                bitmapSize = bitmapSize / 8; /* To bytes */

                uint32_t reqPages = (bitmapSize + PAGE_SIZE) / PAGE_SIZE;

                buddyPmm[i].listHead.next = 0;
                buddyPmm[i].listHead.prev = 0;
                buddyPmm[i].map = (uint8_t*) bootmem_alloc(reqPages);

                __clear_page_area(buddyPmm[i].map, reqPages);
        }

        retValue = maxOrderBlockCount;

        return retValue;
}

void* alloc_page()
{
        void *retAddr = 0;

        return retAddr;
}

void* alloc_pages(const uint32_t order)
{
        if (MAX_ORDER <= order) {
                klog("[pmm] can't allocate blocks larger than 2^(MAX_ORDER - 1)\n");
                return 0;
        }

        klog("[pmm] allocate an 2^%u order block (%u bytes)\n", order, SIZEOF_BLOCK(order));

        void *retAddr = 0;

        /* Find block */
        uint32_t startOrder = 0;
        for (; startOrder < MAX_ORDER; startOrder++) {
                if (SIZEOF_BLOCK(startOrder) < SIZEOF_BLOCK(order)) {
                        klog("[pmm] order 2^%u size not enough. skip\n", startOrder);
                        continue;
                } else if (!buddyPmm[startOrder].listHead.next) {
                        klog("[pmm] order 2^%u doesn't have free blocks. skip\n", startOrder);
                        continue;
                }

                klog("[pmm] order 2^%u has free blocks. start here\n", startOrder);
                break;
        }

        if (MAX_ORDER <= startOrder) {
                klog("[pmm] no available free blocks. fail\n");
                return 0;
        }

        /* Remove from list */
        list_head_t *targetBlock = buddyPmm[startOrder].listHead.next;
        buddyPmm[startOrder].listHead.next = targetBlock->next;
        if (buddyPmm[startOrder].listHead.next) {
                buddyPmm[startOrder].listHead.next->prev = 0;
        }

        targetBlock->next = 0;
        targetBlock->prev = 0;

        uint64_t targetBlockIdx = __addr_to_idx(
                (uint8_t*) targetBlock, SIZEOF_BLOCK(startOrder)
        );

        klog("[pmm] target block: 0x%p (idx: %lu)\n",
                targetBlock,
                targetBlockIdx
        );

        uint32_t currOrder;
        for (currOrder = startOrder; 0 <= currOrder; currOrder--) {
                klog("[pmm] currOrder: %u\n", currOrder);

                if (SIZEOF_BLOCK(currOrder) == SIZEOF_BLOCK(order)) {
                        /* Perfect fit. allocate here */
                        klog("[pmm] perfect fit for 2^%u. allocate\n", currOrder);

                        /* Mark the allocated block as USED (if possible)  */
                        if (currOrder < MAX_ORDER - 1) {
                                uint64_t targetBlockIdx = __addr_to_idx(
                                        (uint8_t*) targetBlock, SIZEOF_BLOCK(currOrder)
                                );

                                klog("[pmm] mark idx %lu in 2^%u as USED\n", targetBlockIdx, currOrder);

                                BUDDY_MARK_USED(buddyPmm[currOrder].map, targetBlockIdx);
                        }

                        retAddr = (void*) targetBlock;
                        break;
                } else {
                        /* Split */
                        list_head_t *buddy = __buddy(
                                (uint8_t*) targetBlock, currOrder - 1);
                        klog("[pmm] split. targetBlock: 0x%p, it's buddy: 0x%p\n", targetBlock, buddy);
        
                        /* Mark the split block as USED (if possible) */
                        if (currOrder < MAX_ORDER - 1) {
                                uint64_t targetBlockIdx = __addr_to_idx(
                                        (uint8_t*) targetBlock, SIZEOF_BLOCK(currOrder)
                                );

                                BUDDY_MARK_USED(buddyPmm[currOrder].map, targetBlockIdx);
                        }

                        /* Add buddy to ONE BELOW freelist */
                        __append_to_order(currOrder - 1, buddy);
                }
        }

        targetBlockIdx = __addr_to_idx(
                (uint8_t*) retAddr, SIZEOF_BLOCK(currOrder)
        );
        
        if (BUDDY_GET_MARK(buddyPmm[currOrder].map, targetBlockIdx)) {
                klog("[pmm] allocate block 0x%p on 2^%u OK\n", retAddr, currOrder);
        } else {
                klog("[pmm] allocate block 0x%p on 2^%u FAIL\n", retAddr, currOrder);
        }


        return retAddr;
}

void free_page(void *targetAddr)
{
        return;
}

void free_pages(void *targetAddr, const uint32_t order)
{
        if (!targetAddr || MAX_ORDER <= order) {
                klog("[pmm] target is NULL or order is larger then 2^(MAX_ORDER - 1). fail\n");
                return;
        }

        uint64_t targetIdx = __addr_to_idx((uint8_t*) targetAddr, SIZEOF_BLOCK(order));

        if (!BUDDY_GET_MARK(buddyPmm[order].map, targetIdx)) {
                klog("[pmm] trying to free an empty area. fail\n");
                return;
        }

        /* Mark as free */
        BUDDY_MARK_FREE(buddyPmm[order].map, targetIdx);

        /* 2^(MAX_ORDER - 1) block don't coalese */
        if (order == (MAX_ORDER - 1)) {
                klog("[pmm] 2^(MAX_ORDER - 1) block don't coalese. append\n");
                __append_to_order(order, targetAddr);
                return;
        }

        /* Coalese? */
        for (uint32_t currOrder = order; currOrder < MAX_ORDER - 1; currOrder++) {
                list_head_t *buddy = __buddy(targetAddr, currOrder);
                uint64_t buddyIdx = __addr_to_idx((uint8_t*) buddy, SIZEOF_BLOCK(currOrder));

                klog("[pmm] buddy 0x%p (idx: %lu, ord: %u)\n", buddy, buddyIdx, currOrder);

                /* Coalese */
                if (!BUDDY_GET_MARK(buddyPmm[currOrder].map, buddyIdx)) {
                        klog("[pmm] buddy is free. coalese\n");

                        __remove_from_order(currOrder, buddy);
                        __append_to_order(currOrder + 1, targetAddr);
                } else {
                        klog("[pmm] buddy is not free. append to list\n");

                        __append_to_order(currOrder, targetAddr);
                        break;
                }
        }      
}

/* START DEBUG ONLY */

void pmm_klog_buddy(void)
{
        for (uint32_t i = 0; i < MAX_ORDER; i++) {

                klog("[pmm] buddyPmm[%u].listHead ", i);

                uint32_t maxPrint = 3;
                list_head_t *iter = buddyPmm[i].listHead.next;
                while (iter && 0 < maxPrint--) {
                        kprintf("0x%p -> 0x%p | ", iter, iter->next);
                        iter = iter->next;
                }
                kprintf("\n");
        } 
}

/* END DEBUG ONLY */
