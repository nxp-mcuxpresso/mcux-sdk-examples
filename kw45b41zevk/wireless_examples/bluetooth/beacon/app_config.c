/*! *********************************************************************************
* \addtogroup App Config
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2021, 2023 NXP
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
#include "beacon.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

#if defined(gIBeaconAdvData_c) && (gIBeaconAdvData_c)
#define mAdvAppleId             0x4C, 0x00
#define mAdvAppleSubType        0x02
#define mAdvAppleSubTypeLength  0x15
#define mAdvIBeaconMajor        0x00, 0x01
#define mAdvIBeaconMinor        0x00, 0x02
#define mIBeaconUuid            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#else
#define mAdvCompanyId           0x25, 0x00
#define mBeaconId               0xBC
/* This value will be set to a random value on BLE App init */
#define mUuid                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif /* gIBeaconAdvData_c */

#if defined(gBeaconAE_c) && (gBeaconAE_c)
#define mDefaultTxPower               gBleAdvTxPowerNoPreference_c
#ifndef mBeaconExtHandleId_c
#define mBeaconExtHandleId_c          1
#endif
#define mLePhy1M_c                    1
#define mLePhy2M_c                    2
#define mLePhyCoded_c                 3

#endif /*gBeaconAE_c */


/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
/* Advertising Parameters */
gapAdvertisingParameters_t gAppAdvParams = {
    /* minInterval */         800 /* 500 ms */, \
    /* maxInterval */         1600 /* 1 s */, \
    /* advertisingType */     gAdvNonConnectable_c, \
    /* addressType */         gBleAddrTypePublic_c, \
    /* directedAddressType */ gBleAddrTypePublic_c, \
    /* directedAddress */     {0, 0, 0, 0, 0, 0}, \
    /* channelMap */          (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c), \
    /* filterPolicy */        gProcessAll_c \
};

/* Advertising Data */
static uint8_t adData0[1] = { (uint8_t)gLeGeneralDiscoverableMode_c | (uint8_t)gBrEdrNotSupported_c };
#if defined(gIBeaconAdvData_c) && (gIBeaconAdvData_c)
static uint8_t adData1[25] = {
                       /* Apple Company Identifier */
                       mAdvAppleId,
                       /* Apple Sub Type */
                       mAdvAppleSubType,
                       /* Apple Sub Type Length */
                       mAdvAppleSubTypeLength,
                       /* iBeacon UUID */
                       mIBeaconUuid,
                       /* iBeacon Major */
                       mAdvIBeaconMajor,
                       /* iBeacon Major */
                       mAdvIBeaconMinor,
                       /* RSSI at 1m */
                       0x1E};
#else
static uint8_t adData1[26] = {
                       /* Company Identifier*/
                       mAdvCompanyId,
                       /* Beacon Identifier */
                       mBeaconId,
                       /* UUID */
                       mUuid,
                       /* A */
                       0x00, 0x00,
                       /* B */
                       0x00, 0x00,
                       /* C */
                       0x00, 0x00,
                       /* RSSI at 1m */
                       0x1E};
#endif /* gIBeaconAdvData_c */

static gapAdStructure_t advScanStruct[] = {
  {
    .length = NumberOfElements(adData0) + 1,
    .adType = gAdFlags_c,
    .aData = (void *)adData0
  },
  {
    .length = NumberOfElements(adData1) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (void *)adData1
  }
};

gapAdvertisingData_t gAppAdvertisingData =
{
    NumberOfElements(advScanStruct),
    (void *)advScanStruct
};

#if defined(gBeaconAE_c) && (gBeaconAE_c)
#define extAdData2  "\n\rTrain schedule from Paris, Gare du Nord \n\r "
#define extAdData3  "Departures 10:00 - 10:59: \n\r London :05, Barcelona :10, Madrid :15,  Rome :20, Milan :25, Berlin :30, Munich :35, Frankfurt :40, Bucharest :45, Vienna :50, Budapest :55\n\r"
#define extAdData4  "Arrivals 10:00 - 10:59: \n\r London :05, Barcelona :10, Madrid :15,  Rome :20, Milan :25, Berlin :30, Munich :35, Frankfurt :40, Bucharest :45, Vienna :50, Budapest :55"

static gapAdStructure_t extAdvScanStruct[] = {
  {
    .length = NumberOfElements(adData0) + 1,
    .adType = gAdFlags_c,
    .aData = (uint8_t*)adData0
  },
  {
    .length = SizeOfString(extAdData2) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdData2
  },
  {
    .length = SizeOfString(extAdData3) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdData3
  },
/* Adding the following data structures will trigger chained advertising indications */
#if gBeaconLargeExtAdvData_c
  {
    .length = SizeOfString(extAdData4) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdData4
  },
  {
    .length = SizeOfString(extAdData3) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdData3
  },
  {
    .length = SizeOfString(extAdData4) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdData4
  },
  {
    .length = SizeOfString(extAdData3) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdData3
  },
  {
    .length = SizeOfString(extAdData4) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdData4
  },
  {
    .length = SizeOfString(extAdData3) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdData3
  },
  {
    .length = SizeOfString(extAdData4) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdData4
  },
  {
    .length = SizeOfString(extAdData3) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdData3
  },
  {
    .length = SizeOfString(extAdData2) + 1,
    .adType = gAdManufacturerSpecificData_c,
    .aData = (uint8_t*)extAdData2
  }
#endif
};

gapAdvertisingData_t gAppExtAdvertisingData =
{
    NumberOfElements(extAdvScanStruct),
    (void *)extAdvScanStruct
};

static gapAdStructure_t extAdvScanStructNoData[] = {
  {
    .length = NumberOfElements(adData0) + 1,
    .adType = gAdFlags_c,
    .aData = (uint8_t*)adData0
  }
};

gapAdvertisingData_t gAppExtAdvertisingNoData =
{
    NumberOfElements(extAdvScanStructNoData),
    (void *)extAdvScanStructNoData
};

/*Default Extended Advertising Parameters */
gapExtAdvertisingParameters_t gExtAdvParams =
{
    /* SID */                       mBeaconExtHandleId_c, \
    /* handle */                    mBeaconExtHandleId_c, \
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
    /* primaryPHY */                (gapLePhyMode_t)mLePhyCoded_c, \
    /* secondaryPHY */              (gapLePhyMode_t)mLePhy2M_c, \
    /* secondaryAdvMaxSkip */       0, \
    /* enableScanReqNotification*/  FALSE \

};

/*Default Periodic Advertising Parameters */
gapPeriodicAdvParameters_t gPeriodicAdvParams =
{
    /* handle */                    mBeaconExtHandleId_c, \
    /* addTxPowerInAdv*/            TRUE, \
    /* minInterval */               1600 /* 1 s */, \
    /* maxInterval */               3200 /* 2 s */, \

};
#endif /* gBeaconAE_c */
