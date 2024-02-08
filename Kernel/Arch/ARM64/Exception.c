/* 
 *      File: Exception.c
 *      Author: Tuna CICI
 *
 *      ARM64 exception handlers
 */

#include "ARM64/Exception.h"

#include "LibKern/Console.h"

extern uint64_t kstart;

void handle_spx_syn(exception_frame *frame)
{
        klog("[arm64/exception] Received Synchronous: 0x%x\n", frame->elr);
        asm volatile("wfi" ::: "memory");

        return;
}

void handle_spx_irq(exception_frame *frame)
{
        klog("[arm64/exception] Received IRQ\n");

        return;
}

void handle_spx_fiq(exception_frame *frame)
{
        klog("[arm64/exception] Received FIQ\n");

        return;
}

void handle_spx_ser(exception_frame *frame)
{
        klog("[arm64/exception] Received SError\n");

        return;
}
