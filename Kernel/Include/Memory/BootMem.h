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

#define BM_ARENA_SIZE      4096 /* Pages */
#define BM_MAP_SIZE        (BM_ARENA_SIZE / 8) /* Cuz -> uint8_t */

#define BM_MAP_GET(map, idx) (map[idx / 8] & (1 << (idx % 8)))
#define BM_MAP_SET(map, idx) (map[idx / 8] |= (1 << (idx % 8)))
#define BM_MAP_RST(map, idx) (map[idx / 8] &= ~(1 << (idx % 8)))

uint16_t bootmem_init(const uint8_t *startAddr);
void*    bootmem_alloc(const uint16_t numPages);

/* START DEBUG ONLY */
void bootmem_klog_map(void);
/* END DEBUG ONLY */

#endif /* BOOTMEM_H */
