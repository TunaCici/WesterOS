/*
 * This file (NBSS.c) implements the Non-Blocking Buddy System
 */

#ifndef NBBS_H
#define NBBS_H

#include <stdint.h>

#include "LibKern/String.h"

#include "Memory/PageDef.h"
#include "Memory/BootMem.h"
#include "Memory/NBBS.h"

static volatile uint8_t *tree = 0;
static volatile uint32_t *index = 0;

static volatile uint64_t tree_size = 0; /* bytes */
static volatile uint64_t index_size = 0; /* bytes */

/*
 * Maximum allowed alloc size is: 2^(max_level - 1) * min_size
 * Naturally max_allowed <= total_memory
*/
static volatile uint32_t max_level = 10;

static volatile uint64_t min_size = PAGE_SIZE;
static volatile uint64_t max_size = 0;

int nb_init(uint64_t base_addr, uint64_t size)
{
        if (base_addr == 0 | size == 0) {
                return 1;
        }

        /* Calculate required tree size */
        uint32_t total_nodes = 1; // we start at idx 1
        for (uint32_t i = 0; i < max_level; i++) {
                total_nodes += (size / ((0x1 << i) * min_size));
        }

        /* Calculate required index size */
        uint32_t total_elems = (size / min_size);

        tree_size = total_nodes * 1;   // each node is 1 byte
        index_size = total_elems * 4; // each elem is 4 byte

        /* Allocate */
        uint32_t req_pages = (tree_size + PAGE_SIZE - 1) / PAGE_SIZE;
        tree = (uint8_t*) bootmem_alloc(req_pages);

        if (!tree) {
                return 1;
        } 

        req_pages = (index_size + PAGE_SIZE - 1) / PAGE_SIZE;
        index = (uint32_t*) bootmem_alloc(req_pages);

        if (!index) {
                return 1;
        }

        /* Initialize */
        memset((void*) tree, 0x0, tree_size);
        memset((void*) index, 0x0, index_size);

        return 0;
}

void* nb_alloc(uint64_t size)
{

}

void* nb_free(void *addr)
{
        
}


#endif /* NBBS_H */