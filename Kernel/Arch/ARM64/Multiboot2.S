/*
*       File: Multiboot2.S
*       Originally from: intermezzOS
*       Modified by: Tuna CICI
*
*       Defines the GNU Multiboot2 header for ARMv8
*/

.section .multiboot_header
.align 8
header_start:
        .word 0xe85250d6                // magic number
        .word 0                         // protected mode code
        .word header_end - header_start // header length

        /* checksum */
        .word 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

        /* tags */
        .hword 0    // type
        .hword 0   // flags
        .word 8    // size
header_end:
