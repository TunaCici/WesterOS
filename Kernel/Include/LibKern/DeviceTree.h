/*
 * Device Tree Blob parser definitions
 *
 * Author: Tuna CICI
 */

#pragma once

#include <stdint.h>

#define TO_LE(val) __builtin_bswap32(val) /* [GCC's] to little-endian */

#define FDT_MAGIC 0xD00DFEED /* big-endian */

enum { /* all big-endian */
        FDT_BEGIN_NODE = 0x00000001,
        FDT_END_NODE = 0x00000002,
        FDT_PROP = 0x00000003,
        FDT_NOP = 0x00000004,
        FDT_END = 0x00000009
};

typedef struct fdt_header {
        uint32_t magic;
        uint32_t totalsize;
        uint32_t off_dt_struct;
        uint32_t off_dt_strings;
        uint32_t off_mem_rsvmap;
        uint32_t version;
        uint32_t last_comp_version;
        uint32_t boot_cpuid_phys;
        uint32_t size_dt_strings;
        uint32_t size_dt_struct;
} fdt_header;

/* Special */
uint8_t dtb_get_memory(void* base, uint64_t *ram_start, uint64_t* ram_end);
uint8_t dtb_get_cpus(void *base, uint64_t *cpu_count);

/* TODO: Generic */
uint8_t dtb_init(void *base);
uint8_t dtb_next(void *dev_name);
