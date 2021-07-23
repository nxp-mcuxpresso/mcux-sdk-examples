/*
 * Copyright 2018-2019 NXP
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
#define DEMO_IAP_EEPROM_PAGE (1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#define EEPROM_PAGE_SIZE (FSL_FEATURE_EEPROM_SIZE / FSL_FEATURE_EEPROM_PAGE_COUNT)

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_WriteBuf[EEPROM_PAGE_SIZE / sizeof(uint32_t)];
static uint32_t s_ReadBuf[EEPROM_PAGE_SIZE / sizeof(uint32_t)];

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

    /* Board pin, clock, debug console init */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_Eeprom);

    PRINTF("\r\nIAP EEPROM example\r\n");

    memset(s_ReadBuf, 0, EEPROM_PAGE_SIZE);
    /* Prepare the Write buffer. */
    for (i = 0U; i < EEPROM_PAGE_SIZE; i++)
    {
        *(((uint8_t *)(&s_WriteBuf[0])) + i) = i;
    }

    /* Write EEPROM */
    PRINTF("\r\nWrite EEPROM page %d\r\n", DEMO_IAP_EEPROM_PAGE);
    status = IAP_WriteEEPROMPage(DEMO_IAP_EEPROM_PAGE, s_WriteBuf, SystemCoreClock);
    if (status != kStatus_IAP_Success)
    {
        PRINTF("\r\nWrite EEPROM page failed\r\n");
    }

    /* Read EEPROM */
    PRINTF("\r\nRead EEPROM page %d\r\n", DEMO_IAP_EEPROM_PAGE);
    status = IAP_ReadEEPROMPage(DEMO_IAP_EEPROM_PAGE, s_ReadBuf, SystemCoreClock);
    if (status != kStatus_IAP_Success)
    {
        PRINTF("\r\nRead EEPROM page failed\r\n");
    }

    /* Compare the read result with the wirte buffer. */
    for (i = 0U; i < EEPROM_PAGE_SIZE / sizeof(uint32_t); i++)
    {
        if (s_ReadBuf[i] != s_WriteBuf[i])
        {
            PRINTF("\r\nEEPROM Page read/Write failed\r\n");
            break;
        }
    }

    PRINTF("\r\nEnd of IAP EEPROM Example\r\n");
    while (1)
    {
    }
}
