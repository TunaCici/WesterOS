#include "gtest/gtest.h"

#include <cstdint>

extern "C" {
        #include "Memory/PageDef.h"
        #include "Memory/BootMem.h"
        #include "Memory/Physical.h"
}

/* TODO: Explain what's going on here */
#define TEST_PM_PLAYGROUND_SIZE 8 * 1024 * 1024 + SIZEOF_BLOCK(MAX_ORDER - 1) - 1

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
