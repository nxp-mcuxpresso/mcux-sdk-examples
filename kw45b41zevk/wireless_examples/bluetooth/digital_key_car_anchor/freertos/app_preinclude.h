/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright 2020-2024 NXP
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
#define gAppButtonCnt_c                 2

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

#define gCentralInitiatedPairing_d
#define gUseCustomIRK_d                 1

#define APP_BD_ADDR     {0xC5, 0xBC, 0x70, 0x37, 0x60, 0xC4}
#define APP_SMP_IRK     {0x0A, 0x2D, 0xF4, 0x65, 0xE3, 0xBD, 0x7B, 0x49, 0x1E, 0xB4, 0xC0, 0x95, 0x95, 0x13, 0x46, 0x73}

/*! BLE CCC Digital Key UUID */
#define gBleSig_CCC_DK_UUID_d                    0xFFF5U

#define gAppUseShellInApplication_d     1

#define gAppLowpowerEnabled_d           0
/* Disable LEDs when enabling low power */
#if (defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d>0))
  #undef gAppLedCnt_c
  #define gAppLedCnt_c                    0
  #define gAppLpuart0WakeUpSourceEnable_d 1
#endif

/*! *********************************************************************************
 *     CCC Configuration
 ********************************************************************************** */
/* Scan for non-CCC key fobs */
#define gAppScanNonCCC_d                        0

#define gcAdvertisingIntervalCCC_1M_c           (68U)   /* 42.5 ms */
#define gcAdvertisingIntervalCCC_CodedPhy_c     (135U)  /* 84.3 ms */

#define gDK_DefaultVehiclePsm_c                 (0x0081U) /* Range 0x0080-0x00FF */
#define gDKMessageMaxLength_c                   (255U)
#define mAppLeCbInitialCredits_c                (32768U)
#define gCCCL2caTimeout_c                       (5U)    /* Seconds */
#define gDummyPayload_c       {0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC}

#define gDummyPayloadLength_c 16

#define gHandoverDemo_d        0

#define gAppSecureMode_d        0
#define gA2BEnabled_d           0
#define gA2BInitiator_d         0

#if (gHandoverDemo_d == 1)
#define gHandoverIncluded_d 1
#endif /* (gHandoverDemo_d == 1) */

#if (gA2BEnabled_d == 1) && (gAppSecureMode_d == 0)
#error "The A2B feature is available only in Secure Mode"
#endif
#if (gHandoverDemo_d || gA2BEnabled_d)
#define gA2ASerialInterface_d 1
#else
#define gA2ASerialInterface_d 0
#endif /* (gHandoverDemo_d || gAppSecureMode_d) */

/* Coding scheme for passive entry */
#define mLongRangeAdvCodingScheme_c     gAdv_CodingScheme_S2_S2_c

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d                    1

/*! The minimum heap size needed (measured with MEM_STATISTICS) */
#define MinimalHeapSize_c               14000

#define SHELL_BUFFER_SIZE               (128U)
#define SHELL_TASK_STACK_SIZE           (1200U)
#define gMainThreadStackSize_c          (1300U)
#define TM_TASK_STACK_SIZE              (512U)
#define BUTTON_TASK_STACK_SIZE          (512U)
#define SERIAL_MANAGER_TASK_STACK_SIZE  (512U)
#define gHost_TaskStackSize_c           (1760U)
/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */
#define gLegacyAdvSetHandle_c               	0x00U
#define gExtendedAdvSetHandle_c             	0x01
#define gNoAdvSetHandle_c                   	0xFF

#define gAppMaxConnections_c                	8U

/* Must open an L2CAP channel for each CCC peer */
#define gL2caMaxLeCbChannels_c              	gAppMaxConnections_c

#define gAppExtAdvEnable_d                  	1

 /* Enable Serial Manager interface */
#if gA2ASerialInterface_d
#define gAppUseSerialManager_c                2
#else
#define gAppUseSerialManager_c                1
#endif

/* Enable 5.0 optional features */
#define gBLE50_d                              1

/* Enable 5.1 optional features */
#define gBLE51_d                              1

/* Enable 5.2 optional features */
#define gBLE52_d                              1


/* Enable EATT */
#define gEATT_d                               1

/* Disable GATT caching */
#define gGattCaching_d                        0

/* Disable GATT automatic robust caching */
#define gGattAutomaticRobustCachingSupport_d  0

/* See documentation on how to enable DBAF */
#define gBLE60_DecisionBasedAdvertisingFilteringSupport_d FALSE
/*! *********************************************************************************
 *  Auto Configuration
 ********************************************************************************** */


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
