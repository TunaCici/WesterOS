/*
 * Early boot stage. Sanity checks for hardware devices & "calls" kmain(...)
 *
 * Author: Tuna CICI
 */

#include <stdint.h>
#include <stdarg.h>

#include "ARM64/Machine.h"
#include "ARM64/RegisterSet.h"

#include "MemoryLayout.h"

#include "LibKern/DeviceTree.h"
#include "LibKern/Time.h"
#include "LibKern/Console.h"

extern void kmain(void);

void _halt(const char *s)
{
        klog("Halting due to: %s", s);
        wfi();
}

void start(void)
{
        volatile uint32_t arch;
        volatile uint32_t val32;
        volatile uint64_t val64;

        /* Hard-coded device/board info */
        /* TODO: Replace this with a DTB parser */
        const char      *_cpuModel  = "Cortex A-72";
        const uint32_t  _coreCount =  2u;

        klog("WesterOS early boot stage\n");
        klog("Running sanity checks...\n");

        /* TODO: Any better way to early print? */
        klog("WARN: Raw printing directly to PL011 @ 0x%x\n", PL011_BASE);

        /* -------- CPU -------- */
        klog("Checking CPU\n");
        MRS("MIDR_EL1", val32);
        arch = (val32 & 0xFF000000) >> 24;

        if (arch == 0x41) {
                klog("---- Implementer: ARM\n");
        } else {
                _halt("---- Unknown Implementer\n");
        }

        klog("---- Model: %s\n", _cpuModel);
        klog("---- SMP: %u\n", _coreCount);

        MRS("CNTFRQ_EL0", val64);
        val64 = val64 / 1000000; /* Hz to MHz */

        klog("---- Running @ %lu MHz\n", val64);

        klog("---- Current exception level: ");
        MRS("CurrentEL", val32);
        val32 = (val32 & 0x0C) >> 2;

        switch(val32) {
                case 0:
                        kprintf("EL0 (User mode)\n");
                break;
                case 1:
                        kprintf("EL1 (Kernel mode)\n");
                break;
                case 2:
                        kprintf("EL2 (Hypervisor mode)\n");
                break;
                case 3:
                        kprintf("EL3 (Secure Monitor mode)\n");
                break;
                default:
                        _halt("Unknown exception level\n");
                break;
        }

        /* Check if interrupts are enabled */
        debug_disable();
        irq_enable();
        fiq_disable();
        serror_enable();
        isb();

        klog("---- IRQ: ");
        MRS("DAIF", val32);

        if (val32 & DAIF_IRQ) {
                kprintf("Masked\n");
        } else {
                kprintf("Unmasked\n");
        }

        klog("---- FIQ: ");

        if (val32 & DAIF_FIQ) {
                kprintf("Masked\n");
        } else {
                kprintf("Unmasked\n");
        }

        klog("---- Debug: ");

        if (val32 & DAIF_DEBUG) {
                kprintf("Masked\n");
        } else {
                kprintf("Unmasked\n");
        }

        klog("---- SError: ");

        if (val32 & DAIF_SERROR) {
                kprintf("Masked\n");
        } else {
                kprintf("Unmasked\n");
        }

        /* -------- Machine Layout -------- */
        klog("QEMU ARM Virt Machine memory layout:\n");

        klog("---- BootROM Code: 0x%x - 0x%x (reserved)\n",
                BOOTROM_START, BOOTROM_END
        );

        klog ("---- GICv2: 0x%x - 0x%x (controller)\n",
                GIC_BASE, GIC_END
        );

        klog ("---- PL011 UART: 0x%x - 0x%x (mmio)\n",
                PL011_BASE, PL011_END
        );

        klog ("---- PL031  RTC: 0x%x - 0x%x (mmio)\n",
                PL031_BASE, PL031_END
        );

        klog ("---- PL061 GPIO: 0x%x - 0x%x (mmio)\n", 
                PL061_BASE, PL061_END
        );

        klog ("---- QEMU fw_cfg: 0x%x - 0x%x (qemu)\n",
                FW_CFG_BASE, FW_CFG_END
        );

        klog ("---- Device Tree Blob: 0x%x - 0x%x (reserved)\n",
                DTB_START, DTB_END
        );


        klog("Everything's OK. Calling the Kernel now...\n");
        kmain();
}
