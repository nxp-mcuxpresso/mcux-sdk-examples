/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#include <stdint.h>
#include <stdlib.h>
#include "spi_dev.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Flash W25Q64 command. */
#define SPI_FLASH_CMD_WRITE_ENABLE                      0x06U
#define SPI_FLASH_CMD_VOLATILE_SR_WRITE_ENABLE          0x50U
#define SPI_FLASH_CMD_WRITE_DISABLE                     0x04U
#define SPI_FLASH_CMD_RELEASE_POWER_DOWN                0xABU
#define SPI_FLASH_CMD_READ_MANUFACTURER                 0x90U
#define SPI_FLASH_CMD_JEDEC_ID                          0x9FU
#define SPI_FLASH_CMD_READ_UNIQUE_ID                    0x4BU
#define SPI_FLASH_CMD_READ_DATA                         0x03U
#define SPI_FLASH_CMD_FAST_READ                         0x0BU
#define SPI_FLASH_CMD_PAGE_PROGRAM                      0x02U
#define SPI_FLASH_CMD_SECTOR_ERASE                      0x20U
#define SPI_FLASH_CMD_BLOCK_ERASE_32K                   0x52U
#define SPI_FLASH_CMD_BLOCK_ERASE_64K                   0xD8U
#define SPI_FLASH_CMD_CHIP_ERASE                        0xC7U
#define SPI_FLASH_CMD_READ_STATUS_REG1                  0x05U
#define SPI_FLASH_CMD_WRITE_STATUS_REG1                 0x01U
#define SPI_FLASH_CMD_READ_STATUS_REG2                  0x35U
#define SPI_FLASH_CMD_WRITE_STATUS_REG2                 0x31U
#define SPI_FLASH_CMD_READ_STATUS_REG3                  0x15U
#define SPI_FLASH_CMD_WRITE_STATUS_REG3                 0x11U
#define SPI_FLASH_CMD_READ_SFDP_REG                     0x5AU
#define SPI_FLASH_CMD_ERASE_SECURITY_REG                0x44U
#define SPI_FLASH_CMD_PROGRAM_SECURITY_REG              0x42U
#define SPI_FLASH_CMD_READ_SECURITY_REG                 0x48U
#define SPI_FLASH_CMD_GLOBAL_BLOCK_LOCK                 0x7EU
#define SPI_FLASH_CMD_GLOBAL_BLOCK_UNLOCK               0x98U
#define SPI_FLASH_CMD_READ_BLOCK_LOCK                   0x3DU
#define SPI_FLASH_CMD_INDIVIDUAL_BLOCK_LOCK             0x36U
#define SPI_FLASH_CMD_INDIVIDUAL_BLOCK_UNLOCK           0x39U
#define SPI_FLASH_CMD_ERASE_PROGRAM_SUSPEND             0x75U
#define SPI_FLASH_CMD_ERASE_PROGRAM_RESUME              0x7AU
#define SPI_FLASH_CMD_POWER_DOWN                        0xB9U
#define SPI_FLASH_CMD_ENABLE_RESET                      0x66U
#define SPI_FLASH_CMD_RESET_DEVICE                      0x99U
#define SPI_FLASH_CMD_ENTER_QSPI_MODE                   0x38U
#define SPI_FLASH_CMD_FAST_READ_DUAL_OUTPUT             0x3BU
#define SPI_FLASH_CMD_FAST_READ_DUAL_IO                 0xBBU
#define SPI_FLASH_CMD_DEVICE_ID_DUAL_IO                 0x92U
#define SPI_FLASH_CMD_QUAD_PAGE_PROGRAM                 0x32U
#define SPI_FLASH_CMD_FAST_READ_QUAD_OUTPUT             0x6BU
#define SPI_FLASH_CMD_DEVICE_ID_QUAD_IO                 0x94U
#define SPI_FLASH_CMD_FAST_READ_QUAD_IO                 0xEBU
#define SPI_FLASH_CMD_SET_BURST_WITH_WRAP               0x77U

#define SPI_FLASH_STATUS1_BUSY                          (0x01U << 0U)
#define SPI_FLASH_STATUS1_WRITE_ENABLE_LATCH            (0x01U << 1U)
#define SPI_FLASH_STATUS1_BLOCK_PROTECT_0               (0x01U << 2U)
#define SPI_FLASH_STATUS1_BLOCK_PROTECT_1               (0x01U << 3U)
#define SPI_FLASH_STATUS1_BLOCK_PROTECT_2               (0x01U << 4U)
#define SPI_FLASH_STATUS1_TOP_BOTTON_PROTECT            (0x01U << 5U)
#define SPI_FLASH_STATUS1_SECTOR_PROTECT                (0x01U << 6U)
#define SPI_FLASH_STATUS1_STATUS_REG_PROTECT            (0x01U << 7U)

#define SPI_FLASH_DUMMY_DATA 0xFFU

#define SPI_FLASH_SECTOR_SIZE (1024U * 4U)
#define SPI_FLASH_PAGE_SIZE (256U)

#define SPI_FLASH_MID 0xEFU
#define SPI_FLASH_DID 0x16U

typedef struct _spi_flash
{
    spi_dev_t *spi_dev;
} spi_flash_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

void SPI_Flash_CreateHandle(spi_flash_t *flash, spi_dev_t *spi_dev);
void SPI_Flash_Reset(spi_flash_t *flash);
uint8_t SPI_Flash_GetStatus(spi_flash_t *flash, uint8_t statusReg);
void SPI_Flash_GetID(spi_flash_t *flash, uint8_t *MID, uint8_t *DID);
void SPI_Flash_FastRead(spi_flash_t *flash, uint32_t address, uint8_t *data, uint32_t dataSize);
void SPI_Flash_FastReadDualOutput(spi_flash_t *flash, uint32_t address, uint8_t *data, uint32_t dataSize);
void SPI_Flash_FastReadQuadOutput(spi_flash_t *flash, uint32_t address, uint8_t *data, uint32_t dataSize);
void SPI_Flash_FastReadDualIO(spi_flash_t *flash, uint32_t address, uint8_t *data, uint32_t dataSize);
void SPI_Flash_FastReadQuadIO(spi_flash_t *flash, uint32_t address, uint8_t *data, uint32_t dataSize);
void SPI_Flash_EraseSector(spi_flash_t *flash, uint32_t address);
void SPI_Flash_ProgramPage(spi_flash_t *flash, uint32_t address, const uint8_t *data, uint32_t dataSize);
void SPI_Flash_QuadInputProgramPage(spi_flash_t *flash, uint32_t address, const uint8_t *data, uint32_t dataSize);
void SPI_Flash_Lock(spi_flash_t *flash, bool lock);

#if defined(__cplusplus)
extern "C" }
#endif

#endif