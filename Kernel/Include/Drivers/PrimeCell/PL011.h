/*
 * PrimeCell UART PL011 driver definitions
 *
 * Author: Tuna CICI
 */


#ifndef PL011_H
#define PL011_H

#include <stdint.h>

enum {
        UARTDR = 0x000,
        UARTRSR = 0x004,
        UARTFR = 0x018,

        UARTILPR = 0x020,
        
        UARTIBRD = 0x024,
        UARTFBRD = 0x028,
        
        UARTLCR_H = 0x02C,
        UARTCR = 0x030,
        
        UARTIFLS = 0x034,
        UARTIMSC = 0x038,
        UARTRIS = 0x03C,
        UARTMIS = 0x040,
        UARTICR = 0x044,
        
        UARTDMACR = 0x048,

        UARTPeriphID0 = 0xFE0,
        UARTPeriphID1 = 0xFE4,
        UARTPeriphID2 = 0xFE8,
        UARTPeriphID3 = 0xFEC,

        UARTPCellID0 = 0xFF0,
        UARTPCellID1 = 0xFF4,
        UARTPCellID2 = 0xFF8,
        UARTPCellID3 = 0xFFC
};

#endif /* PL011_h */
