/*! *********************************************************************************
 * \defgroup BLE OTAP Client L2CAP CoC
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2023 NXP
*
*
* \file
*
* This file is the interface file for the BLE OTAP L2CAP Coc Client application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef OTAP_CLIENT_L2CAP_CREDIT_H
#define OTAP_CLIENT_L2CAP_CREDIT_H

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

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
extern gapAdvertisingParameters_t gAdvParams;
extern gapAdvertisingData_t gAppAdvertisingData;
extern gapScanResponseData_t gAppScanRspData;

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void);

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */

#ifdef __cplusplus
}
#endif


#endif /* OTAP_CLIENT_L2CAP_CREDIT_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
