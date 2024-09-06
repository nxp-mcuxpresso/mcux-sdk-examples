/*! *********************************************************************************
* \addtogroup App Config
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
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
#include "gatt_db_handles.h"
#include "ble_conn_manager.h"
#include "adv_ext_peripheral.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define smpEdiv         0x1F99
#define mcEncryptionKeySize_c  16
#define mDefaultTxPower               gBleAdvTxPowerNoPreference_c
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
#define mcDbafEncryptionKeySize_c 16
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef enum
{
    mLegacyAdvSetId_c = 1,
    mExtAdvScannableSetId_c,
    mExtAdvConnectableSetId_c,
    mExtAdvNonConnNonScannSetId_c
}setId_t;
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/* Default Advertising Parameters. Values can be changed at runtime
    to align with profile requirements */
gapAdvertisingParameters_t gAdvParams = {
    /* minInterval */         0x12C0,
    /* maxInterval */         0x1900,
    /* advertisingType */     gAdvConnectableUndirected_c,
    /* addressType */         gBleAddrTypePublic_c,
    /* directedAddressType */ gBleAddrTypePublic_c,
    /* directedAddress */     {0, 0, 0, 0, 0, 0},
    /* channelMap */          (gapAdvertisingChannelMapFlags_t) (gGapAdvertisingChannelMapDefault_c),
    /* filterPolicy */        gProcessAll_c
};

/* Legacy Advertising Parameters */
gapExtAdvertisingParameters_t gExtAdvParamsLegacy =
{
    /* SID */                       (uint8_t)mLegacyAdvSetId_c, \
    /* handle */                    0xff, \
    /* minInterval */               800 /* 500 ms */, \
    /* maxInterval */               1600 /* 1 s */, \
    /* ownAddrType */               gBleAddrTypePublic_c,\
    /* ownAddress */                {0, 0, 0, 0, 0, 0}, \
    /* peerAddrType */              gBleAddrTypePublic_c,\
    /* peerAddress */               {0, 0, 0, 0, 0, 0}, \
    /* channelMap */                (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */              gProcessAll_c, \
    /* extAdvProperties */          (bleAdvRequestProperties_t)(gAdvReqLegacy_c | gAdvReqConnectable_c | gAdvReqScannable_c), \
    /* TxPower */                   mDefaultTxPower, \
    /* primaryPHY */                (gapLePhyMode_t)gLePhy1M_c, \
    /* secondaryPHY */              (gapLePhyMode_t)gLePhy1M_c, \
    /* secondaryAdvMaxSkip */       0, \
    /* enableScanReqNotification*/  FALSE \
};
/* Scanning and Advertising Data */
static uint8_t adData0[1] =  { (uint8_t)gLeGeneralDiscoverableMode_c | (uint8_t)gBrEdrNotSupported_c };
static gapAdStructure_t advScanStruct[3] = {
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
    .aData = (uint8_t*)"EA*PRPH"
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


/* Extended Scannable Advertising Parameters */
gapExtAdvertisingParameters_t gExtAdvParamsScannable =
{
    /* SID */                       (uint8_t)mExtAdvScannableSetId_c, \
    /* handle */                    0xff, \
    /* minInterval */               1600 /* 1 s */, \
    /* maxInterval */               3200 /* 2 s */, \
    /* ownAddrType */               gBleAddrTypePublic_c,\
    /* ownAddress */                {0, 0, 0, 0, 0, 0}, \
    /* peerAddrType */              gBleAddrTypePublic_c,\
    /* peerAddress */               {0, 0, 0, 0, 0, 0}, \
    /* channelMap */                (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */              gProcessAll_c, \
    /* extAdvProperties */          (bleAdvRequestProperties_t)(gAdvReqScannable_c | gAdvIncludeTxPower_c), \
    /* TxPower */                   mDefaultTxPower, \
    /* primaryPHY  */               (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryPHY */              (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryAdvMaxSkip */       0, \
    /* enableScanReqNotification*/  TRUE \
};
#define extScannableData0 "\
\n\rEA Scan Response Data 01 EA Scan Response Data 02 EA Scan Response Data 03\
\n\rEA Scan Response Data 04 EA Scan Response Data 05 EA Scan Response Data 06\
\n\rEA Scan Response Data 07 EA Scan Response Data 08 EA Scan Response Data 09"    


#define extScannableData1 "\
\n\rEA Scan Response Data 11 EA Scan Response Data 12 EA Scan Response Data 13\
\n\rEA Scan Response Data 14 EA Scan Response Data 15 EA Scan Response Data 16\
\n\rEA Scan Response Data 17 EA Scan Response Data 18 EA Scan Response Data 19"
                                             
#define extScannableData2 "\
\n\rEA Scan Response Data 21 EA Scan Response Data 22 EA Scan Response Data 23\
\n\rEA Scan Response Data 24 EA Scan Response Data 25 EA Scan Response Data 26\
\n\rEA Scan Response Data 27 EA Scan Response Data 28 EA Scan Response Data 29"
                                             
#define extScannableData3 "\
\n\rEA Scan Response Data 31 EA Scan Response Data 32 EA Scan Response Data 33\
\n\rEA Scan Response Data 34 EA Scan Response Data 35 EA Scan Response Data 36\
\n\rEA Scan Response Data 37 EA Scan Response Data 38 EA Scan Response Data 39"
                                             
static gapAdStructure_t extAdvScannableData[] = {
  {
    .length = NumberOfElements(adData0) + 1,
    .adType = gAdFlags_c,
    .aData = (uint8_t*)adData0
  },
  {
    .adType = gAdShortenedLocalName_c,
    .length = 9,
    .aData = (uint8_t*)"EA*PRPH"
  },
  {
    .length = (uint8_t)NumberOfElements(extScannableData0) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extScannableData0
  },
  {
    .length = (uint8_t)NumberOfElements(extScannableData1) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extScannableData1
  },
  {
    .length = (uint8_t)NumberOfElements(extScannableData2) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extScannableData2
  },  
  {
    .length = (uint8_t)NumberOfElements(extScannableData3) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extScannableData3
  }
};  
gapAdvertisingData_t gAppExtAdvDataScannable =
{
    NumberOfElements(extAdvScannableData),
    (void *)extAdvScannableData
};

/* Extended Connectable Advertising Parameters */
gapExtAdvertisingParameters_t gExtAdvParamsConnectable =
{
    /* SID */                       (uint8_t)mExtAdvConnectableSetId_c, \
    /* handle */                    0xff, \
    /* minInterval */               1600 /* 1 s */, \
    /* maxInterval */               3200 /* 2 s */, \
    /* ownAddrType */               gBleAddrTypePublic_c,\
    /* ownAddress */                {0, 0, 0, 0, 0, 0}, \
    /* peerAddrType */              gBleAddrTypePublic_c,\
    /* peerAddress */               {0, 0, 0, 0, 0, 0}, \
    /* channelMap */                (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */              gProcessAll_c, \
    /* extAdvProperties */          (bleAdvRequestProperties_t)(gAdvReqConnectable_c | gAdvIncludeTxPower_c), \
    /* TxPower */                   mDefaultTxPower, \
    /* primaryPHY */                (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryPHY */              (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryAdvMaxSkip */       0, \
    /* enableScanReqNotification*/  FALSE \
};
#define extConnectableData0 "\
\n\rEA Connectable Data 01 EA Connectable Data 02 EA Connectable Data 03\
\n\rEA Connectable Data 04 EA Connectable Data 05 EA Connectable Data 06\
\n\rEA Connectable Data 07 EA Connectable Data 08 EA Connectable Data 09"    


static gapAdStructure_t extAdvConnectableData[] = {
  {
    .length = NumberOfElements(adData0) + 1,
    .adType = gAdFlags_c,
    .aData = (uint8_t*)adData0
  },
  {
    .length = NumberOfElements(uuid_service_temperature) + 1,
    .adType = gAdComplete128bitServiceList_c,
    .aData = (uint8_t *)uuid_service_temperature
  },
  {
    .adType = gAdShortenedLocalName_c,
    .length = 9,
    .aData = (uint8_t*)"EA*PRPH"
  },
  {
    .length = (uint8_t)NumberOfElements(extConnectableData0) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extConnectableData0
  }
};  
gapAdvertisingData_t gAppExtAdvDataConnectable =
{
    NumberOfElements(extAdvConnectableData),
    (void *)extAdvConnectableData
};
/* Extended Non Connectable Non Scannable Advertising Parameters */
gapExtAdvertisingParameters_t gExtAdvParamsNonConnNonScann =
{
    /* SID */                       (uint8_t)mExtAdvNonConnNonScannSetId_c, \
    /* handle */                    0xff, \
    /* minInterval */               1600 /* 41.875 ms */, \
    /* maxInterval */               3200 /* 42.5 ms */, \
    /* ownAddrType */               gBleAddrTypePublic_c,\
    /* ownAddress */                {0, 0, 0, 0, 0, 0}, \
    /* peerAddrType */              gBleAddrTypePublic_c,\
    /* peerAddress */               {0, 0, 0, 0, 0, 0}, \
    /* channelMap */                (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */              gProcessAll_c, \
    /* extAdvProperties */          (bleAdvRequestProperties_t)(gAdvIncludeTxPower_c), \
    /* TxPower */                   mDefaultTxPower, \
    /* primaryPHY  */               (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryPHY */              (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryAdvMaxSkip */       0, \
    /* enableScanReqNotification*/  FALSE \
};

#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
/* DBAF Scannable Advertising Parameters */
gapExtAdvertisingParameters_t gDbafParamsScannable =
{
    /* SID */                       (uint8_t)mExtAdvScannableSetId_c, \
    /* handle */                    0xff, \
    /* minInterval */               1600 /* 1 s */, \
    /* maxInterval */               3200 /* 2 s */, \
    /* ownAddrType */               gBleAddrTypePublic_c,\
    /* ownAddress */                {0, 0, 0, 0, 0, 0}, \
    /* peerAddrType */              gBleAddrTypePublic_c,\
    /* peerAddress */               {0, 0, 0, 0, 0, 0}, \
    /* channelMap */                (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */              gProcessAll_c, \
    /* extAdvProperties */          (bleAdvRequestProperties_t)(gAdvReqScannable_c | gAdvUseDecisionPDU_c | gAdvIncludeTxPower_c), \
    /* TxPower */                   mDefaultTxPower, \
    /* primaryPHY  */               (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryPHY */              (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryAdvMaxSkip */       0, \
    /* enableScanReqNotification*/  TRUE \
};

/* DBAF Connectable Advertising Parameters */
gapExtAdvertisingParameters_t gDbafParamsConnectable =
{
    /* SID */                       (uint8_t)mExtAdvConnectableSetId_c, \
    /* handle */                    0xff, \
    /* minInterval */               1600 /* 1 s */, \
    /* maxInterval */               3200 /* 2 s */, \
    /* ownAddrType */               gBleAddrTypePublic_c,\
    /* ownAddress */                {0, 0, 0, 0, 0, 0}, \
    /* peerAddrType */              gBleAddrTypePublic_c,\
    /* peerAddress */               {0, 0, 0, 0, 0, 0}, \
    /* channelMap */                (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */              gProcessAll_c, \
    /* extAdvProperties */          (bleAdvRequestProperties_t)(gAdvReqConnectable_c | gAdvUseDecisionPDU_c | gAdvIncludeTxPower_c), \
    /* TxPower */                   mDefaultTxPower, \
    /* primaryPHY */                (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryPHY */              (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryAdvMaxSkip */       0, \
    /* enableScanReqNotification*/  FALSE \
};

/* DBAF Non Connectable Non Scannable Advertising Parameters */
gapExtAdvertisingParameters_t gDbafParamsNonConnNonScann =
{
    /* SID */                       (uint8_t)mExtAdvNonConnNonScannSetId_c, \
    /* handle */                    0xff, \
    /* minInterval */               1600 /* 41.875 ms */, \
    /* maxInterval */               3200 /* 42.5 ms */, \
    /* ownAddrType */               gBleAddrTypePublic_c,\
    /* ownAddress */                {0, 0, 0, 0, 0, 0}, \
    /* peerAddrType */              gBleAddrTypePublic_c,\
    /* peerAddress */               {0, 0, 0, 0, 0, 0}, \
    /* channelMap */                (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */              gProcessAll_c, \
    /* extAdvProperties */          (bleAdvRequestProperties_t)(gAdvUseDecisionPDU_c | gAdvIncludeTxPower_c), \
    /* TxPower */                   mDefaultTxPower, \
    /* primaryPHY  */               (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryPHY */              (gapLePhyMode_t)gLePhyCoded_c,\
    /* secondaryAdvMaxSkip */       0, \
    /* enableScanReqNotification*/  FALSE \
};
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */

#define extAdvNonConnNonScanDataId1_0 "\
\n\rEA Non Connectable Non Scanable DataId1 01 EA Non Connectable Non Scanable DataId1 02\
\n\rEA Non Connectable Non Scanable DataId1 03 EA Non Connectable Non Scanable DataId1 04"
                               
#define extAdvNonConnNonScanDataId1_1 "\
\n\rEA Non Connectable Non Scanable DataId1 11 EA Non Connectable Non Scanable DataId1 12\
\n\rEA Non Connectable Non Scanable DataId1 13 EA Non Connectable Non Scanable DataId1 14"

#define extAdvNonConnNonScanDataId1_2 "\
\n\rEA Non Connectable Non Scanable DataId1 21 EA Non Connectable Non Scanable DataId1 22\
\n\rEA Non Connectable Non Scanable DataId1 23 EA Non Connectable Non Scanable DataId1 24"
                                           
#define extAdvNonConnNonScanDataId1_3 "\
\n\rEA Non Connectable Non Scanable DataId1 31 EA Non Connectable Non Scanable DataId1 32\
\n\rEA Non Connectable Non Scanable DataId1 33 EA Non Connectable Non Scanable DataId1 34"

#define extAdvNonConnNonScanDataId1_4 "\
\n\rEA Non Connectable Non Scanable DataId1 41 EA Non Connectable Non Scanable DataId1 42\
\n\rEA Non Connectable Non Scanable DataId1 43 EA Non Connectable Non Scanable DataId1 44"
                                             
static gapAdStructure_t extAdvNonConnNonScanDataId1[] = {
  {
    .length = NumberOfElements(adData0) + 1,
    .adType = gAdFlags_c,
    .aData = (uint8_t*)adData0
  },
  {
    .adType = gAdShortenedLocalName_c,
    .length = 9,
    .aData = (uint8_t*)"EA*PRPH"
  },
  {
    .length = NumberOfElements(extAdvNonConnNonScanDataId1_0) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvNonConnNonScanDataId1_0
  },
  {
    .length = NumberOfElements(extAdvNonConnNonScanDataId1_1) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvNonConnNonScanDataId1_1
  },
  {
    .length = NumberOfElements(extAdvNonConnNonScanDataId1_2) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvNonConnNonScanDataId1_2
  },  
  {
    .length = NumberOfElements(extAdvNonConnNonScanDataId1_3) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvNonConnNonScanDataId1_3
  },  
  {
    .length = NumberOfElements(extAdvNonConnNonScanDataId1_4) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvNonConnNonScanDataId1_4
  }
};  
gapAdvertisingData_t gAppExtAdvDataId1NonConnNonScan =
{
    NumberOfElements(extAdvNonConnNonScanDataId1),
    (void *)extAdvNonConnNonScanDataId1
};

#define extAdvNonConnNonScanDataId2_0 "\
\n\rEA Non Connectable Non Scanable DataId2 01 EA Non Connectable Non Scanable DataId2 02\
\n\rEA Non Connectable Non Scanable DataId2 03 EA Non Connectable Non Scanable DataId2 04"
                               
#define extAdvNonConnNonScanDataId2_1 "\
\n\rEA Non Connectable Non Scanable DataId2 11 EA Non Connectable Non Scanable DataId2 12\
\n\rEA Non Connectable Non Scanable DataId2 13 EA Non Connectable Non Scanable DataId2 14"

#define extAdvNonConnNonScanDataId2_2 "\
\n\rEA Non Connectable Non Scanable DataId2 21 EA Non Connectable Non Scanable DataId2 22\
\n\rEA Non Connectable Non Scanable DataId2 23 EA Non Connectable Non Scanable DataId2 24"
                                           
#define extAdvNonConnNonScanDataId2_3 "\
\n\rEA Non Connectable Non Scanable DataId2 31 EA Non Connectable Non Scanable DataId2 32\
\n\rEA Non Connectable Non Scanable DataId2 33 EA Non Connectable Non Scanable DataId2 34"

#define extAdvNonConnNonScanDataId2_4 "\
\n\rEA Non Connectable Non Scanable DataId2 41 EA Non Connectable Non Scanable DataId2 42\
\n\rEA Non Connectable Non Scanable DataId2 43 EA Non Connectable Non Scanable DataId2 44"                                           
                                             
static gapAdStructure_t extAdvNonConnNonScanDataId2[] = {
  {
    .length = NumberOfElements(adData0) + 1,
    .adType = gAdFlags_c,
    .aData = (uint8_t*)adData0
  },
  {
    .adType = gAdShortenedLocalName_c,
    .length = 9,
    .aData = (uint8_t*)"EA*PRPH"
  },
  {
    .length = NumberOfElements(extAdvNonConnNonScanDataId2_0) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvNonConnNonScanDataId2_0
  },
  {
    .length = NumberOfElements(extAdvNonConnNonScanDataId2_1) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvNonConnNonScanDataId2_1
  },
  {
    .length = NumberOfElements(extAdvNonConnNonScanDataId2_2) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvNonConnNonScanDataId2_2
  },  
  {
    .length = NumberOfElements(extAdvNonConnNonScanDataId2_3) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvNonConnNonScanDataId2_3
  },  
  {
    .length = NumberOfElements(extAdvNonConnNonScanDataId2_4) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvNonConnNonScanDataId2_4
  }
};  
gapAdvertisingData_t gAppExtAdvDataId2NonConnNonScan =
{
    NumberOfElements(extAdvNonConnNonScanDataId2),
    (void *)extAdvNonConnNonScanDataId2
};
/*Default Periodic Advertising Parameters */
gapPeriodicAdvParameters_t gPeriodicAdvParams =
{
    /* handle */                    0xff, \
    /* addTxPowerInAdv*/            TRUE, \
    /* minInterval */               1600 /* 1 s */, \
    /* maxInterval */               3200 /* 2 s */, \

};
#define extAdvPeriodicDataId1_0 "\
\n\rEA Periodic Data Id1 01 EA Periodic Data Id1 02 EA Periodic Data Id1 03 EA Periodic Data Id1 04\
\n\rEA Periodic Data Id1 05 EA Periodic Data Id1 06 EA Periodic Data Id1 07 EA Periodic Data Id1 08"
                               
#define extAdvPeriodicDataId1_1 "\
\n\rEA Periodic Data Id1 11 EA Periodic Data Id1 12 EA Periodic Data Id1 13 EA Periodic Data Id1 14\
\n\rEA Periodic Data Id1 15 EA Periodic Data Id1 16 EA Periodic Data Id1 17 EA Periodic Data Id1 18"

#define extAdvPeriodicDataId1_2 "\
\n\rEA Periodic Data Id1 21 EA Periodic Data Id1 22 EA Periodic Data Id1 23 EA Periodic Data Id1 24\
\n\rEA Periodic Data Id1 25 EA Periodic Data Id1 26 EA Periodic Data Id1 27 EA Periodic Data Id1 28"

#define extAdvPeriodicDataId1_3 "\
\n\rEA Periodic Data Id1 31 EA Periodic Data Id1 32 EA Periodic Data Id1 33 EA Periodic Data Id1 34\
\n\rEA Periodic Data Id1 35 EA Periodic Data Id1 36 EA Periodic Data Id1 37 EA Periodic Data Id1 38"

#define extAdvPeriodicDataId1_4 "\
\n\rEA Periodic Data Id1 41 EA Periodic Data Id1 42 EA Periodic Data Id1 43 EA Periodic Data Id1 44\
\n\rEA Periodic Data Id1 45 EA Periodic Data Id1 46 EA Periodic Data Id1 47 EA Periodic Data Id1 48"
                                           
#define extAdvPeriodicDataId1_5 "\
\n\rEA Periodic Data Id1 51 EA Periodic Data Id1 52 EA Periodic Data Id1 53 EA Periodic Data Id1 54\
\n\rEA Periodic Data Id1 55 EA Periodic Data Id1 56 EA Periodic Data Id1 57 EA Periodic Data Id1 58"
                                             
static gapAdStructure_t extAdvPeriodicDataId1[] = {
  {
    .length = NumberOfElements(extAdvPeriodicDataId1_0) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId1_0
  },
  {
    .length = NumberOfElements(extAdvPeriodicDataId1_1) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId1_1
  },
  {
    .length = NumberOfElements(extAdvPeriodicDataId1_2) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId1_2
  },
  {
    .length = NumberOfElements(extAdvPeriodicDataId1_3) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId1_3
  },  
  {
    .length = NumberOfElements(extAdvPeriodicDataId1_4) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId1_4
  },  
  {
    .length = NumberOfElements(extAdvPeriodicDataId1_5) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId1_5
  }
};  
gapAdvertisingData_t gAppExtAdvDataId1Periodic =
{
    NumberOfElements(extAdvPeriodicDataId1),
    (void *)extAdvPeriodicDataId1
};

#define extAdvPeriodicDataId2_0 "\
\n\rEA Periodic Data Id2 01 EA Periodic Data Id2 02 EA Periodic Data Id2 03 EA Periodic Data Id2 04\
\n\rEA Periodic Data Id2 05 EA Periodic Data Id2 06 EA Periodic Data Id2 07 EA Periodic Data Id1 08"
                               
#define extAdvPeriodicDataId2_1 "\
\n\rEA Periodic Data Id2 11 EA Periodic Data Id2 12 EA Periodic Data Id2 13 EA Periodic Data Id2 14\
\n\rEA Periodic Data Id2 15 EA Periodic Data Id2 16 EA Periodic Data Id2 17 EA Periodic Data Id2 18"

#define extAdvPeriodicDataId2_2 "\
\n\rEA Periodic Data Id2 21 EA Periodic Data Id2 22 EA Periodic Data Id2 23 EA Periodic Data Id2 24\
\n\rEA Periodic Data Id2 25 EA Periodic Data Id2 26 EA Periodic Data Id2 27 EA Periodic Data Id2 28"

#define extAdvPeriodicDataId2_3 "\
\n\rEA Periodic Data Id2 31 EA Periodic Data Id2 32 EA Periodic Data Id2 33 EA Periodic Data Id2 34\
\n\rEA Periodic Data Id2 35 EA Periodic Data Id2 36 EA Periodic Data Id2 37 EA Periodic Data Id2 38"

#define extAdvPeriodicDataId2_4 "\
\n\rEA Periodic Data Id2 41 EA Periodic Data Id2 42 EA Periodic Data Id2 43 EA Periodic Data Id2 44\
\n\rEA Periodic Data Id2 45 EA Periodic Data Id2 46 EA Periodic Data Id2 47 EA Periodic Data Id2 48"
                                           
#define extAdvPeriodicDataId2_5 "\
\n\rEA Periodic Data Id2 51 EA Periodic Data Id2 52 EA Periodic Data Id2 53 EA Periodic Data Id2 54\
\n\rEA Periodic Data Id2 55 EA Periodic Data Id2 56 EA Periodic Data Id2 57 EA Periodic Data Id2 58"
                                             
static gapAdStructure_t extAdvPeriodicDataId2[] = {
  {
    .length = NumberOfElements(extAdvPeriodicDataId2_0) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId2_0
  },
  {
    .length = NumberOfElements(extAdvPeriodicDataId2_1) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId2_1
  },
  {
    .length = NumberOfElements(extAdvPeriodicDataId2_2) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId2_2
  },
  {
    .length = NumberOfElements(extAdvPeriodicDataId2_3) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId2_3
  },  
  {
    .length = NumberOfElements(extAdvPeriodicDataId2_4) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId2_4
  },  
  {
    .length = NumberOfElements(extAdvPeriodicDataId2_5) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdvPeriodicDataId2_5
  }
};  
gapAdvertisingData_t gAppExtAdvDataId2Periodic =
{
    NumberOfElements(extAdvPeriodicDataId2),
    (void *)extAdvPeriodicDataId2
};

#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
/* DBAF resolvable tag key */
static uint8_t  dbafKey[mcDbafEncryptionKeySize_c] = 
    {0x44, 0x65, 0x63, 0x69, 0x73, 0x69, 0x6F, 0x6E,
    0x42, 0x61, 0x73, 0x65, 0x64, 0x41, 0x64, 0x76};

/* DBAF Decision Data Parameters */
gapAdvertisingDecisionData_t gAdvDecisionData =
{
    /* pKey */                      dbafKey, \
    /* pPrand */                    NULL, \
    /* pDecisionData */             NULL, \
    /* dataLength */                0, \
    /* resolvableTagPresent */      FALSE \
};
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
/* SMP Data */
#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U))
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
#endif /* gAppUsePairing_d */

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
#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U))
static gapSecurityRequirements_t        deviceSecurity = gGapDefaultSecurityRequirements_d;
static gapServiceSecurityRequirements_t serviceSecurity[3] = {
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
#endif /* gAppUsePairing_d */
