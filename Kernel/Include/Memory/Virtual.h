/*
 * Virtual memory manager for the ARMv8-A architecture
 *
 * References:
 * https://developer.arm.com/documentation/den0024/a/The-Memory-Management-Unit
 * https://github.com/ARM-software/u-boot/blob/master/arch/arm/include/asm/armv8/mmu.h
 * https://opensource.apple.com/source/xnu/xnu-6153.61.1/osfmk/arm64/proc_reg.h.auto.html
 * https://lowenware.com/blog/aarch64-mmu-programming/
 *
 * Author: Tuna CICI
 */

#ifndef VIRTUAL_H
#define VIRTUAL_H

#include <stdint.h>

#include "ARM64/Memory.h"

#include "Memory/PageDef.h"

void init_kernel_pgtbl(void);
void init_tcr(void);
void init_mair(void);

#endif /* VIRTUAL_H */