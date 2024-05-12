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

static volatile uint64_t base_address = 0;
static volatile uint64_t total_memory = 0;
static volatile uint32_t depth = 0;
static volatile uint64_t base_level = 0;
static volatile uint64_t min_size = 0;
static volatile uint64_t max_size = 0;
static volatile uint32_t release_count = 0;

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
        base_level = 0;
        max_size = EXP2(depth - base_level) * min_size;

        /* Calculate required tree size - root node is at index 1  */
        uint32_t total_nodes = EXP2(depth + 1);

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
        /* Occupy the node */
        uint8_t free = 0;
        if (!BCAS(&tree[node], &free, BUSY)) {
                return node;
        }

        uint32_t current = node;
        uint32_t child = 0;

        /* Propagate the info about the occupancy up to the ancestor node(s) */
        while (base_level < LEVEL(current)) {
                child = current;
                current = current >> 1;

                uint8_t curr_val = 0;
                uint8_t new_val = 0;

                do {
                        curr_val = tree[current];

                        if (curr_val & OCC) {
                                freenode(node, LEVEL(child));
                                return current;
                        }

                        new_val = clean_coal(curr_val, child);
                        new_val = mark(new_val, child);
                } while (!BCAS(&tree[current], &curr_val, new_val));
        }

        return 0;
}

void* nb_alloc(uint64_t size)
{
        if (max_size < size) {
                return 0;
        }

        if (size < min_size) {
                size = min_size;
        }

        nb_alloc_again:;
        uint32_t ts = release_count;
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
                                uint32_t curr_level = LEVEL(i);
                                uint32_t fail_level = LEVEL(failed_at);

                                uint32_t d = EXP2(curr_level - fail_level);
                                i = (failed_at + 1) * d;
                        }
                }
        }

        /* A release occured, try again */
        if (ts != release_count) {
                goto nb_alloc_again;
        }

        return (void*) 0;
}

void free_unmark(uint32_t node, uint32_t upper_bound)
{
        uint32_t current = node;
        uint32_t child = 0;

        uint8_t curr_val = 0;
        uint8_t new_val = 0;

        do {
                child = current;
                current = current >> 1;

                do {
                        curr_val = tree[current];

                        if (!is_coal(curr_val, child)) {
                                return;
                        }
                        
                        new_val = unmark(curr_val, child);
                } while (!BCAS(&tree[current], &curr_val, new_val));
        } while (upper_bound < LEVEL(current) && !is_occ_buddy(new_val, child));
}

void freenode(uint32_t node, uint32_t upper_bound)
{
        /* TODO: should I check for double frees? */
        if (is_free(tree[node])) {
                return;
        }

        /* Phase 1. Ancestors of the node are marked as coalescing */
        uint32_t current = node >> 1;
        uint32_t child = node;

        while (base_level < LEVEL(child)) {
                uint8_t curr_val = 0;
                uint8_t new_val = 0;
                uint8_t old_val = 0;
                
                do {
                        curr_val = tree[current];
                        new_val = set_coal(curr_val, child);
                        old_val = VCAS(&tree[current], &curr_val, new_val);
                } while (old_val != curr_val);
                
                if (is_occ_buddy(old_val, child) && !is_coal_buddy(old_val, child)) {
                        break;
                }

                child = current;
                current = current >> 1;
        }

        /* Phase 2. Mark the node as free */
        tree[node] = 0;

        /* Phase 3. Propagate node release upward and possibly merge buddies */
        if (LEVEL(node) != base_level) {
                free_unmark(node, upper_bound);
        }
}

void nb_free(void *addr)
{
        /* Range check (is this necessary?) */
        uint64_t u_addr = (uint64_t) addr;
        if (u_addr < base_address || (base_address + total_memory) < u_addr) {
                return;
        }

        uint32_t n = (u_addr - base_address) / min_size;
        freenode(index[n], base_level);
        FAD(&release_count, 1);
}


#endif /* NBBS_H */