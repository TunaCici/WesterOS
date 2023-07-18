/*
 * Time & timing related functionalities for the Kernel (mostly arch dependent)
 *
 * Author: Tuna CICI
 */

#include <stdint.h>

#include "ARM64/Machine.h"
#include "LibKern/Time.h"

uint64_t arm64_uptime(void)
{
        uint64_t uptime = 0; /* nanosec resolution */

        volatile uint64_t CNTVCT_EL0 = 0;
        volatile uint64_t CNTFRQ_EL0 = 0;

        MRS("CNTVCT_EL0", CNTVCT_EL0);
        MRS("CNTFRQ_EL0", CNTFRQ_EL0);

        uptime = CNTVCT_EL0 * NANO_PER_SEC / CNTFRQ_EL0;

        return uptime;
}

/* Block the CPU for 'msec' miliseconds */
void __attribute__((optimize("O0"))) ksleep(const uint64_t mSec) 
{
        volatile uint64_t CNTVCT_EL0 = 0;
        volatile uint64_t CNTFRQ_EL0 = 0;

        uint64_t entry = 0;
        uint64_t curr = 0;
 
        if (mSec == 0) {
                return;
        }

        MRS("CNTVCT_EL0", CNTVCT_EL0);
        MRS("CNTFRQ_EL0", CNTFRQ_EL0);

        entry = CNTVCT_EL0 * 1000 / CNTFRQ_EL0; /* Miliseconds passed so far */

        do {
                MRS("CNTVCT_EL0", CNTVCT_EL0);
                curr = CNTVCT_EL0 * 1000 / CNTFRQ_EL0;
        } while ((curr - entry) < mSec);
}
