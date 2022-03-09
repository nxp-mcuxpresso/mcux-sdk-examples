/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_sd.h"
#include "sdmmc_config.h"
#include "fsl_debug_console.h"

#include "tx_api.h"
#include "fx_api.h"

static bool is_sdcard_ready = false;

static VOID _fx_sdcard_driver_read(FX_MEDIA *media_ptr)
{
    sd_card_t *sdcard = media_ptr->fx_media_driver_info;
    uint32_t start_sector;
    uint8_t *dest_buffer;
    uint32_t sector_count;
    status_t status;

    start_sector = (uint32_t)media_ptr->fx_media_driver_logical_sector;
    sector_count = (uint32_t)media_ptr->fx_media_driver_sectors;
    dest_buffer = (uint8_t *)media_ptr->fx_media_driver_buffer;

    assert(sector_count != 0);
    assert(dest_buffer != NULL);

    status = SD_ReadBlocks(sdcard, dest_buffer, start_sector, sector_count);
    if (status != kStatus_Success)
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
    }
    else
    {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
    }
}

static VOID _fx_sdcard_driver_write(FX_MEDIA *media_ptr)
{
    sd_card_t *sdcard = media_ptr->fx_media_driver_info;
    uint32_t start_sector;
    uint8_t *src_buffer;
    uint32_t sector_count;
    status_t status;

    start_sector = (uint32_t)media_ptr->fx_media_driver_logical_sector;
    sector_count = (uint32_t)media_ptr->fx_media_driver_sectors;
    src_buffer = (uint8_t *)media_ptr->fx_media_driver_buffer;

    assert(sector_count != 0);
    assert(src_buffer != NULL);

    status = SD_WriteBlocks(sdcard, src_buffer, start_sector, sector_count);
    if (status != kStatus_Success)
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
    }
    else
    {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
    }
}

static VOID _fx_sdcard_driver_init(FX_MEDIA *media_ptr)
{
    sd_card_t *sdcard = media_ptr->fx_media_driver_info;

    /* no need to know freed clusters */
    media_ptr->fx_media_driver_free_sector_update = FX_FALSE;

    if(is_sdcard_ready)
    {
        return;
    }

    /* init SD card */
    if (kStatus_Success != SD_CardInit(sdcard))
    {
        SD_CardDeinit(sdcard);
        is_sdcard_ready = false;
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
        return;
    }

    is_sdcard_ready = true;
    media_ptr->fx_media_driver_status = FX_SUCCESS;
}

static VOID _fx_sdcard_driver_uninit(FX_MEDIA *media_ptr)
{
    sd_card_t *sdcard = media_ptr->fx_media_driver_info;

    SD_CardDeinit(sdcard);
    is_sdcard_ready = false;
    media_ptr->fx_media_driver_status = FX_SUCCESS;
}

static VOID _fx_sdcard_driver_boot_read(FX_MEDIA *media_ptr)
{
    sd_card_t *sdcard = media_ptr->fx_media_driver_info;
    uint32_t start_sector;
    uint8_t *dest_buffer;
    uint32_t sector_count;
    status_t status;

    start_sector = 0;
    sector_count = 1;
    dest_buffer = (uint8_t *)media_ptr->fx_media_driver_buffer;

    status = SD_ReadBlocks(sdcard, dest_buffer, start_sector, sector_count);
    if (status != kStatus_Success)
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
    }
    else
    {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
    }
}

static VOID _fx_sdcard_driver_boot_write(FX_MEDIA *media_ptr)
{
    sd_card_t *sdcard = media_ptr->fx_media_driver_info;
    uint32_t start_sector;
    uint8_t *src_buffer;
    uint32_t sector_count;
    status_t status;

    start_sector = 0;
    sector_count = 1;
    src_buffer = (uint8_t *)media_ptr->fx_media_driver_buffer;

    status = SD_WriteBlocks(sdcard, src_buffer, start_sector, sector_count);
    if (status != kStatus_Success)
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
    }
    else
    {
        media_ptr->fx_media_driver_status = FX_SUCCESS;
    }
}

VOID _fx_sdcard_driver(FX_MEDIA *media_ptr)
{

    switch(media_ptr->fx_media_driver_request)
    {
        case FX_DRIVER_READ:
            _fx_sdcard_driver_read(media_ptr);
            break;
        case FX_DRIVER_WRITE:
            _fx_sdcard_driver_write(media_ptr);
            break;
        case FX_DRIVER_FLUSH:
        case FX_DRIVER_ABORT:
        case FX_DRIVER_RELEASE_SECTORS:
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        case FX_DRIVER_INIT:
            _fx_sdcard_driver_init(media_ptr);
            break;
        case FX_DRIVER_UNINIT:
            _fx_sdcard_driver_uninit(media_ptr);
            break;
        case FX_DRIVER_BOOT_READ:
            _fx_sdcard_driver_boot_read(media_ptr);
            break;
        case FX_DRIVER_BOOT_WRITE:
            _fx_sdcard_driver_boot_write(media_ptr);
            break;
        default:
        {
            /* Invalid driver request. */
            PRINTF("fx_sdcard_driver: invalid request, %d\r\n",
                   media_ptr->fx_media_driver_request);
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            break;
        }
    }
}
