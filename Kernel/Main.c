/*
 * Main kernel entry. What TODO? (further research required)
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#define BUFFER_SIZE 1024

void kmain(void)
{
        const uint8_t magic = 42;
        uint64_t buffer[BUFFER_SIZE];

        /* Do something weird */
        for(;;)
        {
                uint8_t i = 0;
                buffer[i++ % BUFFER_SIZE] += magic;
        }
}