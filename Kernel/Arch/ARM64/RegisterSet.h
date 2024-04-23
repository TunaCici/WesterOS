/*
 * Generic register set for AArch64
 *
 * Reference: seL4/include/arch/arm/arch/64/mode/machine/registerset.h
 * Author: Tuna CICI
 */

#pragma once

/* CurrentEL register */
#define PEXPL1                  (1ULL << 2)
#define PEXPL2                  (1ULL << 3)

/* PSTATE register */
#define PMODE_FIQ               (1ULL << 6)
#define PMODE_IRQ               (1ULL << 7)
#define PMODE_SERROR            (1ULL << 8)
#define PMODE_DEBUG             (1ULL << 9)
#define PMODE_EL0t              0ULL
#define PMODE_EL1t              4ULL
#define PMODE_EL1h              5ULL

/* DAIF register */
#define DAIF_FIQ                (1ULL << 6)
#define DAIF_IRQ                (1ULL << 7)
#define DAIF_SERROR             (1ULL << 8)
#define DAIF_DEBUG              (1ULL << 9)
#define DAIFSET_MASK            0xFULL

/* SCTLR register */
#define SCTLR_M                 (1ULL << 0)
