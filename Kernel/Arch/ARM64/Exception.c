/* 
 *      File: Exception.c
 *      Author: Tuna CICI
 *
 *      ARM64 exception handlers
 */

#include "ARM64/Exception.h"
#include "ARM64/Machine.h"

#include "LibKern/Console.h"

extern uint64_t kstart;

/* TODO: Cross this bridge when you get here */
void handle_spx_syn(exception_frame *frame)
{        
        /* Exception Class */
        uint8_t ec =
                (frame->esr >> ESR_EC_OFFSET) & ((1 << (ESR_EC_SIZE + 1)) - 1);

        switch (ec) {
        case EC_DATA_ABORT:
                klog("[arm64/exception] DATA_ABORT!!!\n");
                wfi();
        break;
        case EC_DATA_ABORT_UNCHANGED:
                klog("[arm64/exception] DATA_ABORT!!!\n");
                wfi();
        break;
        default:
                klog("[arm64/exception] SYN ESR[EC]: 0x%lx\n", ec);
                wfi();
        break;
        }
}

void handle_spx_irq(exception_frame *frame)
{
        klog("[arm64/exception] IRQ\n");

        return;
}

void handle_spx_fiq(exception_frame *frame)
{
        klog("[arm64/exception] FIQ\n");

        return;
}

void handle_spx_ser(exception_frame *frame)
{
        klog("[arm64/exception] SError\n");

        return;
}
