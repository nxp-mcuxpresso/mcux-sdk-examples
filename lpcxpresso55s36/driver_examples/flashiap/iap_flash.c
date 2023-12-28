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
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

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
    uint32_t systemFreqHz     = 0U;
    uint32_t pflashBlockBase  = 0U;
    uint32_t pflashTotalSize  = 0U;
    uint32_t pflashSectorSize = 0U;
    uint32_t PflashPageSize   = 0U;

    /* Init board hardware. */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
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

    /* Adjust the clock cycle for accessing the flash memory according to the system clock. */
    systemFreqHz = CLOCK_GetFreq(kCLOCK_CoreSysClk);
    PRINTF("\r\n Config flash memory access time. \r\n");
    CLOCK_SetFLASHAccessCyclesForFreq(systemFreqHz);

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
PFLASH_PAGE_INDEX = 1 means the last page,
PFLASH_PAGE_INDEX = 2 means (the last page -1 )...
*/
#ifndef PFLASH_PAGE_INDEX
#define PFLASH_PAGE_INDEX 10U
#endif

    destAdrss = pflashBlockBase + (pflashTotalSize - (PFLASH_PAGE_INDEX * PflashPageSize));

    PRINTF("\r\n Erase a page of flash");
#if defined(FSL_SUPPORT_ERASE_SECTOR_NON_BLOCKING) && FSL_SUPPORT_ERASE_SECTOR_NON_BLOCKING
    PRINTF("\r\n Calling FLASH_EraseNonBlocking() API.");
    status = FLASH_EraseNonBlocking(&s_flashDriver, destAdrss, PflashPageSize, kFLASH_ApiEraseKey);
    if (status != kStatus_Success)
    {
        error_trap();
    }
#else
    status = FLASH_Erase(&s_flashDriver, destAdrss, PflashPageSize, kFLASH_ApiEraseKey);
    if (status != kStatus_Success)
    {
        error_trap();
    }
#endif /* FSL_SUPPORT_ERASE_SECTOR_NON_BLOCKING */

#if defined(FSL_SUPPORT_ERASE_SECTOR_NON_BLOCKING) && FSL_SUPPORT_ERASE_SECTOR_NON_BLOCKING
    /* Delay 4ms to wait for the erase to complete. */
    SysTick_DelayTicks(4U);

    /* Before operating the flash, check whether the previous operation command is completed.*/
    status = FLASH_GetCommandState(&s_flashDriver);
    if (status != kStatus_Success)
    {
        error_trap();
    }
#endif /* FSL_SUPPORT_ERASE_SECTOR_NON_BLOCKING */

    /* Verify if the given flash range is successfully erased. */
    PRINTF("\r\n Calling FLASH_VerifyErase() API.");
    status = FLASH_VerifyErase(&s_flashDriver, destAdrss, PflashPageSize);
    if (status == kStatus_Success)
    {
        PRINTF("\r\n Successfully erased page: 0x%x -> 0x%x\r\n", destAdrss, (destAdrss + PflashPageSize));
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

    /* Verify if the given flash region is successfully programmed with given data */
    PRINTF("\r\n Calling FLASH_VerifyProgram() API.");
    status = FLASH_VerifyProgram(&s_flashDriver, destAdrss, sizeof(s_buffer), (const uint8_t *)s_buffer, &failedAddress,
                                 &failedData);
    if (status != kStatus_Success)
    {
        error_trap();
    }

    /* Check if the flash page is blank before reading to avoid hardfault. */
    status = FLASH_VerifyErase(&s_flashDriver, destAdrss, PflashPageSize);
    if (status == kStatus_Success)
    {
        PRINTF("Error: trying to Read blank flash page!\n");
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
    status = FLASH_Erase(&s_flashDriver, destAdrss, PflashPageSize, kFLASH_ApiEraseKey);

    app_finalize();

    return 0;
}
