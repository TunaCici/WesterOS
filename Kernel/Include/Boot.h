/*
 * Structures used to pass information from Shim to the kernel
 * WARN: There is a high chance this method will be later changed
 *
 * Author: Tuna CICI
 */

#ifndef BOOT_H
#define BOOT_H

#include <stdint.h>

typedef struct boot_sysinfo {
        uint64_t k_phy_base;
        uint64_t k_vir_base;
        uint64_t k_size;

        uint64_t vector_base;

        uint64_t dtb_base;
        uint64_t dtb_size;
} boot_sysinfo;

#endif /* BOOT_H */