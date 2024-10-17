/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright 2019-2024 NXP
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
#define gAppLedCnt_c                    0

/*! *********************************************************************************
 * 	App Configuration
 ********************************************************************************** */
/*! Maximum number of connections supported for this application */
#define gAppMaxConnections_c            8U

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
 /* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d                    1

/*! *********************************************************************************
 *     HKB Definition
 ********************************************************************************** */
#define SERIAL_MANAGER_NON_BLOCKING_DUAL_MODE   0

/*! The minimum heap size needed (measured with MEM_STATISTICS) */
#define MinimalHeapSize_c               17000

/* Enable/Disable FSCI */
#define gFsciIncluded_c                 1

/* Enable/Disable FSCI Low Power Commands*/
#define gFSCI_IncludeLpmCommands_c      0

/* Defines FSCI length - set this to FALSE is FSCI length has 1 byte */
#define gFsciLenHas2Bytes_c             1

/* Defines FSCI maximum payload length */
#define gFsciMaxPayloadLen_c            1660

/* Enable/Disable Ack transmission */
#define gFsciTxAck_c                    0

/* Enable/Disable Ack reception */
#define gFsciRxAck_c                    0

/* Enable FSCI Rx restart with timeout */
#define gFsciRxTimeout_c                1
#define mFsciRxTimeoutUsePolling_c      1

/* Use Misra Compliant version of FSCI module */
#define gFsciUseDedicatedTask_c         1
/*! *********************************************************************************
 *     RTOS Configuration
 ********************************************************************************** */
#if defined(SDK_OS_FREE_RTOS)
/* Fsci Task Stack size */
#if defined(DEBUG)
#define gFsciTaskStackSize_c            4600
#else
#define gFsciTaskStackSize_c            2600
#endif

#define gHost_TaskStackSize_c           1760

#endif

/*! Enable/Disable PowerDown functionality in Application - In case of troubles with lowpower,
      turn it off for debug purpose to ensure your application is fine without lowpower  */
#define gAppLowpowerEnabled_d           0

/*! Enable/Disable to enable LPUART0 as wake up source by default
 *  This will prevent WAKE domain to go in retention and keep FRO6M running
 *  so the power consumption will increase during low power period.
 *  This assumes LPUART0 is used with Serial Manager (applicative serial interface).
 *  No effect if low power is disabled. */
#define gAppLpuart0WakeUpSourceEnable_d 0
          
/*! Lowpower Constraint setting for various BLE states (Advertising, Scanning, connected mode)
    The value shall map with the type defintion PWR_LowpowerMode_t in PWR_Interface.h
      0 : no LowPower, WFI only
      1 : Reserved
      2 : Deep Sleep
      3 : Power Down
    Note that if a Ble State is configured to Power Down mode, please make sure
       gLowpowerPowerDownEnable_d variable is set to 1 in Linker Script
    The PowerDown mode will allow lowest power consumption but the wakeup time is longer
       and the first 16K in SRAM is reserved to ROM code (this section will be corrupted on
       each power down wakeup so only temporary data could be stored there.)     */          
#define gAppLowPowerModeConstraints_c   2
          
/* Disable LEDs when enabling low power */
#if (defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d>0))
  #undef gAppLedCnt_c
  #define gAppLedCnt_c                0
#endif

/*! Enable the SWD pins to be managed into low-power */
#define gBoard_ManageSwdPinsInLowPower_d    0

/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */

/* Enable Serial Manager interface */
#define gAppUseSerialManager_c              1

#define gUseHciTransport_d                  0

/* Enable/Disable Dynamic GattDb functionality */
#define gGattDbDynamic_d                    1

/* Enable FSCI BLE blackbox functionality */
#define gFsciBleBBox_d                      1

/* Indicates the Host layers which are enabled
 * for FSCI communication*/
#define gFsciBleEnabledLayersMask_d         0x0164

/* Enable 5.3 optional features */
#define gBLE53_d                            1

#define gGapSimultaneousEAChainedReports_c  2

/*! *********************************************************************************
 *     BLE LL Configuration
 ***********************************************************************************/
/*  ble_ll_config.h file lists the parameters with their default values. User can override
 *    the parameter here by defining the parameter to a user defined value. */
#define gAppExtAdvEnable_d                      1
#define gLlScanPeriodicAdvertiserListSize_c     8

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
