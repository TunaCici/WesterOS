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

#include "LibKern/Console.h"

#include "Memory/PageDef.h"
#include "Memory/BootMem.h"
#include "Memory/Physical.h"

/* Main buddy data structure */
static volatile void *baseAddr = 0;
static volatile free_area_t buddyPmm[MAX_ORDER] = {0};

void __clear_page_area(uint8_t *addr, uint32_t size)
{
        for (uint32_t i = 0; i < size; i++) {
                addr[i] = 0x00;
        }
}

/* Assumes the memory pool is contiguous */
uint64_t __block_to_idx(const list_head_t *block, const uint32_t order)
{
        return ((uint8_t*) block - (uint8_t*) baseAddr) / SIZEOF_BLOCK(order);
}

list_head_t* __buddy(const list_head_t *block, const uint32_t order)
{
        list_head_t *retValue = 0;

        retValue = (list_head_t*) ((uint64_t) block ^ (PAGE_SIZE << order));

        return retValue;
}

void __try_to_mark(list_head_t *block, const uint32_t order)
{
        /* Mark the allocated block as USED (if possible)  */
        if (order < MAX_ORDER - 1) {
                uint64_t idx = __block_to_idx(block, order);

                KLOG("[pmm] mark idx %lu in 2^%u as USED\n", idx, order);

                BUDDY_MARK_USED(buddyPmm[order].map, idx);
        }
}

void __append_to_order(list_head_t *block, const uint32_t order)
{
        if (!block || MAX_ORDER <= order) {
                return;
        }

        /* Append to head */
        if (buddyPmm[order].listHead.next == 0) {
                buddyPmm[order].listHead.next = block;
                block->prev = 0;
                return;
        }

        /* Or to the end (TODO: appending to head seems more efficient) */
        list_head_t *iter = buddyPmm[order].listHead.next;
        while(iter->next) {
                iter = iter->next;
        }
        iter->next = block;

        block->next = 0;
        block->prev = iter;
}

void __remove_from_order(list_head_t *block, const uint32_t order)
{
        if (!block || MAX_ORDER <= order) {
                return;
        }

        /* Remove from head */
        if (!block->prev) {
                buddyPmm[order].listHead.next = 0;
                return;
        }

        /* Or somewhere else */
        block->prev->next = block->next;
        if (block->next) {
                block->next->prev = block->prev;
        }
        block->next = 0;
        block->prev = 0;
}

uint64_t init_allocator(const void *start, const void *end)
{
        uint64_t retValue = 0; /* number of available MAX_ORDER - 1 blocks */
        
        list_head_t *alignedStart = (list_head_t*) PALIGN((uint8_t*) start);
        list_head_t *alignedEnd = (list_head_t*) PALIGN((uint8_t*) end - PAGE_SIZE + 1);

        /* Align to MAX_ORDER block */
        alignedStart = (list_head_t*) CUSTOM_ALIGN(alignedStart,
                SIZEOF_BLOCK(MAX_ORDER - 1));

        /* At least one MAX_ORDER - 1 must exist! */
        if ((uint8_t*) alignedEnd - (uint8_t*) alignedStart < SIZEOF_BLOCK(MAX_ORDER - 1)) {
                return 0;
        } 

        baseAddr = alignedStart;
        
        /* Initialize 2^(MAX_ORDER - 1) blocks */
        buddyPmm[MAX_ORDER - 1].listHead.next = alignedStart;
        buddyPmm[MAX_ORDER - 1].listHead.prev = 0;
        buddyPmm[MAX_ORDER - 1].map = 0;

        /* Add blocks to freeList */
        uint64_t blockCount = 1;
        list_head_t *prevBlock = 0;
        const uint32_t moveBy = SIZEOF_BLOCK(MAX_ORDER - 1) / 16; /* ptr arith */

        for (list_head_t *i = alignedStart; i < alignedEnd; i += moveBy) {
                if ((i + moveBy) < alignedEnd) {
                        i->next = (i + moveBy);
                        i->prev = prevBlock;

                        blockCount++;
                        prevBlock = i;
                } 
        }

        /* Initialize the rest of the blocks */
        for (uint32_t i = 0; i < MAX_ORDER - 1; i++) {
                uint32_t bitmapSize = 0; /* Bytes */

                bitmapSize = blockCount << (MAX_ORDER - 1 - i); /* Bit */
                bitmapSize = (bitmapSize + 7) & ~7; /* Align to uint8_t */
                bitmapSize = bitmapSize / 8; /* To bytes */

                uint32_t reqPages = (bitmapSize + PAGE_SIZE) / PAGE_SIZE;

                buddyPmm[i].listHead.next = 0;
                buddyPmm[i].listHead.prev = 0;
                buddyPmm[i].map = (uint8_t*) bootmem_alloc(reqPages);

                __clear_page_area(buddyPmm[i].map, reqPages);
        }

        retValue = blockCount;

        return retValue;
}

void* alloc_page()
{
        return alloc_pages(0);
}

void* alloc_pages(const uint32_t order)
{
        if (MAX_ORDER <= order) {
                KLOG("[pmm] can't allocate blocks larger than 2^(MAX_ORDER - 1)\n");
                return 0;
        }

        KLOG("[pmm] allocate an 2^%u order block (%u bytes)\n", order, SIZEOF_BLOCK(order));

        void *retAddr = 0;

        /* Find block */
        uint32_t startOrder = 0;
        for (; startOrder < MAX_ORDER; startOrder++) {
                if (SIZEOF_BLOCK(startOrder) < SIZEOF_BLOCK(order)) {
                        KLOG("[pmm] order 2^%u size not enough. skip\n", startOrder);
                        continue;
                } else if (!buddyPmm[startOrder].listHead.next) {
                        KLOG("[pmm] order 2^%u doesn't have free blocks. skip\n", startOrder);
                        continue;
                }

                KLOG("[pmm] order 2^%u has free blocks. start here\n", startOrder);
                break;
        }

        if (MAX_ORDER <= startOrder) {
                KLOG("[pmm] no available free blocks. fail\n");
                return 0;
        }

        /* Remove from list */
        list_head_t *block = buddyPmm[startOrder].listHead.next;
        buddyPmm[startOrder].listHead.next = block->next;
        if (buddyPmm[startOrder].listHead.next) {
                buddyPmm[startOrder].listHead.next->prev = 0;
        }

        block->next = 0;
        block->prev = 0;

        uint64_t idx = __block_to_idx(block, startOrder);

        KLOG("[pmm] choosen block: 0x%p (idx: %lu)\n", block, idx);

        uint32_t currOrder;
        for (currOrder = startOrder; 0 <= currOrder; currOrder--) {
                KLOG("[pmm] currOrder: %u\n", currOrder);

                if (SIZEOF_BLOCK(currOrder) == SIZEOF_BLOCK(order)) {
                        /* Perfect fit. allocate here */
                        KLOG("[pmm] perfect fit for 2^%u. allocate\n", currOrder);

                        __try_to_mark(block, currOrder);

                        retAddr = (void*) block;
                        break;
                } else {
                        /* Split */
                        list_head_t *buddy = __buddy(block, currOrder - 1);
                        KLOG("[pmm] split. targetBlock: 0x%p, it's buddy: 0x%p\n", block, buddy);
        
                        __try_to_mark(block, currOrder);
                        __append_to_order(buddy, currOrder - 1);
                }
        }

        idx = __block_to_idx(retAddr, currOrder);
        
        if (BUDDY_GET_MARK(buddyPmm[currOrder].map, idx)) {
                KLOG("[pmm] allocate block 0x%p on 2^%u OK\n", retAddr, currOrder);
        } else {
                KLOG("[pmm] allocate block 0x%p on 2^%u FAIL\n", retAddr, currOrder);
        }


        return retAddr;
}

void free_page(void *addr)
{
        free_pages(addr, 0);
}

void free_pages(void *addr, const uint32_t order)
{
        if (!addr || MAX_ORDER <= order) {
                KLOG("[pmm] target is NULL or order is larger then 2^(MAX_ORDER - 1). fail\n");
                return;
        }

        uint64_t idx = __block_to_idx(addr, order);

        if (!BUDDY_GET_MARK(buddyPmm[order].map, idx)) {
                KLOG("[pmm] trying to free an empty area. fail\n");
                return;
        }

        BUDDY_MARK_FREE(buddyPmm[order].map, idx);

        /* 2^(MAX_ORDER - 1) block don't coalese */
        if (order == (MAX_ORDER - 1)) {
                KLOG("[pmm] 2^(MAX_ORDER - 1) block don't coalese. append\n");
                __append_to_order(addr, idx);
                return;
        }

        /* Coalese? */
        for (uint32_t curr = order; curr < MAX_ORDER - 1; curr++) {
                list_head_t *buddy = __buddy(addr, curr);
                uint64_t buddyIdx = __block_to_idx(addr, curr);

                KLOG("[pmm] buddy 0x%p (idx: %lu, ord: %u)\n", buddy, buddyIdx, curr);

                /* Coalese */
                if (!BUDDY_GET_MARK(buddyPmm[curr].map, buddyIdx)) {
                        KLOG("[pmm] buddy is free. coalese\n");

                        __remove_from_order(buddy, curr);
                        __append_to_order(addr, curr + 1);
                } else {
                        KLOG("[pmm] buddy is not free. append to list\n");

                        __append_to_order(addr, curr);
                        break;
                }
        }      
}

/* START DEBUG ONLY */

void pmm_klog_buddy(void)
{
        for (uint32_t i = 0; i < MAX_ORDER; i++) {

                KLOG("[pmm] buddyPmm[%u].listHead ", i);

                uint32_t maxPrint = 3;
                list_head_t *iter = buddyPmm[i].listHead.next;
                while (iter && 0 < maxPrint--) {
                        KPRINTF("0x%p -> 0x%p | ", iter, iter->next);
                        iter = iter->next;
                }
                KPRINTF("\n");
        } 
}

/* END DEBUG ONLY */
