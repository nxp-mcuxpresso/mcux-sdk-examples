/*! *********************************************************************************
* \addtogroup App Config
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2017 NXP
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

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
/* Advertising Parameters */
const gapAdvertisingParameters_t gAppAdvParams = {
#if gAppAdvertisingInterval
    /* minInterval */         gAppAdvertisingInterval,
    /* maxInterval */         gAppAdvertisingInterval,
#else
    /* minInterval */         800 /* 500 ms */,
    /* maxInterval */         1600 /* 1 s */,
#endif
#if gAppConnectableAdv_d
    /* advertisingType */     gAdvConnectableUndirected_c,
#else
    /* advertisingType */     gAdvNonConnectable_c,
#endif
    /* addressType */         gBleAddrTypePublic_c,
    /* directedAddressType */ gBleAddrTypePublic_c,
    /* directedAddress */     {0, 0, 0, 0, 0, 0},
    /* channelMap */          (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c),
    /* filterPolicy */        gProcessAll_c
};

/* Advertising Data */
static const uint8_t adData0[1] = { (uint8_t)gLeGeneralDiscoverableMode_c | (uint8_t)gBrEdrNotSupported_c };
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

static const gapAdStructure_t advScanStruct[] = {
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
