/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*
 * Copyright 2020 - 2024 NXP
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
 *  - Number of button on the board,
 *  - Number of LEDs,
 *  - etc...
 */
/*! *********************************************************************************
 *     Board Configuration
 ********************************************************************************** */
/* Number of Button required by the application */
#define gAppButtonCnt_c                 0

/* Number of LED required by the application */
#define gAppLedCnt_c                    2U

/*! Enable Debug Console (PRINTF) */
#define gDebugConsoleEnable_d           0

/*! *********************************************************************************
 *     App Configuration
 ********************************************************************************** */
/*! Maximum number of connections supported for this application */
#define gAppMaxConnections_c           8U

/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d               0

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d               0

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d               0

#define gPasskeyValue_c                999999

/*! Repeated Attempts - Mitigation for pairing attacks */
#define gRepeatedAttempts_d             0

/* Enable Advertising Extension shell commands */
#define BLE_SHELL_AE_SUPPORT            1

#if BLE_SHELL_AE_SUPPORT

#define gGapSimultaneousEAChainedReports_c     2

/* User defined payload pattern and length of extended advertising data */
#define SHELL_EXT_ADV_DATA_PATTERN      "\n\rEXTENDED_ADVERTISING_DATA_LARGE_PAYLOAD"
#define SHELL_EXT_ADV_DATA_SIZE        (500U)
#endif /* BLE_SHELL_AE_SUPPORT */

/* Enable Decision Based Advertising Filtering shell commands */
#define BLE_SHELL_DBAF_SUPPORT            0

#if BLE_SHELL_DBAF_SUPPORT

#define gBLE60_d        1
#define gBLE60_DecisionBasedAdvertisingFilteringSupport_d       1
#define gExpmDecisionBasedAdvertisingFilteringBit_d     BIT1

#define gMaxNumDecisionInstructions_c     8U

/* Increase shell buffer size & task stack size to allow longer commands */
#define SHELL_BUFFER_SIZE       128U
#define SHELL_TASK_STACK_SIZE   1200U

#endif /* BLE_SHELL_DBAF_SUPPORT */

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d                    1

/*! The minimum heap size needed (measured with MEM_STATISTICS) */
#define MinimalHeapSize_c               16000

 /*! *********************************************************************************
 *   Xcvr Configuration
 ********************************************************************************** */
#define gAppMaxTxPowerDbm_c             10

/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */
#define gMaxServicesCount_d             6
#define gMaxServiceCharCount_d          6

/* Enable Serial Manager interface */
#define gAppUseSerialManager_c          1

/* Enable 5.0 optional features */
#define gBLE50_d                        1

/* Enable 5.1 optional features */
#define gBLE51_d                        1

/* Enable 5.2 optional features */
#define gBLE52_d                        1

/* Enable EATT */
#define gEATT_d                         1

/* Enable Dynamic GATT database */
#define gGattDbDynamic_d                1

/*! *********************************************************************************
 *     BLE LL Configuration
 ***********************************************************************************/
/*  ble_ll_config.h file lists the parameters with their default values. User can override
 *    the parameter here by defining the parameter to a user defined value. */

#define gAppExtAdvEnable_d                   1
#define gLlScanPeriodicAdvertiserListSize_c (8U)
/* disable autonomous feature exchange */
#define gL1AutonomousFeatureExchange_d 0

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
