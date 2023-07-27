/*
 * Console printing/logging functionalities for the Kernel
 *
 * Author: Tuna CICI
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdarg.h>

void kprint_uint(uint64_t uval, uint8_t base);
void kprint_int(int64_t val, uint8_t base);
void kprint_str(const char *str);

void kprintf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void klog(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#endif /* CONSOLE_H */
