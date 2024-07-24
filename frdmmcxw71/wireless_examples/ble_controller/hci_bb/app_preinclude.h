/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_PREINCLUDE_H_
#define _APP_PREINCLUDE_H_


/*! *********************************************************************************
 *     Board Configuration
 ********************************************************************************** */
/* Number of Button required by the application */
#define gAppButtonCnt_c         0

/* Number of LED required by the application */
#define gAppLedCnt_c            0

#define gDebugConsoleEnable_d   0

/*! *********************************************************************************
 * 	App Configuration
 ********************************************************************************** */

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* Do not modify. Not used for this application */
#define gAppUseNvm_d                    0

/* Enable Serial Manager interface */
#define gAppUseSerialManager_c          1

/* Defines Size for Timer Task*/
#define gTmrTaskStackSize_c             384

#define gAppUseSensors_d                0

/*
 * TimerManager Configuration
 */
#define FSL_OSA_BM_TIMER_CONFIG         FSL_OSA_BM_TIMER_SYSTICK

/*! *********************************************************************************
 *     RTOS Configuration
 ********************************************************************************** */


/*! *********************************************************************************
 *     BLE LL Configuration
 ***********************************************************************************/
/*  ble_ll_config.h file lists the parameters with their default values. User can override
 *    the parameter here by defining the parameter to a user defined value. */
/* enable periodic advertiser list */
#define gAppExtAdvEnable_d                   1
#define gLlScanPeriodicAdvertiserListSize_c (8U)
/* disable autonomous feature exchange */
#define gL1AutonomousFeatureExchange_d 0

#define RL_BUFFER_COUNT (4U)

/*By default DTM through HCI interface is used (gAppUseDtm2Wire not defined).
    If DTM 2-wire interface is prefered, please define gAppUseDtm2Wire.*/
/*#define gAppUseDtm2Wire                      1*/

#define gEnableCoverage                        0
#if (defined(gEnableCoverage) && (gEnableCoverage == 1))
#undef gDebugConsoleEnable_d
#define gDebugConsoleEnable_d 1
#endif

/* define the max tx power setting, allowed value 0, 7 or 10 */
#define gAppMaxTxPowerDbm_c                    10U

/* Include common configuration file and board configuration file */
#include "app_preinclude_common.h"
#endif /* _APP_PREINCLUDE_H_ */
