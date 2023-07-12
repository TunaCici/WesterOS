/*
 * Time & timing related functionalities for the Kernel (mostly arch dependent)
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#include "LibKern/Time.h"

uint64_t arm64_uptime(void)
{
        uint64_t uptime = 0; /* nanosec resolution */

        volatile uint64_t CNTVCT_EL0 = 0;
        volatile uint64_t CNTFRQ_EL0 = 0;

        asm("MRS %[r], CNTVCT_EL0;" : [r]"=r" (CNTVCT_EL0));
        asm("MRS %[r], CNTFRQ_EL0;" : [r]"=r" (CNTFRQ_EL0));

        uptime = CNTVCT_EL0 * NANO_PER_SEC / CNTFRQ_EL0;

        return uptime;
}
