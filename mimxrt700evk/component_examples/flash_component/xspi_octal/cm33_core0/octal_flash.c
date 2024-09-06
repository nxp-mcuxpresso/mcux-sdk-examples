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

#include "fsl_power.h"
#include "fsl_xspi.h"
#include "fsl_xspi_nor_flash.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FLASH_PAGE_SIZE                 256
#define NOR_FLASH_START_ADDRESS (XSPI0_AMBA_BASE)

xspi_memory_config_t xspiMemConfig =
{
    .enableXspiDoze = false,
    .ptrCustomLut = NULL,
    .numPadUsed = kXSPI_8PAD,
    .enableClknPad = false,
    .sampleClkConfig.sampleClkSource = kXSPI_SampleClkFromExternalDQS,  /*!< Device support Data strobe signal.  */
    .sampleClkConfig.enableDQSLatency = false,
    .sampleClkConfig.dllConfig.dllMode = kXSPI_AutoUpdateMode,
    .sampleClkConfig.dllConfig.useRefValue = true,
    .sampleClkConfig.dllConfig.enableCdl8 = false,
    .addrMode = kXSPI_DeviceByteAddressable,
    .xspiRootClk = 400000000,  /*!< 400MHz */
    .tgId = kXSPI_TargetGroup0,
    
    .ptrXspiNorAhbAccessConfig = NULL,
    .ptrXspiNorIPAccessConfig = NULL,
};

nor_config_t norConfig = {
  .memControlConfig = &xspiMemConfig,
  .driverBaseAddr = (void *)XSPI0_BASE,
};
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

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitNorFlashPins();

    CLOCK_AttachClk(kMAIN_PLL_PFD1_to_XSPI0);
    CLOCK_SetClkDiv(kCLOCK_DivXspi0Clk, 1u);     /*400MHz*/
    
    POWER_DisablePD(kPDRUNCFG_APD_XSPI0);
    POWER_DisablePD(kPDRUNCFG_PPD_XSPI0);
    POWER_ApplyPD();


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
