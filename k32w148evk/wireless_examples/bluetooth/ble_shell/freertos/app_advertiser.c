/*! *********************************************************************************
* Copyright 2021, 2024 NXP
*
* \file
*
* This is a source file for the common application advertising code.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "app_conn.h"
#include "fsl_component_panic.h"
#include "fwk_messaging.h"

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
static void App_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void App_AdvertiserHandler(gapGenericEventType_t  eventType);

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appAdvertisingParams_t *mpAdvParams = NULL;
static appExtAdvertisingParams_t *mpExtAdvParams = NULL;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
*\fn          bleResult_t BluetoothLEHost_StartAdvertising(
*                 appAdvertisingParams_t   *pAdvParams,
                  gapAdvertisingCallback_t pfAdvertisingCallback,
                  gapConnectionCallback_t  pfConnectionCallback
              )
*\brief       Set advertising data, set advertising parameters and start advertising.
*
*\param  [in] pAdvParams             Pointer to the structure containing the
*                                    advertising parameters.
*\param  [in] pfAdvertisingCallback  Callback used by the application to receive
*                                    advertising events.
*                                    Can be NULL.
*\param  [in] pfConnectionCallback   Callback used by the application to receive
*                                    connection events.
*                                    Can be NULL.
*
*\return      bleResult_t            Result of the operation.
********************************************************************************** */
bleResult_t BluetoothLEHost_StartAdvertising(
    appAdvertisingParams_t *pAdvParams,
    gapAdvertisingCallback_t pfAdvertisingCallback,
    gapConnectionCallback_t pfConnectionCallback
)
{
    pfAdvCallback = pfAdvertisingCallback;
    pfConnCallback = pfConnectionCallback;

    pfAdvertiserHandler = App_AdvertiserHandler;
    mpAdvParams = pAdvParams;
    return Gap_SetAdvertisingParameters(pAdvParams->pGapAdvParams);
}

/*! *********************************************************************************
*\fn          bleResult_t BluetoothLEHost_StartExtAdvertising(
*                 appExtAdvertisingParams_t *pExtAdvParams,
                  gapAdvertisingCallback_t  pfAdvertisingCallback,
                  gapConnectionCallback_t   pfConnectionCallback
              )
*\brief       Set advertising data, set advertising parameters and start extended
*             advertising.
*
*\param  [in] pAdvParams             Pointer to the structure containing the
*                                    advertising parameters.
*\param  [in] pfAdvertisingCallback  Callback used by the application to receive
*                                    advertising events.
*                                    Can be NULL.
*\param  [in] pfConnectionCallback   Callback used by the application to receive
*                                    connection events.
*                                    Can be NULL.
*
*\return      bleResult_t            Result of the operation.
********************************************************************************** */
bleResult_t BluetoothLEHost_StartExtAdvertising(
    appExtAdvertisingParams_t *pExtAdvParams,
    gapAdvertisingCallback_t pfAdvertisingCallback,
    gapConnectionCallback_t pfConnectionCallback
)
{
    pfAdvCallback = pfAdvertisingCallback;
    pfConnCallback = pfConnectionCallback;

    pfAdvertiserHandler = App_AdvertiserHandler;
    mpExtAdvParams = pExtAdvParams;
    return Gap_SetExtAdvertisingParameters(pExtAdvParams->pGapExtAdvParams);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
*\private
*\fn          void App_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
*\brief       Sends the GAP Advertising Event triggered by the Host Stack to the
*             application.
*
*\param  [in] pAdvertisingEvent      Pointer to the advertising event.
*
*\retval      void.
********************************************************************************** */
static void App_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
{
    appMsgFromHost_t *pMsgIn = NULL;

    pMsgIn = MSG_Alloc(GetRelAddr(appMsgFromHost_t,msgData) + sizeof(gapAdvertisingEvent_t));

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGapAdvertisementMsg_c;
    pMsgIn->msgData.advMsg.eventType = pAdvertisingEvent->eventType;
    pMsgIn->msgData.advMsg.eventData = pAdvertisingEvent->eventData;

    /* Put message in the Host Stack to App queue */
    (void)MSG_Queue(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
}

/*! *********************************************************************************
*\private
*\fn          void App_AdvertiserHandler(gapGenericEventType_t  eventType)
*\brief       This function handles the advertising events state machine.
*
*\param  [in] eventType              Received advertising event type.
*
*\retval      void.
********************************************************************************** */
static void App_AdvertiserHandler(gapGenericEventType_t  eventType)
{
    switch (eventType)
    {
        case gAdvertisingParametersSetupComplete_c:
        {
            (void)Gap_SetAdvertisingData(mpAdvParams->pGapAdvData,
                                         mpAdvParams->pScanResponseData);
        }
        break;

        case gAdvertisingDataSetupComplete_c:
        {
            (void)Gap_StartAdvertising(App_AdvertisingCallback,
                                       App_ConnectionCallback);
        }
        break;

        case gAdvertisingSetupFailed_c:
        {
            panic(0,0,0,0);
        }
        break;

        case gExtAdvertisingParametersSetupComplete_c:
        {
            (void)Gap_SetExtAdvertisingData(mpExtAdvParams->handle,
                                            mpExtAdvParams->pGapAdvData,
                                            mpExtAdvParams->pScanResponseData);
        }
        break;

        case gExtAdvertisingDataSetupComplete_c:
        {
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
            if((mpExtAdvParams->pGapDecisionData == NULL) || ((mpExtAdvParams->pGapExtAdvParams->extAdvProperties & (bleAdvRequestProperties_t)gAdvUseDecisionPDU_c) == (bleAdvRequestProperties_t)0x00U))
            {
#endif /* gBLE60_DecisionBasedAdvertisingFilteringSupport_d */
                (void)Gap_StartExtAdvertising(App_AdvertisingCallback,
                                              App_ConnectionCallback,
                                              mpExtAdvParams->handle,
                                              mpExtAdvParams->duration,
                                              mpExtAdvParams->maxExtAdvEvents);
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
            }
            else
            {
                (void)Gap_SetExtAdvertisingDecisionData(mpExtAdvParams->handle, mpExtAdvParams->pGapDecisionData);
            }
#endif /* gBLE60_DecisionBasedAdvertisingFilteringSupport_d */
        }
        break;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
        case gExtAdvertisingDecisionDataSetupComplete_c:
        {
            (void)Gap_StartExtAdvertising(App_AdvertisingCallback,
                                          App_ConnectionCallback,
                                          mpExtAdvParams->handle,
                                          mpExtAdvParams->duration,
                                          mpExtAdvParams->maxExtAdvEvents);
        }
        break;
#endif /* gBLE60_DecisionBasedAdvertisingFilteringSupport_d */
        case gExtAdvertisingSetRemoveComplete_c:
        {
            /* TBD */
        }
        break;

        default:
        {
            ; /* MISRA rule 16.4 */
        }
        break;
    }
}
