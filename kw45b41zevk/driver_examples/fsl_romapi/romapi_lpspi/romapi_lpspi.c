/*
 * Copyright 2021 NXP
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
#define LPSPI_BAUD_RATE        4000000
#define LPSPI_FLASH_BASE       0x0
#define LPSPI_FLASH_PAGE_SIZE  0x0
#define LPSPI_FLASH_TOTAL_SIZE 0x10000
#define FLASH_NS   FMU0_NS
#define BUFFER_LEN 512

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void error_trap(void);
void app_finalize(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Buffer for program */
static uint32_t s_buffer[BUFFER_LEN / sizeof(uint32_t)];
/*! @brief Buffer for readback */
static uint32_t s_buffer_rbc[BUFFER_LEN / sizeof(uint32_t)];

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
    PRINTF("\r\n\r\n\r\n\t---- HALTED DUE TO LPSPI ERROR! ----");
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
    PRINTF("\r\n End of LPSPI flash Example \r\n");
    while (1)
    {
    }
}

int main(void)
{
    status_t result;    /* Return code from each flash driver function */
    uint32_t destAdrss; /* Address of the target location */

    uint32_t lpspiFlashBase      = 0;
    uint32_t lpspiFlashPageSize  = 0;
    uint32_t lpspiFlashTotalSize = 0;
    /* Init hardware */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* print welcome message */
    PRINTF("\r\n ROM API LPSPI Example Start \r\n");

    /* Calling SPI_EepromInit */
    PRINTF("\r\n Initializing LPSPI flash \r\n");
    result = SPI_EepromInit(LPSPI_BAUD_RATE);
    if (kStatus_FLASH_Success != result)
    {
        error_trap();
    }

    /* Get flash properties*/
    lpspiFlashBase      = LPSPI_FLASH_BASE;
    lpspiFlashTotalSize = LPSPI_FLASH_TOTAL_SIZE;
    lpspiFlashPageSize  = LPSPI_FLASH_PAGE_SIZE;

    /* Print flash information - PFlash. */
    PRINTF("\r\n LPSPI flash Information: ");
    PRINTF("\r\n LPSPI flash base address: %d, Hex: (0x%x)", (lpspiFlashBase / 1024), lpspiFlashBase);
    PRINTF("\r\n LPSPI flash page size: %d B", lpspiFlashTotalSize);
    PRINTF("\r\n LPSPI flash total size: %d KB, Hex: (0x%x)", (lpspiFlashTotalSize / 1024), lpspiFlashTotalSize);

    PRINTF("\r\n Erase a page of LPSPI flash");
/*
PAGE_INDEX_FROM_END = 1 means the last page,
PAGE_INDEX_FROM_END = 2 means (the last page - 1) ...
*/
#ifndef PAGE_INDEX_FROM_END
#define PAGE_INDEX_FROM_END 1U
#endif
    /* Erase a sector from destAdrss. */
    destAdrss = lpspiFlashBase + (lpspiFlashTotalSize - (PAGE_INDEX_FROM_END * lpspiFlashPageSize));
    result    = SPI_EepromErase(destAdrss, kSize_ErasePage);
    if (kStatus_Success != result)
    {
        error_trap();
    }
    PRINTF("\r\n Successfully erased page 0x%x -> 0x%x\r\n", destAdrss, (destAdrss + lpspiFlashPageSize));

    /* Print message for user. */
    PRINTF("\r\n Program a buffer to a page of LPSPI flash \r\n ");
    /* Prepare user buffer. */
    for (uint32_t i = 0; i < (BUFFER_LEN / sizeof(uint32_t)); i++)
    {
        s_buffer[i] = i;
    }

    /* Program user buffer into flash*/
    result = SPI_EepromWrite((uint8_t *)s_buffer, BUFFER_LEN, destAdrss);
    if (kStatus_Success != result)
    {
        error_trap();
    }

    /* Verify programming by reading back from flash */
    result = SPI_EepromRead((uint8_t *)s_buffer_rbc, BUFFER_LEN, destAdrss, EepormCmd_FastRead);
    if (kStatus_Success != result)
    {
        error_trap();
    }

    if (memcmp(s_buffer_rbc, s_buffer, BUFFER_LEN) == 0)
    {
        PRINTF("\r\n Successfully programmed and verified location LPSPI flash 0x%x -> 0x%x \r\n", (destAdrss),
               (destAdrss + BUFFER_LEN));
    }

    app_finalize();

    return 0;
}
