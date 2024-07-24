/*! *********************************************************************************
* \addtogroup App Config
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2019, 2021-2023 NXP
*
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
#include "ble_conn_manager.h"
#include "wireless_uart.h"
#include "gatt_db_handles.h"


/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define smpEdiv                 0x1F99
#define mcEncryptionKeySize_c   16

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
/* Default Advertising Parameters. Values can be changed at runtime
    to align with profile requirements */
gapAdvertisingParameters_t gAdvParams = {
    /* minInterval */         gGapAdvertisingIntervalRangeMinimum_c,
    /* maxInterval */         gGapAdvertisingIntervalRangeMinimum_c,
    /* advertisingType */     gAdvConnectableUndirected_c,
    /* addressType */         gBleAddrTypePublic_c,
    /* directedAddressType */ gBleAddrTypePublic_c,
    /* directedAddress */     {0, 0, 0, 0, 0, 0},
    /* channelMap */          (gapAdvertisingChannelMapFlags_t) (gGapAdvertisingChannelMapDefault_c),
    /* filterPolicy */        gProcessAll_c
};

/* Scanning and Advertising Data */
static uint8_t adData0[1] =  { (gapAdTypeFlags_t)(gLeGeneralDiscoverableMode_c | gBrEdrNotSupported_c) };
static gapAdStructure_t advScanStruct[3] = {
  {
    .length = NumberOfElements(adData0) + 1,
    .adType = gAdFlags_c,
    .aData = (uint8_t *)adData0
  },
  {
    .length = NumberOfElements(uuid_service_wireless_uart) + 1,
    .adType = gAdComplete128bitServiceList_c,
    .aData = (uint8_t *)uuid_service_wireless_uart
  },
  {
    .adType = gAdShortenedLocalName_c,
    .length = 7,
    .aData = (uint8_t*)"NXP_WU"
  }
};

gapAdvertisingData_t gAppAdvertisingData =
{
    NumberOfElements(advScanStruct),
    (void *)advScanStruct
};

gapScanResponseData_t gAppScanRspData =
{
    0,
    NULL
};

gapScanningParameters_t gScanParams =
{
    /* type */              gScanTypeActive_c,
    /* interval */          gGapScanIntervalDefault_d,
    /* window */            gGapScanWindowDefault_d,
    /* ownAddressType */    gBleAddrTypePublic_c,
    /* filterPolicy */      gScanAll_c,
    /* scanning PHY */      gLePhy1MFlag_c
};

/* Default Connection Request Parameters */
gapConnectionRequestParameters_t gConnReqParams =
{
    .scanInterval = 36,
    .scanWindow = 18,
    .filterPolicy = gUseDeviceAddress_c,
    .ownAddressType = gBleAddrTypePublic_c,
    .connIntervalMin = 16,
    .connIntervalMax = 16,
    .connLatency = 0,
    .supervisionTimeout = 0x0C80,
    .connEventLengthMin = 0,
    .connEventLengthMax = 0xFFFF,
    .initiatingPHYs = gLePhy1MFlag_c,
};
#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U))
/* SMP Data */
gapPairingParameters_t gPairingParameters = {
    .withBonding = (bool_t)gAppUseBonding_d,
    .securityModeAndLevel = gSecurityMode_1_Level_3_c,
    .maxEncryptionKeySize = mcEncryptionKeySize_c,
    .localIoCapabilities = gIoKeyboardDisplay_c,
    .oobAvailable = FALSE,
    .centralKeys = (gapSmpKeyFlags_t) (gLtk_c | gIrk_c),
    .peripheralKeys = (gapSmpKeyFlags_t) (gLtk_c | gIrk_c),
    .leSecureConnectionSupported = FALSE,
    .useKeypressNotifications = FALSE,
};
#endif
/* LTK */
static  uint8_t smpLtk[gcSmpMaxLtkSize_c] =
    {0xD6, 0x93, 0xE8, 0xA4, 0x23, 0x55, 0x48, 0x99,
     0x1D, 0x77, 0x61, 0xE6, 0x63, 0x2B, 0x10, 0x8E};

/* RAND*/
static  uint8_t smpRand[gcSmpMaxRandSize_c] =
    {0x26, 0x1E, 0xF6, 0x09, 0x97, 0x2E, 0xAD, 0x7E};

/* IRK */
static  uint8_t smpIrk[gcSmpIrkSize_c] =
    {0x0A, 0x2D, 0xF4, 0x65, 0xE3, 0xBD, 0x7B, 0x49,
     0x1E, 0xB4, 0xC0, 0x95, 0x95, 0x13, 0x46, 0x73};

/* CSRK */
static  uint8_t smpCsrk[gcSmpCsrkSize_c] =
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
};

#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U))
/* Device Security Requirements */
static gapSecurityRequirements_t  deviceSecurity = {
        .securityModeLevel = gSecurityMode_1_Level_3_c,
        .authorization = FALSE,
        .minimumEncryptionKeySize = mcEncryptionKeySize_c
};


gapDeviceSecurityRequirements_t deviceSecurityRequirements = {
    .pSecurityRequirements          = (void*)&deviceSecurity,
    .cNumServices                   = 0,
    .aServiceSecurityRequirements   = NULL
};
#endif