#include "gtest/gtest.h"

#include <algorithm>
#include <cstdint>

extern "C" {
        #include "Memory/PageDef.h"
        #include "Memory/BootMem.h"
}

TEST(MemoryBootMem, init)
{
        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(PAGE_SIZE, BM_ARENA_SIZE_BYTE));
        std::fill_n(playground, BM_ARENA_SIZE_BYTE, 0x0);

        uint32_t avail_bytes = bootmem_init((void*) playground);
        EXPECT_EQ(avail_bytes, BM_ARENA_SIZE_BYTE);
}

TEST(MemoryBootMem, alloc)
{
        uint8_t *playground = static_cast<uint8_t*>(
                std::aligned_alloc(PAGE_SIZE, BM_ARENA_SIZE_BYTE));

        std::fill_n(playground, BM_ARENA_SIZE_BYTE, 0x0);

        uint32_t avail_bytes = bootmem_init((void*) playground);
        EXPECT_EQ(avail_bytes, BM_ARENA_SIZE_BYTE);

        /* 0 maps to PAGE_SIZE */
        void *tmp = bootmem_alloc(0);
        EXPECT_EQ(tmp, playground);

        /* more than what's allowed */
        tmp = bootmem_alloc(BM_ARENA_SIZE_BYTE + 1);
        EXPECT_TRUE(tmp == nullptr);

        /* simple */
        tmp = bootmem_alloc(1024);
        EXPECT_EQ(tmp, playground + PAGE_SIZE);

        /* does the map work? */
        tmp = bootmem_alloc(PAGE_SIZE);
        EXPECT_EQ(tmp, playground + 2 * PAGE_SIZE);

        tmp = bootmem_alloc(2 * PAGE_SIZE);
        EXPECT_EQ(tmp, playground + 3 * PAGE_SIZE);

        /* rest of the arena */
        tmp = bootmem_alloc(BM_ARENA_SIZE_BYTE - 5 * PAGE_SIZE);
        EXPECT_EQ(tmp, playground + 5 * PAGE_SIZE);

        /* no more space;( */
        tmp = bootmem_alloc(1);
        EXPECT_TRUE(tmp == nullptr);

        /* addresses are valid? */
        for (auto i = 0; i < BM_ARENA_SIZE_BYTE; i++) {
                playground[i] = 42;
                EXPECT_EQ(playground[i], 42);
        }
}
