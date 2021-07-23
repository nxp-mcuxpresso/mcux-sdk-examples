/*
 * Copyright 2019 NXP
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
#define DEMO_IAP_FAIM_PAGE (1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#define FAIM_PAGE_SIZE (FSL_FEATURE_FAIM_SIZE / FSL_FEATURE_FAIM_PAGE_COUNT)

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_WriteBuf[FAIM_PAGE_SIZE / sizeof(uint32_t)];
static uint32_t s_ReadBuf[FAIM_PAGE_SIZE / sizeof(uint32_t)];

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
    /* Select the main clock as source clock of USART0 (debug console) */
    CLOCK_Select(BOARD_DEBUG_USART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_BootClockFRO30M();
    BOARD_InitDebugConsole();

    PRINTF("\r\nIAP FAIM example\r\n");

    memset(s_ReadBuf, 0, FAIM_PAGE_SIZE);
    /* Prepare the Write buffer. */
    for (i = 0U; i < FAIM_PAGE_SIZE; i++)
    {
        *(((uint8_t *)(&s_WriteBuf[0])) + i) = i;
    }

    /* Write FAIM */
    PRINTF("\r\nWrite FAIM sector %d\r\n", DEMO_IAP_FAIM_PAGE);
    status = IAP_WriteFAIMPage(DEMO_IAP_FAIM_PAGE, s_WriteBuf);
    if (status != kStatus_IAP_Success)
    {
        PRINTF("\r\nWrite FAIM page failed\r\n");
    }

    /* Read FAIM */
    PRINTF("\r\nRead FAIM sector %d\r\n", DEMO_IAP_FAIM_PAGE);
    status = IAP_ReadFAIMPage(DEMO_IAP_FAIM_PAGE, s_ReadBuf);
    if (status != kStatus_IAP_Success)
    {
        PRINTF("\r\nRead FAIM page failed\r\n");
    }

    /* Compare the read result with the wirte buffer. */
    for (i = 0U; i < FAIM_PAGE_SIZE / sizeof(uint32_t); i++)
    {
        if (s_ReadBuf[i] != s_WriteBuf[i])
        {
            PRINTF("\r\nFAIM Page read/Write failed\r\n");
            break;
        }
    }

    PRINTF("\r\nEnd of IAP FAIM Example\r\n");
    while (1)
    {
    }
}
