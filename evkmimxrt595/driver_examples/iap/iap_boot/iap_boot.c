/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_iap.h"

#include "pin_mux.h"
#include "board.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 *******************************************************************************/
#define EXAMPLE_BOOT_INTERFACES        "0: USART 2: SPI 3: USB HID 4:FlexSPI 6:SD 7:MMC"
#define EXAMPLE_BOOT_INTERFACE_FLEXSPI (4U)
#define EXAMPLE_BOOT_INTERFACE_SD      (6U)
#define EXAMPLE_BOOT_INTERFACE_MMC     (7U)

/*******************************************************************************
 * Prototypes
 *******************************************************************************/
bool EXAMPLE_IsValidInterface(uint8_t idx);

/*******************************************************************************
 * Variables
 *******************************************************************************/

/*******************************************************************************
 * Code
 *******************************************************************************/

bool EXAMPLE_IsValidInterface(uint8_t idx)
{
    return (idx <= 7U) && (idx != 1U) && (idx != 5U);
}
static uint8_t getIndex(char ch)
{
    uint8_t idx = 0xFFU;

    if (ch >= '0' && ch <= '9')
    {
        idx = ch - '0';
    }

    return idx;
}

int main(void)
{
    iap_boot_option_t option = {0};
    uint8_t i;
    char ch;

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("===== IAP Boot example, input the parameter: =====\r\n");
    while (true)
    {
        PRINTF("Boot mode (0: master, 1: ISP) : ");
        ch = GETCHAR();
        PUTCHAR(ch);
        i = getIndex(ch);
        if (i > 1U)
        {
            PRINTF(" ... wrong value, please input again\r\n");
        }
        else
        {
            PRINTF("\r\n");
            break;
        }
    }
    option.option.B.tag  = IAP_BOOT_OPTION_TAG;
    option.option.B.mode = (uint8_t)ch;

    while (true)
    {
        PRINTF("Boot interface (%s) : ", EXAMPLE_BOOT_INTERFACES);
        ch = GETCHAR();
        PUTCHAR(ch);
        i = getIndex(ch);
        if (!EXAMPLE_IsValidInterface(i))
        {
            PRINTF(" ... wrong value, please input again\r\n");
        }
        else
        {
            PRINTF("\r\n");
            break;
        }
    }
    option.option.B.bootInterface = i;

    if (i == EXAMPLE_BOOT_INTERFACE_FLEXSPI)
    {
        while (true)
        {
            PRINTF("Boot image index (0-1) : ");
            ch = GETCHAR();
            PUTCHAR(ch);
            i = getIndex(ch);
            if (i > 1U)
            {
                PRINTF(" ... wrong value, please input again\r\n");
            }
            else
            {
                PRINTF("\r\n");
                break;
            }
        }
        option.option.B.bootImageIndex = i;

        while (true)
        {
            PRINTF("Boot FlexSPI instance (0-3):\r\n");
            PRINTF("FlexSPI PortA1 = 0\r\n");
            PRINTF("FlexSPI PortB1 = 1\r\n");
            PRINTF("FlexSPI PortA2 = 2\r\n");
            PRINTF("FlexSPI PortB2 = 3\r\n");
            PRINTF("Selection : ");
            ch = GETCHAR();
            PUTCHAR(ch);
            i = getIndex(ch);
            if (i > 3U)
            {
                PRINTF(" ... wrong value, please input again\r\n");
            }
            else
            {
                PRINTF("\r\n");
                break;
            }
        }
        option.option.B.instance = i;
    }
    else if (i == EXAMPLE_BOOT_INTERFACE_SD || i == EXAMPLE_BOOT_INTERFACE_MMC)
    {
        while (true)
        {
            PRINTF("Boot SD/MMC instance (0-1) : ");
            ch = GETCHAR();
            PUTCHAR(ch);
            i = getIndex(ch);
            if (i > 1U)
            {
                PRINTF(" ... wrong value, please input again\r\n");
            }
            else
            {
                PRINTF("\r\n");
                break;
            }
        }
        option.option.B.instance = i;
    }
    else
    {
    }

    IAP_RunBootLoader(&option); /* The function will never return. */

    return 0;
}
