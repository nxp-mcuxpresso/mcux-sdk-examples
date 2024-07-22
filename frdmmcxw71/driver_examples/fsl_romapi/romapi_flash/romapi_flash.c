/*
 * Copyright 2018 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_flash_api.h"
#include "fsl_lpspi_flash.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FLASH_NS   FMU0
#define BUFFER_LEN 4

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void error_trap(void);
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

/*******************************************************************************
 * Code
 ******************************************************************************/

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
    PRINTF("\r\n End of PFlash Example \r\n");
    while (1)
    {
    }
}

/*!
 * @brief Use Standard Software Drivers (SSD) to modify pflash.
 *
 * @details This function uses SSD to demonstrate flash mode:
 *            + Check pflash information.
 *            + Erase a sector and verify.
 *            + Program a sector and verify.
 */
int main(void)
{
    status_t result;    /* Return code from each flash driver function */
    uint32_t destAdrss; /* Address of the target location */

    uint32_t pflashBlock0Base = 0;
    uint32_t pflashBlock0Size = 0;
    uint32_t pflashSectorSize = 0;
    uint32_t pflashBlockCount = 0;
    uint32_t pflashTotalSize  = 0;
    /* Init hardware */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Clean up Flash, Cache driver Structure*/
    memset(&s_flashDriver, 0, sizeof(flash_config_t));

    /* Setup flash driver structure for device and initialize variables. */
    result = FLASH_Init(&s_flashDriver);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }

#if defined(SMSCM_CACHE_CLEAR_MASK) && SMSCM_CACHE_CLEAR_MASK
    /* disable flash cache/Prefetch */
    FLASH_CACHE_Disable();
#endif /* SMSCM_CACHE_CLEAR_MASK */

    /* Get flash properties*/
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflash0BlockBaseAddr, &pflashBlock0Base);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflash0BlockSize, &pflashBlock0Size);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflash0SectorSize, &pflashSectorSize);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflash0BlockCount, &pflashBlockCount);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflash0TotalSize, &pflashTotalSize);

    /* print welcome message */
    PRINTF("\r\n PFlash Example Start \r\n");
    /* Print flash information - PFlash. */
    PRINTF("\r\n PFlash Information: ");
    PRINTF("\r\n Program Flash block base address:\t%d, Hex: (0x%x)", (pflashBlock0Base / 1024), pflashBlock0Base);
    PRINTF("\r\n Program Flash block Size:\t\t%d KB, Hex: (0x%x)", (pflashBlock0Size / 1024), pflashBlock0Size);
    PRINTF("\r\n Program Flash block Sector Size:\t%d KB, Hex: (0x%x)", (pflashSectorSize / 1024), pflashSectorSize);
    PRINTF("\r\n Program Flash block count Size:\t%d", pflashBlockCount);
    PRINTF("\r\n Total Program Flash Size:\t\t%d KB, Hex: (0x%x)\r\n", (pflashTotalSize / 1024), pflashTotalSize);

    PRINTF("\r\n Erase a sector of flash");
/*
SECTOR_INDEX_FROM_END = 1 means the last sector,
SECTOR_INDEX_FROM_END = 2 means (the last sector - 1) ...
*/
#ifndef SECTOR_INDEX_FROM_END
#define SECTOR_INDEX_FROM_END 1U
#endif
    /* Erase a sector from destAdrss. */
    destAdrss = pflashBlock0Base + (pflashBlock0Size - (SECTOR_INDEX_FROM_END * pflashSectorSize));
    result    = FLASH_Erase(&s_flashDriver, FLASH_NS, destAdrss, pflashSectorSize, kFLASH_ApiEraseKey);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }

    /* Verify sector if it's been erased. */
    result = FLASH_VerifyEraseSector(&s_flashDriver, FLASH_NS, destAdrss, pflashSectorSize);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }
    /* Print message for user. */
    PRINTF("\r\n Successfully Erased Sector 0x%x -> 0x%x\r\n", destAdrss, (destAdrss + pflashSectorSize));

    /* Print message for user. */
    PRINTF("\r\n Program a buffer to a phrase of flash ");
    /* Prepare user buffer. */
    for (uint32_t i = 0; i < BUFFER_LEN; i++)
    {
        s_buffer[i] = i;
    }

    /* Program user buffer into flash*/
    result = FLASH_Program(&s_flashDriver, FLASH_NS, destAdrss, (uint32_t *)s_buffer, sizeof(s_buffer));
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }

    /* Verify programming by reading back from flash directly*/
    for (uint32_t i = 0; i < BUFFER_LEN; i++)
    {
        s_buffer_rbc[i] = *(volatile uint32_t *)(destAdrss + i * 4);
        if (s_buffer_rbc[i] != s_buffer[i])
        {
            error_trap();
        }
    }

    PRINTF("\r\n Successfully Programmed and Verified Location 0x%x -> 0x%x \r\n", destAdrss,
           (destAdrss + sizeof(s_buffer)));

    app_finalize();

    return 0;
}
