/*
*	File: Entry.S
*	Originally from: sudharson14/xv6-OS-for-arm-v8
*	Modified by: Tuna CICI
*	
*	Entry point for the WesterOS kernel for ARMv8.
*	Initialize the Stack pointer, clear the BSS and jump to Start.c
*/

.text
.align 16
.global _start

_start:
	/* initialize stack pointers for svc modes */
	mov     x0, #1     // select SP_EL1 for stack pointer
	msr     spsel, x0
	isb

	/* Stack size of 'init_stktop' */
	adrp    x0, init_stktop
	mov     sp, x0

	/* clear the entry bss section, the svc stack, and kernel page table */
	ldr     x1, =edata_entry
	ldr     x2, =end_entry
	mov     x3, #0x00

	/* Paint a random register with a known value for DEBUG in QEMU */
	# mov   x3, #0xab 


1:
	cmp     x1, x2
	b.gt    2f
	str     x3, [x1]
	add     x1, x1, #0x08
	blt     1b
2:
	bl      start
	b .
