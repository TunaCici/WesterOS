/*
*
* QEMU ARM Virtual Machine Definitions
*
* Original Source: https://www.qemu.org/docs/master/system/arm/virt.html
*/

#ifndef __ARM_VIRT__
#define __ARM_VIRT__

/* Memory */
#define FLASH_START     0x00000000
#define RAM_START       0x40000000
#define RAM_SIZE        0x80000000
#define RAM_END         (RAM_START + RAM_SIZE)

/* Device Tree Blob (DTB) */
#define DTB_START       0x40000000

/* QEMU: fw_cfg */
#define FW_CFG_BASE     0x09020000

/* Device: ARM PrimeCell RTC (PL031) */
#define PL031_BASE      0x09010000

/* Device: ARM PrimeCell UART (PL011) */
#define PL011_BASE      0x09000000

/* Devixe: ARM PrimeCell GPIO (PL061) */
#define PL061_BASE      0x09030000

/* Device: Generic Interrupt Controller (GIC) */
#define GIC_PBASE       0x08000000

#endif /* __ARM_VIRT__ */
