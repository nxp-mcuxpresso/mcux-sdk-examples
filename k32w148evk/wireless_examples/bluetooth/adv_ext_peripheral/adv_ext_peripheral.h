/*! *********************************************************************************
 * \defgroup Extended Advertising Peripheral
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2020-2021, 2023-2024 NXP
*
*
* \file
*
* This file is the interface file for the extended advertising peripheral application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef ADV_EXT_PERIPHERAL_H
#define ADV_EXT_PERIPHERAL_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "fsl_component_button.h"

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/

/* Profile Parameters */

#define gReducedPowerMinAdvInterval_c   1600 /* 1 s */
#define gReducedPowerMaxAdvInterval_c   4000 /* 2.5 s */

#define gGoToSleepAfterDataTime_c       5 /* 5 s*/

#ifndef gAppDeepSleepMode_c
#define gAppDeepSleepMode_c 1
#endif

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
/* Legacy Advertising Parameters */
extern gapExtAdvertisingParameters_t gExtAdvParamsLegacy;
/* Extended Scannable Advertising Parameters */
extern gapExtAdvertisingParameters_t gExtAdvParamsScannable;
/* Extended Connectable Advertising Parameters */
extern gapExtAdvertisingParameters_t gExtAdvParamsConnectable;
/* Extended Non Connectable Non Scannable Advertising Parameters */
extern gapExtAdvertisingParameters_t gExtAdvParamsNonConnNonScann;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
/* DBAF Scannable Advertising Parameters */
extern gapExtAdvertisingParameters_t gDbafParamsScannable;
/* DBAF Connectable Advertising Parameters */
extern gapExtAdvertisingParameters_t gDbafParamsConnectable;
/* DBAF Non Connectable Non Scannable Advertising Parameters */
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
extern gapExtAdvertisingParameters_t gDbafParamsNonConnNonScann;
extern gapAdvertisingData_t          gAppExtAdvDataScannable;
extern gapAdvertisingData_t          gAppExtAdvDataConnectable;
extern gapAdvertisingData_t          gAppExtAdvDataId1NonConnNonScan;
extern gapAdvertisingData_t          gAppExtAdvDataId2NonConnNonScan;
extern gapAdvertisingData_t          gAppExtAdvDataId1Periodic;
extern gapAdvertisingData_t          gAppExtAdvDataId2Periodic;
/* Default advertising parameters */
extern gapAdvertisingParameters_t    gAdvParams;
/* Advertising data */
extern gapAdvertisingData_t          gAppAdvertisingData;
/* Scan response data */
extern gapScanResponseData_t         gAppScanRspData;
/* Periodic advertising parameters */
extern gapPeriodicAdvParameters_t    gPeriodicAdvParams;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
/* DBAF Decision Data Parameters */
extern gapAdvertisingDecisionData_t gAdvDecisionData;
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
extern button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam);
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
extern button_status_t BleApp_HandleKeys1(void *buttonHandle, button_callback_message_t *message,void *callbackParam);
#endif /*gAppButtonCnt_c > 1*/
#endif /*gAppButtonCnt_c > 0*/

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void BleApp_Start(uint8_t mode);

#ifdef __cplusplus
}
#endif


#endif /* ADV_EXT_PERIPHERAL_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
