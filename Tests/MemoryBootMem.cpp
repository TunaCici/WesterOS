#include "gtest/gtest.h"

#include <cstdint>

extern "C" {
        #include "Memory/PageDef.h"
        #include "Memory/BootMem.h"
}

TEST(MemoryBootMem, init)
{
        uint8_t *playground = new uint8_t[(BM_ARENA_SIZE + 1) * PAGE_SIZE];
        for (auto i = 0; i < BM_ARENA_SIZE * PAGE_SIZE; i++) {
                playground[i] = 0;
        }

        uint32_t pageCount = bootmem_init((void*) playground);
        EXPECT_EQ(pageCount, BM_ARENA_SIZE);
}

TEST(MemoryBootMem, alloc)
{
        uint8_t *playground = new uint8_t[(BM_ARENA_SIZE + 1) * PAGE_SIZE];
        for (auto i = 0; i < BM_ARENA_SIZE * PAGE_SIZE; i++) {
                playground[i] = 0;
        }

        playground = (uint8_t*) PALIGN(playground);

        uint32_t pageCount = bootmem_init((void*) playground);
        EXPECT_EQ(pageCount, BM_ARENA_SIZE);

        /* stupid request - 0 */
        void *tmp = bootmem_alloc(0);
        EXPECT_TRUE(tmp == 0);


        /* more than what's allowed */
        tmp = bootmem_alloc(BM_ARENA_SIZE + 1);
        EXPECT_TRUE(tmp == 0);

        /* simple */
        tmp = bootmem_alloc(1);
        EXPECT_EQ(tmp, playground);

        /* does the map work? */
        tmp = bootmem_alloc(1);
        EXPECT_EQ(tmp, playground + PAGE_SIZE);
        tmp = bootmem_alloc(2);
        EXPECT_EQ(tmp, playground + PAGE_SIZE * 2);

        /* EVERYONE!! */
        tmp = bootmem_alloc(BM_ARENA_SIZE - 4);
        EXPECT_EQ(tmp, playground + PAGE_SIZE * 4);

        /* no more space;( */
        tmp = bootmem_alloc(1);
        EXPECT_TRUE(tmp == 0);

        /* addresses are valid? */
        for (auto i = 0; i < BM_ARENA_SIZE * PAGE_SIZE; i++) {
                playground[i] = 42;
                EXPECT_EQ(playground[i], 42);
        }
}
