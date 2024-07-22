/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_flash.h"
#include "fsl_flash_ffr.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#if defined(LPCAC_INVALIDATE) && LPCAC_INVALIDATE
#include "fsl_cache_lpcac.h"
#endif

#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SECTOR_INDEX_FROM_END 2U /* start from the last 2 page*/
#define BUFFER_LEN 512 / 4

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void error_trap();
void app_finalize(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Flash driver Structure */
static flash_config_t s_flashDriver;
/*! @brief Buffer for program */
static uint32_t s_buffer[BUFFER_LEN];
/*! @brief Buffer for readback */
static uint32_t s_buffer_rbc[BUFFER_LEN];

volatile uint32_t g_systickCounter;

/*******************************************************************************
 * Code
 ******************************************************************************/
void SysTick_Handler(void)
{
    if (g_systickCounter != 0U)
    {
        g_systickCounter--;
    }
}

void SysTick_DelayTicks(uint32_t n)
{
    g_systickCounter = n;
    while (g_systickCounter != 0U)
    {
    }
}

/*
 * @brief Gets called when an error occurs.
 *
 * @details Print error message and trap forever.
 */
void error_trap(void)
{
    PRINTF("\r\n\r\n\r\n\t---- HALTED DUE TO FLASH ERROR! ----");
    while (1)
    {
    }
}

/*
 * @brief Gets called when the app is complete.
 *
 * @details Print finshed message and trap forever.
 */
void app_finalize(void)
{
    /* Print finished message. */
    PRINTF("\r\n End of PFlash Example! \r\n");
    while (1)
    {
    }
}

int main()
{
    status_t status;
    uint32_t destAdrss; /* Address of the target location */
    uint32_t i, failedAddress, failedData;
    uint32_t pflashBlockBase  = 0U;
    uint32_t pflashTotalSize  = 0U;
    uint32_t pflashSectorSize = 0U;
    uint32_t PflashPageSize   = 0U;

    /* Init board hardware. */
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Clean up Flash, Cache driver Structure*/
    memset(&s_flashDriver, 0, sizeof(flash_config_t));

    /* Print basic information for Flash Driver API.*/
    PRINTF("\r\n Flash driver API tree demo application. \r\n");
    /* Initialize flash driver */
    PRINTF("\r\n Initializing flash driver.");
    if (FLASH_Init(&s_flashDriver) != kStatus_Success)
    {
        error_trap();
    }
    PRINTF("\r\n Flash init successfull! \r\n");

    PRINTF("\r\n Initializing FFR driver.");
    if (FFR_Init(&s_flashDriver) != kStatus_Success)
    {
        error_trap();
    }
    PRINTF("\r\n FFR init successfull! \r\n");

    /* Adjust the clock cycle for accessing the flash memory according to the system clock. */
    PRINTF("\r\n Config flash memory access time. \r\n");

    /* Set systick reload value to generate 1ms interrupt */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        error_trap();
    }

    /* Get flash properties kFLASH_ApiEraseKey */
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashBlockBaseAddr, &pflashBlockBase);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashTotalSize, &pflashTotalSize);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashPageSize, &PflashPageSize);

    /* print welcome message */
    PRINTF("\r\n PFlash Information:");
    /* Print flash information - PFlash. */
    PRINTF("\r\n kFLASH_PropertyPflashBlockBaseAddr = 0x%X", pflashBlockBase);
    PRINTF("\r\n kFLASH_PropertyPflashSectorSize = %d", pflashSectorSize);
    PRINTF("\r\n kFLASH_PropertyPflashTotalSize = %d", pflashTotalSize);
    PRINTF("\r\n kFLASH_PropertyPflashPageSize = 0x%X", PflashPageSize);

/*
SECTOR_INDEX_FROM_END = 1 means the last sector,
SECTOR_INDEX_FROM_END = 2 means (the last sector -1 )...
*/
#ifndef SECTOR_INDEX_FROM_END
#define SECTOR_INDEX_FROM_END 2U
#endif

    destAdrss = pflashBlockBase + (pflashTotalSize - (SECTOR_INDEX_FROM_END * pflashSectorSize));

    PRINTF("\r\n Erase a sector of flash");
    status = FLASH_Erase(&s_flashDriver, destAdrss, pflashSectorSize, kFLASH_ApiEraseKey);
    if (status != kStatus_Success)
    {
        error_trap();
    }

#if defined(LPCAC_INVALIDATE) && LPCAC_INVALIDATE
    L1CACHE_InvalidateCodeCache();
#endif

    /* Verify if the given flash range is successfully erased. */
    PRINTF("\r\n Calling FLASH_VerifyErase() API.");
    status = FLASH_VerifyErase(&s_flashDriver, destAdrss, pflashSectorSize);
    if (status == kStatus_Success)
    {
        PRINTF("\r\n Successfully erased sector: 0x%x -> 0x%x\r\n", destAdrss, (destAdrss + pflashSectorSize));
    }
    else
    {
        error_trap();
    }

    /* Prepare user buffer. */
    for (i = 0; i < BUFFER_LEN; i++)
    {
        s_buffer[i] = i;
    }

    /* Start programming specified flash region */
    PRINTF("\r\n Calling FLASH_Program() API.");
    status = FLASH_Program(&s_flashDriver, destAdrss, (uint8_t *)s_buffer, sizeof(s_buffer));
    if (status != kStatus_Success)
    {
        error_trap();
    }

#if defined(LPCAC_INVALIDATE) && LPCAC_INVALIDATE
    L1CACHE_InvalidateCodeCache();
#endif

    /* Verify if the given flash region is successfully programmed with given data */
    PRINTF("\r\n Calling FLASH_VerifyProgram() API.");
    status = FLASH_VerifyProgram(&s_flashDriver, destAdrss, sizeof(s_buffer), (const uint8_t *)s_buffer, &failedAddress,
                                 &failedData);
    if (status != kStatus_Success)
    {
        error_trap();
    }

    /* Verify programming by reading back from flash directly */
    for (uint32_t i = 0; i < BUFFER_LEN; i++)
    {
        s_buffer_rbc[i] = *(volatile uint32_t *)(destAdrss + i * 4);
        if (s_buffer_rbc[i] != s_buffer[i])
        {
            error_trap();
        }
    }

    PRINTF("\r\n Successfully programmed and verified location: 0x%x -> 0x%x \r\n", destAdrss,
           (destAdrss + sizeof(s_buffer)));

    /* resume flash memory status */
    status = FLASH_Erase(&s_flashDriver, destAdrss, pflashSectorSize, kFLASH_ApiEraseKey);

    app_finalize();

    return 0;
}
