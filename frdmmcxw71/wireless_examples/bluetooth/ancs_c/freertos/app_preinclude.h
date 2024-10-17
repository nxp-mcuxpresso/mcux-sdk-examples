/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*
 * Copyright 2024 NXP
 *
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _APP_PREINCLUDE_H_
#define _APP_PREINCLUDE_H_

/*!
 *  Application specific configuration file only
 *  Board Specific Configuration shall be added to board.h file directly such as :
 *  - Number of buttons on the board,
 *  - Number of LEDs,
 *  - etc...
 */
/*! *********************************************************************************
 *     Board Configuration
 ********************************************************************************** */
/* Number of Buttons required by the application */
#define gAppButtonCnt_c                 2

/* Number of LEDs required by the application */
#define gAppLedCnt_c                    2

/*! Enable Debug Console (PRINTF) */
#define gDebugConsoleEnable_d           0

/*! *********************************************************************************
 *     App Configuration
 ********************************************************************************** */
/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d                1

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d                1

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d                0

#define gPasskeyValue_c                 999999

/*! Maximum Number of Services that can be stored during Service Discovery */
#define gMaxServicesCount_d             10U

/*! Maximum Number of Characteristics per Service that can be stored during Service Discovery */
#define gMaxServiceCharCount_d          10U

/*! Maximum Number of Descriptors per Service that can be stored during Service Discovery */
#define gMaxCharDescriptorsCount_d      5U

/*! Repeated Attempts - Mitigation for pairing attacks */
#define gRepeatedAttempts_d             0

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d                    1

/* Defines Rx Buffer Size for Serial Manager */
#define gSerialMgrRxBufSize_c           32

/* Defines Tx Queue Size for Serial Manager */
#define gSerialMgrTxQueueSize_c         5

/* Defines Size for Serial Manager Task*/
#define gSerialTaskStackSize_c          700

/*! The minimum heap size needed (measured with MEM_STATISTICS) */
#define MinimalHeapSize_c               17000

/*! *********************************************************************************
 *     RTOS Configuration
 ********************************************************************************** */
#if defined(SDK_OS_FREE_RTOS)
/* The following stack sizes have been chosen based on a worst case scenario. 
 * For different compilers and optimization levels they can be reduced. */

#define gHost_TaskStackSize_c           1920

#endif

/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */

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
