/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2019-2024 NXP
* All rights reserved.
*
* \file
*
* This file is the configuration file for the lowpower central reference design application
*
* SPDX-License-Identifier: BSD-3-Clause
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
/**********************************************************************************
 *     Board Configuration
 ***********************************************************************************/

/*! Default board configuration is DCDC in buck mode, Set this flag to zero if your board
      is configured in bypass mode     */
//#define gBoardDcdcBuckMode_d            0

/*! Set Core frequency to 96Mhz , core voltage to 1.1v */
//#define gAppHighSystemClockFrequency_d  1

/*! Enable DCDC frequency stabilization to reduce peaks in Buck mode,
    DCDC will not manage current higher than 40mA */
#define gBoardDcdcFreqStabEnabled_d     1

/**********************************************************************************
 *     Debug Configuration
 ***********************************************************************************/

/*! Enable Debug Console (PRINTF) */
#define gDebugConsoleEnable_d           0

/*! Enable Panic logs through PRINTF */
#define PANIC_ENABLE_LOG                1

/*! Enable NXP proprietary debug */
//#define gDbg_Enabled_d                  1

/**********************************************************************************
 *     Basic Application Configuration
 ***********************************************************************************/

/*! Number of Button required by the application
      - first button : enable / disable Scanning                      */
#define gAppButtonCnt_c                 2

/*! Number of LED required by the application */
#define gAppLedCnt_c                    0

/*! Enable NVM to be used as non volatile storage management by the host stack
    This is mandatory if one of these gAppUseBonding_d, gRngSeedHwParamStorage_d
      or gControllerPreserveXcvrDacTrimValue_d are set */
#define gAppUseNvm_d                    1

/*! Enable Serial Console with external device or host using the LPUART
    Define the Number of Serial Manager interfaces - typical value is 0 or 1
      Lowpower does not support more than 1 serial interface
    Warning   : Memory usage is increased  when set to 1 , roughly :
      1KB of RAM, several KB of Flash (depend of the string size)     */
#define gAppUseSerialManager_c          1

/**********************************************************************************
 *     BLE Application Configuration
 ***********************************************************************************/

/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d                1

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d                1

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d                1

#define gPasskeyValue_c                 999999

#define gMaxServicesCount_d             6

/*! Enable BLE5.0 features Extended Scanning.
       If enabled, the OPT Host lib is required (MUST set lib_ble_OPT_host_cm33_iar.a as lib in linker setting)
       The scanning PHY can be Coded S=8 (LR) only when this flag is set*/
//#define gAppExtAdvEnable_d                1

/*! Set the default Tx power in dBm - Range is between -127 and 5*/
#define mConnectionDefaultTxPower_c     0

/*! Max TX power  , default is 10bDm,  reduce the value to lower the power consumption
    Possible values are 0, 7 or 10.
    shall be higher than mConnectionDefaultTxPower_c */
//#define gAppMaxTxPowerDbm_c        10

/*! Change the PHY mode during connection event for RX and TX
      0 or not defined : No change - default to 1Mb
      1: Coded Phy
      2: 2Mb Phy                         */
//#define gAppConnectionPhyMode             1
//#define gAppConnectionPhyMode             2

/*! Configure scan interval
    Default is gGapScanIntervalDefault_d (0x0010U)
    Max is gGapScanIntervalMax_d (0x4000U)
    Min is gGapScanIntervalMin_d (0x0004U) */
#define gAppScanInterval_d              0x0010U

/*! Configure scan window
    Default is gGapScanWindowDefault_d (0x0010U)
    Max is gGapScanWindowMax_d (0x4000U)
    Min is gGapScanWindowMin_d (0x0004U)
      Set to 0x8 to have roughly 50% duty cycle on Scan */
#define gAppScanWindow_d                0x0008U

/*! Restart Scan when Connection ends
    Application still goes into Deep PowerDown mode on button press during scanning */
#define gAppRestartScanAfterConnect     0

/*! Scanning period timeout in seconds
    The device stops scanning when the timer expires */
#define gAppScanningTimeout_c           30

/*! Application Disconnect Timeout in seconds
    The device will automaticaly disconnect when the timer expires
    Note the lowpower peripheral may disconnect before the timer expires
        Check flag gAppDisconnectOnTimeoutOnly_s on lowpower peripheral app_preinclude.h file */
#define gAppDisconnectTimeout_c         5

/*! Define the source clock accuracy. Possible values:
 * Value |       Accuracy in ppm
 *   0   |       251 ppm to 500 ppm
 *   1   |       151 ppm to 250 ppm
 *   2   |       101 ppm to 150 ppm
 *   3   |        76 ppm to 100 ppm
 *   4   |        51 ppm to  75 ppm
 *   5   |        31 ppm to  50 ppm
 *   6   |        21 ppm to  30 ppm
 *   7   |         0 ppm to  20 ppm
 */
#define BOARD_32KHZ_SRC_CLK_ACCURACY    4

/*! If defined, the application will start a lowpower timer when going into no activity state.
    When the timer expires, the device will wake up and start scanning again.
    If gAppStateNoActivityRamRetention_d is set to 1, this option is useful to be able to restart Scan
    on wake up from timer expiration. */
//#define gAppWakeUpTimerAfterNoActivityMs  60000

/*! \brief Number of half slot before LL interrupt to wakeup the 32MHz of the NBU, depends on the 32Mhz crystal on the
 *  board.
 */
#define BOARD_LL_32MHz_WAKEUP_ADVANCE_HSLOT 2

/*! \brief Delay from the 32MHz wake up of the LL to wake up the radio domain in number of 30.5us period. Default value
    is set to 0x10 in fwk_plaform.c file. The lower this value is, the safest it is, but it increases power consumption.
    If you increase too much this value it is possible to have some issue in LL scheduling event as the power domain
    will be awaken too late. The most power optimized value without impacting LL scheduling depends on the board and the
    value of BOARD_LL_32MHz_WAKEUP_ADVANCE_HSLOT. As an example for BOARD_LL_32MHz_WAKEUP_ADVANCE_HSLOT equals to 2 on
    NXP EVK we can increase it to 0x14.
 */
#define BOARD_RADIO_DOMAIN_WAKE_UP_DELAY 0x10U

/**********************************************************************************
 *     Framework Configuration
 ***********************************************************************************/

/*! Selects the allocation scheme that will be used by the MemManagerLight
    0: Allocates the first free block available in the heap, no matter its size
    1: Allocates the first free block whose size is at most the double of the requested size
    2: Allocates the first free block whose size is at most the 4/3 of the requested size     */
//#define cMemManagerLightReuseFreeBlocks 1

/**********************************************************************************
 *     Lowpower Configuration
 ********************************************************************************** */

/*! Enable/Disable PowerDown functionality in Application - In case of troubles with lowpower,
      turn it off for debug purpose to ensure your application is fine without lowpower  */
#define gAppLowpowerEnabled_d           1

/*! Lowpower Constraint setting for various BLE states (Advertising, Scanning, connected mode)
    The value shall map with the type defintion PWR_LowpowerMode_t in PWR_Interface.h
      0 : no LowPower, WFI only
      1 : Reserved
      2 : Deep Sleep
      3 : Power Down
      4 : Deep Power Down
    Note that if a Ble State is configured to Power Down mode, please make sure
       gLowpowerPowerDownEnable_d variable is set to 1 in Linker Script
    The PowerDown mode will allow lowest power consumption but the wakeup time is longer
       and the first 16K in SRAM is reserved to ROM code (this section will be corrupted on
       each power down wakeup so only temporary data could be stored there.)
	   Power down feature not supported. */

/*! Advertising not supported on central */
//#define gAppLowPowerConstraintInAdvertising_c          2
#define gAppLowPowerConstraintInScanning_c             2
#define gAppLowPowerConstraintInConnected_c            2
#define gAppLowPowerConstraintInNoBleActivity_c        4

/**********************************************************************************
 *     RTOS Configuration
 ***********************************************************************************/

/*! Specific Flags definitions when FreeRTOS is used */
#if defined(SDK_OS_FREE_RTOS)

/*! The following stack sizes have been chosen based on a worst case scenario.
 * For different compilers and optimization levels they can be reduced. */
#define BUTTON_TASK_STACK_SIZE          364
#define SERIAL_MANAGER_TASK_STACK_SIZE  300
#define TM_TASK_STACK_SIZE              324

/*! The Application main task blocks with a timeout of gAppTaskWaitTimeout_ms_c ms
 *  This demonstrates the FreeRTOS tickless support with low power. The device
 *  will go to low power when entering idle, and will wake up after this time
 *  to unblock the main task */
#define gAppTaskWaitTimeout_ms_c        (8000)

#if !defined(gDebugConsoleEnable_d) || (gDebugConsoleEnable_d == 0)

/*! Defines main task stack size */
//#define gMainThreadStackSize_c          1024

/* The size used for the Idle task, in dwords. */
#define configMINIMAL_STACK_SIZE        180

#else
/*! Increase various stack sizes when debug console enabled to allow PRINTF
    gMainThreadStackSize_c is already equal to 2600 for mbedTLS, no need to increase yet */
#define configMINIMAL_STACK_SIZE        400
#endif
#endif


/**********************************************************************************
 *     BLE LL Configuration
 ***********************************************************************************/

/*!  Phy setting depending on gAppConnectionPhyMode previsouly set */
#if defined(gAppConnectionPhyMode ) &&  (gAppConnectionPhyMode == 2)     /* 2Mb Phy */
#define gConnInitiatePhyUpdateRequest_c         (1U)                     /* This flag needs to be set to update the PHY settings */
#define gConnDefaultTxPhySettings_c             (gLePhy2MFlag_c)
#define gConnDefaultRxPhySettings_c             (gLePhy2MFlag_c)
#define gConnPhyUpdateReqTxPhySettings_c        (gLePhy2MFlag_c)
#define gConnPhyUpdateReqRxPhySettings_c        (gLePhy2MFlag_c)

#elif defined(gAppConnectionPhyMode ) &&  (gAppConnectionPhyMode == 1)   /* Coded Phy */
#define gConnInitiatePhyUpdateRequest_c         (1U)                     /* This flag needs to be set to update the PHY settings */
#define gConnDefaultTxPhySettings_c             (gLePhyCodedFlag_c)
#define gConnDefaultRxPhySettings_c             (gLePhyCodedFlag_c)
#define gConnPhyUpdateReqTxPhySettings_c        (gLePhyCodedFlag_c)
#define gConnPhyUpdateReqRxPhySettings_c        (gLePhyCodedFlag_c)
#else
/*!  1Mb Phy default : no change */
#endif

/*! Include common configuration file and board configuration file */
#include "app_preinclude_common.h"
#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
