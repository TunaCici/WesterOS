/*
 * Entry point for the WesterOS kernel for ARMv8
 * Initialize the Stack pointer, clear the BSS and jump to Start.c
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

	/* Init stack (`init_stktop` bytes) */
	adrp    x0, init_stktop
	mov     sp, x0

	/* Clear the entry bss section, the svc stack, and kernel page table */
	ldr     x1, =edata_entry
	ldr     x2, =end_entry
	mov     x3, #0x00

	/* Enable access to FPU & Advanced SIMD (NEON) */
	mrs	x0, CPACR_EL1
	orr	x0, x0, #(0x3 << 20)
	msr	CPACR_EL1, x0
	isb
1:
	cmp     x1, x2
	b.gt    2f
	str     x3, [x1]
	add     x1, x1, #0x08
	b.lt     1b
2:
	bl      start
	b .
