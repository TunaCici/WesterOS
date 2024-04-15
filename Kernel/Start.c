/*
 * Early boot stage. Sanity checks for hardware devices & "calls" kmain(...)
 *
 * Author: Tuna CICI
 */

#include <stdint.h>
#include <stdarg.h>

#include "ARM64/Machine.h"
#include "ARM64/RegisterSet.h"

#include "MemoryLayout.h"

extern void kmain(void);

void _utoa(uint64_t uval, char *buff, uint8_t base) {
        const char digits[] = "0123456789ABCDEF";
        int i = 0;

        /* Convert to string. Digits are in reverse order */
        do {
                buff[i++] = digits[uval % base];
        } while((uval /= base) != 0);
        buff[i--] = '\0';

        /* Reverse the string */
        for (int j = 0; j < i; j++, i--) {
                char tmp = buff[j];

                buff[j] = buff[i];
                buff[i] = tmp;
        }
}

void _uart_putc(const int c)
{
        volatile uint8_t *uart0 = (uint8_t*) PL011_BASE;
        *uart0 = c;
}


void _puts(const char *s)
{
        while (*s != '\0') {
                _uart_putc(*s);
                s++;
        }
}

void _halt(const char *s)
{
        _puts("Halting due to: ");
        _puts(s);

        wfi();
}

void start(void)
{
        volatile uint32_t arch = 0;
        volatile uint32_t val32 = 0;
        volatile uint64_t val64 = 0;

        char buff[64] = {0};

        /* Hard-coded device/board info */
        /* TODO: Replace this with a DTB parser */
        const char      *_cpuModel  = "Cortex A-72";
        const uint32_t  _coreCount =  2u;

        _puts("WesterOS early boot stage\n");
        _puts("Running sanity checks\n");

        /* TODO: Any better way to early print? */
        _puts("WARN: Raw printing directly to PL011 @ 0x");
        _utoa(PL011_BASE, buff, 16);
        _puts(buff);
        _puts("\n");

        /* -------- CPU -------- */
        _puts("Checking CPU\n");
        MRS("MIDR_EL1", val32);
        arch = (val32 & 0xFF000000) >> 24;

        if (arch == 0x41) {
                _puts("---- Implementer: ARM\n");
        } else {
                _halt("---- Unknown Implementer\n");
        }

        _puts("---- Model: ");
        _puts(_cpuModel);
        _puts("\n");

        _puts("---- SMP: ");
        _utoa(_coreCount, buff, 10);
        _puts(buff);
        _puts("\n");

        MRS("CNTFRQ_EL0", val64);
        val64 = val64 / 1000000; /* Hz to MHz */

        _puts("---- Running @ ");
        _utoa(val64, buff, 10);
        _puts(buff);
        _puts(" MHz\n");

        _puts("---- Current exception level: ");
        MRS("CurrentEL", val32);
        val32 = (val32 & 0x0C) >> 2;

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
        debug_disable();
        irq_enable();
        fiq_disable();
        serror_enable();
        isb();

        _puts("---- IRQ: ");
        MRS("DAIF", val32);

        if (val32 & DAIF_IRQ) {
                _puts("Masked\n");
        } else {
                _puts("Unmasked\n");
        }

        _puts("---- FIQ: ");

        if (val32 & DAIF_FIQ) {
                _puts("Masked\n");
        } else {
                _puts("Unmasked\n");
        }

        _puts("---- Debug: ");

        if (val32 & DAIF_DEBUG) {
                _puts("Masked\n");
        } else {
                _puts("Unmasked\n");
        }

        _puts("---- SError: ");

        if (val32 & DAIF_SERROR) {
                _puts("Masked\n");
        } else {
                _puts("Unmasked\n");
        }

        /*TODO: Initialize page tables here */

        _puts("+----------------------------------------------------+\n");
        _puts("|  __    _______  __, ____________ _ __    ___  __,  |\n");
        _puts("| ( /   /(  /    (   (  /  (  /   ( /  )  /  ()(     |\n");
        _puts("|  / / /   /--    `.   /     /--   /--<  /   /  `.   |\n");
        _puts("| (_/_/  (/____/(___)_/    (/____//   \\_(___/ (___)  |\n");
        _puts("|                                                    |\n");
        _puts("| Everything is OK. Calling the kernel now...        |\n");
        _puts("+----------------------------------------------------+\n");

        kmain();
}
