/*
 * String & memory functions for the kernel (for ARM64 only)
 *
 * Reference: https://github.com/ARM-software/optimized-routines/
 *
 * Copyright (c) 2019-2023, Arm Limited.
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void *memcpy(void *__restrict, const void *__restrict, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
void *memchr(const void *, int, size_t);
void *memrchr(const void *, int, size_t);
int memcmp(const void *, const void *, size_t);

char *strcpy(char *__restrict, const char *__restrict);
int strcmp(const char *, const char *);
char *strchr(const char *, int);
char *strrchr(const char *, int);
size_t strlen(const char *);
size_t strnlen(const char *, size_t);
int strncmp(const char *, const char *, size_t);

#endif /* STRING_H */
