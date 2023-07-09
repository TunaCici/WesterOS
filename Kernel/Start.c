#include <stdint.h>
#include <stdarg.h>

#include "ARM_Virt.h"

extern void kmain(void);

/* UART */
void _uart_putc(const char c)
{
        volatile uint32_t *uart = (uint32_t*)PL011_BASE;
        *uart = c;
}

void _puts(const char *s)
{
        while (*s) {
                _uart_putc(*s++);
        }
}

uint32_t _strlen(const char *s)
{
        uint32_t len = 0;
        while (*s++) {
                len++;
        }
        return len;
}

/* uint32_t to str */
void _uitoa(uint32_t n, char *s)
{
        int i = 0;
        int j = 0;
        char tmp;

        do {
                s[i] = n % 10 + '0';
                n /= 10;
                i++;
        } while (n > 0);

        s[i] = '\0';

        for (j = 0; j < i / 2; j++) {
                tmp = s[j];
                s[j] = s[i - j - 1];
                s[i - j - 1] = tmp;
        }
}

/* uint32_t to hex str */
void _uitoh(uint32_t n, char *s)
{
        int i = 0;
        int j = 0;
        char tmp;

        do {
                s[i] = n % 16 + '0';
                if (s[i] > '9') {
                        s[i] += 7;
                }
                n /= 16;
                i++;
        } while (n > 0);

        s[i] = '\0';

        for (j = 0; j < i / 2; j++) {
                tmp = s[j];
                s[j] = s[i - j - 1];
                s[i - j - 1] = tmp;
        }
}

void _halt(const char *s)
{
        _puts(s);
        for(;;);
}

/* Check if DTB exists */
void _fdt_check(void)
{
        volatile uint32_t *address = (uint32_t*) DTB_START;
        volatile uint32_t *magic = (uint32_t*) 0xEDFE0DD0; /* 0xDOODFEED */

        if (*address == magic) {
                _puts("DTB");
        } else {
                _halt("DTB not");
        }

        _puts(" found at Physical Address: 0x40000000\n");
}

/* Kernel & user page table addresses. Reserved in Kernel/kernel.ld */
extern uint64_t _kernel_pgtbl;
extern uint64_t _user_pgtbl;
extern uint64_t _K_l2_pgtbl;
extern uint64_t _U_l2_pgtbl;

uint64_t *kernel_pgtbl = &_kernel_pgtbl;
uint64_t *user_pgtbl = &_user_pgtbl;

extern void *end; /* First address after kernel. Defined in Kernel/kernel.ld */

void start(void)
{
        uint32_t arch;
        uint32_t val32;
        uint64_t val64;
        char *buf[64];

        /* TODO: Init CPU & Basic I/O before kmain() */
        _puts("WesterOS early boot stage\n");

        _puts("Running sanity checks...\n");

        _puts("Checking CPU\n");
        asm("MRS %[r], MIDR_EL1":[r]"=r" (val32));
        arch = (val32 & 0xFF000000) >> 24;

        if (arch == 0x41) {
                _puts("\tImplementer: ARM\n");
        } else {
                _halt("\t Unknown Implementer\n");
        }

        asm("MRS %[r], CurrentEL":[r]"=r" (val32));
        val32 = (val32 & 0x0C) >> 2;

        _puts("\tCurrent exception level: ");
        switch(val32) {
                case 0:
                        _puts("EL0 (User mode)\n");
                break;
                case 1:
                        _puts("EL1 (Kernel mode)\n");
                break;
                case 2:
                        _puts("EL2 (Hypervisor mode)\n");
                break;
                case 3:
                        _puts("EL3 (Secure Monitor mode)\n");
                break;
                default:
                        _halt("Unknown exception level\n");
                break;
        }

        /* Check if interrupts are enabled */
        asm("MRS %[r], DAIF":[r]"=r" (val32));
        _puts("\tIRQ: ");
        if (val32 & 0x80) {
                _puts("Enabled\n");
        } else {
                _puts("Disabled\n");
        }

        _puts("Checking main memory\n");
        _puts("\tDRAM size: ");
        val32 = RAM_SIZE;
        val32 /= 1024 * 1024;
        _uitoa(val32, buf);
        _puts(buf);
        _puts(" MiB\n");

        /* Page table buffer registers */
        asm("MRS %[r], MAIR_EL1":[r]"=r" (val64));


        _fdt_check();

        kmain();
}