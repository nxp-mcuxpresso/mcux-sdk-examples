/*! *********************************************************************************
* \addtogroup App Config
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* This file contains configuration data for the application and stack
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "gap_interface.h"
#include "ble_constants.h"
#if !defined(MULTICORE_APPLICATION_CORE) || (!MULTICORE_APPLICATION_CORE)
#include "gatt_db_handles.h"
#else
#define UUID128(name, ...) extern uint8_t name[16];
#include "gatt_uuid128.h"
#undef UUID128
#include "dynamic_gatt_database.h"
#endif
#include "ble_conn_manager.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define smpEdiv                         0x1F99
#define mcEncryptionKeySize_c           16

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

#if gAppExtAdvEnable_d
gapExtAdvertisingParameters_t gAppExtAdvParams =
{
    /* SID */                       	0, \
    /* handle */                    	0, \
    /* minInterval */               	gAppAdvertisingInterval, \
    /* maxInterval */               	gAppAdvertisingInterval, \
    /* ownAddrType */                   gBleAddrTypePublic_c,\
    /* ownAddress */             	{0, 0, 0, 0, 0, 0}, \
    /* peerAddrType */                  gBleAddrTypePublic_c,\
    /* peerAddress */                   {0, 0, 0, 0, 0, 0}, \
    /* channelMap */                    (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */                  gProcessAll_c, \
    /* extAdvProperties */     	        (bleAdvRequestProperties_t)(gAdvReqConnectable_c | gAdvIncludeTxPower_c), \
    /* TxPower */                       mAdvertisingDefaultTxPower_c, \
    /* primaryPHY */                    (gapLePhyMode_t)gLePhy1M_c, \
    /* secondaryPHY */                  (gapLePhyMode_t)gLePhy1M_c, \
    /* secondaryAdvMaxSkip */           0, \
    /* enableScanReqNotification*/      FALSE \
};
#endif

/* Default Advertising Parameters. Values can be changed at runtime
    to align with profile requirements */
gapAdvertisingParameters_t gAdvParams = {
    /* minInterval */         gAppAdvertisingInterval,
    /* maxInterval */         gAppAdvertisingInterval,
#if gAppConnectableAdv_d
    /* advertisingType */     gAdvConnectableUndirected_c,
#else
    /* advertisingType */       gAdvNonConnectable_c,
#endif
    /* addressType */         gBleAddrTypePublic_c,
    /* directedAddressType */ gBleAddrTypePublic_c,
    /* directedAddress */     {0, 0, 0, 0, 0, 0},
    /* channelMap */          (gapAdvertisingChannelMapFlags_t) (gGapAdvertisingChannelMapDefault_c),
    /* filterPolicy */        gProcessAll_c
};

/* Scanning and Advertising Data */
#if !defined(gAppAdvNoPayload_d) || (gAppAdvNoPayload_d == 0)
static const uint8_t adData0[1] =  { (uint8_t)gLeGeneralDiscoverableMode_c | (uint8_t)gBrEdrNotSupported_c };
static const gapAdStructure_t advScanStruct[3] = {
  {
    .length = NumberOfElements(adData0) + 1,
    .adType = gAdFlags_c,
    .aData = (uint8_t *)adData0
  },
  {
    .length = NumberOfElements(uuid_service_temperature) + 1,
    .adType = gAdComplete128bitServiceList_c,
    .aData = (uint8_t *)uuid_service_temperature
  },
  {
    .adType = gAdShortenedLocalName_c,
    .length = 9,
    .aData = (uint8_t*)"NXP_TEMP"
  }
};
#endif

/* Set ADV payload  */
gapAdvertisingData_t gAppAdvertisingData =
{
#if defined(gAppAdvNoPayload_d) && (gAppAdvNoPayload_d == 1)
    0,
    NULL
#else
    NumberOfElements(advScanStruct),
    (void *)advScanStruct
#endif
};

/* Set SCAN_RSP payload - See limitation on the SCAN_RSP payload size in comment for
      gAppAdvUseScanRspPayload setting in app_preinclude.h */
gapScanResponseData_t gAppScanRspData =
{
#if defined(gAppAdvUseScanRspPayload) && (gAppAdvUseScanRspPayload == 1)
    NumberOfElements(advScanStruct),
    (void *)advScanStruct
#else
    0,
    NULL
#endif
};

/* SMP Data */
gapPairingParameters_t gPairingParameters = {
    .withBonding = (bool_t)gAppUseBonding_d,
    .securityModeAndLevel = gSecurityMode_1_Level_3_c,
    .maxEncryptionKeySize = mcEncryptionKeySize_c,
    .localIoCapabilities = gIoDisplayOnly_c,
    .oobAvailable = FALSE,
    .centralKeys = (gapSmpKeyFlags_t) (gLtk_c | gIrk_c),
    .peripheralKeys = (gapSmpKeyFlags_t) (gLtk_c | gIrk_c),
    .leSecureConnectionSupported = TRUE,
    .useKeypressNotifications = FALSE,
};

/* LTK */
static uint8_t smpLtk[gcSmpMaxLtkSize_c] =
    {0xD6, 0x93, 0xE8, 0xA4, 0x23, 0x55, 0x48, 0x99,
     0x1D, 0x77, 0x61, 0xE6, 0x63, 0x2B, 0x10, 0x8E};

/* RAND*/
static uint8_t smpRand[gcSmpMaxRandSize_c] =
    {0x26, 0x1E, 0xF6, 0x09, 0x97, 0x2E, 0xAD, 0x7E};

/* IRK */
static uint8_t smpIrk[gcSmpIrkSize_c] =
    {0x0A, 0x2D, 0xF4, 0x65, 0xE3, 0xBD, 0x7B, 0x49,
     0x1E, 0xB4, 0xC0, 0x95, 0x95, 0x13, 0x46, 0x73};

/* CSRK */
static uint8_t smpCsrk[gcSmpCsrkSize_c] =
    {0x90, 0xD5, 0x06, 0x95, 0x92, 0xED, 0x91, 0xD7,
     0xA8, 0x9E, 0x2C, 0xDC, 0x4A, 0x93, 0x5B, 0xF9};

gapSmpKeys_t gSmpKeys = {
    .cLtkSize = mcEncryptionKeySize_c,
    .aLtk = (void *)smpLtk,
    .aIrk = (void *)smpIrk,
    .aCsrk = (void *)smpCsrk,
    .aRand = (void *)smpRand,
    .cRandSize = gcSmpMaxRandSize_c,
    .ediv = smpEdiv,
    .addressType = gBleAddrTypePublic_c,
    .aAddress = NULL
};

/* Device Security Requirements */
static const gapSecurityRequirements_t        deviceSecurity = gGapDefaultSecurityRequirements_d;
static const gapServiceSecurityRequirements_t serviceSecurity[3] = {
  {
    .requirements = {
        .securityModeLevel = gSecurityMode_1_Level_3_c,
        .authorization = FALSE,
        .minimumEncryptionKeySize = gDefaultEncryptionKeySize_d
    },
    .serviceHandle = (uint16_t)service_temperature
  },
  {
    .requirements = {
        .securityModeLevel = gSecurityMode_1_Level_3_c,
        .authorization = FALSE,
        .minimumEncryptionKeySize = gDefaultEncryptionKeySize_d
    },
    .serviceHandle = (uint16_t)service_battery
  },
  {
    .requirements = {
        .securityModeLevel = gSecurityMode_1_Level_3_c,
        .authorization = FALSE,
        .minimumEncryptionKeySize = gDefaultEncryptionKeySize_d
    },
    .serviceHandle = (uint16_t)service_device_info
  }
};

gapDeviceSecurityRequirements_t deviceSecurityRequirements = {
    .pSecurityRequirements          = (void*)&deviceSecurity,
    .cNumServices                   = 3,
    .aServiceSecurityRequirements   = (void*)serviceSecurity
};
