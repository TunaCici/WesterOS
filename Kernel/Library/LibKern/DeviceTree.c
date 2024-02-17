/*
 * Device Tree Blob parser
 *
 * Author: Tuna CICI
 */

#include "LibKern/String.h"
#include "LibKern/DeviceTree.h"
#include "LibKern/Console.h"
#include "LibKern/Time.h"

volatile static fdt_header *header = 0;

static inline uint8_t _dtb_valid(void *base)
{
        return TO_LE(((fdt_header*) base)->magic) == FDT_MAGIC;
}

static void _token_parse(const uint32_t token)
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

static inline uint8_t _match(const char *node_name, const char* str)
{
        return !strncmp(node_name, str, strlen(str));
}

static void* _find_node(fdt_header *hdr, const char* name)
{
        if (!hdr || !name) {
                return 0;
        }

        void *result = 0;

        uint32_t *token = (uint8_t*) hdr + TO_LE(hdr->off_dt_struct);

        for (uint32_t i = 0; i < TO_LE(hdr->totalsize) / 4; i++) {
                if (TO_LE(*token) == FDT_BEGIN_NODE && _match(token + 1, name)) {
                        result = token;
                        break;
                }

                token++;
        }


        return result;
}

uint8_t dtb_mem_info(void *base, uint64_t *mem_start, uint64_t *mem_end)
{
        if (!mem_start || !mem_end) {
                return 1;
        }

        uint8_t result = 1; /* unkown error */

        if (!_dtb_valid(base)) {
                return result;
        }

        fdt_header *hdr = (fdt_header*) base;
        
        uint32_t *mem_node = _find_node(hdr, "memory");
        if (!mem_node) {
                klog("[devicetree] couldn't found any 'memory'\n");
        }

        /* Skip until the 'reg' property - TODO: Unsafe */
        uint32_t *iter = mem_node;
        while (TO_LE(*iter) != FDT_PROP) {
                iter++;
        }
        
        fdt_prop *reg_prop = (fdt_prop*) (iter + 1);                
        uint32_t *reg_arr = (uint32_t*) (reg_prop + 1);
        
        /*
         * reg_arr[0]: [63:32] bits of 'memory start address'
         * reg_arr[1]: [31:0] bits of 'memory start address'
         * reg_arr[2]: [63:32] bits of 'memory size'
         * reg_arr[3]: [32:0] bits of 'memory size'
        */
        
        *mem_start = ((uint64_t) TO_LE(reg_arr[0]) << 32u) + TO_LE(reg_arr[1]);
        *mem_end = *mem_start;
        *mem_end += ((uint64_t) TO_LE(reg_arr[2]) << 32u) + TO_LE(reg_arr[3]);

        result = 0; /* success */

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
