/*
* Physical memory layout of QEMU ARM Virtual Machine
*
* Original src: https://www.qemu.org/docs/master/system/arm/virt.html
*/

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#define FLASH_START     0x00000000

/* QEMU: Bootrom (such as SeaBios or UEFI) */
#define BOOTROM_START   0x00000000
#define BOOTROM_SIZE    0x08000000      /* 128 MiB */
#define BOOTROM_END     (BOOTROM_START + BOOTROM_SIZE)

/* Generic Interrupt Controller (GIC) */
#define GIC_BASE        0x08000000
#define GIC_SIZE        0x00020000      /* 128 Kib */
#define GIC_END         (GIC_BASE + GIC_SIZE)

/* ARM PrimeCell UART (PL011) */
#define PL011_BASE      0x09000000
#define PL011_SIZE      0x00001000      /* 4 KiB */
#define PL011_END       (PL011_BASE + PL011_SIZE)

/* ARM PrimeCell RTC (PL031) */
#define PL031_BASE      0x09010000
#define PL031_SIZE      0x00001000      /* 4 KiB */
#define PL031_END       (PL031_BASE + PL031_SIZE)

/* ARM PrimeCell GPIO (PL061) */
#define PL061_BASE      0x09030000
#define PL061_SIZE      0x00001000      /* 4KiB */
#define PL061_END       (PL061_BASE + PL061_SIZE)

/* QEMU: fw_cfg */
#define FW_CFG_BASE     0x09020000
#define FW_CFG_SIZE     0x00000018      /* 24 Bytes */
#define FW_CFG_END      (FW_CFG_BASE + FW_CFG_SIZE)

/* Device Tree Blob (DTB) */
#define DTB_START       0x40000000

/* 'Usable' Memory */
#define RAM_START       0x40000000
#define RAM_SIZE        0x80000000 /* Hard-coded to 2 GiB */
#define RAM_END         (RAM_START + RAM_SIZE)

#endif /* MEMORY_LAYOUT_H */
