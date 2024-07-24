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
* This file is the configuration file for the lowpower peripheral reference design application
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
      - first button : enable / disable Advertising
      - second button : print a feedback message - used to test IO wake up source */
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

/*! Set maximum number of connection supported by the application
 *  Set by default to 1 in app_preinclude_common.h
 *  Connection number over 2 is not tested.
 *  You may face issues when setting gAppAdvUseLowPowerTimers to 1.
 *  Best configuration to test 2 connections is:
 *  - using lowpower_central app and a phone.
 *  - bond both devices seperately.
 *  - set gAppDisconnectOnTimeoutOnly_s to 1.
 *  - increase gAppConnectionTimeoutInSecs_c to 30sec. */
//#define gAppMaxConnections_c           (2U)

/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d                1

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d                1

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d                1

#define gPasskeyValue_c                 999999

/*! Enable BLE5.0 features such as Extended Advertising and Extended Scanning.
    If enabled, the OPT Host lib is required (must set lib_ble_OPT_host_cm33_iar.a as lib in linker setting)    */
//#define gAppExtAdvEnable_d              1

/*! Set the default Tx power in dBm - This signed integer value must be less than or equal to gAppMaxTxPowerDbm_c*/
#define mAdvertisingDefaultTxPower_c    0
#define mConnectionDefaultTxPower_c     0

/*! Max TX power  , default is 10bDm,  reduce the value to lower the power consumption
    Possible values are 0, 7 or 10.
    shall be higher than mAdvertisingDefaultTxPower_c and mConnectionDefaultTxPower_c */
#define gAppMaxTxPowerDbm_c        0

/*! ADV interval in Slots : 800 gives 500ms advertising interval */
#define gAppAdvertisingInterval         800

/*! ADV expiration timeout - Stop ADV when timers expires (in seconds) */
#define gAppAdvTimeout_c                30

/*! ADV empty payload - Empty the ADV payload
    Will reduce the payload to the Header (2 bytes) and the advertiser device address (6 bytes).
    This option reduces the power consumption when there is no information to be transmitted in
       advertising message (ADV for connection only or if Data is contained in the
       Scan Rsp payload (see gAppAdvUseScanRspPayload)
    Save power as the TX ADV time duration is smaller */
#define gAppAdvNoPayload_d              0

/*! Put ADV message in the Scan Response
    Useful when gAppAdvNoPayload_d is 1 to save power: data will be transmitted only as scan response
      Warning : Not supported by the lowpower central application - requires app update
      Warning : Meaningless if gAppConnectableAdv_d is 0
    Warning, Size of the ScanRSP payload in addition to ADV payload shall not exceed 46 bytes
      If it exceeds 46 bytes, then the TX interslot interval shall be increased to 1.825ms (3 slots)
      impacting significantly the active time in ADV and power consumption */
#define gAppAdvUseScanRspPayload        0

/*! ADV connectable - Set to 0 if ADV is not connectable,
    Save some power by removing the RX scan after the TX  and reducing the TX interslot interval */
#define gAppConnectableAdv_d            1

/*! Restart ADV after disconnection - Mainly used for stress tests
    Advertising will still be stopped on button press to go to deeper lowpower mode
        (configured by gAppLowPowerConstraintInNoBleActivity_c MACRO)    */
#define gAppRestartAdvAfterConnect      1

/*! Connection timeout value after connection establishement
   If gAppDisconnectOnTimeoutOnly_s is kept to 0, the connection ends when the Connection timeout
      expires. However, in case the temperature request is not received (could be caused by unsuccesful
      pairing or bonding or any other reason), The connection ends on timeout expiration,
      timeout value given by this define.
   Warning: when bonding/pairing is enabled, it can take up to 8 seconds to connect and
      get temperature request. The connection timeout shall be big enough to allow this.        */
#define gAppConnectionTimeoutInSecs_c   15

/*! Disconnect when timeout expires only, do not disconnect when temperature is sent.
    When set to 0, the disconnect happens immediatly after the temperature is sent
    When set to 1, the disconnect happens only on Connection timeout expiration ( value
       given by gAppConnectionTimeoutInSecs_c )
    Set the flag to 1 when carrying power measurement to allow more time in connected mode     */
#define gAppDisconnectOnTimeoutOnly_s   0

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
    When the timer expires, the device will wake up and start advertising again. */
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

/*! Uncomment to enable LPUART0 as wake up source by default
 *  This will prevent WAKE domain to go in retention and keep FRO6M running
 *  so the power consumption will increase during low power period.
 *  This assumes LPUART0 is used with Serial Manager (applicative serial interface)
 *  In our application this feature is disabled by default as we use button1 to dynamically
 *  enable/disable LPUART0 wake up source, see BleApp_HandleKeys1 function */
//#define gAppLpuart0WakeUpSourceEnable_d 1

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

#define gAppLowPowerConstraintInAdvertising_c          2
/* Scanning not supported on peripheral */
//#define gAppLowPowerConstraintInScanning_c             2
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

/*! The size used for the Idle task, in dwords. */
#define configMINIMAL_STACK_SIZE        180

#else
/*! Increase various stack sizes when debug console enabled to allow PRINTF
    gMainThreadStackSize_c is already equal to 2600 for mbedTLS, no need to increase yet */
#define configMINIMAL_STACK_SIZE        400
#endif
#endif


/**********************************************************************************
 *     Flag dependencies
 ***********************************************************************************/

#if defined(gAppExtAdvEnable_d) && (gAppExtAdvEnable_d == 1)
/*! Allow application to support LePhy 1M, 2M and Coded */
#define gConnDefaultTxPhySettings_c             (gLePhy1MFlag_c | gLePhy2MFlag_c | gLePhyCodedFlag_c)
#define gConnDefaultRxPhySettings_c             (gLePhy1MFlag_c | gLePhy2MFlag_c | gLePhyCodedFlag_c)
#endif

/*! Time between the beginning of two consecutive advertising PDU's        */
#if ( gAppExtAdvEnable_d || !gAppConnectableAdv_d )
/*! If Extended ADV is set or device is not connectable, and LR feature is not used,
    TX interslot can be reduced to 0.625ms only.
   If both Primary PHY and Secondary PHY are set to CODED (s=8), Tx interslot
    needs to be 1.875ms  */
#define mcAdvertisingPacketInterval_c   0x01      /* 0.625 msec */
#else
/*! If payload size is below 3 bytes, set value to 0x02 (See gAppAdvUseScanRspPayload)
   If above 3 bytes => set to 0x03 (1.875ms) to allow scan req/scan to happen
   between 2 ADV TX                                                       */
#define mcAdvertisingPacketInterval_c   0x02      /* 1.25 msec */
#endif

/*! Include common configuration file and board configuration file */
#include "app_preinclude_common.h"
#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
