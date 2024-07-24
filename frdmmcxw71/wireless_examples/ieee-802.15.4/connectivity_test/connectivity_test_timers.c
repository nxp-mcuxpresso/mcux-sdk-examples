/*! *********************************************************************************
* Copyright 2021 NXP
* All rights reserved.
*
* \file
*
* This file provides timer functionality to the connectivity_test application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#include "connectivity_test_timers.h"

#if defined(gConnTestUsePhyTimers_c) && (gConnTestUsePhyTimers_c == 1)

void ConnTestTimers_InitTimer(ConnTestTimer_t *tmr, ConnTestTimerCallback_t callback)
{
    tmr->tmrId    = gInvalidTimerId_c;
    tmr->callback = callback;
}

void ConnTestTimers_StartDelay(ConnTestTimer_t *tmr, uint32_t time_ms)
{
    phyTimeEvent_t delay_evt;

    /* convert interval to symbols */
    delay_evt.timestamp = ((uint64_t)time_ms * 1000) / 16;
    delay_evt.timestamp += PhyTime_GetTimestamp();
    delay_evt.callback = tmr->callback;
    delay_evt.parameter = 0;

    tmr->tmrId = PhyTime_ScheduleEvent(&delay_evt);
}

void ConnTestTimers_StopDelay(ConnTestTimer_t *tmr)
{
    PhyTime_CancelEvent(tmr->tmrId);
}

uint64_t ConnTestTimers_GetTime(void)
{
#ifdef gPHY_802_15_4g_d
    return PhyTime_GetTimestampUs();
#else
	return 16 * PhyTime_GetTimestamp();
#endif
}

#else

void ConnTestTimers_InitTimer(ConnTestTimer_t *tmr, ConnTestTimerCallback_t callback)
{
    (void)TM_Open((timer_handle_t)tmr);
    (void)TM_InstallCallback((timer_handle_t)tmr, (timer_callback_t)callback, NULL);
}

void ConnTestTimers_StartDelay(ConnTestTimer_t *tmr, uint32_t time_ms)
{
    TM_Start(tmr, kTimerModeSingleShot, time_ms);
}

void ConnTestTimers_StopDelay(ConnTestTimer_t *tmr)
{
    TM_Stop(tmr);
}

uint64_t ConnTestTimers_GetTime(void)
{
    return TM_GetTimestamp();
}
#endif