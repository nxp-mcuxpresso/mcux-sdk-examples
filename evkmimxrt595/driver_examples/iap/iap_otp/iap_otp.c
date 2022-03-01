/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_iap.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 *******************************************************************************/
#define EXAMPLE_OTP_CLK         SystemCoreClock
#define EXAMPLE_FUSE_LAST_INDEX 511

/*******************************************************************************
 * Prototypes
 *******************************************************************************/

/*******************************************************************************
 * Variables
 *******************************************************************************/

/*******************************************************************************
 * Code
 *******************************************************************************/
int main(void)
{
    uint32_t status;
    uint32_t index;
    uint32_t buffer;
    char ch;

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("IAP OTP example\r\n");

    /* Init OTP */
    status = IAP_OtpInit(EXAMPLE_OTP_CLK);
    if (status != kStatus_Success)
    {
        PRINTF("OTP Init failed\r\n");
        return -1;
    }

    if (true)
    {
        while (1)
        {
            ch    = 0;
            index = 0;
            PRINTF("Please input the OTP word index to read(0~%d):", EXAMPLE_FUSE_LAST_INDEX);
            while (ch != 0x0D)
            {
                ch = GETCHAR();
                if ((ch >= '0') && (ch <= '9'))
                {
                    PUTCHAR(ch);
                    index = index * 10 + (ch - 0x30U);
                }
            }
            assert(index <= EXAMPLE_FUSE_LAST_INDEX);
            status = IAP_OtpFuseRead(index, &buffer);
            if (status != kStatus_Success)
            {
                PRINTF("\r\nFuse read failed\r\n");
                return -1;
            }
            PRINTF("\r\nFuse[%d] value: 0x%X\r\n\r\n", index, buffer);
        }
    }
    else
    {
#if defined(ENABLE_OTP_WRITE) && (ENABLE_OTP_WRITE)
        /* Program OTP Fuse*/
        status = IAP_OtpFuseProgram(DEMO_ADDR, DEMO_DATA, false);
        if (status != kStatus_Success)
        {
            PRINTF("Fuse Program failed\r\n");
            return -1;
        }
        else
        {
            if (IAP_OtpFuseRead(DEMO_ADDR, &buffer) != kStatus_Success)
            {
                PRINTF("\r\nFuse read failed\r\n");
            }
            else
            {
                PRINTF("\r\nProgram Fuse[%d] value = 0x%X\r\n\r\n", DEMO_ADDR, buffer);
            }
            return 0;
        }
#endif
    }
}
