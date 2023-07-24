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

#include "Memory/Physical.h"

#define BUFFER_SIZE 1024

/* Kernel & user page table addresses. Defined in Kernel/kernel.ld */
extern uint64_t _kernel_pgtbl;
extern uint64_t _user_pgtbl;
extern uint64_t _K_l2_pgtbl;
extern uint64_t _U_l2_pgtbl;

extern uint64_t kstart;
extern uint64_t kend;

uint64_t *kernel_pgtbl = &_kernel_pgtbl;
uint64_t *user_pgtbl = &_user_pgtbl;

void kmain(void)
{
        const uint8_t *kernelBase = (uint8_t*) &kstart;
        const uint8_t *kernelEnd = (uint8_t*) &kend;
        const uint8_t *ramEnd = (uint8_t*) RAM_END;
        
        uint64_t pageCount = init_allocator(kernelEnd, ramEnd);
        
        klog("[kmain] Amount of pages available: %u\n", &pageCount);

        /* Do something weird */
        klog("[kmain] imma just sleep\n");
        for(;;)
        {
                klog("[kmain] Zzz..\n");
                ksleep(3000);
        }
}
