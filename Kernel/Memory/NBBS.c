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
static volatile uint64_t base_address = 0;

static volatile uint32_t depth = 0;
static volatile uint64_t tree_size = 0; /* bytes */
static volatile uint64_t index_size = 0; /* bytes */

static volatile uint64_t total_memory = 0;
static volatile uint64_t max_level = 0;
static volatile uint64_t min_size = 0;
static volatile uint64_t max_size = 0;

int nb_init(uint64_t base, uint64_t size)
{
        if (base == 0 | size == 0) {
                return 1;
        }

        if (size < min_size) {
                return 1;
        }

        /* Setup */
        base_address = base;       
        total_memory = size;
        min_size = PAGE_SIZE;
        depth = LOG2_LOWER(total_memory / min_size);
        max_level = 0;
        max_size = EXP2(depth - max_level) * min_size;

        /* Calculate required tree size */
        uint32_t total_nodes = 1; // we have garbage node at idx 0
        for (uint32_t i = 0; i <= depth; i++) {
                total_nodes += (total_memory / (EXP2(i) * min_size));
        }

        /* Calculate required index size */
        uint32_t total_pages = (total_memory / min_size);

        tree_size = total_nodes * 1;  // each node is 1 byte
        index_size = total_pages * 4; // each page id is 4 byte

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

uint32_t try_alloc(uint32_t node)
{
        return 1;
}

void* nb_alloc(uint64_t size)
{
        if (max_size < size) {
                return 0;
        }

        if (size < min_size) {
                size = min_size;
        }

        uint32_t level = LOG2_LOWER(total_memory / size);

        if (depth < level) {
                level = depth;
        }

        /* Range of nodes at target level */
        uint32_t start_node = EXP2(level);
        uint32_t end_node = EXP2(level + 1);

        for (uint32_t i = start_node; i < end_node; i++) {
                if (is_free(tree[i])) {
                        uint32_t failed_at = try_alloc(i);

                        if (!failed_at) {
                                /* TODO: Explain what's going on here */
                                uint32_t leaf = leftmost(i, depth) - EXP2(depth); 
                                index[leaf] = i;

                                return (void*) (base_address + leaf * min_size);
                        } else {
                                /* Skip the entire subtree [of failed] */
                                uint32_t curr_level = LOG2_LOWER(i);
                                uint32_t fail_level = LOG2_LOWER(failed_at);

                                uint32_t d = EXP2(curr_level - fail_level);
                                i = (failed_at + 1) * d;
                        }
                }
        }

        return (void*) 0;
        
}

void* nb_free(void *addr)
{
        
}


#endif /* NBBS_H */