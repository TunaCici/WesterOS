/*
 * Copyright 2020, Data61, CSIRO (ABN 41 687 119 230)
 *
 * Ref: seL4/include/arch/arm/armv/armv8-a/64/armv/machine.h
 *    : android/kernel/msm/.../arch/arm64/include/asm/irqflags.h 
 * Author: Tuna CICI
 */

#pragma once

#define GET_PARange(ID_AA64MMFR0_EL1) (((ID_AA64MMFR0_EL1) >> 0) & 0b1111)

static inline void wfi(void)
{
    asm volatile("wfi" ::: "memory");
}

static inline void dsb(void)
{
    asm volatile("dsb sy" ::: "memory");
}

static inline void dsb_ishst(void)
{
    asm volatile("dsb ishst" ::: "memory");
}

static inline void dmb(void)
{
    asm volatile("dmb sy" ::: "memory");
}

static inline void isb(void)
{
    asm volatile("isb sy" ::: "memory");
}

/*
 * Mask / unmask DAIF via DAIFSet / DAIFClr registers
 *
 * Ref: developer.arm.com/documentation/100933/0100/Processor-state-in-exception-handling
 *
 * D (0b1___): Debug 
 * A (0b_1__): SError
 * I (0b__1_): IRQ
 * F (0b___1): FIQ
 */
static inline void debug_enable(void)
{
	asm volatile("msr daifclr, #8" ::: "memory");
}

static inline void serror_enable(void)
{
	asm volatile("msr daifclr, #4" ::: "memory");
}

static inline void irq_enable(void)
{
	asm volatile("msr daifclr, #2" ::: "memory");
}

static inline void fiq_enable(void)
{
	asm volatile("msr daifclr, #1" ::: "memory");
}

static inline void debug_disable(void)
{
	asm volatile("msr daifset, #8" ::: "memory");
}

static inline void serror_disable(void)
{
	asm volatile("msr daifset, #4" ::: "memory");
}

static inline void irq_disable(void)
{
	asm volatile("msr daifset, #2" ::: "memory");
}

static inline void fiq_disable(void)
{
	asm volatile("msr daifset, #1" ::: "memory");
}

#define MRS(reg, v)  asm volatile("mrs %x0," reg : "=r"(v))
#define MSR(reg, v)                                \
    do {                                           \
        uint64_t _v = v;                             \
        asm volatile("msr " reg ",%x0" :: "r" (_v));\
    }while(0)

