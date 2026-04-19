/*
 * Entry point for the WesterOS kernel for ARM64
 * Initializes stack pointer, clears the BSS and jump to Start.c
 *
 * Originally from: sudharson14/xv6-OS-for-arm-v8
 * Modified by: Tuna CICI	
 */

.text
.align 16
.global _start

_start:
	/* Select SP_EL1 for stack pointer - am i sure? */
	mov     x0, #1
	msr     spsel, x0
	isb

	/* Init stack (`_shim_stack_top` bytes) */
	adrp    x0, _shim_stack_top
	mov     sp, x0

	/* Clear the shim bss section & the svc stack */
	ldr     x1, =_shim_bss_start
	ldr     x2, =_shim_end
	mov     x3, #0x00

	/* Enable access to NEON & FP */
	// No trap to all NEON & FP instructions
	mrs	x0, CPACR_EL1
	orr	x0, x0, #(0b11 << 20)
	msr	CPACR_EL1, x0
	isb

1:
	/* Others sleep */
	cmp     x1, x2
	b.gt    2f
	str     x3, [x1]
	add     x1, x1, #0x08
	b.lt     1b
2:
	/* One core works */
	bl      start
	b .
