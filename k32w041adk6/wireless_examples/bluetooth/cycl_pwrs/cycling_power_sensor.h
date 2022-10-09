/*! *********************************************************************************
 * \defgroup Cycling Power Sensor
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* This file is the interface file for the Cycling Power Sensor application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef CYCLING_POWER_SENSOR_H
#define CYCLING_POWER_SENSOR_H

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/
/* Profile Parameters */

#define gFastConnMinAdvInterval_c       32U /* 20 ms */
#define gFastConnMaxAdvInterval_c       48U /* 30 ms */

#define gReducedPowerMinAdvInterval_c   1600U /* 1 s */
#define gReducedPowerMaxAdvInterval_c   4000U /* 2.5 s */

#define gFastConnAdvTime_c              30U  /* s */
#define gReducedPowerAdvTime_c          300U /* s */

#if gAppUseBonding_d
#define gFastConnWhiteListAdvTime_c     10U /* s */
#else
#define gFastConnWhiteListAdvTime_c     0U
#endif

/* Fast Connection Parameters used for Power Vector Notifications */
#define gFastConnMinInterval_c          32U /* 40 ms */
#define gFastConnMaxInterval_c          32U /* 40 ms */
#define gFastConnSlaveLatency_c         0U
#define gFastConnTimeoutMultiplier_c    0x03E8U

#define gMeasurementReportInterval      500U /* 500 ms*/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
extern gapAdvertisingData_t         gAppAdvertisingData;
extern gapScanResponseData_t        gAppScanRspData;
extern gapConnectionParameters_t    gAppFastConnParams;
extern gapAdvertisingParameters_t   gAdvParams;
extern gapSmpKeys_t                 gSmpKeys;
extern gapPairingParameters_t       gPairingParameters;

extern gapDeviceSecurityRequirements_t deviceSecurityRequirements;
/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void BleApp_Init(void);
void BleApp_Start(void);
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent);

#ifdef __cplusplus
}
#endif 


#endif /* CYCLING_POWER_SENSOR_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
