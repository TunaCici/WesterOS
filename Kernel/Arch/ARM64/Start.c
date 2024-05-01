/*
 * Entry point for the WesterOS kernel for ARM64 cntd.
 * Initialize CPU cores, setup the iniital page tables & enable MMU
 * Finally, pass information & control to the kernel
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#include "ARM64/Machine.h"
#include "ARM64/RegisterSet.h"
#include "ARM64/Memory.h"

#include "Boot.h"
#include "MemoryLayout.h"

/* in Main.c */
extern void kmain(boot_sysinfo, uint64_t);

/* in Kernel/kernel.ld */
extern uint64_t _k_phy_base;
extern uint64_t _k_vir_base;
extern uint64_t _k_size;
extern uint64_t _k_stack_top;
uint64_t k_phy_base = (uint64_t) (&_k_phy_base);
uint64_t k_vir_base = (uint64_t) (&_k_vir_base);
uint64_t k_size = (uint64_t) (&_k_size);
uint64_t k_stack_top = (uint64_t) (&_k_stack_top);

/* in Kernel/Arch/ARM64/Vector.S */
extern uint64_t _vector_table; 
uint64_t vector_table = (uint64_t) (&_vector_table);

/* Higher half - TTBR1_EL1 */
uint64_t k_l0_pgtbl[ENTRY_SIZE] __attribute__((aligned(GRANULE_SIZE)));
uint64_t k_l1_pgtbl[ENTRY_SIZE] __attribute__((aligned(GRANULE_SIZE)));

/* Lower half - TTBR0_EL1 */
uint64_t u_l0_pgtbl[ENTRY_SIZE] __attribute__((aligned(GRANULE_SIZE)));
uint64_t u_l1_pgtbl[ENTRY_SIZE] __attribute__((aligned(GRANULE_SIZE)));

void _utoa(uint64_t uval, char *buff, uint8_t base)
{
        const char digits[] = "0123456789ABCDEF";
        int i = 0;

        /* Convert to string. Digits are in reverse order */
        do {
                buff[i++] = digits[uval % base];
        } while((uval /= base) != 0);
        buff[i--] = '\0';

        /* Reverse the string */
        for (int j = 0; j < i; j++, i--) {
                char tmp = buff[j];

                buff[j] = buff[i];
                buff[i] = tmp;
        }
}

void _puts(const char *s)
{
        volatile uint8_t *uart0 = (uint8_t*) PL011_BASE;

        while (*s != '\0') {
                *uart0 = *s;
                s++;
        }
}

/* TODO: Do this properly */
void _init_mair(void)
{
        uint64_t mair_el1 = 0;

        /* Refer to Kernel/Arch/ARM64/Memory.h */
        mair_el1 |= (DEVICE_nGnRnE_MAIR << DEVICE_nGnRnE_IDX * 8);
        mair_el1 |= (DEVICE_nGnRE_MAIR << DEVICE_nGnRE_IDX * 8);
        mair_el1 |= (DEVICE_GRE_MAIR << DEVICE_GRE_IDX * 8);
        mair_el1 |= (NORMAL_NC_MAIR << NORMAL_NC_IDX * 8);
        mair_el1 |= (NORMAL_MAIR << NORMAL_IDX * 8);
        mair_el1 |= (NORMAL_WT_MAIR << NORMAL_WT_IDX * 8);

        asm("MSR MAIR_EL1, %[v]": :[v]"r" (mair_el1):);
        asm("ISB": : :);
}

void _init_kernel_pgtbl(void)
{
        for (int i = 0; i < ENTRY_SIZE; i++) {
                k_l0_pgtbl[i] = 0x0ULL;
                k_l1_pgtbl[i] = 0x0ULL;
        }

        /* Level 1 */
        {
                uint64_t blk = 0;

                blk = ENTRY_VALID(blk);
                blk = ENTRY_BLOCK(blk);

                blk = BLK_SET_AIDX(blk, NORMAL_IDX);
                blk = BLK_SET_NS(blk, 0);
                blk = BLK_SET_AP(blk, AP_PRIV_RW);
                blk = BLK_SET_SH(blk, SH_OUTER);
                blk = BLK_SET_AF(blk, 1);
                blk = BLK_SET_NG(blk, 0);

                blk = BLK_SET_L1_OA(blk, k_phy_base);

                blk = BLK_SET_HINT(blk, 0);
                blk = BLK_SET_PXN(blk, 0);
                blk = BLK_SET_XN(blk, 0);

                k_l1_pgtbl[1] = blk;
        }

        /* Level 0 */
        {
                uint64_t tbl = 0;

                tbl = ENTRY_VALID(tbl);
                tbl = ENTRY_TABLE(tbl);

                tbl = TBL_SET_NEXT(tbl, ((uint64_t) k_l1_pgtbl));

                tbl = TBL_SET_PXN(tbl, 0);
                tbl = TBL_SET_XN(tbl, 0);
                tbl = TBL_SET_AP(tbl, 0);
                tbl = TBL_SET_NS(tbl, 0);

                k_l0_pgtbl[0] = tbl;
        }

        MSR("TTBR1_EL1", ((uint64_t) k_l0_pgtbl));
        isb();
}


void _init_user_pgtbl(void)
{
        for (int i = 0; i < ENTRY_SIZE; i++) {
                u_l0_pgtbl[i] = 0x0ULL;
                u_l1_pgtbl[i] = 0x0ULL;
        }

        /* Level 1 */
        {
                uint64_t blk = 0;

                blk = ENTRY_VALID(blk);
                blk = ENTRY_BLOCK(blk);

                blk = BLK_SET_AIDX(blk, NORMAL_IDX);
                blk = BLK_SET_NS(blk, 0);
                blk = BLK_SET_AP(blk, AP_PRIV_RW);
                blk = BLK_SET_SH(blk, SH_OUTER);
                blk = BLK_SET_AF(blk, 1);
                blk = BLK_SET_NG(blk, 0);

                blk = BLK_SET_L1_OA(blk, 0x40000000);

                blk = BLK_SET_HINT(blk, 0);
                blk = BLK_SET_PXN(blk, 0);
                blk = BLK_SET_XN(blk, 0);

                u_l1_pgtbl[1] = blk;
        }

        /* Level 1 - Dev */
        {
                uint64_t blk = 0;

                blk = ENTRY_VALID(blk);
                blk = ENTRY_BLOCK(blk);

                blk = BLK_SET_AIDX(blk, DEVICE_nGnRE_IDX);
                blk = BLK_SET_NS(blk, 0);
                blk = BLK_SET_AP(blk, AP_PRIV_RW);
                blk = BLK_SET_SH(blk, SH_NON);
                blk = BLK_SET_AF(blk, 1);
                blk = BLK_SET_NG(blk, 0);

                blk = BLK_SET_L1_OA(blk, 0x000000000);

                blk = BLK_SET_HINT(blk, 0);
                blk = BLK_SET_PXN(blk, 0);
                blk = BLK_SET_XN(blk, 0);

                u_l1_pgtbl[0] = blk;
        }

        /* Level 0 */
        {
                uint64_t tbl = 0;

                tbl = ENTRY_VALID(tbl);
                tbl = ENTRY_TABLE(tbl);

                tbl = TBL_SET_NEXT(tbl, ((uint64_t) u_l1_pgtbl));

                tbl = TBL_SET_PXN(tbl, 0);
                tbl = TBL_SET_XN(tbl, 0);
                tbl = TBL_SET_AP(tbl, 0);
                tbl = TBL_SET_NS(tbl, 0);

                u_l0_pgtbl[0] = tbl;
        }

        MSR("TTBR0_EL1", ((uint64_t) u_l0_pgtbl));
        isb();
}

void _init_tcr(void)
{
        uint64_t tcr_el1 = 0;
        uint64_t reg = 0;

        /* DS: max. output addr (OA) and virtual addr (VA) size set to 48-bit */
        tcr_el1 &= TCR_DS_48BITS;

        /* IPS: effective output addr (OA) set to ID_AA64MMFR0_EL1.PARange */
        MRS("ID_AA64MMFR0_EL1", reg);
        tcr_el1 &= TCR_IPS_CLEAR;
        tcr_el1 |= (GET_PARange(reg) << TCR_IPS_SHIFT);

        /* T1SZ: input address (IA) size offset of mem region for TTBR1_EL1 */
        /* T0SZ: input address (IA) size offset of mem region for TTBR0_EL1 */
        tcr_el1 |= TCR_T1SZ << TCR_T1SZ_SHIFT;
        tcr_el1 |= TCR_T0SZ << TCR_T0SZ_SHIFT;

        /* HPDN1: enable hierarchical permissions for TTBR1_EL1 */
        /* HPDN0: enable hierarchical permissions for TTBR0_EL1 */
        tcr_el1 &= TCR_HPDN1_ENABLE;
        tcr_el1 &= TCR_HPDN0_ENABLE;

        /* A1: TTBR0 decides the ASID value (?) */
        tcr_el1 &= TCR_A1_TTBR0;

        /* EPD1: perform table walk on TTBR1 after TLB miss */
        /* EPD0: perform table walk on TTBR0 after TLB miss */
        tcr_el1 &= TCR_EPD1_DISABLE;
        tcr_el1 &= TCR_EPD0_DISABLE;

        /* TG1: granule size for TTBR1 region */
        /* TG0: granule size for TTBR0 region */
        tcr_el1 &= TCR_TG1_GRANULE_CLEAR;
        tcr_el1 &= TCR_TG0_GRANULE_CLEAR;

#if GRANULE_SIZE == 4096
        tcr_el1 |= TCR_TG1_GRANULE_4KB;
        tcr_el1 |= TCR_TG0_GRANULE_4KB;
#elif GRANULE_SIZE == 16392
        tcr_el1 |= TCR_TG1_GRANULE_16KB;
        tcr_el1 |= TCR_TG0_GRANULE_16KB;
#elif GRANULE_SIZE == 65568
        tcr_el1 |= TCR_TG1_GRANULE_64KB;
        tcr_el1 |= TCR_TG0_GRANULE_64KB;
#else
        return; // we fucked up
#endif

        /* Save TCR_EL1 */
        MSR("TCR_EL1", tcr_el1);
        isb();
}

void _init_sctlr(void)
{
        uint64_t sctlr_el1 = 0;

        /* Read SCTLR_EL1 */
        MRS("SCTLR_EL1", sctlr_el1);
        isb();

        /* Enable MMU */
        sctlr_el1 |= SCTLR_M;

        /* Save SCTLR_EL1 */
        MSR("SCTLR_EL1", sctlr_el1);
        asm("ISB": : :);
}

void start(void)
{
        uint32_t arch = 0;
        uint32_t val32 = 0;
        uint64_t val64 = 0;
        char buff[64] = {0};

        boot_sysinfo boot_params = {0};

        /* Hard-coded device/board info */
        /* TODO: Replace this with a DTB parser */
        const char      *_cpuModel  = "Cortex A-72";
        const uint32_t  _coreCount =  2u;

        _puts("Early boot stage\n");
        _puts("Running sanity checks\n");

        /* TODO: Any better way to early print? */
        _puts("WARN: Raw printing directly to PL011 @ 0x");
        _utoa(PL011_BASE, buff, 16);
        _puts(buff);
        _puts("\n");

        /* -------- CPU -------- */
        _puts("Checking CPU\n");
        MRS("MIDR_EL1", val32);
        arch = (val32 & 0xFF000000) >> 24;

        if (arch == 0x41) {
                _puts("---- Implementer: ARM\n");
        } else {
                _puts("---- Unknown Implementer\n");
                return;
        }

        _puts("---- Model: ");
        _puts(_cpuModel);
        _puts("\n");

        _puts("---- SMP: ");
        _utoa(_coreCount, buff, 10);
        _puts(buff);
        _puts("\n");

        MRS("CNTFRQ_EL0", val64);
        val64 = val64 / 1000000; /* Hz to MHz */

        _puts("---- Running @ ");
        _utoa(val64, buff, 10);
        _puts(buff);
        _puts(" MHz\n");

        _puts("---- Current exception level: ");
        MRS("CurrentEL", val32);
        val32 = (val32 & 0x0C) >> 2;

        switch(val32) {
                case 0:
                        _puts("EL0 (User mode)\n");
                break;
                case 1:
                        _puts("EL1 (Kernel mode)\n");
                break;
                case 2:
                        _puts("EL2 (Hypervisor mode)\n");
                break;
                case 3:
                        _puts("EL3 (Secure Monitor mode)\n");
                break;
                default:
                        _puts("Unknown exception level\n");
                        return;
                break;
        }

        /* Populate boot_sysinfo - will be passed to the kernel */
        boot_params.k_phy_base = k_phy_base;
        boot_params.k_vir_base = k_vir_base;
        boot_params.k_size = k_size;

        boot_params.vector_base = vector_table;

        boot_params.dtb_base = DTB_START;
        boot_params.dtb_size = DTB_SIZE;

        /* Disable interrupts */
        debug_disable();
        irq_enable();
        fiq_disable();
        serror_enable();
        isb();

        _puts("---- IRQ: ");
        MRS("DAIF", val32);

        if (val32 & DAIF_IRQ) {
                _puts("Masked\n");
        } else {
                _puts("Unmasked\n");
        }

        _puts("---- FIQ: ");

        if (val32 & DAIF_FIQ) {
                _puts("Masked\n");
        } else {
                _puts("Unmasked\n");
        }

        _puts("---- Debug: ");

        if (val32 & DAIF_DEBUG) {
                _puts("Masked\n");
        } else {
                _puts("Unmasked\n");
        }

        _puts("---- SError: ");

        if (val32 & DAIF_SERROR) {
                _puts("Masked\n");
        } else {
                _puts("Unmasked\n");
        }

        MSR("VBAR_EL1", vector_table);
        isb();

        _puts("Initializing Translation Control Register (TCR)\n");
        _init_tcr();

        _puts("Initializing Memory Attribute Indirection Register (MAIR_EL1)\n");
        _init_mair();

        _puts("Initializing kernel page tables (TTBR1_EL1)\n");
        _init_kernel_pgtbl();

        _puts("Initializing user page tables (TTBR0_EL1)\n");
        _init_user_pgtbl();

        _puts("Flushing instruction & TLB caches\n");
        ic_ialluis();
        tlbi_vmalle1();
        dsb_ish();

        _puts("Initializing System Control Register (SCTLR_EL1)\n");
        _init_sctlr();

        _puts("+----------------------------------------------------+\n");
        _puts("|  __    _______  __, ____________ _ __    ___  __,  |\n");
        _puts("| ( /   /(  /    (   (  /  (  /   ( /  )  /  ()(     |\n");
        _puts("|  / / /   /--    `.   /     /--   /--<  /   /  `.   |\n");
        _puts("| (_/_/  (/____/(___)_/    (/____//   \\_(___/ (___)  |\n");
        _puts("|                                                    |\n");
        _puts("| Everything is OK. Calling the kernel now...        |\n");
        _puts("+----------------------------------------------------+\n");

        /* Jump the stack pointer the stack pointer */
        asm volatile ("mov sp, %0" :: "r" (k_stack_top));
        
        kmain(boot_params, 0);
}
