/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_iap.h"
#include "fsl_iap_ffr.h"
#include "fsl_common.h"
#include "fsl_power.h"
////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////
static void verify_status(status_t status);
static void error_trap();
////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
#define BUFFER_LEN 512 / 4
static uint32_t s_buffer_rbc[BUFFER_LEN];
////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
int main()
{
    flash_config_t flashInstance;
    static uint32_t status;
    uint32_t destAdrss; /* Address of the target location */
    uint32_t failedAddress, failedData;
    uint32_t pflashBlockBase            = 0;
    uint32_t pflashTotalSize            = 0;
    uint32_t pflashSectorSize           = 0;
    uint32_t PflashPageSize             = 0;
    const uint32_t s_buffer[BUFFER_LEN] = {1, 2, 3, 4};
    /* Init board hardware. */
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    /* enable clock for GPIO*/
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);

    BOARD_InitBootPins();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();

    /* Print basic information for Flash Driver API.*/
    PRINTF("\r\nFlash driver API tree Demo Application...\r\n");
    /* Initialize flash driver */
    PRINTF("Initializing flash driver...\r\n");
    if (FLASH_Init(&flashInstance) == kStatus_Success)
    {
        PRINTF("Flash init successfull!!. Halting...\r\n");
    }
    else
    {
        error_trap();
    }
    /* Get flash properties kFLASH_ApiEraseKey */
    FLASH_GetProperty(&flashInstance, kFLASH_PropertyPflashBlockBaseAddr, &pflashBlockBase);
    FLASH_GetProperty(&flashInstance, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);
    FLASH_GetProperty(&flashInstance, kFLASH_PropertyPflashTotalSize, &pflashTotalSize);
    FLASH_GetProperty(&flashInstance, kFLASH_PropertyPflashPageSize, &PflashPageSize);

    /* print welcome message */
    PRINTF("\r\n PFlash Example Start \r\n");
    /* Print flash information - PFlash. */
    PRINTF("\tkFLASH_PropertyPflashBlockBaseAddr = 0x%X\r\n", pflashBlockBase);
    PRINTF("\tkFLASH_PropertyPflashSectorSize = %d\r\n", pflashSectorSize);
    PRINTF("\tkFLASH_PropertyPflashTotalSize = %d\r\n", pflashTotalSize);
    PRINTF("\tkFLASH_PropertyPflashPageSize = 0x%X\r\n", PflashPageSize);

/*
PAGE_INDEX_FROM_END = 1 means the last page,
PAGE_INDEX_FROM_END = 2 means (the last page -1 )...
*/
#ifndef PAGE_INDEX_FROM_END
#define PAGE_INDEX_FROM_END 1U
#endif

    destAdrss = pflashBlockBase + (pflashTotalSize - (PAGE_INDEX_FROM_END * PflashPageSize));

    PRINTF("\r\nCalling FLASH_Erase() API...\r\n");
    status = FLASH_Erase(&flashInstance, destAdrss, PflashPageSize, kFLASH_ApiEraseKey);
    verify_status(status);
    PRINTF("Done!\r\n");

    /* Verify if the given flash range is successfully erased. */
    PRINTF("Calling FLASH_VerifyErase() API...\r\n");
    status = FLASH_VerifyErase(&flashInstance, destAdrss, PflashPageSize);
    verify_status(status);
    if (status == kStatus_Success)
    {
        PRINTF("FLASH Verify erase successful!\n");
    }
    else
    {
        error_trap();
    }

    /* Start programming specified flash region */
    PRINTF("Calling FLASH_Program() API...\r\n");
    status = FLASH_Program(&flashInstance, destAdrss, (uint8_t *)s_buffer, sizeof(s_buffer));
    verify_status(status);

    /* Verify if the given flash region is successfully programmed with given data */
    PRINTF("Calling FLASH_VerifyProgram() API...\r\n");
    status = FLASH_VerifyProgram(&flashInstance, destAdrss, sizeof(s_buffer), (const uint8_t *)s_buffer, &failedAddress,
                                 &failedData);
    verify_status(status);

    if (status == kStatus_Success)
    {
        PRINTF("FLASH Verify Program successful!\n");
    }
    else
    {
        error_trap();
    }

    /* Check if the flash page is blank before reading to avoid hardfault. */
    status = FLASH_VerifyErase(&flashInstance, destAdrss, PflashPageSize);
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

    PRINTF("\r\n Successfully Programmed and Verified Location 0x%x -> 0x%x \r\n", destAdrss,
           (destAdrss + sizeof(s_buffer)));

    /* resume flash memory status */
    status = FLASH_Erase(&flashInstance, destAdrss, PflashPageSize, kFLASH_ApiEraseKey);

    PRINTF("Done!\r\n");

    while (1)
    {
    }
}

void verify_status(status_t status)
{
    char *tipString = "Unknown status";
    switch (status)
    {
        case kStatus_Success:
            tipString = "Done.";
            break;
        case kStatus_InvalidArgument:
            tipString = "Invalid argument.";
            break;
        case kStatus_FLASH_AlignmentError:
            tipString = "Alignment Error.";
            break;
        case kStatus_FLASH_AccessError:
            tipString = "Flash Access Error.";
            break;
        case kStatus_FLASH_CommandNotSupported:
            tipString = "This API is not supported in current target.";
            break;
        default:
            break;
    }
    PRINTF("%s\r\n\r\n", tipString);
}

/*
 * @brief Gets called when an error occurs.
 */
void error_trap(void)
{
    PRINTF("\r\n\r\n\r\n\t---- HALTED DUE TO FLASH ERROR! ----");
    while (1)
    {
    }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
