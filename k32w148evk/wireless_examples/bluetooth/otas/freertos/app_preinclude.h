/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright 2019-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_PREINCLUDE_H_
#define _APP_PREINCLUDE_H_

/*!
 *  Application specific configuration file only
 *  Board Specific Configuration shall be added to board.h file directly such as :
 *  - Number of button on the board,
 *  - Number of LEDs,
 *  - etc...
 */
/*! *********************************************************************************
 *     Board Configuration
 ********************************************************************************** */
/* Number of Button required by the application */
#define gAppButtonCnt_c                 1U

/* Number of LED required by the application */
#define gAppLedCnt_c                    2U

/*! Enable Debug Console (PRINTF) */
#define gDebugConsoleEnable_d           0

/*! *********************************************************************************
 *     App Configuration
 ********************************************************************************** */
/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d   1

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d   1

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d   0

#define gPasskeyValue_c                999999

/*! Repeated Attempts - Mitigation for pairing attacks */
#define gRepeatedAttempts_d             0

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d                    1

/*! The minimum heap size needed (measured with MEM_STATISTICS) */
#define MinimalHeapSize_c               17500

/* Enable/Diable FSCI */
#define gFsciIncluded_c                 1

/* Enable/Disable FSCI 2 byte command length */
#define gFsciLenHas2Bytes_c             1

/* Enable/Disable the FSCI BLE OTAP module */
#define gFsciBleOtapEnabled_d           1

/* FSCI payload - set his high enough for long frames support */
#define gFsciMaxPayloadLen_c            520

/* Use Misra Compliant version of FSCI module */
#define gFsciUseDedicatedTask_c         1

/* Enable or disable SerialManager_Task() handle RX data available notify */
#define SERIAL_MANAGER_TASK_HANDLE_RX_AVAILABLE_NOTIFY 1

#define gFsciTaskPriority_c (2U)
#define SERIAL_MANAGER_TASK_PRIORITY (3U)

#define SERIAL_MANAGER_RING_BUFFER_SIZE (300U)

/*! *********************************************************************************
 *     RTOS Configuration
 ********************************************************************************** */
#if defined(SDK_OS_FREE_RTOS)
/* The following stack sizes have been chosen based on a worst case scenario.
 * For different compilers and optimization levels they can be reduced. */

#define gHost_TaskStackSize_c           1800

#define gFsciTaskStackSize_c            2264

#define BUTTON_TASK_STACK_SIZE          448

#define SERIAL_MANAGER_TASK_STACK_SIZE  688

#define gMainThreadStackSize_c          2424

#define TM_TASK_STACK_SIZE              384

/* The size used for the Idle task, in dwords. */
#define configMINIMAL_STACK_SIZE        184

#endif

/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */
 /* Enable Serial Manager interface */
#define gAppUseSerialManager_c          1

#define gMaxServicesCount_d             5U
#define gMaxServiceCharCount_d          3U
#define gMaxCharDescriptorsCount_d      2U

/*! *********************************************************************************
 *     BLE LL Configuration
 ***********************************************************************************/
/*  ble_ll_config.h file lists the parameters with their default values. User can override
 *    the parameter here by defining the parameter to a user defined value. */

/*
 * Specific configuration of LL pools by block size and number of blocks for this application.
 * Optimized using the MEM_OPTIMIZE_BUFFER_POOL feature in MemManager,
 * we find that the most optimized combination for LL buffers.
 *
 * If LlPoolsDetails_c is not defined, default LL buffer configuration in app_preinclude_common.h
 * will be applied.
 */

/* Include common configuration file and board configuration file */
#include "app_preinclude_common.h"
#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
