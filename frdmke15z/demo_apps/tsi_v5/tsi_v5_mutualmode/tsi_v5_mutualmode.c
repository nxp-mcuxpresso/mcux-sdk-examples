/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_tsi_v5.h"
#include "TSI_key.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LED1_INIT() LED_RED1_INIT(LOGIC_LED_OFF)
#define LED1_ON()   LED_RED1_ON()
#define LED1_OFF()  LED_RED1_OFF()

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t key_event;
    uint8_t touched_key_id = 0xFFU;

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("TSI mutual mode demo.\r\n");

    LED1_INIT();

    /* Init TSI moudle, calibrate TSI idle value, save as TSI baseline*/
    TSI_KeyInit();

    while (1)
    {
        /* debug key function */
        key_event = TSI_KeyDetect(&touched_key_id);

        if (key_event == kKey_Event_Touch)
        {
            /* if (touched_key_id == 0) */
            {
                LED1_ON();
                PRINTF("TSI key touched.\r\n");
            }
        }
        else if (key_event == kKey_Event_Release)
        {
            /* if (touched_key_id == 0) */
            {
                LED1_OFF();
                PRINTF("TSI key released.\r\n");
            }
        }
        else
        {
            /* Here to deal with other key events */
            key_event = kKey_Event_Idle;
        }
    }
}
