/*
 * WesterOS kernel linker script
 *	
 * Author: Tuna CICI
 */

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)

ENTRY_INIT_STACK_SIZE = 0x2000; /* 8 KiB */
KERNEL_VMA_BASE = 0xFFFF000000000000; /* TTBR1_EL1 lower boundary */

SECTIONS
{
        /*
         * The entry point, before enabling paging
         * The code to enable paging needs to have the same VA & PA
         * Only Entry.S and Start.c run in this initial setting
         */
        
        /* QEMU loads DTB at 0x00000000_40000000. So we skip over it (1MiB) */
        . = 0x40100000;

        .start_sec :
        {
                _start_sec = .;
                
                Build/Entry.o(.text)
                Build/Start.o(.text .text.*)

                Build/Entry.o(.rodata .rodata.*)
                Build/Start.o(.rodata .rodata.*)

                Build/Entry.o(.data .data.*)
                Build/Start.o(.data .data.*)

                PROVIDE(edata_entry = .);

                Build/Entry.o(.bss .bss.* COMMON)
                Build/Start.o(.bss .bss.* COMMON)

                /* Reserve stack space for kernel */
                . = ALIGN(0x1000);
                . += ENTRY_INIT_STACK_SIZE;

                PROVIDE (init_stktop = .);

                PROVIDE(end_entry = .);
                . = ALIGN(0x1000);
        }

        /*
         * Kernel has two adresses:
         * Load Memory Address (LMA): right after '.start_sec'
         * Virtual Memory Address (VMA): right after 'KERNEL_VMA_BASE'
         *
         * https://sourceware.org/binutils/docs/ld/Basic-Script-Concepts.html
         */
        
        /* . = KERNEL_VMA_BASE; */
        PROVIDE (kernel_vma_base = .);

        .text : AT(ADDR(.start_sec) + SIZEOF(.start_sec))
        {
                _code = .;
                *(.text .text.* .gnu.linkonce.t.*)
                *(.rodata*)
                . = ALIGN(0x1000);
        }

        .data : AT(ADDR(.text) + SIZEOF(.text))
        {
                _data = .;
                *(.data .data.*)
                . = ALIGN(0x1000);
        }

        .eh_frame : AT(ADDR(.data) + SIZEOF(.data))
        {
                _ehframe = .;
                *(.eh_frame)
                . = ALIGN(0x1000);
        }

        .bss : AT(ADDR(.eh_frame) + SIZEOF(.eh_frame))
        {
                _bss = .;
                *(.bss .bss.* COMMON)
                . = ALIGN(0x1000);
        }

        PROVIDE (kend = .);
}
