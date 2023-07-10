/*
* Memory layout of QEMU ARM Virtual Machine
*
* Original src: https://www.qemu.org/docs/master/system/arm/virt.html
*/

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#define FLASH_START     0x00000000
#define RAM_START       0x40000000
#define RAM_SIZE        0x80000000
#define RAM_END         (RAM_START + RAM_SIZE)

#define DTB_START       0x40000000 /* Device Tree Blob (DTB) */

/* MMIO */
#define FW_CFG_BASE     0x09020000 /* QEMU: fw_cfg */
#define PL031_BASE      0x09010000 /* ARM PrimeCell RTC (PL031) */
#define PL011_BASE      0x09000000 /* ARM PrimeCell UART (PL011) */
#define PL061_BASE      0x09030000 /* ARM PrimeCell GPIO (PL061) */
#define GIC_PBASE       0x08000000 /* Generic Interrupt Controller (GIC) */

#endif /* MEMORY_LAYOUT_H */
