/*
*	File: Vector.S
*	Author: Tuna CICI
*	
*	AArch64 exception vector table
*/

#include "ARM64/Interrupt.h"

.text
.balign 2048
.global vector_table

.macro store_regs
        stp     x0,  x1,  [sp, #16 * 0]
        stp     x2,  x3,  [sp, #16 * 1]
        stp     x4,  x5,  [sp, #16 * 2]
        stp     x6,  x7,  [sp, #16 * 3]
        stp     x8,  x9,  [sp, #16 * 4]
        stp     x10, x11, [sp, #16 * 5]
        stp     x12, x13, [sp, #16 * 6]
        stp     x14, x15, [sp, #16 * 7]
        stp     x16, x17, [sp, #16 * 8]
        stp     x18, x19, [sp, #16 * 9]
        stp     x20, x21, [sp, #16 * 10]
        stp     x22, x23, [sp, #16 * 11]
        stp     x24, x25, [sp, #16 * 12]
        stp     x26, x27, [sp, #16 * 13]
        stp     x28, x29, [sp, #16 * 14]

        /* Exception Syndrome Register & x30 */
        mrs     x0, esr_el1
        stp     x30, x0, [sp, #16 * 15]

        /* Fault Address Register & Exception Link Register */
        mrs     x0, far_el1
        mrs     x1, elr_el1
        stp     x0, x1, [sp, #16 * 16]

.endm

.macro restore_regs
        /* Fault Address Register & Exception Link Register */
        ldp     x0, x1, [sp, #16 * 16]
        msr     far_el1, x0
        msr     elr_el1, x1

        /* Exception Syndrome Register & x30 */
        ldp     x0, xzr, [sp, #16 * 15]
        msr     esr_el1, x0

        /* General purpose */
        ldp     x0,  x1,  [sp, #16 * 0]
        ldp     x2,  x3,  [sp, #16 * 1]
        ldp     x4,  x5,  [sp, #16 * 2]
        ldp     x6,  x7,  [sp, #16 * 3]
        ldp     x8,  x9,  [sp, #16 * 4]
        ldp     x10, x11, [sp, #16 * 5]
        ldp     x12, x13, [sp, #16 * 6]
        ldp     x14, x15, [sp, #16 * 7]
        ldp     x16, x17, [sp, #16 * 8]
        ldp     x18, x19, [sp, #16 * 9]
        ldp     x20, x21, [sp, #16 * 10]
        ldp     x22, x23, [sp, #16 * 11]
        ldp     x24, x25, [sp, #16 * 12]
        ldp     x26, x27, [sp, #16 * 13]
        ldp     x28, x29, [sp, #16 * 14]
.endm

vector_table:

/*
 * 1. Current EL where SP -> SP_EL0 - TODO
 */
.balign 0x80
        b .
.balign 0x80
        b .
.balign 0x80
        b .
.balign 0x80
        b .
/*
 * 2. Current EL where SP -> SP_ELx
 */
.balign 0x80
        b       _curr_el_spx_syn
.balign 0x80
        b       _curr_el_spx_irq
.balign 0x80
        b       _curr_el_spx_fiq
.balign 0x80
        b       _curr_el_spx_ser
/*
 * 3. Lower EL (for AArch64) - TODO
 */
.balign 0x80
        b .
.balign 0x80
        b .
.balign 0x80
        b .
.balign 0x80
        b .
/*
 * 4. Lower EL (for AArch32) - TODO
 */
.balign 0x80
        b .
.balign 0x80
        b .
.balign 0x80
        b .
.balign 0x80
        b .

.balign 0x04
_curr_el_spx_syn:
        store_regs
        mov     x0, sp
        bl      handle_spx_syn
        restore_regs
        eret

.balign 0x04
_curr_el_spx_fiq:
        store_regs
        mov     x0, sp
        bl      handle_spx_fiq
        restore_regs
        eret

.balign 0x04
_curr_el_spx_irq:
        store_regs
        mov     x0, sp
        bl      handle_spx_irq
        restore_regs
        eret

.balign 0x04
_curr_el_spx_ser:
        store_regs
        mov     x0, sp
        bl      handle_spx_ser
        restore_regs
        eret
