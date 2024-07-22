/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright 2019-2023 NXP
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
 * 	App Configuration
 ********************************************************************************** */
/* Enable Extended Advertising */
#define gBeaconAE_c                     1

#if gBeaconAE_c
/* Use very large extended advertising data */
#define gBeaconLargeExtAdvData_c        0

#endif /* gBeaconAE_c */
/*! Set the Tx power in dBm */
#define mAdvertisingDefaultTxPower_c    0

/*! Repeated Attempts - Mitigation for pairing attacks */
#define gRepeatedAttempts_d             0

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* Do not modify. Not used for this application */
#define gAppUseNvm_d                    1

/*! The minimum heap size needed (measured with MEM_STATISTICS) */
#define MinimalHeapSize_c               8500

/*! Enable/Disable PowerDown functionality in Application - In case of troubles with lowpower,
      turn it off for debug purpose to ensure your application is fine without lowpower  */
#define gAppLowpowerEnabled_d           1

/* Disable LEDs when enabling low power */
#if (defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d>0))
  #undef gAppLedCnt_c
  #define gAppLedCnt_c                0
#endif

/* Number of slots(625us) before the wake up instant before which the hardware needs to exit from deep sleep mode. */
#if (gBeaconAE_c && gBeaconLargeExtAdvData_c)
/* need 3 more slots to reload large extended advertising data */
#define cPWR_BLE_LL_OffsetToWakeupInstant 6
#else
#define cPWR_BLE_LL_OffsetToWakeupInstant 3
#endif

/* Enables / Disables MWS coexistence */
#define gMWS_UseCoexistence_d           0

/*! Enable XCVR calibration storage in Flash */
#define gControllerPreserveXcvrDacTrimValue_d     1

/*! Enable the SWD pins to be managed into low-power */
#define gBoard_ManageSwdPinsInLowPower_d    0

/*! *********************************************************************************
 *     RTOS Configuration
 ********************************************************************************** */
#if defined(SDK_OS_FREE_RTOS)
/* The following stack sizes have been chosen based on a worst case scenario. 
 * For different compilers and optimization levels they can be reduced. */

#define gHost_TaskStackSize_c           1296

#define BUTTON_TASK_STACK_SIZE          356

#define SERIAL_MANAGER_TASK_STACK_SIZE  232

#define gMainThreadStackSize_c          776

#define TM_TASK_STACK_SIZE              256

/* The size used for the Idle task, in dwords. */
#define configMINIMAL_STACK_SIZE        100

#endif
/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */

/* Enable 5.3 features, _OPT bluetooth library must be linked */
#define gBLE53_d                        1

/*! *********************************************************************************
 *     BLE LL Configuration
 ***********************************************************************************/
/*  ble_ll_config.h file lists the parameters with their default values. User can override
 *    the parameter here by defining the parameter to a user defined value. */
#define gAppExtAdvEnable_d              1
#define gLlMaxUsedAdvSet_c              2
#define gLlUsePeriodicAdvertising_d     0
#if gBeaconLargeExtAdvData_c
#define gLlMaxExtAdvDataLength_c        1650U
#else
#define gLlMaxExtAdvDataLength_c        250U
#endif /* gBeaconLargeExtAdvData_c */

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
