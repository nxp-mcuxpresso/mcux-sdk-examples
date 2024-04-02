/*! *********************************************************************************
 * \defgroup Beacon
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2014 Freescale Semiconductor, Inc.
* Copyright 2016-2021, 2023 NXP
*
*
* \file
*
* This file is the interface file for the Beacon application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef BEACON_H
#define BEACON_H

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/

#ifndef gIBeaconAdvData_c
#define gIBeaconAdvData_c 0
#endif

#if defined(gIBeaconAdvData_c) && (gIBeaconAdvData_c)
#define gAdvUuidOffset_c        4U
#else
#define gAdvUuidOffset_c        3U
#endif /* gIBeaconAdvData_c */

#ifndef gBeaconAE_c
#define gBeaconAE_c 0
#endif

#define mBeaconExtHandleId_c          1
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */

extern gapAdvertisingParameters_t gAppAdvParams;
extern gapAdvertisingData_t gAppAdvertisingData;
#if gBeaconAE_c
extern gapExtAdvertisingParameters_t gExtAdvParams;
extern gapAdvertisingData_t          gAppExtAdvertisingData;
extern gapAdvertisingData_t          gAppExtAdvertisingNoData;
extern gapPeriodicAdvParameters_t    gPeriodicAdvParams;
#endif

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void BleApp_Start(void);
extern void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent);

#ifdef __cplusplus
}
#endif


#endif /* BEACON_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
