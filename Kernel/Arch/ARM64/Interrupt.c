/* 
 *      File: Interrupt.c
 *      Author: Tuna CICI
 *
 *      ARM64 vector/exception handlers.      
 */

#include "ARM64/Interrupt.h"

#include "LibKern/Console.h"

void do_irq(interrupt_frame *frame)
{
        klog("[interrupt] irq recv\n");

        return;
}

void do_fiq(interrupt_frame *frame)
{
        klog("[interrupt] fiq recv\n");

        return;
}