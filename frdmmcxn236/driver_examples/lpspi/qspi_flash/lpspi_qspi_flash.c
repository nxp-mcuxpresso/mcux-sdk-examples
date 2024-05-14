/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_lpspi.h"
#include "spi_dev.h"
#include "spi_flash.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_FLASH_SPI (LPSPI6)
#define LPSPI_MASTER_CLK_FREQ CLOCK_GetLPFlexCommClkFreq(6u)
/* The flash can use up to 133MHz clock, chose the one as fast as possible. */
#define LPSPI_BAUDRATE (30 * 1000 * 1000)

#define APP_LPSPI_CS_GPIO GPIO3
#define APP_LPSPI_CS_PIN  23


#define APP_FLASH_SECTOR_ADDR (SPI_FLASH_SECTOR_SIZE * 1U) /* Use the second sector in this example. */
#define APP_FLASH_PAGE_0_ADDR (APP_FLASH_SECTOR_ADDR + 0 * SPI_FLASH_PAGE_SIZE) /* The number 0 page in this example. */
#define APP_FLASH_PAGE_1_ADDR (APP_FLASH_SECTOR_ADDR + 1 * SPI_FLASH_PAGE_SIZE) /* The number 1 page in this example. */
#define APP_FLASH_PAGE_2_ADDR (APP_FLASH_SECTOR_ADDR + 2 * SPI_FLASH_PAGE_SIZE) /* The number 2 page in this example. */
#define APP_FLASH_PAGE_3_ADDR (APP_FLASH_SECTOR_ADDR + 3 * SPI_FLASH_PAGE_SIZE) /* The number 3 page in this example. */
#define APP_FLASH_PAGE_4_ADDR (APP_FLASH_SECTOR_ADDR + 4 * SPI_FLASH_PAGE_SIZE) /* The number 4 page in this example. */
#define APP_FLASH_PAGE_5_ADDR (APP_FLASH_SECTOR_ADDR + 5 * SPI_FLASH_PAGE_SIZE) /* The number 5 page in this example. */

/*******************************************************************************
 * Variables
 ******************************************************************************/


static uint8_t app_write_data[SPI_FLASH_PAGE_SIZE * 2];
static uint8_t app_read_data[SPI_FLASH_SECTOR_SIZE];

static spi_flash_t flash;
static spi_dev_t spi_dev;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void SPI_Flash_PullCS(uint8_t level);
void APP_Init(void);
uint32_t APP_Run(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
void SPI_Flash_PullCS(uint8_t level)
{
    GPIO_PinWrite(APP_LPSPI_CS_GPIO, APP_LPSPI_CS_PIN, level);
}

static void APP_InitLpspiClock(void)
{
    /* SPI clock */
    CLOCK_SetClkDiv(kCLOCK_DivPllClk, 1u);
    CLOCK_AttachClk(kPLL0_to_PLLCLKDIV);

    CLOCK_SetClkDiv(kCLOCK_DivFlexcom6Clk, 1u);
    CLOCK_AttachClk(kPLL_DIV_to_FLEXCOMM6);
}


int main(void)
{
    uint32_t errorCnt = 0;

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    APP_InitLpspiClock();

    PRINTF("LPSPI QSPI Flash Example:\r\n");

    APP_Init();
    errorCnt = APP_Run();

    if (errorCnt > 0)
    {
        PRINTF("Example failed\r\n");
    }
    else
    {
        PRINTF("Example pass\r\n");
    }

    while (1)
    {
    }
}

void APP_Init(void)
{
    uint32_t srcClock_Hz;
    lpspi_master_config_t masterConfig;

    /*Master config*/
    LPSPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate                      = LPSPI_BAUDRATE;
    masterConfig.pcsToSckDelayInNanoSec        = 1000000000U / (masterConfig.baudRate * 2U);
    masterConfig.lastSckToPcsDelayInNanoSec    = 1000000000U / (masterConfig.baudRate * 2U);
    masterConfig.betweenTransferDelayInNanoSec = 1000000000U / (masterConfig.baudRate * 2U);
    masterConfig.dataOutConfig                 = kLpspiDataOutTristate;
    masterConfig.pcsFunc                       = kLPSPI_PcsAsData;

    srcClock_Hz = LPSPI_MASTER_CLK_FREQ;
    LPSPI_MasterInit(APP_FLASH_SPI, &masterConfig, srcClock_Hz);

    /*
     * Init the SPI.
     * The SPI flash needs PCS always LOW during one flash operation.
     * LPSPI can acheive this using PCS continuous mode. Set TCR[CONT] for the
     * first LPSPI frame, set TCR[CONTC] for the last LPSPI frame, for the other
     * LPSPI frames, set both TCR[CONT] and TCR[CONTC]. This will result the code
     * complex, to simplify it, this example use GPIO to toggle CS pin.
     */
    SPI_DEV_Init(&spi_dev, APP_FLASH_SPI, SPI_Flash_PullCS);
    /* Init SPI flash. */
    SPI_Flash_CreateHandle(&flash, &spi_dev);
}

uint32_t APP_Run(void)
{
    uint32_t errorCnt = 0U;

    /*
     * Init data, the data for each page shall be different,
     * in case the data is read from a wrong page, but still check pass.
     */
    for (uint32_t i=0; i<sizeof(app_write_data); i++)
    {
        app_write_data[i] = 7 * i;
    }

    /************************************************
     * Reset the flash
     ***********************************************/
    SPI_Flash_Reset(&flash);
    SDK_DelayAtLeastUs(60, SystemCoreClock);

    /************************************************
     * Test the flash ID
     ***********************************************/
    uint8_t MID, DID;
    PRINTF("Get the flash ID\r\n");
    SPI_Flash_GetID(&flash, &MID, &DID);
    PRINTF("Manufacturer ID: 0x%x\r\n", MID);
    PRINTF("Device ID: 0x%x\r\n", DID);

    if ((SPI_FLASH_MID != MID) || (SPI_FLASH_DID != DID))
    {
        PRINTF("Wrong device ID\r\n");
        errorCnt++;
    }

    SPI_Flash_Lock(&flash, false);

    /************************************************
     * Erase a sector
     * The sector is used in later programming test.
     ***********************************************/
    PRINTF("Erase the sector: 0x%x ~ 0x%x\r\n", APP_FLASH_SECTOR_ADDR, APP_FLASH_SECTOR_ADDR + SPI_FLASH_SECTOR_SIZE);
    SPI_Flash_EraseSector(&flash, APP_FLASH_SECTOR_ADDR);

    /* Confirm the sector is erased. */
    memset(app_read_data, 0, SPI_FLASH_SECTOR_SIZE);
    SPI_Flash_FastReadQuadIO(&flash, APP_FLASH_SECTOR_ADDR, app_read_data, SPI_FLASH_SECTOR_SIZE);
    for (uint32_t i=0; i<SPI_FLASH_SECTOR_SIZE; i++)
    {
        if (0xFFU != app_read_data[i])
        {
            PRINTF("Sector erase failed at byte %d: desired value 0x%x, actual value 0x%x\r\n", i, 0xFFU, app_read_data[i]);
            errorCnt++;
            break;
        }
    }

    /************************************************
     * Programming test
     * Test 1-wire and 4-wire input modes.
     ***********************************************/
    /* Program a page in one step. */
    PRINTF("Program address 0x%x with size 0x%x\r\n", APP_FLASH_PAGE_0_ADDR, SPI_FLASH_PAGE_SIZE);
    SPI_Flash_ProgramPage(&flash, APP_FLASH_PAGE_0_ADDR, app_write_data, SPI_FLASH_PAGE_SIZE);
    /* Confirm the page is programmed to desired value. */
    memset(app_read_data, 0, SPI_FLASH_PAGE_SIZE);
    SPI_Flash_FastReadQuadIO(&flash, APP_FLASH_PAGE_0_ADDR, app_read_data, SPI_FLASH_PAGE_SIZE);
    if (memcmp(app_read_data, app_write_data, SPI_FLASH_PAGE_SIZE))
    {
        PRINTF("Flash page program wrong\r\n");
        errorCnt++;
    }

    /* Program a page in one step use quad input mode. */
    PRINTF("Program address 0x%x with size 0x%x\r\n", APP_FLASH_PAGE_1_ADDR, SPI_FLASH_PAGE_SIZE);
    SPI_Flash_QuadInputProgramPage(&flash, APP_FLASH_PAGE_1_ADDR, app_write_data, SPI_FLASH_PAGE_SIZE);
    /* Confirm the page is programmed to desired value. */
    memset(app_read_data, 0, SPI_FLASH_PAGE_SIZE);
    SPI_Flash_FastReadQuadIO(&flash, APP_FLASH_PAGE_1_ADDR, app_read_data, SPI_FLASH_PAGE_SIZE);
    if (memcmp(app_read_data, app_write_data, SPI_FLASH_PAGE_SIZE))
    {
        PRINTF("Flash page program wrong in quad mode\r\n");
        errorCnt++;
    }

    /* Program a page in two steps. */
    PRINTF("Program address 0x%x with size 0x%x\r\n", APP_FLASH_PAGE_2_ADDR, SPI_FLASH_PAGE_SIZE / 2);
    SPI_Flash_ProgramPage(&flash, APP_FLASH_PAGE_2_ADDR, app_write_data, SPI_FLASH_PAGE_SIZE / 2);
    PRINTF("Program address 0x%x with size 0x%x\r\n", APP_FLASH_PAGE_2_ADDR + (SPI_FLASH_PAGE_SIZE / 2), SPI_FLASH_PAGE_SIZE / 2);
    SPI_Flash_ProgramPage(&flash, APP_FLASH_PAGE_2_ADDR + (SPI_FLASH_PAGE_SIZE / 2), app_write_data + (SPI_FLASH_PAGE_SIZE / 2), SPI_FLASH_PAGE_SIZE / 2);
    /* Confirm the page is programmed to desired value. */
    memset(app_read_data, 0, SPI_FLASH_PAGE_SIZE);
    SPI_Flash_FastReadQuadIO(&flash, APP_FLASH_PAGE_2_ADDR, app_read_data, SPI_FLASH_PAGE_SIZE);
    if (memcmp(app_read_data, app_write_data, SPI_FLASH_PAGE_SIZE))
    {
        PRINTF("Flash page program wrong\r\n");
        errorCnt++;
    }

    /* Program a page in two steps in quad mode. */
    PRINTF("Program address 0x%x with size 0x%x\r\n", APP_FLASH_PAGE_3_ADDR, SPI_FLASH_PAGE_SIZE / 2);
    SPI_Flash_QuadInputProgramPage(&flash, APP_FLASH_PAGE_3_ADDR, app_write_data, SPI_FLASH_PAGE_SIZE / 2);
    PRINTF("Program address 0x%x with size 0x%x\r\n", APP_FLASH_PAGE_3_ADDR + (SPI_FLASH_PAGE_SIZE / 2), SPI_FLASH_PAGE_SIZE / 2);
    SPI_Flash_QuadInputProgramPage(&flash, APP_FLASH_PAGE_3_ADDR + (SPI_FLASH_PAGE_SIZE / 2), app_write_data + (SPI_FLASH_PAGE_SIZE / 2), SPI_FLASH_PAGE_SIZE / 2);
    /* Confirm the page is programmed to desired value. */
    memset(app_read_data, 0, SPI_FLASH_PAGE_SIZE);
    SPI_Flash_FastReadQuadIO(&flash, APP_FLASH_PAGE_3_ADDR, app_read_data, SPI_FLASH_PAGE_SIZE);
    if (memcmp(app_read_data, app_write_data, SPI_FLASH_PAGE_SIZE))
    {
        PRINTF("Flash page program wrong\r\n");
        errorCnt++;
    }

    /************************************************
     * Reading methods test
     * Test different read modes.
     ***********************************************/
    SPI_Flash_QuadInputProgramPage(&flash, APP_FLASH_PAGE_4_ADDR, app_write_data, SPI_FLASH_PAGE_SIZE);
    SPI_Flash_QuadInputProgramPage(&flash, APP_FLASH_PAGE_4_ADDR + SPI_FLASH_PAGE_SIZE, app_write_data + SPI_FLASH_PAGE_SIZE, SPI_FLASH_PAGE_SIZE);

    PRINTF("Test SPI_Flash_FastRead\r\n");
    memset(app_read_data, 0, 2* SPI_FLASH_PAGE_SIZE);
    SPI_Flash_FastRead(&flash, APP_FLASH_PAGE_4_ADDR, app_read_data, 2 * SPI_FLASH_PAGE_SIZE);
    if (0 != memcmp(app_read_data, app_write_data, 2 * SPI_FLASH_PAGE_SIZE))
    {
        PRINTF("Failed\r\n");
        errorCnt++;
    }

    PRINTF("Test SPI_Flash_FastReadDualOutput\r\n");
    memset(app_read_data, 0, 2* SPI_FLASH_PAGE_SIZE);
    SPI_Flash_FastReadDualOutput(&flash, APP_FLASH_PAGE_4_ADDR, app_read_data, 2 * SPI_FLASH_PAGE_SIZE);
    if (0 != memcmp(app_read_data, app_write_data, 2 * SPI_FLASH_PAGE_SIZE))
    {
        PRINTF("Failed\r\n");
        errorCnt++;
    }

    PRINTF("Test SPI_Flash_FastReadQuadOutput\r\n");
    memset(app_read_data, 0, 2* SPI_FLASH_PAGE_SIZE);
    SPI_Flash_FastReadQuadOutput(&flash, APP_FLASH_PAGE_4_ADDR, app_read_data, 2 * SPI_FLASH_PAGE_SIZE);
    if (0 != memcmp(app_read_data, app_write_data, 2 * SPI_FLASH_PAGE_SIZE))
    {
        PRINTF("Failed\r\n");
        errorCnt++;
    }

    PRINTF("Test SPI_Flash_FastReadDualIO\r\n");
    memset(app_read_data, 0, 2* SPI_FLASH_PAGE_SIZE);
    SPI_Flash_FastReadDualIO(&flash, APP_FLASH_PAGE_4_ADDR, app_read_data, 2 * SPI_FLASH_PAGE_SIZE);
    if (0 != memcmp(app_read_data, app_write_data, 2 * SPI_FLASH_PAGE_SIZE))
    {
        PRINTF("Failed\r\n");
        errorCnt++;
    }

    PRINTF("Test SPI_Flash_FastReadQuadIO\r\n");
    memset(app_read_data, 0, 2* SPI_FLASH_PAGE_SIZE);
    SPI_Flash_FastReadQuadIO(&flash, APP_FLASH_PAGE_4_ADDR, app_read_data, 2 * SPI_FLASH_PAGE_SIZE);
    if (0 != memcmp(app_read_data, app_write_data, 2 * SPI_FLASH_PAGE_SIZE))
    {
        PRINTF("Failed\r\n");
        errorCnt++;
    }

    SPI_Flash_Lock(&flash, true);

    return errorCnt;
}
