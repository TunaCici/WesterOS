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

#define OCC_RIGHT       0x1
#define OCC_LEFT        0x2
#define COAL_RIGHT      0x4
#define COAL_LEFT       0x8
#define OCC             0x10
#define BUSY            (OCC | OCC_LEFT | OCC_RIGHT)

#define EXP2(n) (0x1 << (n))
#define LOG2(n) (64 - __builtin_clzll(n)) // 64 bit
#define CAS(addr, cmp, val) __atomic_compare_exchange (addr, cmp, val, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

/*
 * Public APIs
 *
 * TODO: Explain.
 */

int  nb_init(uint64_t base_addr, uint64_t size);
void* nb_alloc(uint64_t size);
void* nb_free(void *addr);

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
