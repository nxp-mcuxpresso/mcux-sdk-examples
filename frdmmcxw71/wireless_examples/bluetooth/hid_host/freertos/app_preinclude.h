/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright 2019, 2021 - 2024 NXP
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
/* Number of Buttons required by the application */
#define gAppButtonCnt_c                 1U

/* Number of LEDs required by the application */
#define gAppLedCnt_c                    2U

/*! Enable Debug Console (PRINTF) */
#define gDebugConsoleEnable_d           0

/*! *********************************************************************************
 *     App Configuration
 ********************************************************************************** */
/*! Number of connections supported by the application */
#define gAppMaxConnections_c    8U

/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d        1

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d        1

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d        0

#define gPasskeyValue_c         999999

/*! Repeated Attempts - Mitigation for pairing attacks */
#define gRepeatedAttempts_d             0

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d            1

/*! The minimum heap size needed (measured with MEM_STATISTICS) */
#define MinimalHeapSize_c               13000

/*! *********************************************************************************
 *     RTOS Configuration
 ********************************************************************************** */
#if defined(SDK_OS_FREE_RTOS)
/* The following stack sizes have been chosen based on a worst case scenario. 
 * For different compilers and optimization levels they can be reduced. */

#define gHost_TaskStackSize_c           1800

#define BUTTON_TASK_STACK_SIZE          400

#define SERIAL_MANAGER_TASK_STACK_SIZE  320

#define gMainThreadStackSize_c          880

#define TM_TASK_STACK_SIZE              400

#define SHELL_TASK_STACK_SIZE           500

/* The size used for the Idle task, in dwords. */
#define configMINIMAL_STACK_SIZE        160

#endif

/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */
#define gMaxServicesCount_d             6
#define gMaxServiceCharCount_d          6

/* Enable Serial Manager interface */
#define gAppUseSerialManager_c          1

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
