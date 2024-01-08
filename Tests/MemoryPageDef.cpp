#include <stdint.h>

#include "gtest/gtest.h"

extern "C" {
        #include "Memory/PageDef.h"
}

TEST(MemoryPageDef, PALIGN)
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

TEST(MemoryPageDef, CUSTOM_ALIGN)
{
        uint8_t *someAddr = (uint8_t*) 0x0;
        constexpr int ALLIGNMENT = 64; /* bytes */

        /* Zero aligns to zero */
        EXPECT_EQ(0x0, CUSTOM_ALIGN(someAddr, ALLIGNMENT));

        /* 0x1 aligns to ALLIGNMENT */
        someAddr = (uint8_t*) 0x1;
        EXPECT_EQ(ALLIGNMENT, CUSTOM_ALIGN(someAddr, ALLIGNMENT));

        /* ALLIGNMENT aligns to ALLIGNMENT*/
        someAddr = (uint8_t*) ALLIGNMENT;
        EXPECT_EQ(ALLIGNMENT, CUSTOM_ALIGN(someAddr, ALLIGNMENT));

        /* [0x2 ... ALLIGNMENT) aligns to ALLIGNMENT */
        for (uint32_t i = 2; i < ALLIGNMENT; i++)
        {
                someAddr = (uint8_t*) i;
                EXPECT_EQ(ALLIGNMENT, CUSTOM_ALIGN(someAddr, ALLIGNMENT));
        }
}
