OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)

ENTRY_INIT_STACK_SIZE = 0x2000;

SECTIONS
{
        /* the entry point, before enabling paging. The code to enable paing
        needs to have the same virtual/physical address. Entry.S and Start.c
        run in this initial setting.*/
        
        /* Why? Cuz -> https://github.com/qemu/qemu/blob/master/hw/arm/virt.c */
        /* Also QEMU loads DTB at 0x4000_0000. So we move by 0x0001_0000 */
        . = 0x40100000;
        
        .start_sec : {
                Build/Entry.o(.text)
                Build/Start.o(.text .text.*)

                Build/Entry.o(.rodata .rodata.*)
                Build/Start.o(.rodata .rodata.*)

                Build/Entry.o(.data .data.*)
                Build/Start.o(.data .data.*)

                PROVIDE(edata_entry = .);

                Build/Entry.o(.bss .bss.* COMMON)
                Build/Start.o(.bss .bss.* COMMON)

                /*define a stack for the entry*/
                . = ALIGN(0x1000);
                . += ENTRY_INIT_STACK_SIZE;

                PROVIDE (init_stktop = .);

                /* define the kernel page table, must be 4K and 4K-aligned*/
                . = ALIGN(0x1000);
                PROVIDE (_kernel_pgtbl = .);
                . += 0x1000;

                /* we also need a user page table*/
                PROVIDE (_user_pgtbl = .);
                . += 0x1000;

                /* level 2 pages 4k * 4*/
                PROVIDE (_K_l2_pgtbl = .);
                . += 0x4000;
                PROVIDE (_U_l2_pgtbl = .);
                . += 0x4000;

                PROVIDE(end_entry = .);
        }

        /* the kernel executes at the higher address space, but loaded
        at the lower memory (0x30000) */

        .text : { /** here to make code also copied into phymem **/
                *(.text .text.* .gnu.linkonce.t.*)
        }

        PROVIDE(etext = .);	/* Define the 'etext' symbol to this value */

        .rodata : {
                *(.rodata .rodata.* .gnu.linkonce.r.*)
        }

        /* aligned the data to a (4K) page, so it can be assigned
        different protection than the code*/
        . = ALIGN(0x1000);

        PROVIDE (data_start = .);

        .data : {
                *(.data .data.*)
        }

        PROVIDE (edata = .);

        .bss : {
                *(.bss .bss.* COMMON)
        }

        . = ALIGN(0x1000);
        PROVIDE (end = .);
}
