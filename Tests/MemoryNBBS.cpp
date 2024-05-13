#include "gtest/gtest.h"

#include <cstdint>
#include <cstdlib>
#include <vector>

extern "C" {
        #include "Memory/PageDef.h"
        #include "Memory/BootMem.h"
        #include "Memory/NBBS.h"
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

#define NBBS_TOTAL_MEMORY 64 * 1024 * 1024
#define NBBS_MIN_SIZE 4 * 1024
#define NBBS_MAX_ORDER 9

#define NBBS_DEPTH 14
#define NBBS_BASE_LEVEL 5
#define NBBS_MAX_SIZE 2 * 1024 * 1204

/* Runner information */
/* ---------------------------- */
/* Threads:                   4 */
/* ---------------------------- */

#define NBBS_THREADS 4

TEST(MemoryNBBS, __helpers)
{
        uint8_t status = 0;

        status = mark(status, 0);
        EXPECT_EQ(OCC_LEFT, status);
        status = unmark(status, 0);
        EXPECT_EQ(0, status);

        status = mark(status, 1);
        EXPECT_EQ(OCC_RIGHT, status);
        status = unmark(status, 1);
        EXPECT_EQ(0, status);

        status = set_coal(status, 0);
        EXPECT_EQ(COAL_LEFT, status);
        EXPECT_NE(0, is_coal(status, 0));
        EXPECT_EQ(0, clean_coal(status, 0));
        status = 0;

        status = set_coal(status, 1);
        EXPECT_EQ(COAL_RIGHT, status);
        EXPECT_NE(0, is_coal(status, 1));
        EXPECT_EQ(0, clean_coal(status, 1));
        status = 0;

        status = mark(status, 0);
        EXPECT_NE(0, is_occ_buddy(status, 1));
        status = mark(status, 1);
        EXPECT_NE(0, is_occ_buddy(status, 0));

        status = set_coal(status, 0);
        EXPECT_NE(0, is_coal_buddy(status, 1));
        status = set_coal(status, 1);
        EXPECT_NE(0, is_coal_buddy(status, 0));

        status = 0;
        EXPECT_NE(0, is_free(status));
        status |= BUSY;
        EXPECT_EQ(0, is_free(status));

        EXPECT_EQ(1, EXP2(0));
        EXPECT_EQ(2, EXP2(1));
        EXPECT_EQ(1024, EXP2(10));

        EXPECT_EQ(0, LOG2_LOWER(1));
        EXPECT_EQ(1, LOG2_LOWER(2));
        EXPECT_EQ(10, LOG2_LOWER(1024));

        EXPECT_EQ(0, LEVEL(1));
        EXPECT_EQ(1, LEVEL(2));
        EXPECT_EQ(10, LEVEL(1024));
}

TEST(MemoryNBBS, init)
{
        /* bootmem allocator is required - for now */
        uint8_t *bootmem_area = new uint8_t[(BM_ARENA_SIZE) * PAGE_SIZE];
        for (auto i = 0; i < BM_ARENA_SIZE * PAGE_SIZE; i++) {
                bootmem_area[i] = 0;
        }
        bootmem_init((void*) bootmem_area);

        /* Alloc 1 max_size block, in case the base_addr is not aligned */
        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(EXP2(NBBS_MAX_ORDER) * NBBS_MIN_SIZE, NBBS_TOTAL_MEMORY)
        );

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

TEST(MemoryNBBS, nb_alloc)
{
        /* Bootmem is required */
        uint8_t *bootmem_area = new uint8_t[(BM_ARENA_SIZE) * PAGE_SIZE];
        for (auto i = 0; i < BM_ARENA_SIZE * PAGE_SIZE; i++) {
                bootmem_area[i] = 0;
        }
        bootmem_init((void*) bootmem_area);

        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(EXP2(NBBS_MAX_ORDER) * NBBS_MIN_SIZE, NBBS_TOTAL_MEMORY)
        );
        for (auto i = 0; i < NBBS_TOTAL_MEMORY; i++) {
                playground[i] = 0;
        }

        nb_init((uint64_t) playground, NBBS_TOTAL_MEMORY);

        /* Size larger than max_size is NOT allowed */
        void *block = nb_alloc(NBBS_MAX_SIZE + 1);
        EXPECT_EQ(0, block);

        /* Alloc 2 of each order (2 is just an arbitrary number) */
        std::vector<uint8_t*> allocs = {};

        /* TODO: Left here */
        for (auto i = NBBS_DEPTH; NBBS_BASE_LEVEL < i; i--) {
                uint32_t block_size = __block_size(i);

                uint8_t *block1 = (uint8_t*) nb_alloc(block_size);
                uint8_t *block2 = (uint8_t*) nb_alloc(block_size);

                EXPECT_TRUE(block1 != 0);
                EXPECT_TRUE(block2 != 0);

                /* Write random value */
                for (auto j = 0; j < __block_size(i); j++) {
                        block1[j] = j % 256;
                        block2[j] = j % 256;
                }

                allocs.push_back(block1);
                allocs.push_back(block2);
        }

        /* Read values */
        auto curr_level = NBBS_DEPTH;
        auto toggle = 1;
        for (uint8_t* curr_block : allocs) {
                for (auto i = 0; i < __block_size(curr_level); i++) {
                        EXPECT_EQ(curr_block[i], i % 256);
                }

                /* Skip to next level after 2 consecutive blocks */
                toggle++;
                if (toggle % 3 == 0) {
                        toggle = 1;
                        curr_level--;
                }
        }
}

