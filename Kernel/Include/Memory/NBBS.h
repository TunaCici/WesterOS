/*
 * Non-Blocking Buddy System - To be used to the new physical memory mngr.
 *
 * As described in:
 * "A Non-blocking Buddy System for Scalable Memory Allocation on Multi-core 
 * Machines by" R. Marotta, M. Ianni, A. Scarselli, A. Pellegrini and F. Quaglia
 *
 * Reference:
 * https://github.com/HPDCS/NBBS
 *
 * Author: Tuna CICI
 */

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

#define EXP2(n) (0x1ULL << (n))
#define LOG2_LOWER(n) (64ULL - __builtin_clzll(n) - 1ULL) // 64 bit
#define CAS(addr, cmp, val) __atomic_compare_exchange_n(addr, cmp, val, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

#define LEVEL(n) LOG2_LOWER(n)

/*
 * Public APIs
 *
 * TODO: Explain.
 */

int  nb_init(uint64_t base_addr, uint64_t size);
void* nb_alloc(uint64_t size);
void nb_free(void *addr);

/*
 * Private APIs
 *
 * TODO: Explain.
 */

uint32_t try_alloc(uint32_t node);
void freenode(uint32_t node, uint32_t level);


/*
 * Helpers
 *
 * TODO:? Explain.
 */

static inline uint8_t clean_coal(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val & !(COAL_LEFT >> (child % 2))));
}

static inline uint8_t mark(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val | (OCC_LEFT >> (child % 2))));
}

static inline uint8_t unmark(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val & !((COAL_LEFT | OCC_LEFT) >> (child % 2))));
}

static inline uint8_t is_coal(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val & (COAL_LEFT >> (child % 2))));
}

static inline uint8_t is_occ_buddy(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val & (OCC_RIGHT << (child % 2))));
}

static inline uint8_t is_coal_buddy(uint8_t val, uint32_t child)
{
        return ((uint8_t) (val & (COAL_RIGHT << (child % 2))));
}

static inline uint8_t is_free(uint8_t val)
{
        return !(val & BUSY);
}

/* TODO: Ugly code; refactor */
static inline uint32_t leftmost(uint32_t node, uint32_t depth)
{
        /* Index to level */
        uint32_t level = LOG2_LOWER(node);

        /* Size (in terms of leaf size) */
        uint64_t block_size = EXP2(depth - level);

        /* Offset within level */
        uint64_t offset = node % EXP2(level);

        /* Leftmost leaf */
        uint32_t leftmost_leaf = EXP2(depth) + offset * block_size;

        return leftmost_leaf;
}
