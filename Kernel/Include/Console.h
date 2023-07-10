/*
* Console printing/logging functionalities for the Kernel
*
* Original src: mit-pdos/xv6-public
* Modified by: Tuna CICI
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdarg.h>

#include "MemoryLayout.h"

static void uart_putc(const char c);

void log();
void kprintf(const char *fmt, ...);
void kprint_str(const char *str);
void kprint_int(uint64_t xx, uint16_t base, uint16_t sign);

#endif /* CONSOLE_H */