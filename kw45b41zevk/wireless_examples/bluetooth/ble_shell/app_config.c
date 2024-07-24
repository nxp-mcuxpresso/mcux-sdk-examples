/*! *********************************************************************************
* \addtogroup App Config
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2019, 2022-2024 NXP
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
#include "ble_shell.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define smpEdiv                 0x1F99
#define mcEncryptionKeySize_c   16

#define mDefaultTxPower         gBleAdvTxPowerNoPreference_c

#ifndef SHELL_EXT_ADV_DATA_MAX_AD_STRUCTURES
#define SHELL_EXT_ADV_DATA_MAX_AD_STRUCTURES    5
#endif
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/* Scanning and Advertising Data */
static gapAdStructure_t    mAdStructures[SHELL_EXT_ADV_DATA_MAX_AD_STRUCTURES];
static gapAdStructure_t    mExtAdStructures[SHELL_EXT_ADV_DATA_MAX_AD_STRUCTURES];
static gapAdStructure_t    mScanAdStructures[SHELL_EXT_ADV_DATA_MAX_AD_STRUCTURES];

gapAdvertisingData_t gAppAdvertisingData =
{
    0,
    mAdStructures
};

gapAdvertisingData_t gAppExtAdvertisingData =
{
    0,
    mExtAdStructures
};

gapAdvertisingData_t gAppPeriodicAdvData =
{
    0,
    mExtAdStructures,
};

gapScanResponseData_t gAppScanRspData =
{
    0,
    NULL
};

gapScanResponseData_t gAppExtScanRspData =
{
    0,
    mScanAdStructures
};

#if defined(BLE_SHELL_DBAF_SUPPORT) && (BLE_SHELL_DBAF_SUPPORT)
/* Decision Based Advertising Data */
uint8_t gaDecisionKey[gcDecisionDataKeySize_c];
uint8_t gaPrand[gcDecisionDataPrandSize_c];
uint8_t gaDecisionData[gcDecisionDataMaxSize_c];
gapAdvertisingDecisionData_t gAppExtAdvDecisionData =
{
    .pKey = gaDecisionKey,
    .pPrand = gaPrand,
    .pDecisionData = gaDecisionData,
    .dataLength = (uint8_t)sizeof(gaDecisionData),
    .resolvableTagPresent = FALSE
};

/* Decision Instructions Data */
gapDecisionInstructionsData_t gaDecisionInstructions[gMaxNumDecisionInstructions_c];
uint8_t gNumDecisionInstructions = 0U;
#endif /* BLE_SHELL_DBAF_SUPPORT */

gapScanningParameters_t gAppScanParams =
{
    /* type */              gScanTypePassive_c, \
    /* interval */          gGapScanIntervalDefault_d, \
    /* window */            gGapScanWindowDefault_d, \
    /* ownAddressType */    gBleAddrTypePublic_c, \
    /* filterPolicy */      (uint8_t)gScanAll_c, \
    /* scanning PHY */      (uint8_t)gLePhy1MFlag_c\
};

gapAdvertisingParameters_t gAdvParams =
{
    /* minInterval */         1600 /* 1 s */, \
    /* maxInterval */         1600 /* 1 s */, \
    /* advertisingType */     gAdvConnectableUndirected_c, \
    /* addressType */         gBleAddrTypePublic_c, \
    /* directedAddressType */ gBleAddrTypePublic_c, \
    /* directedAddress */     {0, 0, 0, 0, 0, 0}, \
    /* channelMap */          (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */        gProcessAll_c \
};

/*Default Extended Advertising Parameters */
gapExtAdvertisingParameters_t gExtAdvParams =
{
    /* SID */                       1, \
    /* handle */                    1, \
    /* minInterval */               1600 /* 1 s */, \
    /* maxInterval */               3200 /* 2 s */, \
    /* ownAddrType */               gBleAddrTypePublic_c,\
    /* ownAddress */                {0, 0, 0, 0, 0, 0}, \
    /* peerAddrType */              gBleAddrTypePublic_c,\
    /* peerAddress */               {0, 0, 0, 0, 0, 0}, \
    /* channelMap */                (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */              gProcessAll_c, \
    /* extAdvProperties */          (bleAdvRequestProperties_t)gAdvIncludeTxPower_c, \
    /* TxPower */                   mDefaultTxPower, \
    /* primaryPHY */                (gapLePhyMode_t)gLePhy1M_c, \
    /* secondaryPHY */              (gapLePhyMode_t)gLePhy1M_c, \
    /* secondaryAdvMaxSkip */       0, \
    /* enableScanReqNotification*/  FALSE \

};

/*Default Periodic Advertising Parameters */
gapPeriodicAdvParameters_t gPeriodicAdvParams =
{
    /* handle */                    1, \
    /* addTxPowerInAdv*/            TRUE, \
    /* minInterval */               1600 /* 1 s */, \
    /* maxInterval */               3200 /* 2 s */, \

};

/* Default Connection Request Parameters */
gapConnectionRequestParameters_t gConnReqParams =
{
    .scanInterval = gGapScanIntervalDefault_d,
    .scanWindow = gGapScanWindowDefault_d,
    .filterPolicy = (uint8_t)gUseDeviceAddress_c,
    .ownAddressType = gBleAddrTypePublic_c,
    .connIntervalMin = gGapDefaultMaxConnectionInterval_d,
    .connIntervalMax = gGapDefaultMaxConnectionInterval_d,
    .connLatency = gGapConnLatencyMin_d,
    .supervisionTimeout = gGapConnSuperTimeoutMax_d,
    .connEventLengthMin = gGapConnEventLengthMin_d,
    .connEventLengthMax = gGapConnEventLengthMax_d,
    .initiatingPHYs = (uint8_t)gLePhy1MFlag_c,
};

gapPairingParameters_t gPairingParameters = {
    .withBonding = TRUE,
    .securityModeAndLevel = gSecurityMode_1_Level_3_c,
    .maxEncryptionKeySize = mcEncryptionKeySize_c,
    .localIoCapabilities = gIoKeyboardDisplay_c,
    .oobAvailable = FALSE,
    .centralKeys = (gapSmpKeyFlags_t)gLtk_c,
    .peripheralKeys = (gapSmpKeyFlags_t)gLtk_c,
    .leSecureConnectionSupported = TRUE,
    .useKeypressNotifications = FALSE,
};


/* LTK */
static uint8_t smpLtk[mcEncryptionKeySize_c] =
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
};
