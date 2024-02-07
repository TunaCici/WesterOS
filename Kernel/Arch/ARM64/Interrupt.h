/*
 *      File: Interrupt.h
 *      Author: Tuna CICI
 *
 *      ARM64 interrupt/exception definitions.
 */

#include <stdint.h>

typedef struct interrupt_frame {
        uint64_t x0;
} interrupt_frame;

void do_irq(interrupt_frame *frame);
void do_fiq(interrupt_frame *frame);
