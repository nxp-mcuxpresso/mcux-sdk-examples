/*! *********************************************************************************
* Copyright 2020 NXP
* All rights reserved.
*
* \file
*
* This file contains configuration data for the application and stacks
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

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

#ifndef OTA_INTERNAL_STORAGE
/* Specifies whether to use external EEPROM instead of internal for OTA */
#define gOTA_externalFlash_d            1
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
#define gOTA_externalFlash_d            0
#define gOtaEepromPostedOperations_d    0
#define gEepromType_d                   gEepromDevice_InternalFlash_c
#endif

/* Enable uart app console */
#define gUartAppConsole_d               0

/*! *********************************************************************************
 *     App Configuration
 ********************************************************************************** */

#define gAppMaxConnections_c            0

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
#define gImageAuthenticate_c            0

#define gOTADisplayProgress_d           0

/*! *********************************************************************************
 *     Framework Configuration
 ********************************************************************************** */

#define cPWR_UsePowerDownMode                1

#define cPWR_FullPowerDownMode               1

#define cPWR_DiscardRunningTimerForPowerDown 1

/* Settings that apply only when cPWR_FullPowerDownMode is set */
#if cPWR_FullPowerDownMode
/* Optimize Advertising interslot interval in us - Default is 1500us if not set */
#define gPWR_AdvertisingInterSlotInt         1328
#endif

/* If set, enables Kmod data saving in PDM (requires PDM library) */
#define gRadioUsePdm_d                  1
#define gAppUseNvm_d                    0
#define gUsePdm_d                       (gAppUseBonding_d | gAppUsePairing_d | gRadioUsePdm_d)

/* Defines Size for Timer Task*/
#define gTmrTaskStackSize_c             700

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.*/
#define AppPoolsDetails_c \
         _block_size_  32  _number_of_blocks_    5 _eol_  \
         _block_size_  64  _number_of_blocks_    5 _eol_  \
         _block_size_ 128  _number_of_blocks_    5 _eol_  \
         _block_size_ 512  _number_of_blocks_    5 _eol_

/* Defines number of timers needed by the application */
#define gTmrApplicationTimers_c         4

/* Defines number of timers needed by the protocol stack */
#define gTmrStackTimers_c               5

/* Set this define TRUE if the PIT frequency is an integer number of MHZ */
#define gTMR_PIT_FreqMultipleOfMHZ_d    0

/* Enable/Disable Periodical Interrupt Timer */
#define gTMR_PIT_Timestamp_Enabled_d    0

/* Enables / Disables the precision timers platform component */
#define gTimestamp_Enabled_d            0

/* Enable/Disable PANIC catch */
#define gUsePanic_c                     1

/* No header area needs to be reserved as no bootloader */

#ifndef OTA_INTERNAL_STORAGE
#define gOtaVerifyWrite_d               2 /* set to 2 if you wish to compare  the Ciphered - Written - Read - Deciphered buffers */
#endif

#define gBootData_None_c                1

#define gLoggingActive_d                0


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
#define gMaxServicesCount_d         0
#define gMaxBondedDevices_c         0
#define gMaxResolvingListSize_c     6

#define gAppMemPoolId_c 0
#define gZbPoolId_c     1

#define gPdmMemPoolId_c 2
#if gOtaEepromPostedOperations_d
  #define gOtaMemPoolId_c 3
#endif


#define NvmPoolsDetails_c


#define gPdmNbSegments             63 /* number of sectors contained in PDM storage */
#define PdmInternalPoolsDetails_c \
    _block_size_ 512                   _number_of_blocks_  2 _pool_id_(gPdmMemPoolId_c) _eol_ \
    _block_size_ (gPdmNbSegments*12)   _number_of_blocks_  1 _pool_id_(gPdmMemPoolId_c) _eol_


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
   /* Defines NVM pools by block size and number of blocks. Must be aligned to 4 bytes.*/
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



#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
