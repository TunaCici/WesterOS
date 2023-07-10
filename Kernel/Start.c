#include <stdint.h>
#include <stdarg.h>

#include "MemoryLayout.h"
#include "Console.h"

extern void kmain(void);


void _halt(const char *s)
{
        kprintf(s);
        for(;;);
}

/* Check if DTB exists */
void _fdt_check(void)
{
        volatile uint32_t *address = (uint32_t*) DTB_START;
        volatile uint32_t *magic = (uint32_t*) 0xEDFE0DD0; /* 0xDOODFEED */

        if (*address == magic) {
                kprintf("DTB");
        } else {
                _halt("DTB not");
        }

        kprintf(" found at Physical Address: 0x%p\n", &address);
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
        char *buf[64];

        /* TODO: Init CPU & Basic I/O before kmain() */
        kprintf("WesterOS early boot stage\n");

        kprintf("Running sanity checks...\n");

        kprintf("Checking CPU\n");
        asm("MRS %[r], MIDR_EL1":[r]"=r" (val32));
        arch = (val32 & 0xFF000000) >> 24;

        if (arch == 0x41) {
                kprintf("\tImplementer: ARM\n");
        } else {
                _halt("\t Unknown Implementer\n");
        }

        asm("MRS %[r], CurrentEL":[r]"=r" (val32));
        val32 = (val32 & 0x0C) >> 2;

        kprintf("\tCurrent exception level: ");
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
        asm("MRS %[r], DAIF":[r]"=r" (val32));
        kprintf("\tIRQ: ");
        if (val32 & 0x80) {
                kprintf("Enabled\n");
        } else {
                kprintf("Disabled\n");
        }

        /* Page table buffer registers */
        asm("MRS %[r], MAIR_EL1":[r]"=r" (val64));

        _fdt_check();

        kmain();
}