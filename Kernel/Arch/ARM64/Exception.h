/*
 *      File: Exception.h
 *      Author: Tuna CICI
 *
 *      ARM64 exception definitions
 */

#include <stdint.h>

/* Ref: AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1- */
#define ESR_EC_OFFSET 26 /* bits */
#define ESR_EC_SIZE 6 /* bits */

enum {
        EC_UNKNOWN = 0b000000,
        EC_WF_INSTRUCTION = 0b000001,
        EC_MCR_MRC_0b1111 = 0b000011,
        EC_MCRR_MRRC_0b1111 = 0b000100,
        EC_MCR_MRC_0b1110 = 0b000101,
        EC_LDC_STC = 0b000110,
        EC_SVE_SIMD_FP_ACCESS = 0b000111,
        EC_LD64B_ST64B = 0b001010,
        EC_MRRC_0b1110 = 0b001100,
        EC_BRANCH_TARGET = 0b001101,
        EC_ILLEGAL_EXECUTION = 0b001110,
        EC_HVC_SVC_AARCH32 = 0b010001,
        EC_HVC_SVC_AARCH64 = 0b010101,
        EC_MSR_MRS_SYSTEM_AARCH64 = 0b011000,
        EC_SVE_ACCESS = 0b011001,
        EC_POINTER_AUTH_FAIL = 0b011100,
        EC_INSTRUCTION_ABORT = 0b100000,
        EC_INSTRUCTION_ABORT_UNCHANGED = 0b100001,
        EC_PC_ALIGNMENT_FAULT = 0b100010,
        EC_DATA_ABORT = 0b100100,
        EC_DATA_ABORT_UNCHANGED = 0b100101,
        EC_SP_ALIGNMENT_FAULT = 0b100110,
        EC_MEMORY_OPERATION = 0b100111,
        EC_TRAPPED_FP_AARCH32 = 0b101000,
        EC_TRAPPED_FP_AARCH64 = 0b101100,
        EC_SERROR = 0b101111,
        EC_BREAKPOINT_LOWER = 0b110000,
        EC_BREAKPOINT_UNCHANGED = 0b110001,
        EC_SOFTWARE_STEP_LOWER = 0b110010,
        EC_SOFTWARE_STEP_UNCHANGED = 0b110011,
        EC_WATCHPOINT_LOWER = 0b110100,
        EC_WATCHPOINT_UNCHANGED = 0b110101,
        EC_BKPT_AARCH32 = 0b111000,
        EC_BRK_AARCH64 = 0b111100
};

typedef struct exception_frame {
        uint64_t x0;
        uint64_t x1;
        uint64_t x2;
        uint64_t x3;
        uint64_t x4;
        uint64_t x5;
        uint64_t x6;
        uint64_t x7;
        uint64_t x8;
        uint64_t x9;
        uint64_t x10;
        uint64_t x11;
        uint64_t x12;
        uint64_t x13;
        uint64_t x14;
        uint64_t x15;
        uint64_t x16;
        uint64_t x17;
        uint64_t x18;
        uint64_t x19;
        uint64_t x20;
        uint64_t x21;
        uint64_t x22;
        uint64_t x23;
        uint64_t x24;
        uint64_t x25;
        uint64_t x26;
        uint64_t x27;
        uint64_t x28;
        uint64_t x29;
        uint64_t x30;
        uint64_t esr;
        uint64_t far;
        uint64_t elr;
} exception_frame;

void handle_spx_syn(exception_frame *frame);
void handle_spx_irq(exception_frame *frame);
void handle_spx_fiq(exception_frame *frame);
void handle_spx_ser(exception_frame *frame);
