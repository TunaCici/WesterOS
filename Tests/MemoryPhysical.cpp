#include "gtest/gtest.h"

#include <cstdint>
#include <vector>

extern "C" {
        #include "Memory/PageDef.h"
        #include "Memory/BootMem.h"
        #include "Memory/Physical.h"
}

/* Test using 8 MiB of space (4x 2 MiB blocks) */

/* With `new` we might NOT get a 2 MiB aligned addr (e.g. 0x200001)           */
/* PMM aligns the start addr to 2 MiB. so, i need to account for misalignment */
/* Allocating one (1) extra block helps solve this issue                      */
/* But this time, If `new` gives us an aligned addr, PMM inits 5x blocks      */
/* I want a deterministic results (4x blocks). Substricting one (1) helps it  */
#define TEST_PM_PLAYGROUND_SIZE 5 * SIZEOF_BLOCK(MAX_ORDER - 1) - 1

TEST(MemoryPhysical, __clear_page_area)
{
        uint8_t *playground = new uint8_t[PAGE_SIZE];
        for (auto i = 0; i < PAGE_SIZE; i++) {
                playground[i] = 42;
        }

        __clear_page_area(playground, PAGE_SIZE);

        for (auto i = 0; i < PAGE_SIZE; i++) {
                EXPECT_EQ(playground[i], 0);
        }
}

TEST(MemoryPhysical, __budy)
{
        list_head_t *addr = 0x0;

        for (auto i = 0; i < MAX_ORDER; i++) {
                list_head_t *buddy = __buddy(addr, i);
                EXPECT_EQ(buddy, (list_head_t*) SIZEOF_BLOCK(i));
                
                list_head_t *buddyBuddy = __buddy(buddy, i);
                EXPECT_EQ(buddyBuddy, addr);
        }        
}

TEST(MemoryPhysical, init)
{
        /* Bootmem is required */
        uint8_t *bootmem_arena = static_cast<uint8_t*>(
                std::aligned_alloc(PAGE_SIZE, BM_ARENA_SIZE_BYTE));
        std::fill_n(bootmem_arena, BM_ARENA_SIZE_BYTE, 0x0);
        
        bootmem_init((void*) bootmem_arena);

        uint8_t *playground = new uint8_t[TEST_PM_PLAYGROUND_SIZE];
        for (auto i = 0; i < TEST_PM_PLAYGROUND_SIZE; i++) {
                playground[i] = 0;
        }

        /* Align to MAX_ORDER - 1 block size (as internal functions do so) */
        playground = (uint8_t*) CUSTOM_ALIGN(playground,
               SIZEOF_BLOCK(MAX_ORDER - 1));

        /* Less than SIZEOF(MAX_ORDER - 1) is not allowed */
        uint64_t blockCount = init_allocator(
                0x0, 
                (void*) (SIZEOF_BLOCK(MAX_ORDER - 1) - 1)
        );
        EXPECT_EQ(blockCount, 0);

        /* Minimum size allowed (1 block) */
        blockCount = init_allocator(
                playground,
                playground + SIZEOF_BLOCK(MAX_ORDER - 1)
        );
        EXPECT_EQ(blockCount, 1);

        /* Ideal size (4 blocks) */
        blockCount = init_allocator(
                playground,
                playground + TEST_PM_PLAYGROUND_SIZE
        );

        EXPECT_EQ(blockCount, 4); /* 8 MiB divided into 4x 2 MiB blocks */
}

TEST(MemoryPhysical, __block_to_idx)
{
        /* Bootmem is required */
        uint8_t *bootmem_arena = static_cast<uint8_t*>(
                std::aligned_alloc(PAGE_SIZE, BM_ARENA_SIZE_BYTE));
        std::fill_n(bootmem_arena, BM_ARENA_SIZE_BYTE, 0x0);

        bootmem_init((void*) bootmem_arena);

        uint8_t *playground = new uint8_t[TEST_PM_PLAYGROUND_SIZE];
        for (auto i = 0; i < TEST_PM_PLAYGROUND_SIZE; i++) {
                playground[i] = 0;
        }

        /* Align to MAX_ORDER - 1 block size (as internal functions do so) */
        playground = (uint8_t*) CUSTOM_ALIGN(playground,
               SIZEOF_BLOCK(MAX_ORDER - 1));

        uint64_t blockCount = init_allocator(
                playground,
                playground + TEST_PM_PLAYGROUND_SIZE
        );

        for (auto i = 0; i < blockCount; i++) {
                uint64_t idx = __block_to_idx(
                        (list_head_t*) (playground + i * SIZEOF_BLOCK(MAX_ORDER - 1)),
                        MAX_ORDER - 1
                );
                EXPECT_EQ(idx, i);
        }
}

TEST(MemoryPhysical, alloc_page)
{
        /* Bootmem is required */
        uint8_t *bootmem_arena = static_cast<uint8_t*>(
                std::aligned_alloc(PAGE_SIZE, BM_ARENA_SIZE_BYTE));
        std::fill_n(bootmem_arena, BM_ARENA_SIZE_BYTE, 0x0);

        bootmem_init((void*) bootmem_arena);

        uint8_t *playground = new uint8_t[TEST_PM_PLAYGROUND_SIZE];
        for (auto i = 0; i < TEST_PM_PLAYGROUND_SIZE; i++) {
                playground[i] = 0;
        }

        /* Align to MAX_ORDER - 1 block size (as internal functions do so) */
        playground = (uint8_t*) CUSTOM_ALIGN(playground,
               SIZEOF_BLOCK(MAX_ORDER - 1));

        uint64_t blockCount = init_allocator(
                playground,
                playground + TEST_PM_PLAYGROUND_SIZE
        );

        std::vector<uint8_t*> allocs = {};
        auto pagesCount = blockCount * SIZEOF_BLOCK(MAX_ORDER - 1) / PAGE_SIZE;
        EXPECT_EQ(pagesCount, 2048); /* 8 MiB == 2048 * PAGE_SIZE */

        /* Allocate single pages */
        for (auto i = 0; i < pagesCount; i++) {
                uint8_t *page = (uint8_t*) alloc_page();
                EXPECT_EQ(page, playground + i * PAGE_SIZE);

                /* Write random value */
                for (auto j = 0; j < PAGE_SIZE; j++) {
                        page[j] = j % 256;
                }

                allocs.push_back(page);
        }

        /* Read values */
        for (auto page : allocs) {
                for (auto i = 0; i < PAGE_SIZE; i++) {
                        EXPECT_EQ(page[i], i % 256);
                }
        }

        /* No available space left at this point */
        uint8_t *page = (uint8_t*) alloc_page();
        EXPECT_TRUE(page == nullptr);
}

TEST(MemoryPhysical, alloc_pages)
{
        /* Bootmem is required */
        uint8_t *bootmem_arena = static_cast<uint8_t*>(
                std::aligned_alloc(PAGE_SIZE, BM_ARENA_SIZE_BYTE));
        std::fill_n(bootmem_arena, BM_ARENA_SIZE_BYTE, 0x0);

        bootmem_init((void*) bootmem_arena);

        uint8_t *playground = new uint8_t[TEST_PM_PLAYGROUND_SIZE];
        for (auto i = 0; i < TEST_PM_PLAYGROUND_SIZE; i++) {
                playground[i] = 0;
        }

        /* Align to MAX_ORDER - 1 block size (as internal functions do so) */
        playground = (uint8_t*) CUSTOM_ALIGN(playground,
               SIZEOF_BLOCK(MAX_ORDER - 1));

        uint64_t blockCount = init_allocator(
                playground,
                playground + TEST_PM_PLAYGROUND_SIZE
        );

        /* Any alloc outside the range [0 ... MAX_ORDER) is not allowed */
        uint8_t *block = (uint8_t*) alloc_pages(MAX_ORDER);
        EXPECT_TRUE(block == nullptr);

        /* Alloc 2 of each order (2 is just an arbitrary number) */
        std::vector<uint8_t*> allocs = {};

        for (auto i = MAX_ORDER - 1; 0 <= i; i--) {
                uint8_t *block1 = (uint8_t*) alloc_pages(i);
                uint8_t *block2 = (uint8_t*) alloc_pages(i);

                EXPECT_TRUE(block1 != nullptr);
                EXPECT_TRUE(block2 != nullptr);

                /* Write random value */
                for (auto j = 0; j < SIZEOF_BLOCK(i); j++) {
                        block1[j] = j % 256;
                        block2[j] = j % 256;
                }

                allocs.push_back(block1);
                allocs.push_back(block2);
        }

        /* Read values */
        auto readOrder = MAX_ORDER - 1;
        auto toggle = 1;
        for (auto block : allocs) {
                for (auto i = 0; i < SIZEOF_BLOCK(readOrder); i++) {
                        EXPECT_EQ(block[i], i % 256);
                }

                /* Skip to next 'readOrder' after 2 consecutive blocks */
                toggle++;
                if (toggle % 3 == 0) {
                        toggle = 1;
                        readOrder--;
                }
        }
}

TEST(MemoryPhysical, free_page)
{
        /* Bootmem is required */
        uint8_t *bootmem_arena = static_cast<uint8_t*>(
                std::aligned_alloc(PAGE_SIZE, BM_ARENA_SIZE_BYTE));
        std::fill_n(bootmem_arena, BM_ARENA_SIZE_BYTE, 0x0);

        bootmem_init((void*) bootmem_arena);

        uint8_t *playground = new uint8_t[TEST_PM_PLAYGROUND_SIZE];
        for (auto i = 0; i < TEST_PM_PLAYGROUND_SIZE; i++) {
                playground[i] = 0;
        }

        /* Align to MAX_ORDER - 1 block size (as internal functions do so) */
        playground = (uint8_t*) CUSTOM_ALIGN(playground,
               SIZEOF_BLOCK(MAX_ORDER - 1));

        uint64_t blockCount = init_allocator(
                playground,
                playground + TEST_PM_PLAYGROUND_SIZE
        );

        std::vector<uint8_t*> allocs1 = {};
        std::vector<uint8_t*> allocs2 = {};

        auto pagesCount = blockCount * SIZEOF_BLOCK(MAX_ORDER - 1) / PAGE_SIZE;
        EXPECT_EQ(pagesCount, 2048); /* 8 MiB == 2048 * PAGE_SIZE */

        /* Allocate single pages */
        for (auto i = 0; i < pagesCount; i++) {
                uint8_t *page = (uint8_t*) alloc_page();
                EXPECT_EQ(page, playground + i * PAGE_SIZE);

                /* Write random value */
                for (auto j = 0; j < PAGE_SIZE; j++) {
                        page[j] = j % 256;
                }

                allocs1.push_back(page);
        }

        /* Free them */
        for (auto page : allocs1) {
                free_page(page);
        }

        /* Allocate again */
        for (auto i = 0; i < pagesCount; i++) {
                uint8_t *page = (uint8_t*) alloc_page();
                EXPECT_EQ(page, playground + i * PAGE_SIZE);

                /* Write random value */
                for (auto j = 0; j < PAGE_SIZE; j++) {
                        page[j] = j % 256;
                }

                allocs2.push_back(page);
        }

        /* Should be the same addresses */
        for (auto i = 0; i < pagesCount; i++) {
                EXPECT_EQ(allocs1[i], allocs2[i]);
        }
}

TEST(MemoryPhysical, free_pages)
{
        /* Bootmem is required */
        uint8_t *bootmem_arena = static_cast<uint8_t*>(
                std::aligned_alloc(PAGE_SIZE, BM_ARENA_SIZE_BYTE));
        std::fill_n(bootmem_arena, BM_ARENA_SIZE_BYTE, 0x0);

        bootmem_init((void*) bootmem_arena);

        uint8_t *playground = new uint8_t[TEST_PM_PLAYGROUND_SIZE];
        for (auto i = 0; i < TEST_PM_PLAYGROUND_SIZE; i++) {
                playground[i] = 0;
        }

        /* Align to MAX_ORDER - 1 block size (as internal functions do so) */
        playground = (uint8_t*) CUSTOM_ALIGN(playground,
               SIZEOF_BLOCK(MAX_ORDER - 1));

        uint64_t blockCount = init_allocator(
                playground,
                playground + TEST_PM_PLAYGROUND_SIZE
        );

        std::vector<uint8_t*> allocs = {};

        auto pagesCount = blockCount * SIZEOF_BLOCK(MAX_ORDER - 1) / PAGE_SIZE;
        EXPECT_EQ(pagesCount, 2048); /* 8 MiB == 2048 * PAGE_SIZE */

        auto currBlockCount = blockCount;
        for (auto i = MAX_ORDER - 1; 0 <= i; i--) {
                /* Allocate all on current order */
                for (auto j = 0; j < currBlockCount; j++) {
                        uint8_t *block = (uint8_t*) alloc_pages(i);
                        EXPECT_TRUE(block != nullptr);

                        allocs.push_back(block);
                }

                /* Free them all */
                for (auto block : allocs) {
                        free_pages(block, i);
                }
                
                /* Should be able to allocate them, again */
                for (auto j = 0; j < currBlockCount; j++) {
                        uint8_t *block = (uint8_t*) alloc_pages(i);
                        EXPECT_TRUE(block != nullptr);
                }

                /* Free them again for the next iter */
                for (auto block : allocs) {
                        free_pages(block, i);
                }

                currBlockCount *= 2;
                allocs.clear();
        }
}
