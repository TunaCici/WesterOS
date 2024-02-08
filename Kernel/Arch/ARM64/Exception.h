/*
 *      File: Exception.h
 *      Author: Tuna CICI
 *
 *      ARM64 exception definitions
 */

#include <stdint-gcc.h>
#include <stdint.h>

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
