/*
 * Non-Blocking Buddy System definitions
 *
 * As described in:
 * "A Non-blocking Buddy System for Scalable Memory Allocation on Multi-core 
 * Machines by" R. Marotta, M. Ianni, A. Scarselli, A. Pellegrini and F. Quaglia
 *
 * Reference:
 * https://ieeexplore.ieee.org/document/9358002
 * https://github.com/HPDCS/NBBS
 *
 * Author: Tuna CICI
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NBBS_H
#define NBBS_H

#include <stdint.h>

/*
 * Tree node status bits:
 *
 *  7       5          4            3            2          1          0
 * +---------+----------+------------+------------+----------+----------+
 * | ignored | occupied |    left    |    rigth   |   left   |   right  |
 * |         |          | coalescent | coalescent | occupied | occupied |
 * +---------+----------+------------+------------+----------+----------+
 */

#define OCC_RIGHT       ((uint8_t) 0x1)
#define OCC_LEFT        ((uint8_t) 0x2)
#define COAL_RIGHT      ((uint8_t) 0x4)
#define COAL_LEFT       ((uint8_t) 0x8)
#define OCC             ((uint8_t) 0x10)
#define BUSY            (OCC | OCC_LEFT | OCC_RIGHT)

/*
 * Configuration
 */

#define NB_MIN_SIZE 4096ULL /* bytes */
#define NB_MAX_ORDER 9U
#define NB_MALLOC(size) bootmem_alloc(size)

/*
 * Math functions
 */

#define EXP2(n) (0x1ULL << (n))
#define LOG2_LOWER(n) (64ULL - __builtin_clzll(n) - 1ULL) // 64 bit

/*
 * Atomic operations:
 *
 * FAD: Fetch-And-Decrement
 * BCAS: Binary-Compare-And-Swap
 * VCAS: Value-Compare-And-Swap
 */

#define FAD(ptr, val) \
        __atomic_add_fetch(ptr, val, __ATOMIC_SEQ_CST)
#define BCAS(ptr, expected, desired) \
        __atomic_compare_exchange_n(ptr, expected, desired, 0, \
                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#define VCAS(ptr, expected, desired) \
        __atomic_compare_exchange_n(ptr, expected, desired, 0, \
                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? (*expected) : 0

/*
 * Public APIs
 */

int  nb_init(uint64_t base_addr, uint64_t size);
void* nb_alloc(uint64_t size);
void nb_free(void *addr);

/*
 * Private APIs
 */

uint32_t __nb_try_alloc(uint32_t node);
void __nb_freenode(uint32_t node, uint32_t upper_bound);
void __nb_unmark(uint32_t node, uint32_t upper_bound);

uint32_t __nb_leftmost(uint32_t node, uint32_t depth);
void __nb_clean_block(void* addr, uint64_t size);

/*
 * Statistics
 */

uint64_t nb_stat_min_size();
uint32_t nb_stat_max_order();

uint64_t nb_stat_tree_size();
uint64_t nb_stat_index_size();
uint32_t nb_stat_depth();
uint32_t nb_stat_base_level();
uint64_t nb_stat_max_size();
uint32_t nb_stat_release_count();

uint64_t nb_stat_total_memory();
uint64_t nb_stat_used_memory();

uint64_t nb_stat_block_size(uint32_t order);
uint64_t nb_stat_total_blocks(uint32_t order);
uint64_t nb_stat_used_blocks(uint32_t order);

uint8_t nb_stat_occupancy_map(uint8_t *buff, uint32_t order);

/*
 * Helpers
 */
static inline uint8_t nb_mark(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val | (OCC_LEFT >> (child % 2))));
}

static inline uint8_t nb_unmark(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val & ~((OCC_LEFT | COAL_LEFT) >> (child % 2))));
}

static inline uint8_t nb_set_coal(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val | (COAL_LEFT >> (child % 2))));
}

static inline uint8_t nb_clean_coal(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val & ~(COAL_LEFT >> (child % 2))));
}

static inline uint8_t nb_is_coal(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val & (COAL_LEFT >> (child % 2))));
}

static inline uint8_t nb_is_occ_buddy(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val & (OCC_RIGHT << (child % 2))));
}

static inline uint8_t nb_is_coal_buddy(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val & (COAL_RIGHT << (child % 2))));
}

static inline uint8_t nb_is_free(uint8_t val)
{
        return !(val & BUSY);
}

static inline uint32_t nb_level(uint32_t node)
{
        return LOG2_LOWER(node);
}

#endif /* NBBS_H */

#ifdef __cplusplus
}
#endif