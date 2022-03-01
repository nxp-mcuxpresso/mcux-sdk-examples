/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <string.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_nor_flash.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
#include "fsl_cache.h"
#endif

#include "fsl_flexspi.h"
#include "fsl_flexspi_nor_flash.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_FLEXSPI           FLEXSPI0
#define EXAMPLE_FLEXSPI_AMBA_BASE FlexSPI0_AMBA_BASE
#define FLASH_SIZE                0x10000 /* 64Mb/KByte */
#define FLASH_PAGE_SIZE           256
#define NOR_FLASH_START_ADDRESS   (20U * 0x1000U)

#ifndef DEMO_ENABLE_FLASH_CHIP_ERASE
#define DEMO_ENABLE_FLASH_CHIP_ERASE 0
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * variables
 ******************************************************************************/

uint8_t mem_writeBuffer[FLASH_PAGE_SIZE];
uint8_t mem_readBuffer[FLASH_PAGE_SIZE] = {0};
extern nor_config_t norConfig;
nor_handle_t norHandle = {NULL};

/*******************************************************************************
 * Code
 ******************************************************************************/
flexspi_mem_config_t mem_Config = {
    .deviceConfig =
        {
            .flexspiRootClk       = 99000000,
            .flashSize            = FLASH_SIZE,
            .CSIntervalUnit       = kFLEXSPI_CsIntervalUnit1SckCycle,
            .CSInterval           = 2,
            .CSHoldTime           = 3,
            .CSSetupTime          = 3,
            .dataValidTime        = 2,
            .columnspace          = 0,
            .enableWordAddress    = 0,
            .AWRSeqIndex          = 0,
            .AWRSeqNumber         = 0,
            .ARDSeqIndex          = NOR_CMD_LUT_SEQ_IDX_READ,
            .ARDSeqNumber         = 1,
            .AHBWriteWaitUnit     = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
            .AHBWriteWaitInterval = 0,
        },
    .devicePort         = kFLEXSPI_PortA1,
    .deviceType         = kSerialNorCfgOption_DeviceType_MacronixOctalDDR,
    .CurrentCommandMode = kSerialNorCommandMode_1_1_1,
    .transferMode       = kSerialNorTransferMode_DDR,
    .quadMode           = kSerialNorQuadMode_NotConfig,
    .enhanceMode        = kSerialNorEnhanceMode_Disabled,
    .commandPads        = kFLEXSPI_8PAD,
    .queryPads          = kFLEXSPI_1PAD,
    .busyOffset         = 0,
    .busyBitPolarity    = 0,

};

nor_config_t norConfig = {
    .memControlConfig = &mem_Config,
    .driverBaseAddr   = EXAMPLE_FLEXSPI,
};



/*Error trap function*/
void ErrorTrap(void)
{
    PRINTF("\n\rError Occured. Please check the configurations.\n\r");
    while (1)
    {
        ;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    status_t status;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    POWER_DisablePD(kPDRUNCFG_APD_FLEXSPI0_SRAM);
    POWER_DisablePD(kPDRUNCFG_PPD_FLEXSPI0_SRAM);
    POWER_ApplyPD();

    CLOCK_AttachClk(kAUX0_PLL_to_FLEXSPI0_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivFlexspi0Clk, 4);

    PRINTF("\r\n***NOR Flash Component Demo Start!***\r\n");

    PRINTF("\r\n***NOR Flash Initialization Start!***\r\n");
    status = Nor_Flash_Init(&norConfig, &norHandle);
    if (status != kStatus_Success)
    {
        PRINTF("\r\n***NOR Flash Initialization Failed!***\r\n");
        ErrorTrap();
    }
    PRINTF("\r\n***NOR Flash Initialization Success!***\r\n");

#if (defined(DEMO_ENABLE_FLASH_CHIP_ERASE) && (DEMO_ENABLE_FLASH_CHIP_ERASE == 0x01U))
    /* Erase whole chip */
    PRINTF("\r\n***NOR Flash Erase Chip Start!***\r\n");
    status = Nor_Flash_Erase_Chip(&norHandle);
    if (status != kStatus_Success)
    {
        PRINTF("\r\n***NOR Flash Erase Chip Failed!***\r\n");
        ErrorTrap();
    }
#endif

    for (uint32_t pageIndex = 0; pageIndex < (norHandle.bytesInSectorSize / norHandle.bytesInPageSize); pageIndex++)
    {
        uint32_t address = NOR_FLASH_START_ADDRESS + norHandle.bytesInPageSize * pageIndex;

        /* Erase Sector */
        status = Nor_Flash_Erase_Sector(&norHandle, address);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Erase Sector Failed!***\r\n");
            ErrorTrap();
        }

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
        DCACHE_InvalidateByRange(EXAMPLE_FLEXSPI_AMBA_BASE + address, norHandle.bytesInPageSize);
#endif

        status = Nor_Flash_Read(&norHandle, address, mem_readBuffer, norHandle.bytesInPageSize);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Read Page Failed!***\r\n");
            ErrorTrap();
        }

        for (uint32_t bytesIndex = 0; bytesIndex < norHandle.bytesInPageSize; bytesIndex++)
        {
            if (mem_readBuffer[bytesIndex] != 0xFF)
            {
                PRINTF("\r\n***NOR Flash Erase Sector Failed!***\r\n");
                ErrorTrap();
            }
        }

        /* Program the page data. */
        /* Initialize the write buffers. */
        for (uint32_t i = 0; i < norHandle.bytesInPageSize; i++)
        {
            mem_writeBuffer[i] = i;
        }

        PRINTF("\r\n");

        status = Nor_Flash_Page_Program(&norHandle, address, mem_writeBuffer);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Page %d Program Failed!***\r\n", pageIndex);
            ErrorTrap();
        }

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
        DCACHE_InvalidateByRange(EXAMPLE_FLEXSPI_AMBA_BASE + address, norHandle.bytesInPageSize);
#endif

        /* Read page data and check if the data read is equal to the data programed. */
        status = Nor_Flash_Read(&norHandle, address, mem_readBuffer, norHandle.bytesInPageSize);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Page %d Read Failed!***\r\n", pageIndex);
            ErrorTrap();
        }

        if (memcmp(mem_writeBuffer, mem_readBuffer, norHandle.bytesInPageSize) != 0)
        {
            PRINTF("\r\n***NOR Flash Page %d Read/Write Failed!***\r\n", pageIndex);
            ErrorTrap();
        }

        PRINTF("\r\n***NOR Flash Page %d Read/Write Success!***\r\n", pageIndex);

        /* Erase Sector */
        status = Nor_Flash_Erase_Sector(&norHandle, address);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Erase Sector Failed!***\r\n");
            ErrorTrap();
        }

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
        DCACHE_InvalidateByRange(EXAMPLE_FLEXSPI_AMBA_BASE + address, norHandle.bytesInPageSize);
#endif

        status = Nor_Flash_Read(&norHandle, address, mem_readBuffer, norHandle.bytesInPageSize);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Page Read Failed!***\r\n");
            ErrorTrap();
        }

        for (uint32_t bytesIndex = 0; bytesIndex < norHandle.bytesInPageSize; bytesIndex++)
        {
            if (mem_readBuffer[bytesIndex] != 0xFF)
            {
                PRINTF("\r\n***NOR Flash Erase Block Failed!***\r\n");
                ErrorTrap();
            }
        }
    }

    PRINTF("\r\n***NOR Flash All Pages Read/Write Success!***\r\n");
    while (1)
    {
    }
}
