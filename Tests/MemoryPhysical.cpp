#include <stdint.h>

#include "gtest/gtest.h"

extern "C" {
        #include "Memory/Physical.h"
}

TEST(MemoryPhysical, LetsGo)
{
        EXPECT_EQ(0, init_allocator(0x0, 0x0));       
}
