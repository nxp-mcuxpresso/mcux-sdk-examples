/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_debug_console.h"

#include "fx_api.h"
#include "lx_api.h"

/* define every block has 16 sector */
#define LX_NOR_SECTOR_SIZE_BYTES    (LX_NOR_SECTOR_SIZE * sizeof(ULONG))

static LX_NOR_FLASH spi_flash;

#ifdef LX_USE_MFLASH
extern UINT _lx_mflash_initialize(LX_NOR_FLASH *nor_flash);
#define LX_INIT_FUNCTION    _lx_mflash_initialize
#else
extern UINT _lx_spi_flash_initialize(LX_NOR_FLASH *spi_flash);
#define LX_INIT_FUNCTION    _lx_spi_flash_initialize
#endif

static VOID _fx_levelx_driver_read(FX_MEDIA *media_ptr)
{
    ULONG start_sector;
    UCHAR *dest_buffer;
    ULONG sector_count;
    UINT status;
    ULONG i;

    start_sector = media_ptr->fx_media_driver_logical_sector;
    sector_count = media_ptr->fx_media_driver_sectors;
    dest_buffer = media_ptr->fx_media_driver_buffer;

    assert(sector_count != 0);
    assert(dest_buffer != NULL);

    for (i = 0; i < sector_count; i++)
    {
        status = lx_nor_flash_sector_read(&spi_flash, start_sector, dest_buffer);
        if (status != LX_SUCCESS)
        {
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            return;
        }

        start_sector++;
        dest_buffer += LX_NOR_SECTOR_SIZE_BYTES;
    }

    media_ptr->fx_media_driver_status = FX_SUCCESS;
}

static VOID _fx_levelx_driver_write(FX_MEDIA *media_ptr)
{
    ULONG start_sector;
    UCHAR *src_buffer;
    ULONG sector_count;
    UINT status;
    ULONG i;

    start_sector = media_ptr->fx_media_driver_logical_sector;
    sector_count = media_ptr->fx_media_driver_sectors;
    src_buffer = media_ptr->fx_media_driver_buffer;

    assert(sector_count != 0);
    assert(src_buffer != NULL);

    for (i = 0; i < sector_count; i++)
    {
        status = lx_nor_flash_sector_write(&spi_flash, start_sector, src_buffer);
        if (status != LX_SUCCESS)
        {
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            return;
        }

        start_sector++;
        src_buffer += LX_NOR_SECTOR_SIZE_BYTES;
    }

    media_ptr->fx_media_driver_status = FX_SUCCESS;
}

static VOID _fx_levelx_driver_release_sectors(FX_MEDIA *media_ptr)
{
    ULONG start_sector;
    ULONG sector_count;
    UINT status;
    ULONG i;

    start_sector = media_ptr->fx_media_driver_logical_sector;
    sector_count = media_ptr->fx_media_driver_sectors;

    for (i = 0; i < sector_count; i++)
    {
        status = lx_nor_flash_sector_release(&spi_flash, start_sector);
        if (status != LX_SUCCESS)
        {
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            return;
        }

        start_sector++;
    }

    media_ptr->fx_media_driver_status = FX_SUCCESS;
}

static VOID _fx_levelx_driver_init(FX_MEDIA *media_ptr)
{
    UINT status;

    /* FileX should tell wear leveling when sectors are no longer in use.  */
    media_ptr->fx_media_driver_free_sector_update = FX_TRUE;

    /* initialize the LevelX driver */
    status = lx_nor_flash_open(&spi_flash, "SPI flash", LX_INIT_FUNCTION);
    if (status != LX_SUCCESS)
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
        return;
    }

    media_ptr->fx_media_driver_status = FX_SUCCESS;
}

static VOID _fx_levelx_driver_uninit(FX_MEDIA *media_ptr)
{
    UINT status;

    status = lx_nor_flash_close(&spi_flash);
    if (status != LX_SUCCESS)
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
        return;
    } 

    media_ptr->fx_media_driver_status = FX_SUCCESS;
}

static VOID _fx_levelx_driver_boot_read(FX_MEDIA *media_ptr)
{
    ULONG start_sector;
    UCHAR *dest_buffer;
    UINT status;

    start_sector = 0;
    dest_buffer = media_ptr->fx_media_driver_buffer;

    status = lx_nor_flash_sector_read(&spi_flash, start_sector, dest_buffer);
    if (status != LX_SUCCESS)
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
        return;
    } 

    media_ptr->fx_media_driver_status = FX_SUCCESS;
}

static VOID _fx_levelx_driver_boot_write(FX_MEDIA *media_ptr)
{
    ULONG start_sector;
    UCHAR *src_buffer;
    UINT status;

    start_sector = 0;
    src_buffer = media_ptr->fx_media_driver_buffer;

    status = lx_nor_flash_sector_write(&spi_flash, start_sector, src_buffer);
    if (status != LX_SUCCESS)
    {
        media_ptr->fx_media_driver_status = FX_IO_ERROR;
        return;
    } 

    media_ptr->fx_media_driver_status = FX_SUCCESS;
}

VOID _fx_levelx_driver(FX_MEDIA *media_ptr)
{
    switch(media_ptr->fx_media_driver_request)
    {
        case FX_DRIVER_READ:
            _fx_levelx_driver_read(media_ptr);
            break;
        case FX_DRIVER_WRITE:
            _fx_levelx_driver_write(media_ptr);
            break;
        case FX_DRIVER_FLUSH:
        case FX_DRIVER_ABORT:
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        case FX_DRIVER_RELEASE_SECTORS:
            _fx_levelx_driver_release_sectors(media_ptr);
            break;
        case FX_DRIVER_INIT:
            _fx_levelx_driver_init(media_ptr);
            break;
        case FX_DRIVER_UNINIT:
            _fx_levelx_driver_uninit(media_ptr);
            break;
        case FX_DRIVER_BOOT_READ:
            _fx_levelx_driver_boot_read(media_ptr);
            break;
        case FX_DRIVER_BOOT_WRITE:
            _fx_levelx_driver_boot_write(media_ptr);
            break;
        default:
        {
            /* Invalid driver request. */
            PRINTF("fx_levelx_driver: invalid request, %d\r\n",
                   media_ptr->fx_media_driver_request);
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            break;
        }
    }
}

