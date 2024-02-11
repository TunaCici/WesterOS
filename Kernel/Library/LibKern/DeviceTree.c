/*
 * Device Tree Blob parser
 *
 * Author: Tuna CICI
 */

#include "LibKern/DeviceTree.h"
#include "LibKern/Console.h"
#include "LibKern/Time.h"

volatile static fdt_header *header = 0;

static inline uint8_t _dtb_valid(void *base)
{
        return TO_LE(((fdt_header*) base)->magic) == FDT_MAGIC;
}

void _token_parse(const uint32_t token)
{
        switch (token) {
        case FDT_BEGIN_NODE:
                klog("[devicetree] FDT_BEGIN_NODE\n");
                break;
        case FDT_END_NODE:
                klog("[devicetree] FDT_END_NODE\n");
                break;
        case FDT_PROP:
                klog("[devicetree] FDT_PROP\n");
                break;
        case FDT_NOP:
                klog("[devicetree] FDT_NOP\n");
                break;
        case FDT_END:
                klog("[devicetree] FDT_END\n");
                break;
        default:
                break;
        }
}

uint8_t dtb_get_memory(void *base, uint64_t *ram_start, uint64_t *ram_end)
{
        if (!ram_start || !ram_end) {
                return 1;
        }

        uint8_t result = 1; /* unkown error */

        if (!_dtb_valid(base)) {
                return result;
        }

        fdt_header *hdr = (fdt_header*) base;

        uint32_t *token = (uint8_t*) base + TO_LE(hdr->off_dt_struct);

        for (uint32_t i = 0; i < TO_LE(hdr->totalsize) / 4; i++) {
                if (TO_LE(*token) == FDT_BEGIN_NODE) {
                        klog("[devicetree] node_name: %s\n", (token + 1));
                }

                token++;
        }

        return result;
} 

uint8_t dtb_init(void *base)
{
        if (!base) {
                return 1;
        }

        uint8_t result = 0; /* success */

        return result;
}

uint8_t dtb_next(void *dev_name)
{
        if (!dev_name) {
                return 1;
        }

        uint8_t result = 0; /* success */

        return result;
}
