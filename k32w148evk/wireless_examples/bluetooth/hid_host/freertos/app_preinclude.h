/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright 2019, 2021 - 2023 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#define BUTTON_TASK_STACK_SIZE          356

#define SERIAL_MANAGER_TASK_STACK_SIZE  232

#define gMainThreadStackSize_c          880

#define TM_TASK_STACK_SIZE              256

#define SHELL_TASK_STACK_SIZE           500

/* The size used for the Idle task, in dwords. */
#define configMINIMAL_STACK_SIZE        140

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
