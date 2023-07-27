/*
 * Console printing/logging functionalities for the Kernel
 *
 * Author: Tuna CICI
 */

#include <stdint.h>
#include <stdarg.h>

#include "MemoryLayout.h"
#include "LibKern/Console.h"
#include "LibKern/Time.h"

static const char digits[] = "0123456789ABCDEF";

static void uart_putc(const char c)
{
        volatile uint32_t *uart = (uint32_t*) PL011_BASE;
        *uart = c; /* TODO: This feels wrong... */
}


/* Timestamp */
/* Ex. [  15.123000] */
/* TODO: What an awful code this turned out to be.. FML! */
static void timestamp(void)
{
        uint64_t uptime = 0;
        uint32_t micro = 0;
        uint16_t sec = 0;

        /* Formatting */
        static const uint8_t precision_sec = 4u;
        static const uint8_t precision_micro = 6u;

        uint8_t idx = 0;
        char buffer[15] = {0};

        uptime = arm64_uptime(); /* This is nanoseconds */
        micro = uptime / NANO_PER_MICRO;
        sec = micro / MICRO_PER_SEC;

        buffer[idx++] = '\0';
        buffer[idx++] = ' ';
        buffer[idx++] = ']';

        for (uint8_t i = 0; i < precision_micro; i++) {
                if (micro % 10 == 0) {
                        buffer[idx++] = '0';
                } else {
                        buffer[idx++] = digits[micro % 10];
                }
                micro /= 10;
        }

        buffer[idx++] = '.';

        for (uint8_t i = 0; i < precision_sec; i++) {
                if (sec % 10 == 0) {
                        buffer[idx++] = ' ';
                } else {
                        buffer[idx++] = digits[sec % 10];
                }
                sec /= 10;
        }

        buffer[idx] = '[';


        for (; 0 < idx; idx--) {
                uart_putc(buffer[idx]);
        }
}

void kprint_uint(uint64_t uval, uint8_t base)
{
        char buffer[64] = {0};
        int i = 0;

        do {
                buffer[i++] = digits[uval % base];
        } while((uval /= base) != 0);

        for (; 0 <= i; i--) {
                uart_putc(buffer[i]);
        }
}

void kprint_int(int64_t val, uint8_t base)
{
        char buffer[64] = {0};
        int i = 0;

        do {
                buffer[i++] = digits[val % base];
        } while((val /= base) != 0);

        if (val < 0) {
                buffer[i++] = '-';
        }

        for (; 0 < i; i--) {
                uart_putc(buffer[i]);
        }
}

void kprint_str(const char *str)
{
        if (str == 0) {
                uart_putc('(');
                uart_putc('n');
                uart_putc('u');
                uart_putc('l');
                uart_putc('l');
                uart_putc(')');
                uart_putc('\0');

                return;
        }

        for (uint16_t i = 0; str[i] != '\0'; i++) {
                uart_putc(str[i]);
        }
}

void __attribute__((format(printf, 1, 2))) kprintf(const char *fmt, ...) 
{
        va_list args;
        va_start(args, fmt);

        if (fmt == 0) {
                return;
        }

        for (uint16_t i = 0; fmt[i] != '\0'; i++) {
                char c = fmt[i];

                if (c != '%') {
                        uart_putc(c);
                        continue;
                }

                c = fmt[++i];

                switch (c) {
                case 'u':
                        kprint_uint(*va_arg(args, uint64_t*), 10);
                        break;
                case 'd':
                        kprint_int(*va_arg(args, int64_t*), 10);
                        break;
                case 'x':
                case 'p':
                        kprint_uint(*va_arg(args, uint64_t*), 16);
                        break;
                case 's':
                       kprint_str(va_arg(args, const char*));
                       break;
                default:
                       uart_putc('%');
                       uart_putc(c);
                       break;
                }
        }

        va_end(args);
}

void __attribute__((format(printf, 1, 2))) klog(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);

        if (fmt == 0) {
                return;
        }

        timestamp();


        for (uint16_t i = 0; fmt[i] != '\0'; i++) {
                char c = fmt[i];

                if (c != '%') {
                        uart_putc(c);
                        continue;
                }

                c = fmt[++i];

                switch (c) {
                case 'u':
                        kprint_uint(*va_arg(args, uint64_t*), 10);
                        break;
                case 'd':
                        kprint_int(*va_arg(args, int64_t*), 10);
                        break;
                case 'x':
                case 'p':
                        kprint_uint(*va_arg(args, uint64_t*), 16);
                        break;
                case 's':
                       kprint_str(va_arg(args, const char*));
                       break;
                default:
                       uart_putc('%');
                       uart_putc(c);
                       break;
                }
        }

        va_end(args);
}
