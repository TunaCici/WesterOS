/*
 * Definitions related to paging.
 *
 * Author: Tuna CICI
 */

#ifndef PAGEDEF_H
#define PAGEDEF_H

#include <stdint.h>

#include "ARM64/Memory.h"

#define PAGE_SIZE GRANULE_SIZE /* Bytes */

#define PALIGN(addr) (((uint64_t) addr + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))
#define CUSTOM_ALIGN(addr, to) (((uint64_t) addr + (to - 1)) & ~(to - 1))

#endif /* PAGEDEF_H */