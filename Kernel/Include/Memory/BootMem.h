/*
 * Early boot memory manager using Free Lists.
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

uint16_t init_bootmem(const uint8_t *startAddr);

void* alloc_bootmem(const uint32_t numPages);
void* free_bootmem(void *targetAddr, const uint32_t numPages);

#endif /* BOOTMEM_H */
