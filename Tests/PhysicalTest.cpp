#include "gtest/gtest.h"

#include <cstdint>
#include <cstdlib>
#include <vector>

extern "C" {
        #include "Memory/PageDef.h"
        #include "Memory/BootMem.h"
        #include "Memory/Physical.h"
}

/* Memory arena information:    */
/* ---------------------------- */
/* Total memory:         64 MiB */
/* Min/alloc/page size:   4 KiB */
/* Max order:                 9 */
/* Depth/level:              14 */
/* Base level:                5 */
/* Max size:              2 MiB */
/* ---------------------------- */

#define NBBS_TOTAL_MEMORY (64 * 1024 * 1024)
#define NBBS_MIN_SIZE (4 * 1024)
#define NBBS_MAX_ORDER 9

#define NBBS_DEPTH 14
#define NBBS_BASE_LEVEL 5
#define NBBS_MAX_SIZE (2 * 1024 * 1024)

/* Runner information */
/* ---------------------------- */
/* Threads:                   4 */
/* ---------------------------- */

#define NBBS_THREADS 4

TEST(Physical, __helpers)
{
        uint8_t status = 0;

        status = nb_mark(status, 0);
        EXPECT_EQ(OCC_LEFT, status);
        status = nb_unmark(status, 0);
        EXPECT_EQ(0, status);

        status = nb_mark(status, 1);
        EXPECT_EQ(OCC_RIGHT, status);
        status = nb_unmark(status, 1);
        EXPECT_EQ(0, status);

        status = nb_set_coal(status, 0);
        EXPECT_EQ(COAL_LEFT, status);
        EXPECT_NE(0, nb_is_coal(status, 0));
        EXPECT_EQ(0, nb_clean_coal(status, 0));
        status = 0;

        status = nb_set_coal(status, 1);
        EXPECT_EQ(COAL_RIGHT, status);
        EXPECT_NE(0, nb_is_coal(status, 1));
        EXPECT_EQ(0, nb_clean_coal(status, 1));
        status = 0;

        status = nb_mark(status, 0);
        EXPECT_NE(0, nb_is_occ_buddy(status, 1));
        status = nb_mark(status, 1);
        EXPECT_NE(0, nb_is_occ_buddy(status, 0));

        status = nb_set_coal(status, 0);
        EXPECT_NE(0, nb_is_coal_buddy(status, 1));
        status = nb_set_coal(status, 1);
        EXPECT_NE(0, nb_is_coal_buddy(status, 0));

        status = 0;
        EXPECT_NE(0, nb_is_free(status));
        status |= BUSY;
        EXPECT_EQ(0, nb_is_free(status));

        EXPECT_EQ(1, EXP2(0));
        EXPECT_EQ(2, EXP2(1));
        EXPECT_EQ(1024, EXP2(10));

        EXPECT_EQ(0, LOG2_LOWER(1));
        EXPECT_EQ(1, LOG2_LOWER(2));
        EXPECT_EQ(10, LOG2_LOWER(1024));
}

TEST(Physical, init)
{
        /* Bootmem allocator is required */
        uint8_t *bootmem_arena = static_cast<uint8_t*>(
                std::aligned_alloc(PAGE_SIZE, BM_ARENA_SIZE_BYTE));
        std::fill_n(bootmem_arena, BM_ARENA_SIZE_BYTE, 0x0);

        bootmem_init((uint64_t) bootmem_arena);

        /* Alloc 1 max_size block, in case the base_addr is not aligned */
        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(EXP2(NBBS_MAX_ORDER) * NBBS_MIN_SIZE,
                        NBBS_TOTAL_MEMORY));

        for (auto i = 0; i < NBBS_TOTAL_MEMORY; i++) {
                playground[i] = 0;
        }

        /* Empty base or size NOT allowed */
        EXPECT_EQ(1, nb_init(0x0, 0));

        /* Less than min/alloc/page size is NOT allowed */
        EXPECT_EQ(1, nb_init((uint64_t) playground, NBBS_MIN_SIZE - 1));

        /* Min/alloc/page size allowed */
        EXPECT_EQ(0, nb_init((uint64_t) playground, NBBS_MIN_SIZE));

        /* Everything we have */
        EXPECT_EQ(0, nb_init((uint64_t) playground, NBBS_TOTAL_MEMORY));
}

TEST(Physical, nb_alloc)
{       
        /* Bootmem allocator is required */
        uint8_t *bootmem_arena = static_cast<uint8_t*>(
                std::aligned_alloc(PAGE_SIZE, BM_ARENA_SIZE_BYTE));
        std::fill_n(bootmem_arena, BM_ARENA_SIZE_BYTE, 0x0);

        bootmem_init((uint64_t) bootmem_arena);

        /* initialize */
        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(NBBS_MAX_SIZE, NBBS_TOTAL_MEMORY)
        );
        std::fill_n(playground, NBBS_TOTAL_MEMORY / sizeof(uint8_t), 0);

        EXPECT_EQ(0, nb_init((uint64_t) playground, NBBS_TOTAL_MEMORY));

        std::vector<int*> allocs = {};

        /* Boundry */
        ASSERT_NE((void*) 0, nb_alloc(0));
        ASSERT_NE((void*) 0, nb_alloc(NBBS_MIN_SIZE));
        ASSERT_NE((void*) 0, nb_alloc(NBBS_MAX_SIZE));
        EXPECT_EQ((void*) 0, nb_alloc(NBBS_MAX_SIZE + 1));

        /* Allocate (on all orders) */
        for (uint32_t i = 0; i <= NBBS_MAX_ORDER; i++) {
                int *alloc = 0;
                uint64_t alloc_size = nb_stat_block_size(i);

                alloc = (int*) nb_alloc(alloc_size);
                ASSERT_NE((void*) 0, alloc);

                allocs.push_back(alloc);
        }

        /* Write */
        for (size_t i = 0; i < allocs.size(); i++) {
                int elem_count = nb_stat_block_size(i) / sizeof(int);

                EXPECT_EQ(
                        allocs[i] + elem_count,
                        std::fill_n(allocs[i], elem_count, i + 1)
                );
        }

        /* Verify */
        for (size_t i = 0; i < allocs.size(); i++) {
                for (uint64_t j = 0 ; j < nb_stat_block_size(i); i += sizeof(int)) {
                        EXPECT_EQ((int) (i + 1), allocs[i][j]);
                }
        }

        /* Allocate rest of the blocks (order 0) */
        int free_blocks = nb_stat_total_blocks(0);

        for (uint32_t i = 0; i <= NBBS_MAX_ORDER; i++) {
                int64_t used_blocks = nb_stat_used_blocks(i) * (1 << i);
                free_blocks -= used_blocks;
        }

        for (int i = 0; i < free_blocks; i++) {
                EXPECT_NE((void*) 0, nb_alloc(NBBS_MIN_SIZE));
        }

        /* No memory left */
        EXPECT_EQ((void*) 0, nb_alloc(NBBS_MIN_SIZE));
}


TEST(Physical, nb_free)
{
        /* Bootmem is required */
        uint8_t *bootmem_arena = static_cast<uint8_t*>(
                std::aligned_alloc(PAGE_SIZE, BM_ARENA_SIZE_BYTE));
        std::fill_n(bootmem_arena, BM_ARENA_SIZE_BYTE, 0x0);

        bootmem_init((uint64_t) bootmem_arena);

        /* Initialize */
        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(NBBS_MAX_SIZE, NBBS_TOTAL_MEMORY)
        );
        std::fill_n(playground, NBBS_TOTAL_MEMORY / sizeof(uint8_t), 0);

        EXPECT_EQ(0, nb_init((uint64_t) playground, NBBS_TOTAL_MEMORY));

        std::vector<void*> allocs1 = {};
        std::vector<void*> allocs2 = {};

        /* NULL does nothing - shouldn't crash */
        nb_free((void*) 0);

        /* Try on all orders */
        for (uint32_t i = 0; i <= NBBS_MAX_ORDER; i++) {
                uint64_t block_count = nb_stat_total_blocks(i);

                /* Alloc all */
                for (uint64_t j = 0; j < block_count; j++) {
                        void *alloc = nb_alloc(nb_stat_block_size(i));
                
                        ASSERT_NE((void*) 0, alloc);
                        allocs1.push_back(alloc);
                }

                /* Verify all allocated */
                ASSERT_EQ(0, block_count - nb_stat_used_blocks(i));

                /* Free all */
                for (auto alloc : allocs1) {
                        nb_free(alloc);
                }
                
                /* Verify all freed */
                ASSERT_EQ(0, nb_stat_used_blocks(i));

                /* Alloc all again */
                for (uint64_t j = 0; j < block_count; j++) {
                        void *alloc = nb_alloc(nb_stat_block_size(i));
                
                        ASSERT_NE((void*) 0, alloc);
                        allocs2.push_back(alloc);
                }

                /* Same allocs should happen */
                ASSERT_EQ(allocs1, allocs2);

                /* Clean */
                for (auto alloc : allocs1) {
                        nb_free(alloc);
                }

                allocs1.clear();
                allocs2.clear();
        }

        /* Coalescing */
        // This alloc is going to prevent others to coalesce
        void *guard = nb_alloc(nb_stat_block_size(NBBS_MIN_SIZE));
        ASSERT_NE((void*) 0, guard);

        /* Alloc rest of the blocks on max_order */
        // We decrement by 1 because the first one is occuiped by 'guard'
        for (uint32_t i = 0; i < nb_stat_total_blocks(NBBS_MAX_ORDER)-1; i++) {
                ASSERT_NE((void*) 0, nb_alloc(NBBS_MAX_SIZE));
        }

        /* Can't alloc due to 'guard' occupying higher blocks */
        ASSERT_EQ((void*) 0x0, nb_alloc(NBBS_MAX_SIZE));

        /* Free and cause coalesing */
        nb_free(guard);

        /* Can alloc */
        ASSERT_NE((void*) 0, nb_alloc(NBBS_MAX_SIZE));
}
