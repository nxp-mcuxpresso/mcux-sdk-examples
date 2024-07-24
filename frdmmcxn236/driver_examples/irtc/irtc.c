/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2022-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_irtc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifndef APP_IRTC_IS_SLAVE
#define APP_IRTC_IS_SLAVE 0
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile static bool alarmHappen = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

#if APP_IRTC_IS_SLAVE
uint8_t APP_GetDaysOfMonth(uint16_t year, uint8_t month)
{
    uint8_t days[] = {31u, 28u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 31u, 30u, 31u};

    /* leap year */
    if ((((year % 4u) == 0u) && ((year % 100u) != 0u)) || ((year % 400u) == 0u))
    {
        days[1] = 29u;
    }

    return days[month - 1u];
}

/*
 * Add second to the inputDateTime, save the results to outputDateTime.
 *
 * For demo purpose, this function only allow add seconds less than one day.
 */
static void RTC_AddSec(const irtc_datetime_t *inputDateTime, irtc_datetime_t *outputDateTime, uint32_t sec)
{
    uint16_t year;
    uint32_t month;
    uint32_t day;
    uint32_t weekDay;
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
    uint32_t monthDay;

    /* For demo purpose, this function only allow add seconds less than one day. */
    assert(sec <= (24UL * 60UL * 60UL));

    year    = inputDateTime->year;
    month   = inputDateTime->month;
    day     = inputDateTime->day;
    weekDay = inputDateTime->weekDay;
    hour    = inputDateTime->hour;
    minute  = inputDateTime->minute;
    second  = inputDateTime->second;

    second += sec;

    minute += (second / 60U);
    second %= 60U;

    hour   += (minute / 60U);
    minute %= 60U;

    day     += (hour / 24U);
    weekDay += (hour / 24U);
    hour    %= 24U;

    weekDay %= 7U;

    monthDay = APP_GetDaysOfMonth(year, month);

    if (day > monthDay)
    {
        day = (day - monthDay);
        month += 1U;

        if (month > 12u)
        {
            month -= 12u;
            year += 1U;
        }
    }

    outputDateTime->year = year;
    outputDateTime->month = month;
    outputDateTime->day = day;
    outputDateTime->weekDay = weekDay;
    outputDateTime->hour = hour;
    outputDateTime->minute = minute;
    outputDateTime->second = second;

    return;
}
#endif

/*!
 * @brief ISR for Alarm interrupt
 *
 * This function change state of busyWait.
 */
void RTC_IRQHandler(void)
{
    uint32_t flags = IRTC_GetStatusFlags(RTC);
    if (0U != flags)
    {
        alarmHappen = (0U != (flags & kIRTC_AlarmFlag));
        /* Unlock to allow register write operation */
        IRTC_SetWriteProtection(RTC, false);
        /*Clear all irtc flag */
        IRTC_ClearStatusFlags(RTC, flags);
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    irtc_config_t irtcConfig;

#if APP_IRTC_IS_SLAVE
    irtc_datetime_t alarmDatetime, datetimeGet;
#else
    irtc_datetime_t datetime, alarmDatetime, datetimeGet;

    datetime.year    = 2015;
    datetime.month   = 1;
    datetime.day     = 21;
    datetime.weekDay = 3;
    datetime.hour    = 18;
    datetime.minute  = 55;
    datetime.second  = 30;

    alarmDatetime.year    = 2015;
    alarmDatetime.month   = 1;
    alarmDatetime.day     = 21;
    alarmDatetime.hour    = 18;
    alarmDatetime.minute  = 55;
    alarmDatetime.second  = 33;
    alarmDatetime.weekDay = 0; /* Don't care for alarm, however this should be set to a valid value */
#endif

    /* Board pin, clock, debug console init */
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Enable fro16k[0] to domain Vbat and Main */
    CLOCK_SetupClk16KClocking(kCLOCK_Clk16KToVbat | kCLOCK_Clk16KToMain);

    /* Init the IRTC module */
    /*
     * irtcConfig.wakeupSelect = true;
     * irtcConfig.timerStdMask = false;
     * irtcConfig.alrmMatch = kRTC_MatchSecMinHr;
     */

    IRTC_GetDefaultConfig(&irtcConfig);
    if (IRTC_Init(RTC, &irtcConfig) == kStatus_Fail)
    {
        return 1;
    }
    
#if !defined(FSL_FEATURE_RTC_HAS_NO_GP_DATA_REG) || (!FSL_FEATURE_RTC_HAS_NO_GP_DATA_REG)
    /* Enable the RTC 32KHz oscillator at CFG0 by writing a 0 */
    IRTC_Enable32kClkDuringRegisterWrite(RTC, true);
#endif
#if !defined(FSL_FEATURE_RTC_HAS_NO_TAMPER_FEATURE) || (!FSL_FEATURE_RTC_HAS_NO_TAMPER_FEATURE)
    /* Clear all Tamper events by writing a 1 to the bits */
    IRTC_ClearTamperStatusFlag(RTC);
#endif

    PRINTF("RTC Example START:\r\n");

#if !APP_IRTC_IS_SLAVE
    /* If IRTC is slave, we can't set date and time, but only can get the value. */
    IRTC_SetDatetime(RTC, &datetime);
#endif
    IRTC_GetDatetime(RTC, &datetimeGet);
    PRINTF("\r\nStart Time is %d/%d/%d %d:%d:%2d\r\n", datetimeGet.year, datetimeGet.month, datetimeGet.day,
           datetimeGet.hour, datetimeGet.minute, datetimeGet.second);
#if APP_IRTC_IS_SLAVE
    /* IRTC is slave, the alarm time is set based on the time got previously. */
    RTC_AddSec(&datetimeGet, &alarmDatetime, 3);
#endif
    IRTC_SetAlarm(RTC, &alarmDatetime);

    /* Enable RTC alarm interrupt */
    IRTC_EnableInterrupts(RTC, kIRTC_AlarmInterruptEnable);

    /* Enable at the NVIC */
    EnableIRQ(RTC_IRQn);

    while (!alarmHappen)
    {
    }

    PRINTF("\r\nRing, ring, ring");
    IRTC_GetDatetime(RTC, &datetimeGet);
    PRINTF("\r\nAlarm Time is %d/%d/%d %d:%d:%2d\r\n", datetimeGet.year, datetimeGet.month, datetimeGet.day,
           datetimeGet.hour, datetimeGet.minute, datetimeGet.second);
    PRINTF("\r\nRTC Example END.\r\n");

    for (;;)
    {
    }
}
