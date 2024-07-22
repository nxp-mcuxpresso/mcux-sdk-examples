/*! *********************************************************************************
* Copyright 2024 NXP
*
*
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef APP_ADVERTISER_H
#define APP_ADVERTISER_H

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
typedef struct appAdvertisingParams_tag
{
    gapAdvertisingParameters_t *pGapAdvParams;        /*!< Pointer to the GAP advertising parameters */
    const gapAdvertisingData_t *pGapAdvData;          /*!< Pointer to the GAP advertising data  */
    const gapScanResponseData_t *pScanResponseData;   /*!< Pointer to the scan response data */
} appAdvertisingParams_t;

typedef struct appExtAdvertisingParams_tag
{
    gapExtAdvertisingParameters_t *pGapExtAdvParams;  /*!< Pointer to the GAP extended advertising parameters */
    gapAdvertisingData_t *pGapAdvData;                /*!< Pointer to the GAP advertising data  */
    gapScanResponseData_t *pScanResponseData;         /*!< Pointer to the scan response data */
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
    gapAdvertisingDecisionData_t* pGapDecisionData;
#endif /* gBLE60_DecisionBasedAdvertisingFilteringSupport_d */
    uint8_t                     handle;
    uint16_t                    duration;
    uint8_t                     maxExtAdvEvents;
} appExtAdvertisingParams_t;

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
extern gapAdvertisingCallback_t pfAdvCallback;
extern appAdvertiserHandler_t pfAdvertiserHandler;

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*\fn           bleResult_t BluetoothLEHost_StartAdvertising(
*                  appAdvertisingParams_t   *pAdvParams,
*                  gapAdvertisingCallback_t pfAdvertisingCallback,
*                  gapConnectionCallback_t  pfConnectionCallback
*              )
*\brief        Set advertising data, set advertising parameters and start advertising.
*
*\param  [in]  pAdvParams               Pointer to the structure containing the
*                                       advertising parameters.
*\param  [in]  pfAdvertisingCallback    Callback used by the application to receive
*                                       advertising events. Can be NULL.
*\param  [in]  pfConnectionCallback     Callback used by the application to receive
*                                       connection events. Can be NULL.
*
*\return       bleResult_t              Result of the operation.
********************************************************************************** */
bleResult_t BluetoothLEHost_StartAdvertising
(
    appAdvertisingParams_t   *pAdvParams,
    gapAdvertisingCallback_t pfAdvertisingCallback,
    gapConnectionCallback_t  pfConnectionCallback
);

/*! *********************************************************************************
*\fn           bleResult_t BluetoothLEHost_StartExtAdvertising(
*                   appExtAdvertisingParams_t *pExtAdvParams,
*                   gapAdvertisingCallback_t  pfAdvertisingCallback,
*                   gapConnectionCallback_t   pfConnectionCallback
*               )
*\brief        Set advertising data, set advertising parameters and start extended
*              advertising.
*
*\param  [in]  pAdvParams               Pointer to the structure containing the
*                                       advertising.
*\param  [in]  pfAdvertisingCallback    Callback used by the application to receive
*                                       advertising events. Can be NULL.
*\param  [in]  pfConnectionCallback     Callback used by the application to receive
*                                       connection events. Can be NULL.
*
*\return       bleResult_t              Result of the operation.
********************************************************************************** */
bleResult_t BluetoothLEHost_StartExtAdvertising
(
    appExtAdvertisingParams_t *pExtAdvParams,
    gapAdvertisingCallback_t  pfAdvertisingCallback,
    gapConnectionCallback_t   pfConnectionCallback
);

#endif /* APP_ADVERTISER_H */
