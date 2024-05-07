/*! *********************************************************************************
 * \defgroup HID Device
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2014 Freescale Semiconductor, Inc.
* Copyright 2016-2019, 2021-2023 NXP
*
*
* \file
*
* This file is the interface file for the HID Device application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef HID_DEVICE_H
#define HID_DEVICE_H

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/

/* Profile Parameters */

#define gFastConnMinAdvInterval_c               32      /* 20 ms */
#define gFastConnMaxAdvInterval_c               48      /* 30 ms */

#define gReducedPowerMinAdvInterval_c           1600    /* 1 s */
#define gReducedPowerMaxAdvInterval_c           4000    /* 2.5 s */

#define gFastConnAdvTime_c                      30      /* s */
#define gReducedPowerAdvTime_c                  300     /* s */

#if gAppUseBonding_d
#define gFastConnFilterAcceptListAdvTime_c      10      /* s */
#else
#define gFastConnFilterAcceptListAdvTime_c      0
#endif

#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
#define gSwiftPairFastConnMinAdvInterval_c      48      /* 30 ms */
#define gSwiftPairFastConnMaxAdvInterval_c      48      /* 30 ms */

#define gSwiftPairReducedPowerMinAdvInterval_c  160    /* 100 ms */
#endif /* gSwiftPairMode_d */

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
extern gapAdvertisingData_t             gAppAdvertisingData;
extern gapScanResponseData_t            gAppScanRspData;
extern gapAdvertisingParameters_t       gAdvParams;

#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
extern gapAdvertisingData_t             gAppSwiftPairAdvertisingData;
#endif /* gSwiftPairMode_d */

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void BleApp_Start(void);

#ifdef __cplusplus
}
#endif


#endif /* HID_DEVICE_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
