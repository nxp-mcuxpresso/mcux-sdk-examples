/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "mflash_drv.h"

#include "fx_levelx_driver.h"
#include "lx_api.h"


#define LX_NOR_DISK_SIZE            (g_disk_infor.disk_size)
#define LX_NOR_DISK_FLASH_OFF       (g_disk_infor.disk_flash_offset)
#define LX_NOR_BLOCK_SIZE           (g_disk_infor.levelx_block_size)

#define LX_NOR_SECTOR_SIZE_BYTES    (LX_NOR_SECTOR_SIZE * sizeof(ULONG))

#define FLASH_ERASED_UINT32         (0xFFFFFFFFUL)

static ULONG _lx_nor_sector_buffer[LX_NOR_SECTOR_SIZE];
static bool is_mflash_driver_ready = false;

extern fx_levelx_disk_infor_t g_disk_infor;


static UINT _lx_mflash_read(ULONG *offset, ULONG *data, ULONG words)
{
    status_t ret;

    assert(data != NULL);

    ret = (status_t)mflash_drv_read((uint32_t)offset,
                                    (uint32_t *)data,
                                    words * sizeof(ULONG));
    if (ret != kStatus_Success)
    {
        PRINTF("ERR: mflash_drv_read(0x%x)\r\n", offset);
        return LX_ERROR;
    }

    return LX_SUCCESS;
}

static UINT _lx_mflash_write(ULONG *offset, ULONG *data, ULONG words)
{
    int8_t wbuf[MFLASH_PAGE_SIZE];
    uint32_t start_offset;
    uint32_t page_offset;
    uint32_t size;
    uint8_t *buffer;
    status_t ret;

    assert(data != NULL);
    assert(words == 1 || words == LX_NOR_SECTOR_SIZE);

    size = words * sizeof(ULONG);

    if (words == 1)
    {
        /*
         * mflash_drv_page_program() programs one page at a time,
         * so set unchanged data to 0x0ff
         */
        memset(wbuf, 0x0ff, MFLASH_PAGE_SIZE);

        page_offset = (uint32_t)offset % MFLASH_PAGE_SIZE;
        memcpy(&wbuf[page_offset], data, size);

        start_offset = (uint32_t)offset / MFLASH_PAGE_SIZE * MFLASH_PAGE_SIZE;

        ret = (status_t)mflash_drv_page_program(start_offset, (uint32_t *)wbuf);
        if (ret != kStatus_Success)
        {
            PRINTF("ERR: mflash_drv_page_program(0x%x)\r\n", start_offset);
            return LX_ERROR;
        }
    }
    else
    {
        /* words = LX_NOR_SECTOR_SIZE */

        buffer = (uint8_t *)data;
        start_offset = (uint32_t)offset;

        for (int index = 0; index < size / MFLASH_PAGE_SIZE; index++)
        {
            ret = (status_t)mflash_drv_page_program(start_offset, (uint32_t *)buffer);
            if (ret != kStatus_Success)
            {
                PRINTF("ERR: mflash_drv_page_program(0x%x)\r\n", start_offset);
                return LX_ERROR;
            }

            start_offset += MFLASH_PAGE_SIZE;
            buffer += MFLASH_PAGE_SIZE;
        }
    }

    return LX_SUCCESS;
}

static UINT _lx_mflash_block_erase(ULONG block, ULONG erase_count)
{
    uint32_t sector_offset;
    status_t ret;
    int index;
    int count;

    LX_PARAMETER_NOT_USED(erase_count);

    sector_offset = LX_NOR_DISK_FLASH_OFF + LX_NOR_BLOCK_SIZE * block;
    count = LX_NOR_BLOCK_SIZE / MFLASH_SECTOR_SIZE;

    for (index = 0; index < count; index++)
    {
        ret = mflash_drv_sector_erase(sector_offset);
        if (ret != kStatus_Success)
        {
            return LX_ERROR;
        }

        sector_offset += MFLASH_SECTOR_SIZE;
    }

    return LX_SUCCESS;
}

static UINT _lx_mflash_block_erased_verify(ULONG block)
{
    uint32_t *buffer = (uint32_t *)_lx_nor_sector_buffer;
    uint32_t offset;
    status_t ret;
    int index;
    int count;
    int i;

    offset = LX_NOR_DISK_FLASH_OFF + LX_NOR_BLOCK_SIZE * block;
    count = LX_NOR_BLOCK_SIZE / LX_NOR_SECTOR_SIZE_BYTES;

    for (index = 0; index < count; index++)
    {
        ret = (status_t)mflash_drv_read(offset, buffer, LX_NOR_SECTOR_SIZE_BYTES);
        if (ret != kStatus_Success)
        {
            PRINTF("ERR: mflash_drv_read(0x%x)\r\n", offset);
            return ret;
        }

        for (i = 0; i < LX_NOR_BLOCK_SIZE / sizeof(uint32_t); i++)
        {
            if (buffer[i] != FLASH_ERASED_UINT32)
                return LX_ERROR;
        }

        offset += LX_NOR_SECTOR_SIZE_BYTES;
    }

    return LX_SUCCESS;
}

static UINT _lx_mflash_system_error(UINT error_code)
{
    PRINTF("ERR: levelx system error (%d)\r\n", error_code);
    return LX_SUCCESS;
}

UINT _lx_mflash_initialize(LX_NOR_FLASH *nor_flash)
{
    status_t ret;

    assert(nor_flash != NULL);

    if (is_mflash_driver_ready == false) {
        ret = mflash_drv_init();
        if (ret != kStatus_Success)
        {
            PRINTF("ERR: mflash_drv_init() error\r\n");
            return LX_ERROR;
        }
        is_mflash_driver_ready = true;
    }

    nor_flash->lx_nor_flash_words_per_block =
        (ULONG) (LX_NOR_BLOCK_SIZE / sizeof(ULONG));

    nor_flash->lx_nor_flash_total_blocks =
        (ULONG) (LX_NOR_DISK_SIZE / LX_NOR_BLOCK_SIZE);

    nor_flash->lx_nor_flash_base_address =
        (ULONG *)LX_NOR_DISK_FLASH_OFF;

    nor_flash->lx_nor_flash_driver_read = _lx_mflash_read;
    nor_flash->lx_nor_flash_driver_write = _lx_mflash_write;

    nor_flash->lx_nor_flash_driver_block_erase =
        _lx_mflash_block_erase;
    nor_flash->lx_nor_flash_driver_block_erased_verify =
        _lx_mflash_block_erased_verify;

    nor_flash->lx_nor_flash_driver_system_error =
        _lx_mflash_system_error;

    nor_flash->lx_nor_flash_sector_buffer =
        _lx_nor_sector_buffer;

    return LX_SUCCESS;
}

status_t erase_flash_disk(uint32_t offset, uint32_t disk_size)
{
    UINT ret;
    int block;

    LX_PARAMETER_NOT_USED(offset);
    LX_PARAMETER_NOT_USED(disk_size);

    assert(LX_NOR_DISK_SIZE != 0);
    assert(LX_NOR_DISK_FLASH_OFF != 0);

    if (is_mflash_driver_ready == false) {
        ret = mflash_drv_init();
        if (ret != kStatus_Success)
        {
            PRINTF("ERR: mflash_drv_init() error\r\n");
            return LX_ERROR;
        }
        is_mflash_driver_ready = true;
    }

    for (block = 0; block < LX_NOR_DISK_SIZE / LX_NOR_BLOCK_SIZE; block++)
    {
        PRINTF(".");
        ret = _lx_mflash_block_erase(block, 1);
        if (ret != LX_SUCCESS)
        {
            return kStatus_Fail;
        }
    }
    PRINTF("\r\n");

    return kStatus_Success;
}

