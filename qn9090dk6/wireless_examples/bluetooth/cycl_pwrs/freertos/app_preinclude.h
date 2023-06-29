/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2019 NXP
 *
 * \file
 *
 * This file is the app configuration file which is pre included.
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
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
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

/*! *********************************************************************************
 * 	Board Configuration
 ********************************************************************************** */
#define gLEDSupported_d         1

/* Defines the number of available keys for the keyboard module: counting buttons and TSI keys  */
#define gKBD_KeysCount_c                2

#if (gKBD_KeysCount_c > 0)
#define gKeyBoardSupported_d            1
#else
#define gKeyBoardSupported_d            0
#endif
//#define gKeyEventNotificationMode_d 0 /* experimental before TMR Wakeup is done */

/* Specifies the number of physical LEDs on the target board */
#define gLEDsOnTargetBoardCnt_c         2

#if (gLEDsOnTargetBoardCnt_c > 0)
#define gLEDSupported_d                 1
#else
#define gLEDSupported_d                 0
#endif

/* Use FRO32K instead of XTAL32K in active and power down modes - XTAL32K no longer required */
#define gClkUseFro32K                   0

/* Disable uart app console */
#define gUartAppConsole_d               1

/*! *********************************************************************************
 * 	App Configuration
 ********************************************************************************** */
/*! Maximum number of connections supported for this application. It's up to 8. */
#define gAppMaxConnections_c            8

/* Note: for tests with Android version of IOT Toolbox must define gAppUseBonding_d and gAppUsePairing_d to 0 */
/* Enable/disable use of bonding capability */
#define gAppUseBonding_d                0

/* Enable/disable use of pairing procedure */
#define gAppUsePairing_d                0

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d                0

#define gPasskeyValue_c                 999999

#if (gAppUseBonding_d) && (!gAppUsePairing_d)
  #error "Enable pairing to make use of bonding"
#endif

/* Enamble or disable DC to DC converter */
#define gDCDC_Mode 1

/* Rf Rx Mode:  kRxModeBalanced, kRxModeHighEfficiency  OR  kRxModeHighPerformance
 *  see rx_mode_t enum  */
#define gBleRfRxMode kRxModeHighPerformance

/* Enable HS Clock to support 2Mbps PHY mode setting */
#define gBleUseHSClock2MbpsPhy_c        0

/*! *********************************************************************************
 * 	Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d                    0

/* If set, enables Kmod data saving in PDM (requires PDM library) */
#define gRadioUsePdm_d                  1


/* gUsePdm_d is not synonymous to gAppUseNvm_d because PDM is used by Radio driver independantly from NVM */
#define gUsePdm_d                       (gAppUseBonding_d | gAppUsePairing_d | gRadioUsePdm_d)


/* Defines Num of Serial Manager interfaces */
#define gSerialManagerMaxInterfaces_c   0

/* Defines Size for Timer Task*/
#define gTmrTaskStackSize_c  700

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.*/
#define AppPoolsDetails_c \
         _block_size_  32  _number_of_blocks_    5 _eol_  \
         _block_size_  64  _number_of_blocks_    5 _eol_  \
         _block_size_ 128  _number_of_blocks_    5 _eol_  \
         _block_size_ 512  _number_of_blocks_    5 _eol_

/* Defines number of timers needed by the application */
#define gTmrApplicationTimers_c         4

/*! Defines number of timers needed by the protocol stack */
#ifndef gL2caMaxLeCbChannels_c
/* If not yet defined above set default value to 2 */
#define gL2caMaxLeCbChannels_c           (2U)
#endif
#if defined(gAppMaxConnections_c)
    #define gTmrStackTimers_c (2 + ((gAppMaxConnections_c) * 2) + gL2caMaxLeCbChannels_c)
#else
    #define gTmrStackTimers_c (32)
#endif

/* Set this define TRUE if the PIT frequency is an integer number of MHZ */
#define gTMR_PIT_FreqMultipleOfMHZ_d    0

/* Enable/Disable Periodical Interrupt Timer */
#define gTMR_PIT_Timestamp_Enabled_d    0

/* Enables / Disables the precision timers platform component */
#define gTimestamp_Enabled_d            0

/* Enable/Disable PANIC catch */
#define gUsePanic_c                     1

/* Debug only */
#define gLoggingActive_d                0
#define gLoggingWithExtraTs             0

/*! *********************************************************************************
 * 	RTOS Configuration
 ********************************************************************************** */
/* Defines the RTOS used */
#define FSL_RTOS_FREE_RTOS      1

/* Defines number of OS events used */
#define osNumberOfEvents        5

/* Defines main task stack size */
#define gMainThreadStackSize_c  1100

/* Defines controller task stack size */
#define gControllerTaskStackSize_c 2048

/* 
 * Defines total heap size used by the OS - 12k.
 * BLE Host Stack task requires a larger stack when using NXP Ultrafast EC P256 library,
 * which has become the default, so increase heap size.
*/
#if defined gAppUsePairing_d && (gAppUsePairing_d > 0)
#define gTotalHeapSize_c        0x3280
#else
#define gTotalHeapSize_c        0x3000
#endif

/*! *********************************************************************************
 * 	BLE Stack Configuration
 ********************************************************************************** */
/* Configure the maximum number of bonded devices. If maximum bonded devices reached,
 * user should remove an old bonded device to store new bonded information. Otherwise,
 * demo application will pair with new deivce with No Bonding type.
 */
#if defined gAppUseBonding_d && (gAppUseBonding_d > 0)
#define gMaxBondedDevices_c             16
#else
#define gMaxBondedDevices_c             1
#endif
#define gMaxResolvingListSize_c         6

/*! *********************************************************************************
 * 	NVM Module Configuration - gAppUseNvm_d shall be defined aboved as 1 or 0
 ********************************************************************************** */
/* USER DO NOT MODIFY THESE MACROS DIRECTLY. */
#define gAppMemPoolId_c 0
#if gAppUseNvm_d
  #define gNvmMemPoolId_c 1
  #if gUsePdm_d
    #define gPdmMemPoolId_c 2
  #endif
#else
  #if gUsePdm_d
    #define gPdmMemPoolId_c 1
  #endif
#endif
#if gAppUseNvm_d
    #define gNvmOverPdm_d               1
    /* Defines NVM pools by block size and number of blocks. Must be aligned to 4 bytes.*/
   #define NvmPoolsDetails_c \
         _block_size_   32   _number_of_blocks_   20 _pool_id_(gNvmMemPoolId_c) _eol_ \
         _block_size_ 60   _number_of_blocks_    10 _pool_id_(gNvmMemPoolId_c) _eol_ \
         _block_size_ 80   _number_of_blocks_    10 _pool_id_(gNvmMemPoolId_c) _eol_ \
         _block_size_ 100  _number_of_blocks_    2 _pool_id_(gNvmMemPoolId_c) _eol_

    /* configure NVM module */
    #define  gNvStorageIncluded_d                (1)
    #define  gNvFragmentation_Enabled_d          (1)
    #define  gUnmirroredFeatureSet_d             (0)
    #define  gNvRecordsCopiedBufferSize_c        (512)
#else
#define NvmPoolsDetails_c
#endif

#if gUsePdm_d
   #define gPdmNbSegments             63 /* number of sectors contained in PDM storage */

   #define PdmInternalPoolsDetails_c \
        _block_size_ 512                   _number_of_blocks_  2 _pool_id_(gPdmMemPoolId_c) _eol_ \
        _block_size_ (gPdmNbSegments*12)  _number_of_blocks_  1 _pool_id_(gPdmMemPoolId_c) _eol_
#else
#define PdmInternalPoolsDetails_c
#endif

/*! *********************************************************************************
 * 	Memory Pools Configuration
 ********************************************************************************** */
/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.
 * DO NOT MODIFY THIS DIRECTLY. CONFIGURE AppPoolsDetails_c
 * If gMaxBondedDevices_c increases, adjust NvmPoolsDetails_c
*/

#if gAppUseNvm_d
    #define PoolsDetails_c \
         AppPoolsDetails_c \
         NvmPoolsDetails_c \
         PdmInternalPoolsDetails_c
#elif gUsePdm_d /* Radio drivers uses PDM but no NVM over PDM */
    #define PoolsDetails_c \
         AppPoolsDetails_c \
         PdmInternalPoolsDetails_c
#else
    #define PoolsDetails_c \
         AppPoolsDetails_c
#endif


#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
