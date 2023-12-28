
#ifndef _FX_LEVELX_DRIVER
#define _FX_LEVELX_DRIVER

#include "fx_api.h"

typedef struct fx_levelx_disk_infor {
    ULONG disk_size;            /* the disk size in byte */
    ULONG disk_flash_offset;    /* the disk offset address in flash */
    ULONG levelx_block_size;    /* the block size in byte */
} fx_levelx_disk_infor_t;

#endif
