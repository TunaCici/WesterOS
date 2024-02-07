/*
*	File: Vector.S
*	Author: Tuna CICI
*	
*	AArch64 exception table code.
*/

#include "ARM64/Interrupt.h"

.text
.balign 2048
.global vector_table

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
        b _curr_el_spx_syn
.balign 0x80
        b _curr_el_spx_irq
.balign 0x80
        b _curr_el_spx_fiq
.balign 0x80
        b _curr_el_spx_see
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
        b .

.balign 0x04
_curr_el_spx_fiq:
        b .

.balign 0x04
_curr_el_spx_irq:
        b .

.balign 0x04
_curr_el_spx_see:
        b .
