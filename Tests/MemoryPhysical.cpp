#include "gtest/gtest.h"

#include <cstdint>

extern "C" {
        #include "Memory/PageDef.h"
        #include "Memory/BootMem.h"
        #include "Memory/Physical.h"
}

/* Test using 8 MiB of space (4x 2 MiB blocks) */

/* With `new` we might NOT get a 2 MiB aligned addr (e.g. 0x200001)           */
/* PMM aligns the start addr to 2 MiB. so, i need to account for misalignment */
/* Allocating one (1) extra block helps solve this issue                      */
/* But this time, If `new` gives us 2 MiB aligned addr, PMM inits 5x blocks   */
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
        /* Less than SIZEOF(MAX_ORDER - 1) is not allowed */
        uint64_t blockCount = init_allocator(
                0x0, (void*) (SIZEOF_BLOCK(MAX_ORDER - 1) - 1)
        );
        EXPECT_EQ(blockCount, 0);

        /* Bootmem is required */
        uint8_t *bootmem_area = new uint8_t[(BM_ARENA_SIZE) * PAGE_SIZE];
        for (auto i = 0; i < BM_ARENA_SIZE * PAGE_SIZE; i++) {
                bootmem_area[i] = 0;
        }
        bootmem_init((void*) bootmem_area);

        uint8_t *playground = new uint8_t[TEST_PM_PLAYGROUND_SIZE];
        for (auto i = 0; i < TEST_PM_PLAYGROUND_SIZE; i++) {
                playground[i] = 0;
        }

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
        uint8_t *bootmem_area = new uint8_t[(BM_ARENA_SIZE) * PAGE_SIZE];
        for (auto i = 0; i < BM_ARENA_SIZE * PAGE_SIZE; i++) {
                bootmem_area[i] = 0;
        }
        bootmem_init((void*) bootmem_area);

        uint8_t *playground = new uint8_t[TEST_PM_PLAYGROUND_SIZE];
        for (auto i = 0; i < TEST_PM_PLAYGROUND_SIZE; i++) {
                playground[i] = 0;
        }

        uint64_t blockCount = init_allocator(
                playground,
                playground + TEST_PM_PLAYGROUND_SIZE
        );

        /* Align to MAX_ORDER - 1 block size (as internal functions do so) */
        playground = (uint8_t*) CUSTOM_ALIGN(playground,
               SIZEOF_BLOCK(MAX_ORDER - 1));

        for (auto i = 0; i < blockCount; i++) {
                uint64_t idx = __block_to_idx(
                        (list_head_t*) (playground + i * SIZEOF_BLOCK(MAX_ORDER - 1)),
                        MAX_ORDER - 1
                );
                EXPECT_EQ(idx, i);
        }
}
