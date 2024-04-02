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
#ifndef _CONNECTIVITY_TEST_TIMERS_H_

#if defined(gConnTestUsePhyTimers_c) && (gConnTestUsePhyTimers_c == 1)
#include "EmbeddedTypes.h"
#include "PhyTypes.h"
#include "PhyInterface.h"

typedef struct {
    phyTimeTimerId_t    tmrId;
    phyTimeCallback_t   callback;
} ConnTestTimer_t;

#define CONN_TEST_TIMER_DEFINE(name)    ConnTestTimer_t name[1]

#else

#include "fsl_component_timer_manager.h"

#define CONN_TEST_TIMER_DEFINE(name)    TIMER_MANAGER_HANDLE_DEFINE(name)
typedef uint32_t ConnTestTimer_t;

#endif

typedef void (*ConnTestTimerCallback_t) ( uint32_t param );

void ConnTestTimers_InitTimer(ConnTestTimer_t *tmr, ConnTestTimerCallback_t callback);
void ConnTestTimers_StartDelay(ConnTestTimer_t *tmr, uint32_t time_ms);
void ConnTestTimers_StopDelay(ConnTestTimer_t *tmr);
uint64_t ConnTestTimers_GetTime(void);

#endif /* _CONNECTIVITY_TEST_TIMERS_H_ */