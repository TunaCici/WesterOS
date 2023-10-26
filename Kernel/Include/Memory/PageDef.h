/*
 * Definitions related to paging.
 *
 * Author: Tuna CICI
 */

#ifndef PAGEDEF_H
#define PAGEDEF_H

#include <stdint.h>

#define PAGE_SIZE 4096 /* Bytes */
#define MAX_ORDER 10 /* Block size: 2^0 ... 2^(MAX_ORDER - 1) * PAGE_SIZE */

#define PALIGN(addr) (((uint64_t) addr + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

#endif /* PAGEDEF_H */