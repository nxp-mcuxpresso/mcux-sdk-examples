/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright 2019, 2022 - 2024 NXP
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
#define gAppButtonCnt_c                 1


/* Number of LEDs required by the application:
 * Beware! On KW45 PB0 is tied to SPI NOR Flash chip select and monochrome LED.
 * If external flash is to be used, avoid using monochrome LED.
 */
#define gAppLedCnt_c                    2
#define gAppRequireRgbLed_c             1

/*! Enable Debug Console (PRINTF) */
#define gDebugConsoleEnable_d           0

/*! *********************************************************************************
 *     App Configuration
 ********************************************************************************** */
#define gOtapClientAtt_d   1

/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d   1

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d   1

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d   0

#define gPasskeyValue_c                999999

/*! Repeated Attempts - Mitigation for pairing attacks */
#define gRepeatedAttempts_d             0

/*! Define as 1 to place OTA storage in external flash */
#define gAppOtaExternalStorage_c        (0U)

/*! Define the offset where to place the OTA partition storage in external flash */
#define gAppOtaStoragePartitionKbSize_c   (512U)


/*! Define to 1 to post OTA transactions to a queue. The queue will be processed
 * in the idle task. This avoids blocking the system for too long in critical tasks
 * as the write to flash operations will be done during idle period. */
#define gAppOtaASyncFlashTransactions_c    1

/* Define max of consecutive OTA transactions processed during idle task (if gAppOtaASyncFlashTransactions_c is enabled)
 * This can be tuned accordingly to an acceptable block time in idle
 * More transactions means higher block time in idle (if the queue reaches this number of transactions) */
#define gAppOtaMaxConsecutiveTransactions_c (4U)

/*! If gAppOtaASyncFlashTransactions_c is enabled the queue of pending transactions
 *  must be dimensioned to store at least 4 operations to avoid buffer
 *  shortage */
#define gAppOtaNumberOfTransactions_c       (4U)

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d                    1

/*! The minimum heap size needed (measured with MEM_STATISTICS) */
#define MinimalHeapSize_c               13000

/* Enable/Disable FSCI */
#define gFsciIncluded_c                 0

/* Enable/Disable FSCI 2 byte command length */
#define gFsciLenHas2Bytes_c             1

/* Enable/Disable the FSCI BLE OTAP module */
#define gFsciBleOtapEnabled_d           1

/* FSCI payload - set his high enough for long frames support */
#define gFsciMaxPayloadLen_c            520


/* Use Misra Compliant version of FSCI module */
#define gFsciUseDedicatedTask_c         1


/*! *********************************************************************************
 *     RTOS Configuration
 ********************************************************************************** */
#if defined(SDK_OS_FREE_RTOS)
/* The following stack sizes have been chosen based on a worst case scenario.
 * For different compilers and optimization levels they can be reduced. */

#define gHost_TaskStackSize_c           1916

#define BUTTON_TASK_STACK_SIZE          680

#define gFsciTaskStackSize_c            1400

#define SERIAL_MANAGER_TASK_STACK_SIZE  328

#define gMainThreadStackSize_c          1456

#define TM_TASK_STACK_SIZE              696

/* The size used for the Idle task, in dwords. */
#define configMINIMAL_STACK_SIZE        232

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
