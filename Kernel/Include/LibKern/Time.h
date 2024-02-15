/*
 * Time & timing related functionalities for the kernel (mostly arch dependent)
 *
 * Author: Tuna CICI
 */

#ifndef TIME_H
#define TIME_H

#define NANO_PER_SEC  1000000000 /* 10^9 seconds */
#define MICRO_PER_SEC 1000000 /* 10^6 seconds */
#define MILLI_PER_SEC 1000 /* 10^3 seconds */

#define NANO_PER_MICRO 1000 /* 10^3 microseconds */

#include <stdint.h>

uint64_t arm64_uptime(void);

void     ksleep(const uint64_t mSec);

#endif /* TIME_H */
