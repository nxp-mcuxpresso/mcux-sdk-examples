/*! *********************************************************************************
* \addtogroup App Config
* @{
********************************************************************************** */
/*! *********************************************************************************
* \file app_config.c
*
* Copyright 2020-2024 NXP
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
#include "gatt_db_handles.h"
#include "ble_conn_manager.h"
#include "app_conn.h"
#include "digital_key_car_anchor.h"
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
#if (defined(gAppScanNonCCC_d) && (gAppScanNonCCC_d == 1U))
gapScanningParameters_t gScanParams =
{
    /* type */              gScanTypePassive_c,
    /* interval */          gGapScanIntervalDefault_d,
    /* window */            gGapScanWindowDefault_d,
    /* ownAddressType */    gBleAddrTypePublic_c,
    /* filterPolicy */      (uint8_t)gScanAll_c,
    /* scanning PHY */      (uint8_t)gLePhy1MFlag_c
};

/* Default Connection Request Parameters */
gapConnectionRequestParameters_t gConnReqParams =
{
    .scanInterval = 36,
    .scanWindow = 36,
    .filterPolicy = (uint8_t)gUseDeviceAddress_c,
    .ownAddressType = gBleAddrTypePublic_c,
    .connIntervalMin = gcConnectionIntervalMinDefault_c,
    .connIntervalMax = gcConnectionIntervalMinDefault_c,
    .connLatency = 0,
    .supervisionTimeout = 0x03E8,
    .connEventLengthMin = 0,
    .connEventLengthMax = 0xFFFF,
    .initiatingPHYs = (uint8_t)gLePhy1MFlag_c,
};
#endif

/* Default Legacy Advertising Parameters */
gapExtAdvertisingParameters_t gLegacyAdvParams =
{
    /* SID */                       0xB, \
    /* handle */                    gLegacyAdvSetHandle_c, \
    /* minInterval */               gcAdvertisingIntervalCCC_1M_c /* 42 ms */, \
    /* maxInterval */               gcAdvertisingIntervalCCC_1M_c /* 42 ms */, \
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

/*Default Extended Advertising Parameters */
gapExtAdvertisingParameters_t gExtAdvParams =
{
    /* SID */                       0xB, \
    /* handle */                    gExtendedAdvSetHandle_c, \
    /* minInterval */               gcAdvertisingIntervalCCC_CodedPhy_c /* 84 ms */, \
    /* maxInterval */               gcAdvertisingIntervalCCC_CodedPhy_c /* 84 ms */, \
    /* ownAddrType */               gBleAddrTypePublic_c,\
    /* ownAddress */                {0, 0, 0, 0, 0, 0}, \
    /* peerAddrType */              gBleAddrTypePublic_c,\
    /* peerAddress */               {0, 0, 0, 0, 0, 0}, \
    /* channelMap */                (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */              gProcessAll_c, \
    /* extAdvProperties */          (bleAdvRequestProperties_t)(gAdvReqConnectable_c | gAdvIncludeTxPower_c), \
    /* TxPower */                   mDefaultTxPower, \
    /* primaryPHY */                (gapLePhyMode_t)gLePhyCoded_c, \
    /* secondaryPHY */              (gapLePhyMode_t)gLePhyCoded_c, \
    /* secondaryAdvMaxSkip */       0, \
    /* enableScanReqNotification*/  FALSE \

};

#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
/* Default DBAF Decision Data Parameters */
gapAdvertisingDecisionData_t gAdvDecisionData =
{
    /* pKey */                      NULL, \
    /* pPrand */                    NULL, \
    /* pDecisionData */             NULL, \
    /* dataLength */                0, \
    /* resolvableTagPresent */      FALSE \
};
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */

/*Default Application Advertising Parameters */
appExtAdvertisingParams_t gAppAdvParams =
{
    &gLegacyAdvParams,
    &gAppAdvertisingData,
    &gAppScanRspData,
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
    &gAdvDecisionData,
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
    gLegacyAdvSetHandle_c,
    gBleExtAdvNoDuration_c,
    gBleExtAdvNoMaxEvents_c
};
/* Scanning and Advertising Data */
static uint8_t adData0[2] = {UuidArray(gBleSig_CCC_DK_UUID_d)};
static uint8_t adData1[19] = {0xE4, 0xCC, 0xDB, 0xE2, 0x2A, 0x2A, 0xA3, 0xA2, 0xE9, 0x11, 0x99, 0xB4, 0xC0, 0xBB, 0x10, 0x58, /* CCCServiceDataIntent UUID */
                                    0x01, /* Intent for vehicle */
                                    0x00, 0x00}; /* Vehicle manufacturer */

static gapAdStructure_t advScanStruct[2] = {
  {
    .length = 3U,
    .adType = gAdComplete16bitServiceList_c,
    .aData = (uint8_t*)adData0
  },
  {
    .length = 20U,
    .adType = gAdServiceData128bit_c,
    .aData = (uint8_t*)adData1
  }
};

gapAdvertisingData_t gAppAdvertisingData =
{
    NumberOfElements(advScanStruct),
    (void *)advScanStruct
};

gapAdvertisingData_t gAppAdvertisingDataEmpty =
{
    0,
    NULL
};

gapScanResponseData_t gAppScanRspData =
{
    0,
    NULL
};

/* SMP Data */
/* CCC Pairing Parameters
   For non-CCC Key Fobs application should update fields accordingly
*/
gapPairingParameters_t gPairingParameters = {
    .withBonding = (bool_t)gAppUseBonding_d,
    .securityModeAndLevel = gSecurityMode_1_Level_4_c,
    .maxEncryptionKeySize = mcEncryptionKeySize_c,
    .localIoCapabilities = gIoNone_c,
    .oobAvailable = TRUE,
    .centralKeys = (gapSmpKeyFlags_t) (gIrk_c),
    .peripheralKeys = (gapSmpKeyFlags_t) (gIrk_c),
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
static uint8_t smpIrk[gcSmpIrkSize_c] = APP_SMP_IRK;

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
static gapSecurityRequirements_t        deviceSecurity = gGapDefaultSecurityRequirements_d;
static gapServiceSecurityRequirements_t serviceSecurity[1] = {
  {
    .requirements = {
        .securityModeLevel = gSecurityMode_1_Level_1_c,
        .authorization = FALSE,
        .minimumEncryptionKeySize = gDefaultEncryptionKeySize_d
    },
    .serviceHandle = (uint16_t)service_dk
  }
};

gapDeviceSecurityRequirements_t deviceSecurityRequirements = {
    .pSecurityRequirements          = (void*)&deviceSecurity,
    .cNumServices                   = 3,
    .aServiceSecurityRequirements   = (void*)serviceSecurity
};
