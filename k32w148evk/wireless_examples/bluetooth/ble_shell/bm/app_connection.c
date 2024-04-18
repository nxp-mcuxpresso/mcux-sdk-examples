/*! *********************************************************************************
* Copyright 2021 - 2024 NXP
*
* \file
*
* This is a source file for the connection common application code.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "app_conn.h"
#include "fsl_component_messaging.h"

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
*\fn           bleResult_t BluetoothLEHost_Connect(
*                  gapConnectionRequestParameters_t*   pParameters,
*                  gapConnectionCallback_t             connCallback
*              )
*\brief        Start connection using the connection parameters specified.
*
*\param  [in]  pParameters     Pointer to the connection parameters.
*\param  [in]  connCallback    Callback used to receive connection events.
*
*\return       bleResult_t     Result of the operation.
*
*\remarks      This function should be used by the application if the callback
*              should be executed in the context of the Application Task.
********************************************************************************** */
bleResult_t BluetoothLEHost_Connect
(
    gapConnectionRequestParameters_t*   pParameters,
    gapConnectionCallback_t             connCallback
)
{
    pfConnCallback = connCallback;
    return Gap_Connect(pParameters, App_ConnectionCallback);
}

/*! *********************************************************************************
*\fn           void App_ConnectionCallback(
*                  deviceId_t peerDeviceId,
*                  gapConnectionEvent_t* pConnectionEvent
*              )
*\brief        Sends the GAP Connection Event triggered by the Host Stack to the
*              application.
*
*\param  [in]  peerDeviceId        The id of the peer device.
*\param  [in]  pConnectionEvent    Pointer to the connection event.
*
*\retval       void.
********************************************************************************** */
void App_ConnectionCallback
(
    deviceId_t            peerDeviceId,
    gapConnectionEvent_t* pConnectionEvent
)
{
    appMsgFromHost_t *pMsgIn = NULL;

    uint32_t msgLen = GetRelAddr(appMsgFromHost_t,msgData) + sizeof(connectionMsg_t);

    if(pConnectionEvent->eventType == gConnEvtKeysReceived_c)
    {
        gapSmpKeys_t    *pKeys = pConnectionEvent->eventData.keysReceivedEvent.pKeys;

        /* add room for pMsgIn->msgType */
        msgLen = sizeof(uint32_t);
        /* add room for pMsgIn->msgData.connMsg.deviceId */
        msgLen += sizeof(uint32_t);
        /* add room for pMsgIn->msgData.connMsg.connEvent.eventType */
        msgLen += sizeof(uint32_t);
        /*
         * add room for pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent
         * pKeys pointer
         */
        msgLen += sizeof(void*);
        /*
         * add room for pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent
         * pKeys data
         */
        msgLen += sizeof(gapSmpKeys_t);
        if (pKeys->aLtk != NULL)
        {
            /* add room for LTK and Rand data */
            msgLen += (uint32_t)pKeys->cLtkSize + (uint32_t)pKeys->cRandSize;
        }
        /* add room for IRK data */
        msgLen += (pKeys->aIrk != NULL) ?
                  (gcSmpIrkSize_c + gcBleDeviceAddressSize_c) : 0U;
        /*  add room for CSRK data */
        msgLen += (pKeys->aCsrk != NULL) ? gcSmpCsrkSize_c : 0U;
        /* add room for device address data */
        msgLen += (pKeys->aAddress != NULL) ? (gcBleDeviceAddressSize_c) : 0U;
    }
    else if (pConnectionEvent->eventType == gConnEvtIqReportReceived_c)
    {
        msgLen += 2U * (uint32_t)pConnectionEvent->eventData.connIqReport.sampleCount;
    }
    else
    {
        /* MISRA compliance */
    }

    pMsgIn = MSG_Alloc(msgLen);

    if (pMsgIn == NULL)
    {
        return;
    }

    pMsgIn->msgType = (uint32_t)gAppGapConnectionMsg_c;
    pMsgIn->msgData.connMsg.deviceId = peerDeviceId;

    if(pConnectionEvent->eventType == gConnEvtKeysReceived_c)
    {
        union
        {
            uint8_t      *pu8;
            gapSmpKeys_t *pObject;
        } temp = {0}; /* MISRA rule 11.3 */

        gapSmpKeys_t    *pKeys = pConnectionEvent->eventData.keysReceivedEvent.pKeys;
        uint8_t         *pCursor =
            (uint8_t*)&pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys;

        pMsgIn->msgData.connMsg.connEvent.eventType = gConnEvtKeysReceived_c;
        pCursor += sizeof(void*); /* skip pKeys pointer */

        temp.pu8 = pCursor;
        pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys =
                   temp.pObject;

        /* Copy SMP Keys structure */
        FLib_MemCpy(pCursor,
                    pConnectionEvent->eventData.keysReceivedEvent.pKeys,
                    sizeof(gapSmpKeys_t));
        pCursor += sizeof(gapSmpKeys_t);

        if (pKeys->aLtk != NULL)
        {
            /* Copy LTK */
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->cLtkSize =
                                pKeys->cLtkSize;
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->aLtk =
                                pCursor;
            FLib_MemCpy(pCursor, pKeys->aLtk, pKeys->cLtkSize);
            pCursor += pKeys->cLtkSize;

            /* Copy RAND */
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->cRandSize =
                                pKeys->cRandSize;
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->aRand =
                                pCursor;
            FLib_MemCpy(pCursor, pKeys->aRand, pKeys->cRandSize);
            pCursor += pKeys->cRandSize;
        }

        if (pKeys->aIrk != NULL)
        {
            /* Copy IRK */
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->aIrk =
                                pCursor;
            FLib_MemCpy(pCursor, pKeys->aIrk, gcSmpIrkSize_c);
            pCursor += gcSmpIrkSize_c;

            /* Copy Address*/
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->addressType =
                                pKeys->addressType;
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->aAddress =
                                pCursor;
            FLib_MemCpy(pCursor, pKeys->aAddress, gcBleDeviceAddressSize_c);
            pCursor += gcBleDeviceAddressSize_c;
        }

        if (pKeys->aCsrk != NULL)
        {
            /* Copy CSRK */
            pMsgIn->msgData.connMsg.connEvent.eventData.keysReceivedEvent.pKeys->aCsrk =
                                pCursor;
            FLib_MemCpy(pCursor, pKeys->aCsrk, gcSmpCsrkSize_c);
        }
    }
    else if (pConnectionEvent->eventType == gConnEvtIqReportReceived_c)
    {
        FLib_MemCpy(&pMsgIn->msgData.connMsg.connEvent.eventData.connIqReport,
                    &pConnectionEvent->eventData.connIqReport,
                    sizeof(pConnectionEvent->eventData.connIqReport));
        pMsgIn->msgData.connMsg.connEvent.eventData.connIqReport.aI_samples = (int8_t*)&pMsgIn->msgData + sizeof(connectionMsg_t);
        FLib_MemCpy(pMsgIn->msgData.connMsg.connEvent.eventData.connIqReport.aI_samples,
                    pConnectionEvent->eventData.connIqReport.aI_samples,
                    pConnectionEvent->eventData.connIqReport.sampleCount);
        pMsgIn->msgData.connMsg.connEvent.eventData.connIqReport.aQ_samples = (int8_t*)&pMsgIn->msgData + sizeof(connectionMsg_t) + pConnectionEvent->eventData.connIqReport.sampleCount;
        FLib_MemCpy(pMsgIn->msgData.connMsg.connEvent.eventData.connIqReport.aQ_samples,
                    pConnectionEvent->eventData.connIqReport.aQ_samples,
                    pConnectionEvent->eventData.connIqReport.sampleCount);
    }
    else
    {
        FLib_MemCpy(&pMsgIn->msgData.connMsg.connEvent,
                    pConnectionEvent,
                    sizeof(gapConnectionEvent_t));
    }

    /* Put message in the Host Stack to App queue */
    (void)MSG_QueueAddTail(&mHostAppInputQueue, pMsgIn);

    /* Signal application */
    (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
