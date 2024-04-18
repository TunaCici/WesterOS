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

#ifndef PHYSICAL_H
#define PHYSICAL_H

#include <stdint.h>

#include "Memory/PageDef.h"

#define MAX_ORDER 10 /* Block size: 2^0 ... 2^(MAX_ORDER - 1) * PAGE_SIZE */

#define SIZEOF_BLOCK(order) ((0x1 << (order)) * PAGE_SIZE ) /* Bytes */

#define BUDDY_MARK_USED(map, idx) (map[idx / 8] |= (1 << (idx % 8)))
#define BUDDY_MARK_FREE(map, idx) (map[idx / 8] &= ~(1 << (idx % 8)))
#define BUDDY_GET_MARK(map, idx) (map[idx / 8] & (1 << (idx % 8)))

/* Used to 'address' blocks in a free_area_t (e.g. 0x4000 -> 0x8000) */
/* Similar to the 'run' structure on xv6: */
/*      https://github.com/mit-pdos/xv6-public/blob/master/kalloc.c */
typedef struct list_head_struct {
        struct list_head_struct *next;
        struct list_head_struct *prev;
} list_head_t;

typedef struct free_area_struct {
        list_head_t listHead;
        uint8_t *map;
} free_area_t;

uint64_t init_allocator(const void *start, const void *end);

/* Private functions - TEST ONLY */
void __clear_page_area(uint8_t *addr, uint32_t size);
uint64_t __block_to_idx(const list_head_t *block, const uint32_t order);
list_head_t* __buddy(const list_head_t *block, const uint32_t order);
void __try_to_mark(list_head_t *block, const uint32_t order);
void __append_to_order(list_head_t *block, const uint32_t order);
void __remove_from_order(list_head_t *block, const uint32_t order);

/* Allocate a single page / 2^order number of pages */
void* alloc_page();
void* alloc_pages(const uint32_t order);

void free_page(void *addr);
void free_pages(void *addr, const uint32_t order);

/* START DEBUG ONLY */
void pmm_klog_buddy(void);
/* END DEBUG ONLY */

#endif /* PHYSICAL_H */
