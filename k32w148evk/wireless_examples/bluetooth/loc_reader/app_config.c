/*! *********************************************************************************
* \addtogroup App Config
* @{
********************************************************************************** */
/*! *********************************************************************************
* \file app_config.c
*
* Copyright 2023 NXP
*
* NXP Confidential Proprietary
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from NXP.
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "gap_interface.h"
#include "ble_constants.h"
#include "gatt_db_handles.h"
#include "ble_conn_manager.h"
#include "app_conn.h"
#include "loc_reader.h"
/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define smpEdiv                 0x1F99
#define mcEncryptionKeySize_c   16

#define mDefaultTxPower         gBleAdvTxPowerNoPreference_c
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
/* Default Advertising Parameters */
gapExtAdvertisingParameters_t gAdvParams =
{
    /* SID */                       0xB, \
    /* handle */                    gAdvSetHandle_c, \
    /* minInterval */               gcAdvertisingInterval_c, \
    /* maxInterval */               gcAdvertisingInterval_c, \
    /* ownAddrType */               gBleAddrTypePublic_c,\
    /* ownAddress */                {0, 0, 0, 0, 0, 0}, \
    /* peerAddrType */              gBleAddrTypePublic_c,\
    /* peerAddress */               {0, 0, 0, 0, 0, 0}, \
    /* channelMap */                (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */              gProcessAll_c, \
    /* extAdvProperties */          (bleAdvRequestProperties_t)(gAdvReqConnectable_c | gAdvReqScannable_c | gAdvReqLegacy_c), \
    /* TxPower */                   mDefaultTxPower, \
    /* primaryPHY */                (gapLePhyMode_t)gLePhy1M_c, \
    /* secondaryPHY */              (gapLePhyMode_t)gLePhy1M_c, \
    /* secondaryAdvMaxSkip */       0, \
    /* enableScanReqNotification*/  FALSE \
};

/*Default Application Advertising Parameters */
appExtAdvertisingParams_t gAppAdvParams =
{
    &gAdvParams,
    &gAppAdvertisingData,
    &gAppScanRspData,
    gAdvSetHandle_c,
    gBleExtAdvNoDuration_c,
    gBleExtAdvNoMaxEvents_c
};
/* Advertising Data */
static gapAdStructure_t advScanStruct[1] = {
  {
    .length = 8U,
    .adType = gAdShortenedLocalName_c,
    .aData = (uint8_t*)"NXP_LOC"
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

/* SMP Data */
gapPairingParameters_t gPairingParameters = {
    .withBonding = (bool_t)gAppUseBonding_d,
    .securityModeAndLevel = gSecurityMode_1_Level_4_c,
    .maxEncryptionKeySize = mcEncryptionKeySize_c,
    .localIoCapabilities = gIoKeyboardDisplay_c,
    .oobAvailable = FALSE,
    .centralKeys = (gapSmpKeyFlags_t) (gIrk_c | gLtk_c),
    .peripheralKeys = (gapSmpKeyFlags_t) (gIrk_c | gLtk_c),
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
