/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* \file app_preinclude.h
*
* Copyright 2023-2024 NXP
*
* NXP Confidential Proprietary
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from NXP.
********************************************************************************** */

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
#define gAppButtonCnt_c                 1

/* Number of LED required by the application */
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
#define gAppUsePrivacy_d                1

/*! Repeated Attempts - Mitigation for pairing attacks */
#define gRepeatedAttempts_d             0

#define gPasskeyValue_c                 999999

/*! Set the Tx power in dBm */
#define mAdvertisingDefaultTxPower_c    0

/*! Advertising interval */
#define gcAdvertisingInterval_c         42

/*! Display distance measurement related timing information */
#define gAppCsTimeInfo_d                0

/*! Display RSSI and Tone Quality Indicator information */
#define gAppParseQualityInfo_d          0

#define gAppDisableControllerLowPower_d     1

/*! Enable/Disable PowerDown functionality in Application */
#define gAppLowpowerEnabled_d           0

/* Disable LEDs when enabling low power */
#if (defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d>0))
  #undef gAppLedCnt_c
  #define gAppLedCnt_c                  0
#endif

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d                    1

/*! The minimum heap size needed (measured with MEM_STATISTICS) */
#define MinimalHeapSize_c               53000

#define SHELL_BUFFER_SIZE               (128U)
#define SHELL_TASK_STACK_SIZE           (1400U)

/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */
#define gAdvSetHandle_c                   0x00U
#define gNoAdvSetHandle_c                       0xFF

#define gAppMaxConnections_c                    1

#define gcGapMaximumActiveConnections_c         gAppMaxConnections_c

#define gAppExtAdvEnable_d                      1

#define gAppUseSerialManager_c                  1

/* Enable BLE 5.0 */
#define gBLE50_d                                1

/* Enable 5.1 features */
#define gBLE51_d                                1

/* Enable 5.2 features */
#define gBLE52_d                                1

/* Disable GATT caching */
#define gGattCaching_d                          0

/* Disable GATT automatic robust caching */
#define gGattAutomaticRobustCachingSupport_d    0

/* Enable Channel Sounding */
#define gBLE_ChannelSounding_d                  1

/* Values match the csRoleType enum */
#define gCsInitiatorRole_c                      BIT0
#define gCsReflectorRole_c                      BIT1

/* Channel Sounding role - default reflector */
#define gCsRole_c                               gCsReflectorRole_c

/* Enable the RAS role of Ranging Requestor */
#define gRasRREQ_d                              1

/* Enalbe Real-Time Data Transfer */
#define gAppRealTimeDataTransfer_c              0

/* Configure high speed CPU clock (96 MHz) */
#define gAppHighSystemClockFrequency_d          1

#define gHost_TaskStackSize_c                   1800

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
