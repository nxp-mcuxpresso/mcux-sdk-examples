/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_nor_flash.h"
#include "fsl_debug_console.h"
#include "fx_levelx_driver.h"

#include "lx_api.h"

/* note that block and sector have different definition in LevelX and the flash driver .*/

#define LX_NOR_SECTOR_SIZE_BYTES    (LX_NOR_SECTOR_SIZE * sizeof(ULONG))
#define LX_NOR_BLOCK_SIZE           lx_nor_block_size

#define LX_NOR_VERIFY_SIZE          LX_NOR_SECTOR_SIZE_BYTES

#define FLASH_ERASED_UINT32         (0xFFFFFFFFUL)
#define SPI_FLASH_BASE_ADDRESS      (0)

static uint32_t lx_nor_block_size; 

static ULONG _lx_nor_sector_buffer[LX_NOR_SECTOR_SIZE];

static nor_config_t flash_config;
static nor_handle_t flash_handle;

extern fx_levelx_disk_infor_t g_disk_infor;

static UINT _lx_spi_flash_read(ULONG *flash_address, ULONG *destination, ULONG words)
{
    uint32_t address = (uint32_t)flash_address;
    int size;
    status_t ret;

    assert(destination != NULL);

    size = words * sizeof(ULONG);

    ret = Nor_Flash_Read(&flash_handle, address, (uint8_t *)destination, size);
    if (ret != kStatus_Success)
    {
        return LX_ERROR;
    }

    return LX_SUCCESS;
}

static UINT _lx_spi_flash_write(ULONG *flash_address, ULONG *source, ULONG words)
{
    uint32_t address = (uint32_t)flash_address;
    int size;
    status_t ret;

    assert(source != NULL);

    size = words * sizeof(ULONG);

    ret = Nor_Flash_Program(&flash_handle, (uint32_t)address, (uint8_t *)source, size);
    if (ret != kStatus_Success)
    {
        return LX_ERROR;
    }

    return LX_SUCCESS;
}

static UINT _lx_spi_flash_block_erase(ULONG block, ULONG erase_count)
{
    uint32_t sector_address;
    status_t ret;
    int index;
    int count;

    LX_PARAMETER_NOT_USED(erase_count);

    sector_address = SPI_FLASH_BASE_ADDRESS + LX_NOR_BLOCK_SIZE * block;
    count = LX_NOR_BLOCK_SIZE / flash_handle.bytesInSectorSize;

    for (index = 0; index < count; index++)
    {
        ret = Nor_Flash_Erase_Sector(&flash_handle, sector_address);
        if (ret != kStatus_Success)
        {
            return LX_ERROR;
        }

        sector_address += flash_handle.bytesInSectorSize;
    }

    return LX_SUCCESS;
}


static UINT _lx_spi_flash_block_erased_verify(ULONG block)
{
    uint32_t *buffer = (uint32_t *)_lx_nor_sector_buffer;
    uint32_t flash_address;
    int index;
    int count;
    int i;
    status_t ret;

    flash_address = SPI_FLASH_BASE_ADDRESS + LX_NOR_BLOCK_SIZE * block;
    count = LX_NOR_BLOCK_SIZE / LX_NOR_VERIFY_SIZE;

    for (index = 0; index < count; index++)
    {
        ret = Nor_Flash_Read(&flash_handle, flash_address, (uint8_t *)buffer,
                             LX_NOR_VERIFY_SIZE);
        if (ret != kStatus_Success)
        {
            return LX_ERROR;
        }

        for (i = 0; i < LX_NOR_VERIFY_SIZE / sizeof(uint32_t); i++)
        {
            if (buffer[i] != FLASH_ERASED_UINT32)
                return LX_ERROR;
        }

        flash_address += LX_NOR_VERIFY_SIZE;
    }

    return LX_SUCCESS;
}

static UINT _lx_spi_flash_system_error(UINT error_code)
{
    PRINTF("ERR: levelx NOR flash system error (%d)\r\n", error_code);

    return LX_SUCCESS;
}

UINT _lx_spi_flash_initialize(LX_NOR_FLASH *nor_flash)
{
    fx_levelx_disk_infor_t *disk_infor;
    status_t ret;
    uint32_t end;

    assert(nor_flash != NULL);

    ret = Nor_Flash_Init(&flash_config, &flash_handle);
    if (ret != kStatus_Success)
    {
        PRINTF("_lx_spi_flash_initialize: Nor_Flash_Init\r\n");
        return LX_ERROR;
    }

    disk_infor = (fx_levelx_disk_infor_t *)&g_disk_infor;

    lx_nor_block_size = disk_infor->levelx_block_size;

    nor_flash->lx_nor_flash_words_per_block = disk_infor->levelx_block_size / sizeof(ULONG);

    nor_flash->lx_nor_flash_total_blocks = disk_infor->disk_size / disk_infor->levelx_block_size;

    nor_flash->lx_nor_flash_base_address = (ULONG *)disk_infor->disk_flash_offset;

    /* assert if the block size is not a multiple of the flash sector size */
    assert(lx_nor_block_size % flash_handle.bytesInSectorSize == 0);

    end = (uint32_t) (nor_flash->lx_nor_flash_base_address +
                        nor_flash->lx_nor_flash_total_blocks * lx_nor_block_size);
        
    if (end > SPI_FLASH_BASE_ADDRESS + flash_handle.bytesInMemorySize)
    {
        PRINTF("_lx_spi_flash_initialize: disk size is out of range\r\n");
        return LX_ERROR;
    }

    nor_flash->lx_nor_flash_driver_read = _lx_spi_flash_read;
    nor_flash->lx_nor_flash_driver_write = _lx_spi_flash_write;
    nor_flash->lx_nor_flash_driver_block_erase = _lx_spi_flash_block_erase;
    nor_flash->lx_nor_flash_driver_block_erased_verify = _lx_spi_flash_block_erased_verify;
    nor_flash->lx_nor_flash_driver_system_error = _lx_spi_flash_system_error;

    nor_flash->lx_nor_flash_sector_buffer = _lx_nor_sector_buffer;

    return LX_SUCCESS;
}
