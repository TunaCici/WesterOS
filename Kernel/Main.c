/*
 * Main kernel entry. What TODO? (further research required)
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#include "ARM64/Machine.h"
#include "MemoryLayout.h"

#include "LibKern/Time.h"
#include "LibKern/Console.h"

#define BUFFER_SIZE 1024

/* Kernel & user page table addresses. Defined in Kernel/kernel.ld */
extern uint64_t _kernel_pgtbl;
extern uint64_t _user_pgtbl;
extern uint64_t _K_l2_pgtbl;
extern uint64_t _U_l2_pgtbl;
extern uint64_t *end; /* First address after kernel. */

uint64_t *kernel_pgtbl = &_kernel_pgtbl;
uint64_t *user_pgtbl = &_user_pgtbl;

void kmain(void)
{
        const uint8_t magic = 42;
        uint64_t buffer[BUFFER_SIZE];

        /* Do something weird */
        klog("[kmain] imma just sleep\n");
        for(;;)
        {
                klog("[kmain] Zzz..\n");
                ksleep(3000);
        }
}
