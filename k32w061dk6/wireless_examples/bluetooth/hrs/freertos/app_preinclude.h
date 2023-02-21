/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019, 2022 NXP
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
#define gLEDSupported_d                 1

/* Defines the number of available keys for the keyboard module */
#define gKBD_KeysCount_c                2

/* Specifies the number of physical LEDs on the target board */
#define gLEDsOnTargetBoardCnt_c         2

/* Use FRO32K instead of XTAL32K in active and power down modes*/
#define gClkUseFro32K                   0

/* Disable uart app console */
#define gUartAppConsole_d               0


/*! *********************************************************************************
 * 	App Configuration
 ********************************************************************************** */
 /*! Number of connections supported by the application */
#define gAppMaxConnections_c            2

/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d                0

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d                0

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d                0

#define gPasskeyValue_c                 999999

#if (gAppUseBonding_d) && (!gAppUsePairing_d)
  #error "Enable pairing to make use of bonding"
#endif

/*! ADV interval in Slots : 2048 matches to 1.28s  */
#define gAppAdvertisingInterval         2048

/*! ADV connectable - Set to 0 if ADV is not connectable*/
#define gAppConnectableAdv_d            1

/* Enable HS Clock to support 2Mbps PHY mode setting */
#define gBleUseHSClock2MbpsPhy_c        0

/* Enable/Disable FSCI Low Power Commands*/
#define gFSCI_IncludeLpmCommands_c      0

/* ADC measurements are done one over gAppADCMeasureCounter wakeup times
 * To do measurements at each wakeup, set it to 0
 * max counter value is 0xffffffff, should be used to skip measurements for a long time
 * default value is 10
 * ADC measurements should be done periodically to monitor battery level and temperature
 * */
//#define gAppADCMeasureCounter_c         10

/*! Use the Lowpower module from the framework :
 * Default lowpower mode for the lowpower module is WFI only
 * For full power down mode, cPWR_FullPowerDownMode shall be set to 1
 */
#define cPWR_UsePowerDownMode           1

/*! Enable Power down modes
 * Need cPWR_UsePowerDownMode to be set to 1 first */
#define cPWR_FullPowerDownMode          1

/*! Settings that apply only when cPWR_FullPowerDownMode is set */
#if cPWR_FullPowerDownMode

/*! Put ADV msg in Scan Response to save power during Advertising event*/
#define APP_MsgInScanRsp 		0

/*! Prevent from disabling the power down mode when switching in connected mode
 * If not set, the powerdown mode will be disabled when going into connected mode */
#define cPWR_NoPowerDownDisabledOnConnect    1

/*! Go to power down even if a timer (not lower timer) is running
 * Warning : timers are lost and not yet recovered on wakeup
 */
#define cPWR_DiscardRunningTimerForPowerDown 1

/*! tells to define configUSE_TICKLESS_IDLE in FreeRTOSConfig */
#define mAppUseTickLessMode_c                1

/* If the gTimestampUseWtimer flag is only used for the tickless mode, in order
 * to reduce the power consumption this flag could be enabled so that only the
 * wtimer1 is used for both os activity scheduling and calculation of the total
 * sleep duration.
 */
//#define gSystickUseWtimer1ForSleepDuration 1

/* Force OSA timed wait : if left undefined OSA operations have no timeout  */
#define mAppTaskWaitTime_c                   (7000U) /* milliseconds */

/*! Use Timebase drift compensation algo in Linklayer when power Down mode is
 * enabled (cPWR_UsePowerDownMode set to 3)
 * To be used with FRO32K (gClkUseFro32K)
 * Allow to decrease Always ON LDO voltage in powerdown for additional power saving
 * in connected mode
 * Keep it disabled for now */
//#define gPWR_UseAlgoTimeBaseDriftCompensate  1

/*! Switch CPU clock to 48MHz FRO at startup - 32MHz (FRO or XTAL) default */
//#define gPWR_CpuClk_48MHz                    1

 /*! Wait for oscillator number of 30us 32kHz ticks - 24 seems to work, 27 is the default value*/
//#define gBleOscWakeDelay_c              (22)

/*!  define gSwdWakeupReconnect_c as 1 to ensure that SWD can wake device up and
 *  attach debugger when powered down */
//#define  gSwdWakeupReconnect_c             1

#if gSwdWakeupReconnect_c
/*! Cannot power down the flash controller, otherwise the SWD fails to read the code from flash */
#define gPWR_FlashControllerPowerDownInWFI   0
#else
/*! Control the power saving of the Flash Controller when CPU is in WFI state
 * 1 - Power down the flash controller and Switch OFF the 48Mhz clock to Flash Controller
 * 2 - Not supported */
#define gPWR_FlashControllerPowerDownInWFI   1
#endif

/*! Reduce the system clock frequency for  CPU / AHB bus/ SRAM during WFI
     This is particularly useful when the CPU is inactive during the Link layer events.
     However, this reduces the number of possible white-list and RAL entries that can be resolved.
     The radio timings need to be trimmed to allow WFI frequency scaling below
      0 : disabled
      32 : reduced down to 32MHz  (no effect if gPWR_CpuClk_48MHz is disabled)
      16 : reduced down to 16MHz : XTAL32M and clock divided by 2 or 48M divided by 3:  Single white-list entry
*/
#define gPWR_FreqScalingWFI                  (16)

/*! BLE Link Layer Fast Correct feature allows a one slot 625us shorter wake up advance */
//#define gBleLL_FastCorrect_d               (0)

/*! In lowpower entry sequence, Disable Link layer to switch to DSM at first step */
//#define gPWR_BleLL_EarlyEnterDsm             (0)

#endif   // cPWR_FullPowerDownMode

/* This setting impacts Slave RX window widening and consequently power consumption
   Customer needs to characterize the 32Khz crystal with regards to temperature and aging
   for better power saving.  keep it to safe value (500ppm) if not done */
#define gLocalSleepClkAccuracyPpm       250

/* Reduce Memory footprint for lowpower in case of low RX throughput application */
#define gController_ReducedRxThoughput  1

#if NDEBUG
#define gBleSlpAlgoDuration           (375)
#endif /* NDEBUG */

/*! *********************************************************************************
 * 	Framework Configuration
 ********************************************************************************** */

#define gMemManagerLight 1
#define gMemManagerLightExtendHeapAreaUsage   1
/* MinimalHeapSize_c need to be adjusted by application */
#define MinimalHeapSize_c                     20000

/*! enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d                    0

/*! If set, enables Kmod data saving in PDM (requires PDM library) */
#define gRadioUsePdm_d                  1

/*! gUsePdm_d is not synonymous to gAppUseNvm_d because PDM is used by Radio driver independantly from NVM */
#define gUsePdm_d                       (gAppUseBonding_d | gAppUsePairing_d | gRadioUsePdm_d)

/*! Defines Num of Serial Manager interfaces */
#define gSerialManagerMaxInterfaces_c   1

/*! Defines Tx Queue Size for Serial Manager */
#define gSerialMgrTxQueueSize_c         30

/*! Defines Size for Serial Manager Task*/
#define gSerialTaskStackSize_c          500

/*! Defines Size for TMR Task*/
#define gTmrTaskStackSize_c             700

/*! Defines pools by block size and number of blocks. Must be aligned to 4 bytes.*/
#define AppPoolsDetails_c \
         _block_size_  32  _number_of_blocks_    60 _eol_  \
         _block_size_  80  _number_of_blocks_    30 _eol_  \
         _block_size_ 128  _number_of_blocks_    30 _eol_  \
         _block_size_ 512  _number_of_blocks_    30 _eol_

/*! Defines number of timers needed by the application */
#define gTmrApplicationTimers_c         8

/* Defines number of timers needed by the protocol stack */
#ifndef gL2caMaxLeCbChannels_c
/* If not yet defined above set default value to 2 */
#define gL2caMaxLeCbChannels_c           (2U)
#endif
#if defined(gAppMaxConnections_c)
    #define gTmrStackTimers_c (2 + ((gAppMaxConnections_c) * 2) + gL2caMaxLeCbChannels_c)
#else
    #define gTmrStackTimers_c (32)
#endif


/*! Set this define TRUE if the PIT frequency is an integer number of MHZ */
#define gTMR_PIT_FreqMultipleOfMHZ_d    0

/*! Enable/Disable Periodical Interrupt Timer */
#define gTMR_PIT_Timestamp_Enabled_d    0

/*! Use 1Hz Timestamping - switch timers from CTIMERS to RTC 16bit timer - 1ms resolution*/
#define gTimestamp_Enabled_d            0

/*! Use Lowpower timers - switch timers from CTIMERS to RTC 16bit timer - 1ms resolution*/
#define gTimerMgrLowPowerTimers         1

/*! Enable/Disable PANIC catch */
#define gUsePanic_c                     1

/*! Debug only */
#define gLoggingActive_d                0
#define DBG_TMR 0
#define DBG_PWR 0
#define DBG_KBD 0

/*! *********************************************************************************
 * 	RTOS Configuration
 ********************************************************************************** */
/*! Defines the RTOS used */
#define FSL_RTOS_FREE_RTOS              1

/*! Defines number of OS events used */
#define osNumberOfEvents                5

/*! Defines main task stack size */
#define gMainThreadStackSize_c          1100

/*! Defines controller task stack size */
#define gControllerTaskStackSize_c      2048

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
#define gMaxServicesCount_d             10
#if defined gAppUseBonding_d && (gAppUseBonding_d > 0)
#define gMaxBondedDevices_c             16
#else
#define gMaxBondedDevices_c             1
#endif
#define gMaxResolvingListSize_c         6

/*! *********************************************************************************
 * 	NVM Module Configuration - gAppUseNvm_d shall be defined aboved as 1 or 0
 ********************************************************************************** */
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
  #define PoolsDetails_c     \
         AppPoolsDetails_c
#endif

/*! *********************************************************************************
 * 	Flag dependencies
 *    Define flags needed by other features to be functional
 *    DO NOT MODIFIED
 ********************************************************************************** */

#if (gAppUseBonding_d) && (!gAppUsePairing_d)
  #error "Enable pairing to make use of bonding"
#endif

#if mAppUseTickLessMode_c
#if defined (gSystickUseWtimer1ForSleepDuration) && gSystickUseWtimer1ForSleepDuration
/* Not required for tickless if gSystickUseWtimer1ForSleepDuration is set to 1 */
#define gTimestampUseWtimer_c 0
#else
/* If Tickless mode is wanted, it is required to base timing on an accurate timebase
 * so counting on 32kHz tick */
#define gTimestampUseWtimer_c 1
#endif
#endif


#endif /* _APP_PREINCLUDE_H_ */


/*! *********************************************************************************
 * @}
 ********************************************************************************** */
