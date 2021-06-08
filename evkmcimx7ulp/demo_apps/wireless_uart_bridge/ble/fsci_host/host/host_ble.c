/*! *********************************************************************************
 * \addtogroup HOST_BBOX_UTILITY
 * @{
 ********************************************************************************** */
/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "fsci_ble.h"
#include "ble_general.h"

#if gFsciIncluded_c
#include "FsciInterface.h"
#include "FsciCommunication.h"
#endif

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* Flag which indicates if function has out parameters */
volatile bool_t bFunctionHasOutParams = FALSE;

#if gFsciIncluded_c && gFsciHostSupport_c
extern clientPacket_t *pFsciHostSyncRsp;

#if gFsciHostSyncUseEvent_c
extern osaEventId_t gFsciHostSyncRspEvent;
#endif
#endif

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

bleResult_t Ble_Initialize(gapGenericCallback_t gapGenericCallback)
{
    return Ble_HostInitialize(gapGenericCallback, NULL);
    ;
}

bleResult_t Ble_GetCmdStatus(void)
{
    bleResult_t result = gBleSuccess_c;
#if gFsciHostSyncUseEvent_c
    osaEventFlags_t flags;
#endif

    /* Wait for result from the serial interface */
    while (!pFsciHostSyncRsp)
    {
#if gFsciHostSyncUseEvent_c
        if (OSA_EventWait(gFsciHostSyncRspEvent, gFSCIHost_RspReady_c, FALSE, 1000, &flags) == osaStatus_Timeout)
        {
            /* Peer cannot respond */
            return gBleOverflow_c;
        }
#else
        FSCI_receivePacket((void *)fsciBleInterfaceId);
#endif
        if (NULL == pFsciHostSyncRsp)
        {
            continue;
        }

        result = (bleResult_t)pFsciHostSyncRsp->structured.payload[0];
    }

    /* Check status and wait for outParameters */
    if (gBleSuccess_c == result)
    {
        while (bFunctionHasOutParams)
        {
#if gFsciHostSyncUseEvent_c
            OSA_EventWait(gFsciHostSyncRspEvent, gFSCIHost_RspParamReady_c, FALSE, osaWaitForever_c, &flags);
#else
            FSCI_receivePacket((void *)fsciBleInterfaceId);
#endif
        }
    }

    /* Free FSCI packet */
    MEM_BufferFree(pFsciHostSyncRsp);

    return result;
}

void Ble_OutParamsReady(void)
{
    /* Reset flag which indicates if function has out parameters */
    bFunctionHasOutParams = FALSE;
}

/*************************************************************************************
 *************************************************************************************
 * Private functions
 *************************************************************************************
 ************************************************************************************/

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
