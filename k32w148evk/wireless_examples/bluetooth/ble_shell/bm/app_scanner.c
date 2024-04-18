/*! *********************************************************************************
* Copyright 2021-2024 NXP
*
* \file
*
* This is a source file for the common application scanning code.
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
STATIC void App_ScanningCallback(gapScanningEvent_t* pScanningEvent);

/************************************************************************************
*************************************************************************************
* Private memory declarations
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
*\fn           bleResult_t BluetoothLEHost_StartScanning(
*                  appScanningParams_t   *pAppScanParams,
*                  gapScanningCallback_t pfCallback
*\brief        Start the Bluetooth LE scanning using the parameters specified.
*
*\param  [in]  pScanningParameters    Pointer to the structure containing the
*                                     scanning parameters.
*\param  [in]  pfCallback             The scanning callback.

*\return       bleResult_t            Result of the operation.
********************************************************************************** */
bleResult_t BluetoothLEHost_StartScanning
(
    appScanningParams_t   *pAppScanParams,
    gapScanningCallback_t pfCallback
)
{
    bleResult_t result = gBleInvalidState_c;
    pfScanCallback = pfCallback;
    result = Gap_StartScanning(pAppScanParams->pHostScanParams,
            App_ScanningCallback,
            pAppScanParams->enableDuplicateFiltering,
            pAppScanParams->duration,
            pAppScanParams->period);
    return result;
}

/*! *********************************************************************************
*\fn           bool_t BluetoothLEHost_MatchDataInAdvElementList(
*                  gapAdStructure_t *pElement,
*                  void             *pData,
*                  uint8_t          iDataLen)
*\brief        Search if the contents from pData can be found in an advertising
*              element.
*
*\param  [in]  pElement               Pointer to the structure containing the ad
*                                     structure element.
*\param  [in]  pData                  Pointer to the data to be searched for.
*\param  [in]  iDataLen               The length of the data.

* \retval      TRUE                   Data was found in this element.
* \retval      FALSE                  Data was not found in this element.
********************************************************************************** */
bool_t BluetoothLEHost_MatchDataInAdvElementList
(
    gapAdStructure_t *pElement,
    void             *pData,
    uint8_t          iDataLen)
{
    uint8_t i;
    bool_t status = FALSE;
    
    if( pElement->length != 0U)
    {
        for (i = 0; i < (pElement->length - 1U); i += iDataLen)
        {
            if (FLib_MemCmp(pData, &pElement->aData[i], iDataLen))
            {
                status = TRUE;
                break;
            }
        }
    }
    return status;
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
*\private
*\fn           void App_ScanningCallback (gapScanningEvent_t* pScanningEvent)
*\brief        Sends the GAP Scanning Event triggered by the Host Stack to the
*              application.
*
*\param  [in]  pScanningEvent         Pointer to the scanning event.
*
*\retval       void.
********************************************************************************** */
STATIC void App_ScanningCallback
(
    gapScanningEvent_t* pScanningEvent
)
{
    appMsgFromHost_t *pMsgIn = NULL;
    uint32_t msgLen = GetRelAddr(appMsgFromHost_t,msgData) + sizeof(gapScanningEvent_t);

    if (pScanningEvent->eventType == gDeviceScanned_c)
    {
        msgLen += pScanningEvent->eventData.scannedDevice.dataLength;
    }
    else if (pScanningEvent->eventType == gExtDeviceScanned_c)
    {
        msgLen += pScanningEvent->eventData.extScannedDevice.dataLength;
    }
    else if (pScanningEvent->eventType == gPeriodicDeviceScanned_c)
    {
        msgLen += pScanningEvent->eventData.periodicScannedDevice.dataLength;
    }
    else if (pScanningEvent->eventType == gConnectionlessIqReportReceived_c)
    {
        msgLen += 2U * (uint32_t)pScanningEvent->eventData.iqReport.sampleCount;
    }
    else
    {
        /* msgLen does not modify for all other event types */
    }

    pMsgIn = MSG_Alloc(msgLen);

    if (pMsgIn != NULL)
    {
        pMsgIn->msgType = (uint32_t)gAppGapScanMsg_c;
        pMsgIn->msgData.scanMsg.eventType = pScanningEvent->eventType;
        
        if (pScanningEvent->eventType == gScanCommandFailed_c)
        {
            pMsgIn->msgData.scanMsg.eventData.failReason =
                pScanningEvent->eventData.failReason;
        }
        else if (pScanningEvent->eventType == gDeviceScanned_c)
        {
#if defined(gAppFilterPeerAdv_c) && (gAppFilterPeerAdv_c == 1)
            bleResult_t status = gBleSuccess_c;
            bool_t peerConnected = FALSE;
            
            status = Gap_CheckIfConnected(pScanningEvent->eventData.scannedDevice.addressType,
                                          pScanningEvent->eventData.scannedDevice.aAddress,
                                          pScanningEvent->eventData.scannedDevice.advertisingAddressResolved,
                                          &peerConnected);
            
            if ((status == gBleSuccess_c) && (peerConnected == FALSE))
            {
#endif /* defined(gAppFilterPeerAdv_c) && (gAppFilterPeerAdv_c == 1) */
                FLib_MemCpy(&pMsgIn->msgData.scanMsg.eventData.scannedDevice,
                            &pScanningEvent->eventData.scannedDevice,
                            sizeof(pScanningEvent->eventData.scannedDevice));
                
                /*
                * Copy data after the gapScanningEvent_t structure and update
                * the data pointer
                */
                pMsgIn->msgData.scanMsg.eventData.scannedDevice.data =
                    (uint8_t*)&pMsgIn->msgData +
                        sizeof(gapScanningEvent_t);
                FLib_MemCpy(pMsgIn->msgData.scanMsg.eventData.scannedDevice.data,
                            pScanningEvent->eventData.scannedDevice.data,
                            pScanningEvent->eventData.scannedDevice.dataLength);
#if defined(gAppFilterPeerAdv_c) && (gAppFilterPeerAdv_c == 1)
            }
            else
            {
                /* Do not send the event to the application */
                MSG_Free(pMsgIn);
                pMsgIn = NULL;
            }
#endif /* defined(gAppFilterPeerAdv_c) && (gAppFilterPeerAdv_c == 1) */
        }
        else if (pScanningEvent->eventType == gExtDeviceScanned_c)
        {
#if defined(gAppFilterPeerAdv_c) && (gAppFilterPeerAdv_c == 1)
            bleResult_t status = gBleSuccess_c;
            bool_t peerConnected = FALSE;
            
            status = Gap_CheckIfConnected(pScanningEvent->eventData.extScannedDevice.addressType,
                                          pScanningEvent->eventData.extScannedDevice.aAddress,
                                          pScanningEvent->eventData.extScannedDevice.advertisingAddressResolved,
                                          &peerConnected);
            
            if ((status == gBleSuccess_c) && (peerConnected == FALSE))
            {
#endif /* defined(gAppFilterPeerAdv_c) && (gAppFilterPeerAdv_c == 1) */
                FLib_MemCpy(&pMsgIn->msgData.scanMsg.eventData.extScannedDevice,
                            &pScanningEvent->eventData.extScannedDevice,
                            sizeof(pScanningEvent->eventData.extScannedDevice));
                
                /*
                * Copy data after the gapScanningEvent_t structure and update
                * the data pointer
                */
                pMsgIn->msgData.scanMsg.eventData.extScannedDevice.pData =
                    (uint8_t*)&pMsgIn->msgData +
                        sizeof(gapScanningEvent_t);
                FLib_MemCpy(pMsgIn->msgData.scanMsg.eventData.extScannedDevice.pData,
                            pScanningEvent->eventData.extScannedDevice.pData,
                            pScanningEvent->eventData.extScannedDevice.dataLength);
#if defined(gAppFilterPeerAdv_c) && (gAppFilterPeerAdv_c == 1)
            }
            else
            {
                /* Do not send the event to the application */
                MSG_Free(pMsgIn);
                pMsgIn = NULL;
            }
#endif /* defined(gAppFilterPeerAdv_c) && (gAppFilterPeerAdv_c == 1) */
        }
        else if (pScanningEvent->eventType == gPeriodicDeviceScanned_c)
        {
            FLib_MemCpy(&pMsgIn->msgData.scanMsg.eventData.periodicScannedDevice,
                        &pScanningEvent->eventData.periodicScannedDevice,
                        sizeof(pScanningEvent->eventData.periodicScannedDevice));
            
            pMsgIn->msgData.scanMsg.eventData.periodicScannedDevice.pData =
                (uint8_t*)&pMsgIn->msgData +
                    sizeof(gapScanningEvent_t);
            FLib_MemCpy(pMsgIn->msgData.scanMsg.eventData.periodicScannedDevice.pData,
                        pScanningEvent->eventData.periodicScannedDevice.pData,
                        pScanningEvent->eventData.periodicScannedDevice.dataLength);
        }
        else if (pScanningEvent->eventType == gConnectionlessIqReportReceived_c)
        {
            FLib_MemCpy(&pMsgIn->msgData.scanMsg.eventData.iqReport,
                        &pScanningEvent->eventData.iqReport,
                        sizeof(pScanningEvent->eventData.iqReport));
            pMsgIn->msgData.scanMsg.eventData.iqReport.aI_samples = (int8_t*)&pMsgIn->msgData + sizeof(gapScanningEvent_t);
            FLib_MemCpy(pMsgIn->msgData.scanMsg.eventData.iqReport.aI_samples,
                        pScanningEvent->eventData.iqReport.aI_samples,
                        pScanningEvent->eventData.iqReport.sampleCount);
            pMsgIn->msgData.scanMsg.eventData.iqReport.aQ_samples = (int8_t*)&pMsgIn->msgData + sizeof(gapScanningEvent_t) + pScanningEvent->eventData.iqReport.sampleCount;
            FLib_MemCpy(pMsgIn->msgData.scanMsg.eventData.iqReport.aQ_samples,
                        pScanningEvent->eventData.iqReport.aQ_samples,
                        pScanningEvent->eventData.iqReport.sampleCount);
        }
        else if (pScanningEvent->eventType == gPeriodicAdvSyncEstablished_c)
        {
            FLib_MemCpy(&pMsgIn->msgData.scanMsg.eventData.syncEstb,
                        &pScanningEvent->eventData.syncEstb,
                        sizeof(pScanningEvent->eventData.syncEstb));
        }
        else if (pScanningEvent->eventType == gPeriodicAdvSyncLost_c)
        {
            FLib_MemCpy(&pMsgIn->msgData.scanMsg.eventData.syncLost,
                        &pScanningEvent->eventData.syncLost,
                        sizeof(pScanningEvent->eventData.syncLost));
        }
        else
        {
            /* no action for all other event types */
        }
    }

    if (pMsgIn != NULL)
    {
        /* Put message in the Host Stack to App queue */
        (void)MSG_QueueAddTail(&mHostAppInputQueue, pMsgIn);
        
        /* Signal application */
        (void)OSA_EventSet(mAppEvent, gAppEvtMsgFromHostStack_c);
    }
}
