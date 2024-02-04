/*
 * Generic register set for AArch64
 *
 * Reference: seL4/include/arch/arm/arch/64/mode/machine/registerset.h
 * Author: Tuna CICI
 */

#pragma once

/* CurrentEL register */
#define PEXPL1                  (1 << 2)
#define PEXPL2                  (1 << 3)

/* PSTATE register */
#define PMODE_FIQ               (1 << 6)
#define PMODE_IRQ               (1 << 7)
#define PMODE_SERROR            (1 << 8)
#define PMODE_DEBUG             (1 << 9)
#define PMODE_EL0t              0
#define PMODE_EL1t              4
#define PMODE_EL1h              5

/* DAIF register */
#define DAIF_FIQ                (1 << 6)
#define DAIF_IRQ                (1 << 7)
#define DAIF_SERROR             (1 << 8)
#define DAIF_DEBUG              (1 << 9)
#define DAIFSET_MASK            0xF


