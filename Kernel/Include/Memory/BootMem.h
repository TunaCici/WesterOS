/*
 * Early boot memory manager using First-Fit allocation.
 *
 * Allows the kernel to have basic dyn. memory before PMM is fully initialized.
 *
 * Author: Tuna CICI
 */

#ifndef BOOTMEM_H
#define BOOTMEM_H

#include <stdint.h>

#include "Memory/PageDef.h"

#define BM_ARENA_SIZE_PAGE 2048 /* Pages */
#define BM_ARENA_SIZE_BYTE (BM_ARENA_SIZE_PAGE * PAGE_SIZE) /* Bytes */
#define BM_MAP_SIZE (BM_ARENA_SIZE_PAGE / 8) /* Bytes () */

#define BM_MAP_GET(map, idx) (map[idx / 8] & (1 << (idx % 8)))
#define BM_MAP_SET(map, idx) (map[idx / 8] |= (1 << (idx % 8)))
#define BM_MAP_RST(map, idx) (map[idx / 8] &= ~(1 << (idx % 8)))

#define BM_IDX_TO_ADDR(idx, baseAddr) (baseAddr + PAGE_SIZE * idx)

uint32_t bootmem_init(const void *startAddr);
void*    bootmem_alloc(uint32_t size);

/* START DEBUG ONLY */
void bootmem_klog_map(void);
/* END DEBUG ONLY */

#endif /* BOOTMEM_H */
