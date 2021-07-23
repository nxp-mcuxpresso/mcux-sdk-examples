/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_rtc.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile bool busyWait;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief ISR for Alarm interrupt
 *
 * This function changes the state of busyWait.
 */
void RTC_IRQHandler(void)
{
    if (RTC_GetStatusFlags(RTC) & kRTC_AlarmFlag)
    {
        busyWait = false;

        /* Clear alarm flag */
        RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t sec;
    uint32_t currSeconds;
    uint8_t index;
    rtc_datetime_t date;

    /* Board pin, clock, debug console init */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_EnableOsc32K(true);

    /* Init RTC */
    RTC_Init(RTC);

    PRINTF("RTC example: set up time to wake up an alarm\r\n");

    /* Set a start date time and start RT */
    date.year   = 2014U;
    date.month  = 12U;
    date.day    = 25U;
    date.hour   = 19U;
    date.minute = 0;
    date.second = 0;

    /* RTC time counter has to be stopped before setting the date & time in the TSR register */
    RTC_EnableTimer(RTC, false);

    /* Set RTC time to default */
    RTC_SetDatetime(RTC, &date);

    /* When working under Normal Mode, the interrupt is controlled by NVIC. */
    EnableIRQ(RTC_IRQn);

    /* Start the RTC time counter */
    RTC_EnableTimer(RTC, true);

    /* This loop will set the RTC alarm */
    while (1)
    {
        busyWait = true;
        sec      = 0;
        /* Get date time */
        RTC_GetDatetime(RTC, &date);

        /* print default time */
        PRINTF("Current datetime: %04d-%02d-%02d %02d:%02d:%02d\r\n", date.year, date.month, date.day, date.hour,
               date.minute, date.second);

        /* Get alarm time from user */
        PRINTF("Please input the number of second to wait for alarm \r\n");
        PRINTF("The second must be positive value\r\n");
        do
        {
            index = 0;
            while (index != 0x0D)
            {
                index = GETCHAR();
                if ((index >= '0') && (index <= '9'))
                {
                    PUTCHAR(index);
                    sec = sec * 10 + (index - 0x30U);
                }
            }
        } while (0U == sec); /* To fix the issue that the user inputs "enter" directly without delay time.. */
        PRINTF("\r\n");

        /* Read the RTC seconds register to get current time in seconds */
        currSeconds = RTC_GetSecondsTimerCount(RTC);

        /* Add alarm seconds to current time */
        currSeconds += sec;

        /* Set alarm time in seconds */
        RTC_SetSecondsTimerMatch(RTC, currSeconds);

        /* Get alarm time */
        RTC_GetAlarm(RTC, &date);

        /* Print alarm time */
        PRINTF("Alarm will occur at: %04d-%02d-%02d %02d:%02d:%02d\r\n", date.year, date.month, date.day, date.hour,
               date.minute, date.second);

        /* Wait until alarm occurs */
        while (busyWait)
        {
        }

        PRINTF("\r\n Alarm occurs !!!! ");
    }
}
