/*
 * This file (NBSS.c) implements the Non-Blocking Buddy System
 */

#include <stdint.h>

#include "LibKern/String.h"

#include "Memory/PageDef.h"
#include "Memory/BootMem.h"
#include "Memory/Physical.h"

/* Meta-data */
static uint8_t *nb_tree = 0;
static uint32_t *nb_index = 0;

static uint64_t nb_tree_size = 0; /* bytes */
static uint64_t nb_index_size = 0; /* bytes */

static uint64_t nb_base_address = 0;
static uint64_t nb_total_memory = 0;
static uint32_t nb_depth = 0;
static uint64_t nb_base_level = 0;
static uint64_t nb_max_size = 0;
static uint32_t nb_release_count = 0;

/* Statistics */
static uint64_t nb_stat_alloc_blocks[NB_MAX_ORDER + 1] = {0};

int nb_init(uint64_t base, uint64_t size)
{
        if (base == 0 || size == 0) {
                return 1;
        }

        if (size < NB_MIN_SIZE) {
                return 1;
        }

        /* Setup */
        nb_base_address = base;       
        nb_total_memory = size;
        
        nb_depth = LOG2_LOWER(nb_total_memory / NB_MIN_SIZE);
        nb_base_level = nb_depth - NB_MAX_ORDER;
        nb_max_size = EXP2(NB_MAX_ORDER) * NB_MIN_SIZE;

        /* Calculate required tree size - root node is at index 1  */
        uint32_t total_nodes = EXP2(nb_depth + 1);

        /* Calculate required index size */
        uint32_t total_pages = (nb_total_memory / NB_MIN_SIZE);

        nb_tree_size = total_nodes * 1;  // each node is 1 byte
        nb_index_size = total_pages * 4; // each leaf index is 4 byte

        /* Allocate */
        nb_tree = (uint8_t*) NB_MALLOC(nb_tree_size);
        if (!nb_tree) {
                return 1;
        }

        nb_index = (uint32_t*) NB_MALLOC(nb_index_size);

        if (!nb_index) {
                return 1;
        }

        /* Initialize */
        memset((void*) nb_tree, 0x0, nb_tree_size);
        memset((void*) nb_index, 0x0, nb_index_size);
        memset((void*) nb_stat_alloc_blocks, 0x0, ((NB_MAX_ORDER + 1) * 8));
        // ------------------------------------------- sizeof(uint64_t) ^
        nb_release_count = 0;

        return 0;
}

uint32_t __nb_try_alloc(uint32_t node)
{
        /* Occupy the node */
        uint8_t free = 0;
        if (!BCAS(&nb_tree[node], &free, BUSY)) {
                return node;
        }

        uint32_t current = node;
        uint32_t child = 0;

        /* Propagate the info about the occupancy up to the ancestor node(s) */
        while (nb_base_level < nb_level(current)) {
                child = current;
                current = current >> 1;

                uint8_t curr_val = 0;
                uint8_t new_val = 0;

                do {
                        curr_val = nb_tree[current];

                        if (curr_val & OCC) {
                                __nb_freenode(node, nb_level(child));
                                return current;
                        }

                        new_val = nb_clean_coal(curr_val, child);
                        new_val = nb_mark(new_val, child);
                } while (!BCAS(&nb_tree[current], &curr_val, new_val));
        }

        return 0;
}

/* TODO: Ugly code; refactor */
uint32_t __nb_leftmost(uint32_t node, uint32_t depth)
{
        /* Index to level */
        uint32_t level = nb_level(node);

        /* Size (in terms of leaf size) */
        uint64_t block_size = EXP2(depth - level);

        /* Offset within level */
        uint64_t offset = node % EXP2(level);

        /* Leftmost leaf */
        uint32_t leftmost_leaf = EXP2(depth) + offset * block_size;

        return leftmost_leaf;
}

void __nb_clean_block(void* addr, uint64_t size)
{
        if (!addr || !size) {
                return;
        }

        memset(addr, 0x0, size);
}

void* nb_alloc(uint64_t size)
{
        if (nb_max_size < size) {
                return 0;
        }

        if (size < NB_MIN_SIZE) {
                size = NB_MIN_SIZE;
        }

        nb_alloc_again:;
        uint32_t ts = nb_release_count;
        uint32_t level = LOG2_LOWER(nb_total_memory / size);

        if (nb_depth < level) {
                level = nb_depth;
        }

        /* Range of nodes at target level */
        uint32_t start_node = EXP2(level);
        uint32_t end_node = EXP2(level + 1);

        for (uint32_t i = start_node; i < end_node; i++) {
                if (nb_is_free(nb_tree[i])) {
                        uint32_t failed_at = __nb_try_alloc(i);

                        if (!failed_at) {
                                /* TODO: Explain what's going on here */
                                uint32_t leaf = __nb_leftmost(
                                        i, nb_depth) - EXP2(nb_depth);
                                nb_index[leaf] = i;

                                FAD(&nb_stat_alloc_blocks[nb_depth - level], 1);
                                
                                return (void*)
                                        (nb_base_address + leaf * NB_MIN_SIZE);
                        } else {
                                /* Skip the entire subtree [of failed] */
                                uint32_t curr_level = nb_level(i);
                                uint32_t fail_level = nb_level(failed_at);

                                uint32_t d = EXP2(curr_level - fail_level);
                                i = ((failed_at + 1) * d) - 1;
                        }
                }
        }

        /* A release occured, try again */
        if (ts != nb_release_count) {
                goto nb_alloc_again;
        }

        return (void*) 0;
}

void __nb_unmark(uint32_t node, uint32_t upper_bound)
{
        uint32_t current = node;
        uint32_t child = 0;

        uint8_t curr_val = 0;
        uint8_t new_val = 0;

        do {
                child = current;
                current = current >> 1;

                do {
                        curr_val = nb_tree[current];

                        if (!nb_is_coal(curr_val, child)) {
                                return;
                        }
                        
                        new_val = nb_unmark(curr_val, child);
                } while (!BCAS(&nb_tree[current], &curr_val, new_val));
        } while (upper_bound < nb_level(current) &&
                        !nb_is_occ_buddy(new_val, child));
}

void __nb_freenode(uint32_t node, uint32_t upper_bound)
{
        /* TODO: should I check for double frees? */
        if (nb_is_free(nb_tree[node])) {
                return;
        }

        /* Phase 1. Ancestors of the node are marked as coalescing */
        uint32_t current = node >> 1;
        uint32_t child = node;

        while (nb_base_level < nb_level(child)) {
                uint8_t curr_val = 0;
                uint8_t new_val = 0;
                uint8_t old_val = 0;
                
                do {
                        curr_val = nb_tree[current];
                        new_val = nb_set_coal(curr_val, child);
                        old_val = VCAS(&nb_tree[current], &curr_val, new_val);
                } while (old_val != curr_val);
                
                if (nb_is_occ_buddy(old_val, child) && 
                        nb_is_coal_buddy(old_val, child)) {
                        break;
                }

                child = current;
                current = current >> 1;
        }

        /* Phase 2. Mark the node as free */
        nb_tree[node] = 0;

        /* Phase 3. Propagate node release upward and possibly merge buddies */
        if (nb_level(node) != nb_base_level) {
                __nb_unmark(node, upper_bound);
        }
}

void nb_free(void *addr)
{
        if (!addr) {
                return;
        }

        uint32_t n = ((uint64_t) addr - nb_base_address) / NB_MIN_SIZE;
        __nb_freenode(nb_index[n], nb_base_level);

        FAD(&nb_release_count, 1);
        FAD(&nb_stat_alloc_blocks[nb_depth - nb_level(nb_index[n])], -1);
}

/* ------------------------------ STATISTICS -------------------------------- */

uint64_t nb_stat_min_size()
{
        return NB_MIN_SIZE;
}

uint32_t nb_stat_max_order()
{
        return NB_MAX_ORDER;
}

uint64_t nb_stat_tree_size()
{
        return nb_tree_size;
}

uint64_t nb_stat_index_size()
{
        return nb_index_size;
}

uint32_t nb_stat_depth()
{
        return nb_depth;
}

uint32_t nb_stat_base_level()
{
        return nb_base_level;
}

uint64_t nb_stat_max_size()
{
        return nb_max_size;
}

uint32_t nb_stat_release_count()
{
        return nb_release_count;
}


uint64_t nb_stat_total_memory()
{
        return nb_total_memory;
}

uint64_t nb_stat_used_memory()
{
        uint64_t used_memory = 0;

        for (uint32_t i = 0; i <= NB_MAX_ORDER; i++) {
                used_memory += nb_stat_alloc_blocks[i] * nb_stat_block_size(i);
        }

        return used_memory;
}

uint64_t nb_stat_block_size(uint32_t order)
{
        if (NB_MAX_ORDER < order) {
                return 0;
        }

        return EXP2(order) * NB_MIN_SIZE;
}

uint64_t nb_stat_total_blocks(uint32_t order)
{
        if (NB_MAX_ORDER < order) {
                return 0;
        }

        return nb_total_memory / nb_stat_block_size(order);
}

uint64_t nb_stat_used_blocks(uint32_t order)
{
        if (NB_MAX_ORDER < order) {
                return 0;
        }
        
        return nb_stat_alloc_blocks[order];
}


uint8_t nb_stat_occupancy_map(uint8_t *buff, uint32_t order)
{
        if (!buff || NB_MAX_ORDER < order) {
                return 1;
        }

        uint32_t start_node = EXP2(nb_depth - order);
        uint32_t end_node = EXP2(nb_depth - order + 1);

        for (uint32_t i = start_node; i < end_node; i++) {
                buff[i - start_node] = !nb_is_free(nb_tree[i]);
        }

        return 0;
}
