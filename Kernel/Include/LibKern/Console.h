/*
 * Console printing/logging functionalities for the kernel
 *
 * Author: Tuna CICI
 */

#pragma once

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdarg.h>

#ifdef DEBUG
        #define KLOG(...) klog(__VA_ARGS__)
        #define KPRINTF(...) kprintf(__VA_ARGS__)
#else
        #define KLOG(fmt, ...)
        #define KPRINTF(fmt, ...)
#endif

void kprint_uint(uint64_t uval, uint8_t base);
void kprint_int(int64_t val, uint8_t base);
void kprint_str(const char *str);

void kprintf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void klog(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#endif /* CONSOLE_H */
