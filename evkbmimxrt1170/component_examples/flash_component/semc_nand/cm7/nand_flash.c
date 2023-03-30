/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_nand_flash.h"
#include "fsl_semc.h"
#include "fsl_semc_nand_flash.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#include "fsl_cache.h"

#define EXAMPLE_SEMC                        SEMC
#define EXAMPLE_SEMC_START_ADDRESS          (0x80000000U)
#define EXAMPLE_SEMC_NAND_AXI_START_ADDRESS (0x9E000000U)
#define EXAMPLE_SEMC_NAND_IPG_START_ADDRESS (0x00000000U)
#define FLASH_PAGE_SIZE                     (2048)
#define CACHE_MAINTAIN                      (1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * variables
 ******************************************************************************/
uint8_t mem_writeBuffer[FLASH_PAGE_SIZE];
uint8_t mem_readBuffer[FLASH_PAGE_SIZE] = {0};
nand_handle_t nandHandle;
extern nand_config_t nandConfig;

/*******************************************************************************
 * Code
 ******************************************************************************/
// semc_nand_timing_config_t timingConfig;

void delayUs(uint32_t delay_us)
{
    uint32_t s_tickPerMicrosecond = CLOCK_GetFreq(kCLOCK_CpuClk) / 1000000U;

    // Make sure this value is greater than 0
    if (!s_tickPerMicrosecond)
    {
        s_tickPerMicrosecond = 1;
    }
    delay_us = delay_us * s_tickPerMicrosecond;
    while (delay_us)
    {
        __NOP();
        delay_us--;
    }
}

semc_nand_config_t semcNandConfig = {
    .cePinMux          = kSEMC_MUXCSX0,                       /*!< The CE# pin mux setting. */
    .axiAddress        = EXAMPLE_SEMC_NAND_AXI_START_ADDRESS, /*!< The base address. */
    .axiMemsize_kbytes = 2 * 1024 * 1024,                     /*!< The memory size is 8*1024*2*1024*1024 = 16Gb. */
    .ipgAddress        = EXAMPLE_SEMC_NAND_IPG_START_ADDRESS, /*!< The base address. */
    .ipgMemsize_kbytes = 2 * 1024 * 1024,                     /*!< The memory size is 8*1024*2*1024*1024 = 16Gb. */
    .rdyactivePolarity = kSEMC_RdyActiveLow,                  /*!< Wait ready polarity. */
    .arrayAddrOption   = kSEMC_NandAddrOption_5byte_CA2RA3,
    .edoModeEnabled    = false,                 /*!< Address mode. */
    .columnAddrBitNum  = kSEMC_NandColum_12bit, /*!< 12bit + 1bit to access the spare area. */
    .burstLen          = kSEMC_Nand_BurstLen64, /*!< Burst length. */
    .portSize          = kSEMC_PortSize8Bit,    /*!< Port size. */
    .timingConfig      = NULL,
};

semc_mem_nand_config_t semcMemConfig = {
    .semcNandConfig   = &semcNandConfig,
    .delayUS          = delayUs,
    .onfiVersion      = kNandOnfiVersion_1p0,
    .readyCheckOption = kNandReadyCheckOption_SR,
    .eccCheckType     = kNandEccCheckType_DeviceECC,
};

nand_config_t nandConfig = {
    .memControlConfig = (void *)&semcMemConfig,
    .driverBaseAddr   = (void *)EXAMPLE_SEMC,
};

void BOARD_InitMem(void)
{
    semc_config_t config;

    /* Initializes the MAC configure structure to zero. */
    memset(&config, 0, sizeof(semc_config_t));

    /*
       Get default configuration:
       config->queueWeight.queueaEnable = true;
       config->queueWeight.queuebEnable = true;
    */
    SEMC_GetDefaultConfig(&config);
    /* Disable queue B weight, which is for AXI bus access to SDRAM slave. */
    config.queueWeight.queuebEnable = false;
    /* Initialize SEMC. */
    SEMC_Init(SEMC, &config);
    /* Set SEMC clk source for NAND flash memory controller usage. */
    semcMemConfig.clkSrc_Hz = CLOCK_GetRootClockFreq(kCLOCK_Root_Semc);
}


/*Error trap function*/
void ErrorTrap(void)
{
    PRINTF("\n\rError occurred. Please check the configurations.\n\r");
    while (1)
    {
        ;
    }
}

int main(void)
{
    status_t status;

    /* Hardware Initialization */
    clock_root_config_t config = {0};
    config.mux                 = 7; /* SYS_PLL3_PFD0: 664.62MHz. */
    config.div                 = 4;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    /* Set semc clock to 166 MHz(rootclock/div: 664.62 / 4 = 166.1MHz).
       Note: SEMC max frequecny is 200MHz, its source selects from
        000 - OSC_RC_48M_DIV2(24MHz)
        001 - OSC_24M(24MHz)
        010 - OSC_RC_400M(400MHz)
        011 - OSC_RC_16M(16MHz)
        100 - SYS_PLL1_DIV5(200MHz)
        101 - SYS_PLL2(528MHz)
        110 - SYS_PLL2_PFD2(594MHz)
        111 - SYS_PLL3_PFD0(664.62MHz)
     */
    CLOCK_SetRootClock(kCLOCK_Root_Semc, &config);
    BOARD_InitMem();
    BOARD_InitDebugConsole();

    PRINTF("\r\n***NAND Flash Component Demo Start!***\r\n");

    PRINTF("\r\n***NAND Flash Initialization Start!***\r\n");
    status = Nand_Flash_Init(&nandConfig, &nandHandle);
    if (status != kStatus_Success)
    {
        PRINTF("\r\n***NAND Flash Initialization Failed!***\r\n");
        ErrorTrap();
    }
    PRINTF("\r\n***NAND Flash Initialization Success!***\r\n");

    /* Erase Block */
    PRINTF("\r\n***NAND Flash Erase The First Block Start!***\r\n");
    status = Nand_Flash_Erase_Block(&nandHandle, 0);
    if (status != kStatus_Success)
    {
        PRINTF("\r\n***NAND Flash Erase block Failed!***\r\n");
        ErrorTrap();
    }

    /* Read and check if it is blank. */
    PRINTF("\r\n***NAND Flash Erase Check Start!***\r\n");
    for (uint32_t pageIndex = 0; pageIndex < nandHandle.pagesInBlock; pageIndex++)
    {
#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
        DCACHE_CleanInvalidateByRange(EXAMPLE_SEMC_NAND_AXI_START_ADDRESS, nandHandle.bytesInPageDataArea);
#endif

        status = Nand_Flash_Read_Page(&nandHandle, pageIndex, mem_readBuffer, nandHandle.bytesInPageDataArea);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NAND Flash Read Page Failed!***\r\n");
            ErrorTrap();
        }

        for (uint32_t bytesIndex = 0; bytesIndex < nandHandle.bytesInPageDataArea; bytesIndex++)
        {
            if (mem_readBuffer[bytesIndex] != 0xFF)
            {
                PRINTF("\r\n***NAND Flash Erase block Check Failed!***\r\n");
                ErrorTrap();
            }
        }

        PRINTF("\r\n***NAND Flash Erase block Success!***\r\n");

        /* Program the page data. */
        PRINTF("\r\n***NAND Flash Page Program Start!***\r\n");
        /* Initialize the write buffers. */
        memset(mem_writeBuffer, 0xaa, sizeof(mem_writeBuffer));
        status = Nand_Flash_Page_Program(&nandHandle, pageIndex, mem_writeBuffer, nandHandle.bytesInPageDataArea);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NAND Flash Page Program Failed!***\r\n");
            ErrorTrap();
        }

        /* Read page data and check if the data read is equal to the data programed. */
        PRINTF("\r\n***NAND Flash Page Read Start!***\r\n");

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
        DCACHE_CleanInvalidateByRange(EXAMPLE_SEMC_NAND_AXI_START_ADDRESS, nandHandle.bytesInPageDataArea);
#endif

        status = Nand_Flash_Read_Page(&nandHandle, pageIndex, mem_readBuffer, nandHandle.bytesInPageDataArea);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NAND Flash Page Read Failed!***\r\n");
            ErrorTrap();
        }

        if (memcmp(mem_writeBuffer, mem_readBuffer, nandHandle.bytesInPageDataArea) != 0)
        {
            PRINTF("\r\n***NAND Flash Page Read Failed!***\r\n");
            ErrorTrap();
        }
    }

    PRINTF("\r\n***NAND Flash Page Read/Write Success!***\r\n");
    while (1)
    {
    }
}
