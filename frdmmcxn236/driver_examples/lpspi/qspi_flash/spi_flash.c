/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "spi_flash.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void SPI_Flash_CreateHandle(spi_flash_t *flash, spi_dev_t *spi_dev)
{
    flash->spi_dev = spi_dev;
}

void SPI_Flash_GetID(spi_flash_t *flash, uint8_t *MID, uint8_t *DID)
{
    uint8_t id[2];
    spi_tx_rx_info_t infos[2] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = (const uint8_t[]){SPI_FLASH_CMD_READ_MANUFACTURER, 0x00U, 0x00U, 0x00U};
    infos[0].rxData  = NULL;
    infos[0].dataLen = 4;
    infos[1].dataWidth = 1;
    infos[1].txData  = NULL;
    infos[1].rxData  = id;
    infos[1].dataLen = 2;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));

    *MID = id[0];
    *DID = id[1];
}

void SPI_Flash_Lock(spi_flash_t *flash, bool lock)
{
    uint8_t cmd = lock ? SPI_FLASH_CMD_GLOBAL_BLOCK_LOCK : SPI_FLASH_CMD_GLOBAL_BLOCK_UNLOCK;

    spi_tx_rx_info_t infos[1] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = &cmd;
    infos[0].rxData  = NULL;
    infos[0].dataLen = 1;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));
}

uint8_t SPI_Flash_GetStatus(spi_flash_t *flash, uint8_t statusReg)
{
    uint8_t status;
    uint8_t cmd;
    static const uint8_t cmds[] = {SPI_FLASH_CMD_READ_STATUS_REG1, SPI_FLASH_CMD_READ_STATUS_REG2, SPI_FLASH_CMD_READ_STATUS_REG3};
    assert((statusReg >= 1 ) && (statusReg <= 3));

    cmd = cmds[statusReg - 1];

    spi_tx_rx_info_t infos[2] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = &cmd;
    infos[0].rxData  = NULL;
    infos[0].dataLen = 1;
    infos[1].dataWidth = 1;
    infos[1].txData  = NULL;
    infos[1].rxData  = &status;
    infos[1].dataLen = 1;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));

    return status;
}

static void SPI_Flash_WaitBusy(spi_flash_t *flash)
{
    while ((SPI_Flash_GetStatus(flash, 1) & SPI_FLASH_STATUS1_BUSY) != 0U)
    {
    }
}

void SPI_Flash_Reset(spi_flash_t *flash)
{
    uint8_t cmd;
    spi_tx_rx_info_t infos[1] = {0};

    cmd = SPI_FLASH_CMD_ENABLE_RESET;
    infos[0].dataWidth = 1;
    infos[0].txData  = &cmd;
    infos[0].rxData  = NULL;
    infos[0].dataLen = 1;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));

    cmd = SPI_FLASH_CMD_RESET_DEVICE;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));
}

void SPI_Flash_WriteEnable(spi_flash_t *flash)
{
    spi_tx_rx_info_t infos[1] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = (const uint8_t[]){SPI_FLASH_CMD_WRITE_ENABLE};
    infos[0].rxData  = NULL;
    infos[0].dataLen = 1;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));
}

void SPI_Flash_FastRead(spi_flash_t *flash, uint32_t address, uint8_t *data, uint32_t dataSize)
{
    uint8_t txData[5];
    txData[0] = SPI_FLASH_CMD_FAST_READ;
    txData[1] = (uint8_t)(address >> 16U);
    txData[2] = (uint8_t)(address >> 8U);
    txData[3] = (uint8_t)(address >> 0U);
    txData[4] = SPI_FLASH_DUMMY_DATA;

    spi_tx_rx_info_t infos[2] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = txData;
    infos[0].rxData  = NULL;
    infos[0].dataLen = sizeof(txData);
    infos[1].dataWidth = 1;
    infos[1].txData  = NULL;
    infos[1].rxData  = data;
    infos[1].dataLen = dataSize;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));
}

void SPI_Flash_FastReadDualOutput(spi_flash_t *flash, uint32_t address, uint8_t *data, uint32_t dataSize)
{
    uint8_t txData[5];
    txData[0] = SPI_FLASH_CMD_FAST_READ_DUAL_OUTPUT;
    txData[1] = (uint8_t)(address >> 16U);
    txData[2] = (uint8_t)(address >> 8U);
    txData[3] = (uint8_t)(address >> 0U);
    txData[4] = SPI_FLASH_DUMMY_DATA;

    spi_tx_rx_info_t infos[2] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = txData;
    infos[0].rxData  = NULL;
    infos[0].dataLen = sizeof(txData);
    infos[1].dataWidth = 2;
    infos[1].txData  = NULL;
    infos[1].rxData  = data;
    infos[1].dataLen = dataSize;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));
}

void SPI_Flash_FastReadQuadOutput(spi_flash_t *flash, uint32_t address, uint8_t *data, uint32_t dataSize)
{
    uint8_t txData[5];
    txData[0] = SPI_FLASH_CMD_FAST_READ_QUAD_OUTPUT;
    txData[1] = (uint8_t)(address >> 16U);
    txData[2] = (uint8_t)(address >> 8U);
    txData[3] = (uint8_t)(address >> 0U);
    txData[4] = SPI_FLASH_DUMMY_DATA;

    spi_tx_rx_info_t infos[2] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = txData;
    infos[0].rxData  = NULL;
    infos[0].dataLen = sizeof(txData);
    infos[1].dataWidth = 4;
    infos[1].txData  = NULL;
    infos[1].rxData  = data;
    infos[1].dataLen = dataSize;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));
}

void SPI_Flash_FastReadDualIO(spi_flash_t *flash, uint32_t address, uint8_t *data, uint32_t dataSize)
{
    uint8_t txData[4];
    txData[0] = (uint8_t)(address >> 16U);
    txData[1] = (uint8_t)(address >> 8U);
    txData[2] = (uint8_t)(address >> 0U);
    txData[3] = 0xFFU; /* Must be 0xFx. */

    spi_tx_rx_info_t infos[3] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = (const uint8_t[]){SPI_FLASH_CMD_FAST_READ_DUAL_IO};
    infos[0].rxData  = NULL;
    infos[0].dataLen = 1;

    infos[1].dataWidth = 2;
    infos[1].txData  = txData;
    infos[1].rxData  = NULL;
    infos[1].dataLen = sizeof(txData);

    infos[2].dataWidth = 2;
    infos[2].txData  = NULL;
    infos[2].rxData  = data;
    infos[2].dataLen = dataSize;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));
}

void SPI_Flash_FastReadQuadIO(spi_flash_t *flash, uint32_t address, uint8_t *data, uint32_t dataSize)
{
    uint8_t txData[6];
    txData[0] = (uint8_t)(address >> 16U);
    txData[1] = (uint8_t)(address >> 8U);
    txData[2] = (uint8_t)(address >> 0U);
    txData[3] = 0xFFU; /* Must be 0xFx. */
    txData[4] = SPI_FLASH_DUMMY_DATA;
    txData[5] = SPI_FLASH_DUMMY_DATA;

    spi_tx_rx_info_t infos[3] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = (const uint8_t[]){SPI_FLASH_CMD_FAST_READ_QUAD_IO};
    infos[0].rxData  = NULL;
    infos[0].dataLen = 1;

    infos[1].dataWidth = 4;
    infos[1].txData  = txData;
    infos[1].rxData  = NULL;
    infos[1].dataLen = sizeof(txData);

    infos[2].dataWidth = 4;
    infos[2].txData  = NULL;
    infos[2].rxData  = data;
    infos[2].dataLen = dataSize;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));
}

void SPI_Flash_EraseSector(spi_flash_t *flash, uint32_t address)
{
    SPI_Flash_WriteEnable(flash);

    uint8_t txData[4];
    txData[0] = SPI_FLASH_CMD_SECTOR_ERASE;
    txData[1] = (uint8_t)(address >> 16U);
    txData[2] = (uint8_t)(address >> 8U);
    txData[3] = (uint8_t)(address >> 0U);

    spi_tx_rx_info_t infos[1] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = txData;
    infos[0].rxData  = NULL;
    infos[0].dataLen = sizeof(txData);

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));

    SPI_Flash_WaitBusy(flash);
}

void SPI_Flash_ProgramPage(spi_flash_t *flash, uint32_t address, const uint8_t *data, uint32_t dataSize)
{
    SPI_Flash_WriteEnable(flash);

    uint8_t txData[4];
    txData[0] = SPI_FLASH_CMD_PAGE_PROGRAM;
    txData[1] = (uint8_t)(address >> 16U);
    txData[2] = (uint8_t)(address >> 8U);
    txData[3] = (uint8_t)(address >> 0U);

    spi_tx_rx_info_t infos[2] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = txData;
    infos[0].rxData  = NULL;
    infos[0].dataLen = sizeof(txData);

    infos[1].dataWidth = 1;
    infos[1].txData  = data;
    infos[1].rxData  = NULL;
    infos[1].dataLen = dataSize;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));

    SPI_Flash_WaitBusy(flash);
}

void SPI_Flash_QuadInputProgramPage(spi_flash_t *flash, uint32_t address, const uint8_t *data, uint32_t dataSize)
{
    SPI_Flash_WriteEnable(flash);

    uint8_t txData[4];
    txData[0] = SPI_FLASH_CMD_QUAD_PAGE_PROGRAM;
    txData[1] = (uint8_t)(address >> 16U);
    txData[2] = (uint8_t)(address >> 8U);
    txData[3] = (uint8_t)(address >> 0U);

    spi_tx_rx_info_t infos[2] = {0};
    infos[0].dataWidth = 1;
    infos[0].txData  = txData;
    infos[0].rxData  = NULL;
    infos[0].dataLen = sizeof(txData);

    infos[1].dataWidth = 4;
    infos[1].txData  = data;
    infos[1].rxData  = NULL;
    infos[1].dataLen = dataSize;

    SPI_DEV_SendReceive(flash->spi_dev, infos, ARRAY_SIZE(infos));

    SPI_Flash_WaitBusy(flash);
}
