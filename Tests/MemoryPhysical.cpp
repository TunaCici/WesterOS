#include <stdint.h>

#include "gtest/gtest.h"

extern "C" {
        #include "Memory/PageDef.h"
}

TEST(MemoryPhysical, palign)
{
        uint8_t *someAddr = (uint8_t*) 0x0;

        /* Zero aligns to zero */
        EXPECT_EQ(0x0, PALIGN(someAddr));

        /* 0x1 aligns to PAGE_SIZE */
        someAddr = (uint8_t*) 0x1;
        EXPECT_EQ(PAGE_SIZE, PALIGN(someAddr));

        /* PAGE_SIZE aligns to PAGE_SIZE*/
        someAddr = (uint8_t*) PAGE_SIZE;
        EXPECT_EQ(PAGE_SIZE, PALIGN(someAddr));

        /* [0x2 ... PAGE_SIZE) aligns to PAGE_SIZE */
        for (uint32_t i = 2; i < PAGE_SIZE; i++)
        {
                someAddr = (uint8_t*) i;
                EXPECT_EQ(PAGE_SIZE, PALIGN(someAddr));
        }
}
