/*
 * Early boot stage. Sanity checks for hardware devices & "loads" kmain(...)
 *
 * Author: Tuna CICI
 */

#include <stdint.h>
#include <stdarg.h>

#include "MemoryLayout.h"
#include "LibKern/Console.h"

extern void kmain(void);


void _halt(const char *s)
{
        klog(s);
        for(;;);
}

/* Check if DTB exists */
void _fdt_check(void)
{
        volatile uint32_t *address = (uint32_t*) DTB_START;
        volatile uint32_t *magic = (uint32_t*) 0xEDFE0DD0; /* 0xDOODFEED */

        if (*address == magic) {
                klog("DTB");
        } else {
                _halt("DTB not");
        }

        kprintf(" found at Physical Address: 0x%p\n", &address);
}


/* Sleep for 'msec' miliseconds */
void __attribute__((optimize("O0"))) ksleep(const uint64_t msec) 
{
        volatile uint64_t CNTVCT_EL0 = 0;
        volatile uint64_t CNTFRQ_EL0 = 0;

        uint64_t entry = 0;
        uint64_t curr = 0;
 
        if (msec == 0) {
                return;
        }

        asm volatile("MRS %[r], CNTVCT_EL0;" : [r]"=r" (CNTVCT_EL0));
        asm volatile("MRS %[r], CNTFRQ_EL0;" : [r]"=r" (CNTFRQ_EL0));

        entry = CNTVCT_EL0 * 1000 / CNTFRQ_EL0; /* Miliseconds passed so far */

        do {       
                asm volatile("MRS %[r], CNTVCT_EL0;" : [r]"=r" (CNTVCT_EL0));

                curr = CNTVCT_EL0 * 1000 / CNTFRQ_EL0;
        } while ((curr - entry) < msec);
}

/* Kernel & user page table addresses. Defined in Kernel/kernel.ld */
extern uint64_t _kernel_pgtbl;
extern uint64_t _user_pgtbl;
extern uint64_t _K_l2_pgtbl;
extern uint64_t _U_l2_pgtbl;
extern uint64_t *end; /* First address after kernel. */

uint64_t *kernel_pgtbl = &_kernel_pgtbl;
uint64_t *user_pgtbl = &_user_pgtbl;

void start(void)
{
        uint32_t arch;
        uint32_t val32;
        uint64_t val64;

        klog("WesterOS early boot stage\n");

        klog("Running sanity checks...\n");

        klog("Checking CPU\n");
        asm volatile("MRS %[r], MIDR_EL1":[r]"=r" (val32));
        arch = (val32 & 0xFF000000) >> 24;

        if (arch == 0x41) {
                klog("\tImplementer: ARM\n");
        } else {
                _halt("\t Unknown Implementer\n");
        }

        asm volatile("MRS %[r], CurrentEL":[r]"=r" (val32));
        val32 = (val32 & 0x0C) >> 2;

        klog("\tCurrent exception level: ");
        switch(val32) {
                case 0:
                        kprintf("EL0 (User mode)\n");
                break;
                case 1:
                        kprintf("EL1 (Kernel mode)\n");
                break;
                case 2:
                        kprintf("EL2 (Hypervisor mode)\n");
                break;
                case 3:
                        kprintf("EL3 (Secure Monitor mode)\n");
                break;
                default:
                        _halt("Unknown exception level\n");
                break;
        }

        /* Check if interrupts are enabled */
        asm volatile("MRS %[r], DAIF" : [r]"=r" (val32));
        klog("\tIRQ: ");
        if (val32 & 0x80) {
                kprintf("Enabled\n");
        } else {
                kprintf("Disabled\n");
        }

        _fdt_check();

        uint32_t delay = 1250u;
        klog("Sleeping for %u miliseconds. Zzz...\n", &delay);
        ksleep(delay);
        klog("Huh? Who woke me up?!\n");

        kmain();
}