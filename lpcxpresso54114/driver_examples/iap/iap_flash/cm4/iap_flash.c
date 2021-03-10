/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_iap.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_IAP_FLASH_SECTOR (1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_PageBuf[FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES / sizeof(uint32_t)];
static uint32_t s_IapFlashPageNum =
    FSL_FEATURE_SYSCON_FLASH_SECTOR_SIZE_BYTES / FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES;
static uint32_t s_IapFlashPage =
    (FSL_FEATURE_SYSCON_FLASH_SECTOR_SIZE_BYTES / FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES) * DEMO_IAP_FLASH_SECTOR;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i;
    status_t status;
#if (defined(FSL_FEATURE_IAP_HAS_FLASH_SIGNATURE_READ) && FSL_FEATURE_IAP_HAS_FLASH_SIGNATURE_READ) || \
    (defined(FSL_FEATURE_IAP_HAS_FLASH_EXTENDED_SIGNATURE_READ) && FSL_FEATURE_IAP_HAS_FLASH_EXTENDED_SIGNATURE_READ)
    uint32_t flashSignature[4];
#endif

    /* Board pin, clock, debug console init */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins_Core0();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("\r\nIAP Flash example\r\n");

#if defined(FLASH_CTRL_FLASHCFG_FLASHTIM_MASK)
    IAP_ConfigAccessFlashTime(kFlash_IAP_OneSystemClockTime);
#endif

    PRINTF("\r\nWriting flash sector %d\r\n", DEMO_IAP_FLASH_SECTOR);
    /* Erase sector before writing */
    IAP_PrepareSectorForWrite(DEMO_IAP_FLASH_SECTOR, DEMO_IAP_FLASH_SECTOR);
    IAP_EraseSector(DEMO_IAP_FLASH_SECTOR, DEMO_IAP_FLASH_SECTOR, SystemCoreClock);
    /* Generate data to be written to flash */
    for (i = 0; i < FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES; i++)
    {
        *(((uint8_t *)(&s_PageBuf[0])) + i) = i;
    }
    /* Program sector */
    for (i = 0; i < s_IapFlashPageNum; i++)
    {
        IAP_PrepareSectorForWrite(DEMO_IAP_FLASH_SECTOR, DEMO_IAP_FLASH_SECTOR);
        status = IAP_CopyRamToFlash((s_IapFlashPage + i) * FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES, &s_PageBuf[0],
                                    FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES, SystemCoreClock);
        if (status != kStatus_IAP_Success)
        {
            PRINTF("\r\nWrite to flash page failed\r\n");
            break;
        }
    }

    /* Verify sector contents */
    for (i = 0; i < s_IapFlashPageNum; i++)
    {
        status = IAP_Compare((s_IapFlashPage + i) * FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES, &s_PageBuf[0],
                             FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES);

        if (status != kStatus_IAP_Success)
        {
            PRINTF("\r\nSector verify failed\r\n");
            break;
        }
    }

    PRINTF("\r\nErasing flash sector %d\r\n", DEMO_IAP_FLASH_SECTOR);
    IAP_PrepareSectorForWrite(DEMO_IAP_FLASH_SECTOR, DEMO_IAP_FLASH_SECTOR);
    IAP_EraseSector(DEMO_IAP_FLASH_SECTOR, DEMO_IAP_FLASH_SECTOR, SystemCoreClock);
    status = IAP_BlankCheckSector(DEMO_IAP_FLASH_SECTOR, DEMO_IAP_FLASH_SECTOR);
    if (status != kStatus_IAP_Success)
    {
        PRINTF("\r\nSector erase failed\r\n");
    }

    PRINTF("\r\nErasing page 1 in flash sector %d\r\n", DEMO_IAP_FLASH_SECTOR);
    /* First write a page */
    IAP_PrepareSectorForWrite(DEMO_IAP_FLASH_SECTOR, DEMO_IAP_FLASH_SECTOR);
    IAP_CopyRamToFlash(s_IapFlashPage * FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES, &s_PageBuf[0],
                       FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES, SystemCoreClock);
    /* Erase page */
    IAP_PrepareSectorForWrite(DEMO_IAP_FLASH_SECTOR, DEMO_IAP_FLASH_SECTOR);
    IAP_ErasePage(s_IapFlashPage, s_IapFlashPage, SystemCoreClock);

    /* Fill page buffer with erased state value */
    for (i = 0; i < FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES; i++)
    {
#if defined(DEMO_INVERSED_FLASH_ERASED_VALUE) && DEMO_INVERSED_FLASH_ERASED_VALUE
        *(((uint8_t *)(&s_PageBuf[0])) + i) = 0x0;
#else
        *(((uint8_t *)(&s_PageBuf[0])) + i) = 0xFF;
#endif /* DEMO_INVERSED_FLASH_ERASED_VALUE */
    }
    /* Verify Erase */
    status = IAP_Compare(s_IapFlashPage * FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES, (uint32_t *)(&s_PageBuf[0]),
                         FSL_FEATURE_SYSCON_FLASH_PAGE_SIZE_BYTES);
    if (status != kStatus_IAP_Success)
    {
        PRINTF("\r\nPage erase failed\r\n");
    }

#if defined(FSL_FEATURE_IAP_HAS_FLASH_EXTENDED_SIGNATURE_READ) && FSL_FEATURE_IAP_HAS_FLASH_EXTENDED_SIGNATURE_READ
    /* Read Extended Flash Signature */
    status = IAP_ExtendedFlashSignatureRead(DEMO_IAP_FLASH_SECTOR, DEMO_IAP_FLASH_SECTOR,
                                            DEMO_IAP_FLASH_NUMBER_OF_WAIT_STATES, flashSignature);
    if (status != kStatus_IAP_Success)
    {
        PRINTF("\r\nExtended read signature failed\r\n");
    }
    else
    {
        PRINTF("\r\nFlash signature value of page %d\r\n", DEMO_IAP_FLASH_SECTOR);
        PRINTF("\r\n");
        for (i = 0; i < 4; i++)
        {
            PRINTF("%X", flashSignature[i]);
        }
        PRINTF("\r\n");
    }
#endif /* FSL_FEATURE_IAP_HAS_FLASH_EXTENDED_SIGNATURE_READ */

#if defined(FSL_FEATURE_IAP_HAS_FLASH_SIGNATURE_READ) && FSL_FEATURE_IAP_HAS_FLASH_SIGNATURE_READ
    /* Read signature of the entire flash memory */
    status = IAP_ReadFlashSignature(flashSignature);
    if (status != kStatus_IAP_Success)
    {
        PRINTF("\r\nRead signature failed\r\n");
    }
    else
    {
        PRINTF("\r\nEntire flash signature\r\n");
        PRINTF("\r\n%X\r\n", flashSignature[0]);
    }
#endif /* FSL_FEATURE_IAP_HAS_FLASH_SIGNATURE_READ */

    PRINTF("\r\nEnd of IAP Flash Example\r\n");
    while (1)
    {
    }
}
