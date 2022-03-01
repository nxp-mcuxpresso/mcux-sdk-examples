/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_spi.h"
#include "fsl_nor_flash.h"
#include "board_setup.h"
#include "fsl_debug_console.h"

#include "tx_api.h"

#define TRANSFER_BAUDRATE           (25000000U)   /* Transfer baudrate - 25M */

/* support the SPI Flash IS25LP128 */
#define FLASH_PAGE_SIZE                     (256UL)
#define FLASH_PAGE_MASK                     (FLASH_PAGE_SIZE - 1)
#define FLASH_SECTOR_SIZE                   (4 * 1024UL)            /* 4 KB per sector */
#define FLASH_TOTAL_SIZE                    (16 * 1024 * 1024UL)    /* 16 MB */

/* SPI flash commands */
#define FLASH_PAGE_PROG_CMD                 0x02
#define FLASH_NORMAL_READ_MODE_CMD          0x03
#define FLASH_READ_STATUS_REG_CMD           0x05
#define FLASH_WRITE_ENABLE_CMD              0x06
#define FLASH_CHIP_ERASE_CMD                0xc7
#define FLASH_SECTOR_ERASE_CMD              0xd7
#define FLASH_READ_PROD_ID_CMD              0x9f

#define FLASH_STATUS_REG_WIP                (1UL)

#define FLASH_DRIVER_BUFFER_SIZE            (1024UL)

/* add 32 bytes memory space for command and address */
#define FLASH_DRIVER_BUFFER_DATA_START      ((uint8_t *)flash_driver_buffer + 32)

static uint8_t flash_driver_buffer[FLASH_DRIVER_BUFFER_SIZE + 32];

static bool is_spi_flash_driver_ready = false;

static status_t spi_flash_transfer(SPI_Type *spi_base,
                                   uint8_t* command, uint32_t command_size,
                                   uint8_t* result, uint32_t result_size)
{
    spi_transfer_t transfer;
    status_t ret;

    assert(command != NULL);

    transfer.txData      = command;
    transfer.rxData      = result;
    transfer.dataSize    = command_size + result_size;
    transfer.configFlags = kSPI_FrameAssert;
    ret = SPI_MasterTransferBlocking(spi_base, &transfer);
    if (ret != kStatus_Success)
    {
        return ret;
    }

    return kStatus_Success;
}

static status_t spi_flash_read_status(SPI_Type *spi_base, uint32_t *result)
{
    uint8_t command[8];
    uint8_t buff[8];
    int cmd_length;
    int result_length;
    status_t ret;

    command[0] = FLASH_READ_STATUS_REG_CMD;
    cmd_length = 1;
    result_length = 1;

    ret = spi_flash_transfer(spi_base, command, cmd_length, buff, result_length);
    if (ret != kStatus_Success) {
        return ret;
    }

    /* skip placeholder byte for command */
    *result = (uint32_t)buff[cmd_length];

    return kStatus_Success;
}

static inline bool spi_flash_is_ready(SPI_Type *spi_base)
{
    status_t ret;
    uint32_t result;

    ret = spi_flash_read_status(spi_base, &result);
    if (ret != kStatus_Success) {
        return false;
    }

    return result & FLASH_STATUS_REG_WIP ? false : true;
}

static status_t spi_flash_write_enable(SPI_Type *spi_base)
{
    uint8_t command[4];
    int cmd_length;
    status_t ret;

    command[0] = FLASH_WRITE_ENABLE_CMD;
    cmd_length = 1;

    ret = spi_flash_transfer(spi_base, command, cmd_length, NULL, 0);
    if (ret != kStatus_Success) {
        return ret;
    }

    return kStatus_Success;
}

static status_t spi_flash_page_program(SPI_Type *spi_base, uint32_t start_addr,
                                       uint8_t* buffer, uint32_t size)
{
    int cmd_length;
    uint8_t *tx_buff;
    status_t ret;

    if (! spi_flash_is_ready(spi_base))
    {
        return kStatus_Fail;
    }

    ret = spi_flash_write_enable(spi_base);
    if (ret != kStatus_Success) {
        return ret;
    }

    cmd_length = 4;
    tx_buff = buffer - cmd_length;

    tx_buff[0] = FLASH_PAGE_PROG_CMD;
    tx_buff[1] = start_addr >> 16 & 0x0ff;
    tx_buff[2] = start_addr >>  8 & 0x0ff;
    tx_buff[3] = start_addr       & 0x0ff;

    ret = spi_flash_transfer(spi_base, tx_buff, size + cmd_length, NULL, 0);
    if (ret != kStatus_Success) {
        return ret;
    }

    while (! spi_flash_is_ready(spi_base))
    {
    }

    return kStatus_Success;
}

static status_t spi_flash_driver_init(void)
{
    spi_master_config_t spi_config;
    SPI_Type *spi_base = EXAMPLE_SPI_MASTER;
    status_t ret;

    if (is_spi_flash_driver_ready)
        return kStatus_Success;

    SPI_MasterGetDefaultConfig(&spi_config);

    spi_config.baudRate_Bps = TRANSFER_BAUDRATE;
    spi_config.sselNum      = EXAMPLE_SPI_SSEL;
    spi_config.sselPol      = (spi_spol_t)EXAMPLE_MASTER_SPI_SPOL;

    ret = SPI_MasterInit(spi_base, &spi_config, EXAMPLE_SPI_MASTER_CLK_FREQ);
    if (ret != kStatus_Success)
    {
        return kStatus_Fail;
    }

    is_spi_flash_driver_ready = true;

    return kStatus_Success;
}

status_t Nor_Flash_Read(nor_handle_t *handle, uint32_t address,
                        uint8_t *buffer, uint32_t length)
{
    SPI_Type *spi_base = (SPI_Type *)handle->driverBaseAddr;
    uint8_t command[4];
    int cmd_length;
    status_t ret;

    assert(length <= FLASH_DRIVER_BUFFER_SIZE);

    if (! spi_flash_is_ready(spi_base))
    {
        return kStatus_Fail;
    }

    cmd_length = 4;
    command[0] = FLASH_NORMAL_READ_MODE_CMD;
    command[1] = address >> 16 & 0x0ff;
    command[2] = address >>  8 & 0x0ff;
    command[3] = address       & 0x0ff;

    ret = spi_flash_transfer(spi_base, command, cmd_length,
                             FLASH_DRIVER_BUFFER_DATA_START - cmd_length,
                             length);
    if (ret != kStatus_Success) {
        return ret;
    }

    memcpy(buffer, FLASH_DRIVER_BUFFER_DATA_START, length);

    return kStatus_Success;
}

status_t Nor_Flash_Program(nor_handle_t *handle, uint32_t address,
                           uint8_t *buffer, uint32_t length)
{
    SPI_Type *spi_base = (SPI_Type *)handle->driverBaseAddr;
    uint32_t prog_size;
    uint32_t start_addr;
    uint32_t end_addr;
    status_t ret;
    
    start_addr = address;
    end_addr = address + length;

    while (start_addr < end_addr)
    {
        /* compute each page size */
        prog_size = ((start_addr & (~FLASH_PAGE_MASK)) + FLASH_PAGE_SIZE) - start_addr;

        if (prog_size > end_addr - start_addr)
        {
            prog_size = end_addr - start_addr;
        }

        memcpy(FLASH_DRIVER_BUFFER_DATA_START, (void *)buffer, prog_size);

        ret = spi_flash_page_program(spi_base, start_addr,
                                     FLASH_DRIVER_BUFFER_DATA_START, prog_size);
        if (ret != kStatus_Success) {
            return kStatus_Fail;
        }

        start_addr += prog_size;
        buffer += prog_size;
    }

    return kStatus_Success;
}

status_t Nor_Flash_Erase_Sector(nor_handle_t *handle, uint32_t address)
{
    SPI_Type *spi_base = (SPI_Type *)handle->driverBaseAddr;
    uint8_t command[4];
    int cmd_length;
    status_t ret;
    
    if (! spi_flash_is_ready(spi_base))
    {
        return kStatus_Fail;
    }

    ret = spi_flash_write_enable(spi_base);
    if (ret != kStatus_Success) {
        return ret;
    }

    cmd_length = 4;
    command[0] = FLASH_SECTOR_ERASE_CMD;
    command[1] = address >> 16 & 0x0ff;
    command[2] = address >>  8 & 0x0ff;
    command[3] = address       & 0x0ff;

    ret = spi_flash_transfer(spi_base, command, cmd_length, NULL, 0);
    if (ret != kStatus_Success) {
        return ret;
    }

    while (! spi_flash_is_ready(spi_base))
    {
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 20);  /* sleep 50ms */
    }

    return kStatus_Success;
}

status_t Nor_Flash_Init(nor_config_t *config, nor_handle_t *handle)
{
    SPI_Type *spi_base = EXAMPLE_SPI_MASTER;

    if (! is_spi_flash_driver_ready) {
        spi_flash_driver_init();
    }

    handle->driverBaseAddr = (void *)spi_base;
    handle->bytesInMemorySize = FLASH_TOTAL_SIZE;
    handle->bytesInSectorSize = FLASH_SECTOR_SIZE;
    handle->bytesInPageSize   = FLASH_PAGE_SIZE;

    return kStatus_Success;
}

status_t spi_flash_erase_flash_disk(uint32_t offset, uint32_t disk_size)
{
    nor_config_t config;
    nor_handle_t handle;
    uint32_t start_addr;
    status_t ret;

    assert(disk_size % FLASH_SECTOR_SIZE == 0);

    ret = Nor_Flash_Init(&config, &handle);
    if (ret != kStatus_Success)
    {
        return kStatus_Fail;
    }

    start_addr = offset;

    while (start_addr < offset + disk_size) {

        PRINTF(".");
        ret = Nor_Flash_Erase_Sector(&handle, start_addr);
        if (ret != kStatus_Success)
        {
            return kStatus_Fail;
        }

        start_addr += FLASH_SECTOR_SIZE;
    }

    PRINTF("\r\n");

    return kStatus_Success;
}

