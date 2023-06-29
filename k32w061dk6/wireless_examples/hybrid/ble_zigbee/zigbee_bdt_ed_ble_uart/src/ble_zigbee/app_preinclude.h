/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*
* Copyright 2020 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef _APP_PREINCLUDE_H_
#define _APP_PREINCLUDE_H_


/*! *********************************************************************************
 *     Board Configuration
 ********************************************************************************** */
#define gLEDSupported_d                 1

/* Defines the number of available keys for the keyboard module */
#define gKBD_KeysCount_c                2
#define gKeyBoardSupported_d            1

/* Specifies the number of physical LEDs on the target board */
#define gLEDsOnTargetBoardCnt_c         2

/* Specifies whether to use external EEPROM instead of internal for OTA */
#define gOTA_externalFlash_d            1
#if (gOTA_externalFlash_d!=0)
/* Specifies the type of EEPROM available on the target board */
#define gEepromType_d                   gEepromDevice_MX25R8035F_c
#define gEepromPostedOperations_d        1
#define gOtaEepromPostedOperations_d     1
#define gEepromSupportReset              1
#define gFlashEraseDuringWrite           0

#if (gFlashEraseDuringWrite!=0)
#define gFlashBlockBitmap_d
#endif
#else
#define gOtaEepromPostedOperations_d    0
#define gEepromType_d                   gEepromDevice_InternalFlash_c
#endif

/* Enable uart app console */
#define gUartAppConsole_d               1

/*! *********************************************************************************
 *     App Configuration
 ********************************************************************************** */
/* dual mode switch logs */
#define APP_SERIAL_LOGS_ENABLED
#define APP_DUAL_MODE_DEBUG            1

#define gAppMaxConnections_c           1

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

/* Enable HS Clock to support 2Mbps PHY mode setting */
#define gBleUseHSClock2MbpsPhy_c        0

/* Enable image certificate authenticate */
#define gImageAuthenticate_c            1

#define gOTADisplayProgress_d           0

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */

/* If set, enables Kmod data saving in PDM (requires PDM library) */
#define gRadioUsePdm_d                  1

#define gUsePdm_d                       (gAppUseBonding_d | gAppUsePairing_d | gRadioUsePdm_d)

/* Defines Rx Buffer Size for Serial Manager */
#define gSerialMgrRxBufSize_c           1000

#define gSerialMgrUseCustomInterface_c  1

#ifndef APP_SERIAL_LOGS_ENABLED
#define gSerialManagerMaxInterfaces_c   1
#else
#define gSerialManagerMaxInterfaces_c   2
#endif

/* Defines Tx Queue Size for Serial Manager */
#define gSerialMgrTxQueueSize_c         30

/* Defines Size for Serial Manager Task*/
#define gSerialTaskStackSize_c          500

/* Defines Size for Timer Task*/
#define gTmrTaskStackSize_c             500

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.*/
#define AppPoolsDetails_c \
         _block_size_  32  _number_of_blocks_    8 _eol_  \
         _block_size_  64  _number_of_blocks_    5 _eol_  \
         _block_size_ 128  _number_of_blocks_    4 _eol_  \
         _block_size_ 512  _number_of_blocks_   10 _eol_

/* Defines number of timers needed by the application */
#define gTmrApplicationTimers_c         8

/* Defines number of timers needed by the protocol stack */
#define gTmrStackTimers_c               32

/* Enables / Disables the precision timers platform component */
#define gTimestamp_Enabled_d            1

/* Enable/Disable PANIC catch */
#define gUsePanic_c                     1

/* No header area needs to be reserved as no bootloader */

#if (gOTA_externalFlash_d!=0)
#define gOtaVerifyWrite_d               2 /* set to 2 if you wish to compare  the Ciphered - Written - Read - Deciphered buffers */
#endif

#define gBootData_None_c                1

#define gLoggingActive_d                0

/* Use the Lowpower module from the framework :
 * Default lowpower mode for the lowpower module is WFI only
 * For full power down mode, cPWR_FullPowerDownMode shall be set to 1
 */
#define cPWR_UsePowerDownMode                1

/* Enable Power down modes
 * Need cPWR_UsePowerDownMode to be set to 1 first */
#define cPWR_FullPowerDownMode               1

/*!=================================================================================================
* SHELL
==================================================================================================*/
#define SHELL_USE_LOGO 0
#define SHELL_USE_AUTO_COMPLETE 0
#define SHELL_USE_ALT_TASK 0
#define SHELL_MAX_COMMANDS 20
#define SHELL_IO_TYPE (gSerialMgrCustom_c)
#define SHELL_IO_NUMBER 2
#define SHELL_USE_ECHO 0
#define SHELL_CB_SIZE 200

/*! *********************************************************************************
 *     RTOS Configuration
 ********************************************************************************** */
/* Defines number of OS events used */
#define osNumberOfEvents                5

/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */
/* Configure the maximum number of bonded devices. If maximum bonded devices reached,
 * user should remove an old bonded device to store new bonded information. Otherwise,
 * demo application will pair with new deivce with No Bonding type.
 */
#define gMaxServicesCount_d         10
#define gMaxBondedDevices_c             16
#define gMaxResolvingListSize_c         6

#define gAppMemPoolId_c 0
#define gZbPoolId_c     1

#if gUsePdm_d
#define gPdmMemPoolId_c 2
#if gOtaEepromPostedOperations_d
#define gOtaMemPoolId_c 3
#endif
#else
#if gOtaEepromPostedOperations_d
#define gOtaMemPoolId_c 1
#endif
#endif

#define NvmPoolsDetails_c

#if gUsePdm_d
   #define gPdmNbSegments             63 /* number of sectors contained in PDM storage */
   #define PdmInternalPoolsDetails_c \
        _block_size_ 512                   _number_of_blocks_  2 _pool_id_(gPdmMemPoolId_c) _eol_ \
        _block_size_ (gPdmNbSegments*12)   _number_of_blocks_  1 _pool_id_(gPdmMemPoolId_c) _eol_
#else
#define PdmInternalPoolsDetails_c
#endif
#if gOtaEepromPostedOperations_d

#if gOTA_externalFlash_d
    #define gOtaEepromPostedOperations_d 1
    #define gUsePasswordCiphering_d 0
    #define gExternalFlashIsCiphered_d 1
    #define PROGRAM_PAGE_SZ (256)
    #define  OTA_NB_PENDING_TRANSACTIONS (4)
#else
    /* Internal flash page size is 512: accumulate whole pages before writing */
    #define PROGRAM_PAGE_SZ (512)
    #define  OTA_NB_PENDING_TRANSACTIONS (2)
#endif
    #define OTA_TRANSACTION_BUFFER_SZ (PROGRAM_PAGE_SZ + 12)
    #define OtaPoolDetails_c \
         _block_size_ OTA_TRANSACTION_BUFFER_SZ  _number_of_blocks_    OTA_NB_PENDING_TRANSACTIONS _pool_id_(gOtaMemPoolId_c) _eol_
#else
/* Nothing */
#define OtaPoolDetails_c
#endif

/*! *********************************************************************************
 *  ZIGBEE memory pool Configuration
 ********************************************************************************** */

#define ZigbeePoolsDetails_c \
         _block_size_  64  _number_of_blocks_    8 _pool_id_(0) _eol_  \
         _block_size_ 128  _number_of_blocks_    2 _pool_id_(0) _eol_  \
         _block_size_ 256  _number_of_blocks_    6 _pool_id_(0) _eol_

/*! *********************************************************************************
 *     Memory Pools Configuration
 ********************************************************************************** */

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.
 * DO NOT MODIFY THIS DIRECTLY. CONFIGURE AppPoolsDetails_c
 * If gMaxBondedDevices_c increases, adjust PdmInternalPoolsDetails_c
*/
#if gUsePdm_d /* Radio drivers uses PDM */
  #if gOtaEepromPostedOperations_d
    #define PoolsDetails_c \
         AppPoolsDetails_c \
         ZigbeePoolsDetails_c \
         PdmInternalPoolsDetails_c \
         OtaPoolDetails_c
  #else
    #define PoolsDetails_c \
         AppPoolsDetails_c \
         ZigbeePoolsDetails_c \
         PdmInternalPoolsDetails_c 
  #endif
#else /* gUsePdm_d not defined */
  #if gOtaEepromPostedOperations_d
  #define PoolsDetails_c     \
         AppPoolsDetails_c \
         ZigbeePoolsDetails_c \
         OtaPoolDetails_c
  #else
  #define PoolsDetails_c     \
         AppPoolsDetails_c \
         ZigbeePoolsDetails_c
  #endif
#endif


#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
