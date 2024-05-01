/*
 * WesterOS shim + kernel linker script
 *	
 * Author: Tuna CICI
 */

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)

SHIM_STACK_SIZE = 0x2000; /* 8 KiB */
KERNEL_STACK_SIZE = 0x10000; /* 64 KiB */

ARM64_TTBR1_BASE = 0xFFFF000000000000; /* TTBR1_EL1 lower boundary (48-bit) */

SECTIONS
{
        /*
         * The entry point, before enabling paging
         * The code to enable paging needs to have the same VA & PA
         * Only Entry.S and Start.c run in this initial setting
         */
        
        /* QEMU loads DTB at 0x00000000_40000000. So we skip over it (1MiB) */
        . = 0x40100000;

        .shim :
        {
                PROVIDE(_shim_start = .);
                
                Build/Entry.o(.text)
                Build/Start.o(.text .text.*)

                Build/Entry.o(.rodata .rodata.*)
                Build/Start.o(.rodata .rodata.*)

                Build/Entry.o(.data .data.*)
                Build/Start.o(.data .data.*)

                PROVIDE(_shim_bss_start = .);

                Build/Entry.o(.bss .bss.* COMMON)
                Build/Start.o(.bss .bss.* COMMON)

                /* Reserve space for Shim stack */
                . = ALIGN(0x1000);
                . += SHIM_STACK_SIZE;
                PROVIDE (_shim_stack_top = .);

                . = ALIGN(0x1000);
                PROVIDE(_shim_end = .);
        }

        /*
         * Kernel has two adresses:
         * Load Memory Address (LMA): located on top of Shim
         * Virtual Memory Address (VMA): located at 'ARM64_TTBR1_BASE + Shim'
         *
         * https://sourceware.org/binutils/docs/ld/Basic-Script-Concepts.html
         */
        
        PROVIDE (_k_phy_base = .);

        /* Reserve space for the kernel stack */
        . = ALIGN(0x1000);
        . += KERNEL_STACK_SIZE;

        PROVIDE(_k_code_base = .);

        /* Switch to VMA */
        . = ARM64_TTBR1_BASE + (ADDR(.shim) + SIZEOF(.shim));
        PROVIDE (_k_vir_base = .);

        /* Skip over the kernel stack */
        . = ALIGN(0x1000);
        . += KERNEL_STACK_SIZE;
        PROVIDE(_k_stack_top = .);

        .text : AT(_k_code_base)
        {
                _code = .;
                *(.text .text.* .gnu.linkonce.t.*)
                *(.rodata*)
                . = ALIGN(0x1000);
        }
        
        .data : AT(LOADADDR(.text) + SIZEOF(.text))
        {
                _data = .;
                *(.data .data.*)
                . = ALIGN(0x1000);
        }

        .eh_frame : AT(LOADADDR(.data) + SIZEOF(.data))
        {
                _ehframe = .;
                *(.eh_frame)
                . = ALIGN(0x1000);
        }

        .bss : AT(LOADADDR(.eh_frame) + SIZEOF(.eh_frame))
        {
                _bss = .;
                *(.bss .bss.* COMMON)
                . = ALIGN(0x1000);
        }
        
        PROVIDE (_k_size = (. - _k_vir_base));
}
