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

#include "fsci_ble_l2cap.h"

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
* Private functions prototypes
*************************************************************************************
************************************************************************************/

static void fsciBleL2capLeCbDataCallback(deviceId_t deviceId, uint16_t lePsm, uint8_t *pPacket, uint16_t packetLength);
static void fsciBleL2capLeCbControlCallback(l2capControlMessageType_t messageType, void *pMessage);

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* L2CAP Credit Based data callback initialized with FSCI empty static function */
static l2caLeCbDataCallback_t l2capLeCbDataCallback = fsciBleL2capLeCbDataCallback;

/* L2CAP Credit Based control callback initialized with FSCI empty static function */
static l2caLeCbControlCallback_t l2capLeCbControlCallback = fsciBleL2capLeCbControlCallback;

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

void fsciBleSetL2capLeCbDataCallback(l2caLeCbDataCallback_t dataCallback)
{
    /* Set callback to an application desired function, only
    if not NULL. Otherwise set it to the FSCI empty static function */
    l2capLeCbDataCallback = (NULL != dataCallback) ? dataCallback : fsciBleL2capLeCbDataCallback;
}

void fsciBleSetL2capLeCbControlCallback(l2caLeCbControlCallback_t controlCallback)
{
    /* Set callback to an application desired function, only
    if not NULL. Otherwise set it to the FSCI empty static function */
    l2capLeCbControlCallback = (NULL != controlCallback) ? controlCallback : fsciBleL2capLeCbControlCallback;
}

void fsciBleL2capHandler(void *pData, void *param, uint32_t fsciBleInterfaceId)
{
    clientPacket_t *pClientPacket = (clientPacket_t *)pData;
    uint8_t *pBuffer              = &pClientPacket->structured.payload[0];

    /* Select the L2CAP function to be called (using the FSCI opcode) */
    switch (pClientPacket->structured.header.opCode)
    {
        case gBleL2capStatusOpCode_c:
        {
            bleResult_t status;

            fsciBleGetEnumValueFromBuffer(status, pBuffer, bleResult_t);
        }
        break;

        case gBleL2capEvtLePsmConnectRequestOpCode_c:
        case gBleL2capEvtLePsmConnectionCompleteOpCode_c:
        case gBleL2capEvtLePsmDisconnectNotificationOpCode_c:
        case gBleL2capEvtNoPeerCreditsOpCode_c:
        case gBleL2capEvtLocalCreditsNotificationOpCode_c:
        {
            l2capControlMessageType_t messageType = gL2ca_LePsmConnectRequest_c;
            uint16_t messageSize                  = 0;
            bool_t bMessageIncluded;

            switch (pClientPacket->structured.header.opCode)
            {
                case gBleL2capEvtLePsmConnectRequestOpCode_c:
                {
                    messageType = gL2ca_LePsmConnectRequest_c;
                    messageSize = sizeof(l2caLeCbConnectionRequest_t);
                }
                break;

                case gBleL2capEvtLePsmConnectionCompleteOpCode_c:
                {
                    messageType = gL2ca_LePsmConnectionComplete_c;
                    messageSize = sizeof(l2caLeCbConnectionComplete_t);
                }
                break;

                case gBleL2capEvtLePsmDisconnectNotificationOpCode_c:
                {
                    messageType = gL2ca_LePsmDisconnectNotification_c;
                    messageSize = sizeof(l2caLeCbDisconnection_t);
                }
                break;

                case gBleL2capEvtNoPeerCreditsOpCode_c:
                {
                    messageType = gL2ca_NoPeerCredits_c;
                    messageSize = sizeof(l2caLeCbNoPeerCredits_t);
                }
                break;

                case gBleL2capEvtLocalCreditsNotificationOpCode_c:
                {
                    messageType = gL2ca_LocalCreditsNotification_c;
                    messageSize = sizeof(l2caLeCbLocalCreditsNotification_t);
                }
                break;

                default:
                    break;
            }

            fsciBleGetBoolValueFromBuffer(bMessageIncluded, pBuffer);

            if (TRUE == bMessageIncluded)
            {
                void *pMessage = MEM_BufferAlloc(messageSize);

                if (NULL != pMessage)
                {
                    switch (pClientPacket->structured.header.opCode)
                    {
                        case gBleL2capEvtLePsmConnectRequestOpCode_c:
                        {
                            fsciBleL2capGetLeCbConnectionRequestFromBuffer((l2caLeCbConnectionRequest_t *)pMessage,
                                                                           &pBuffer);
                        }
                        break;

                        case gBleL2capEvtLePsmConnectionCompleteOpCode_c:
                        {
                            fsciBleL2capGetLeCbConnectionCompleteFromBuffer((l2caLeCbConnectionComplete_t *)pMessage,
                                                                            &pBuffer);
                        }
                        break;

                        case gBleL2capEvtLePsmDisconnectNotificationOpCode_c:
                        {
                            fsciBleL2capGetLeCbDisconnectionFromBuffer((l2caLeCbDisconnection_t *)pMessage, &pBuffer);
                        }
                        break;

                        case gBleL2capEvtNoPeerCreditsOpCode_c:
                        {
                            fsciBleL2capGetLeCbNoPeerCreditsFromBuffer((l2caLeCbNoPeerCredits_t *)pMessage, &pBuffer);
                        }
                        break;

                        case gBleL2capEvtLocalCreditsNotificationOpCode_c:
                        {
                            fsciBleL2capGetLeCbLocalCreditsNotificationFromBuffer(
                                (l2caLeCbLocalCreditsNotification_t *)pMessage, &pBuffer);
                        }
                        break;

                        default:
                            break;
                    }

                    l2capLeCbControlCallback(messageType, pMessage);

                    MEM_BufferFree(pMessage);
                }
                else
                {
                    /* No buffer available - the L2CAP Credit Based callback can not be
                    called */
                    FSCI_Error(gFsciOutOfMessages_c, fsciBleInterfaceId);
                }
            }
            else
            {
                l2capLeCbControlCallback(messageType, NULL);
            }
        }
        break;

        case gBleL2capEvtLeCbDataOpCode_c:
        {
            deviceId_t deviceId;
            uint16_t srcCid;
            uint8_t *pPacket;
            uint16_t packetLength;

            fsciBleGetDeviceIdFromBuffer(&deviceId, &pBuffer);
            fsciBleGetUint16ValueFromBuffer(srcCid, pBuffer);
            fsciBleGetUint16ValueFromBuffer(packetLength, pBuffer);

            if (0 < packetLength)
            {
                pPacket = MEM_BufferAlloc(packetLength);

                if (NULL != pPacket)
                {
                    fsciBleGetArrayFromBuffer(pPacket, pBuffer, packetLength);

                    l2capLeCbDataCallback(deviceId, srcCid, pPacket, packetLength);

                    MEM_BufferFree(pPacket);
                }
                else
                {
                    /* No buffer available - the L2CAP Credit Based callback can not be
                    called */
                    FSCI_Error(gFsciOutOfMessages_c, fsciBleInterfaceId);
                }
            }
            else
            {
                l2capLeCbDataCallback(deviceId, srcCid, NULL, packetLength);
            }
        }
        break;

        default:
        {
            /* Unknown FSCI opcode */
            FSCI_Error(gFsciUnknownOpcode_c, fsciBleInterfaceId);
        }
        break;
    }

    MEM_BufferFree(pData);
}

void fsciBleL2capConnectLePsmCmdMonitor(uint16_t lePsm, deviceId_t deviceId, uint16_t credits)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleL2capAllocFsciPacket(gBleL2capCmdConnectLePsmOpCode_c,
                                    sizeof(uint16_t) + fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint16Value(lePsm, pBuffer);
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(credits, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleL2capSendLeCreditCmdMonitor(deviceId_t deviceId, uint16_t channelId, uint16_t credits)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleL2capAllocFsciPacket(gBleL2capCmdSendLeCreditOpCode_c,
                                    fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint16_t) + sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(channelId, pBuffer);
    fsciBleGetBufferFromUint16Value(credits, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleL2capRegisterLePsmCmdMonitor(uint16_t lePsm, uint16_t lePsmMtu)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleL2capAllocFsciPacket(gBleL2capCmdRegisterLePsmOpCode_c, sizeof(uint16_t) + sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint16Value(lePsm, pBuffer);
    fsciBleGetBufferFromUint16Value(lePsmMtu, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleL2capDeregisterLePsmCmdMonitor(uint16_t lePsm)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    l2capLeCbDataCallback    = fsciBleL2capLeCbDataCallback;
    l2capLeCbControlCallback = fsciBleL2capLeCbControlCallback;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleL2capAllocFsciPacket(gBleL2capCmdDeregisterLePsmOpCode_c, sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint16Value(lePsm, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleL2capDisconnectLeCbChannelCmdMonitor(deviceId_t deviceId, uint16_t channelId)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleL2capAllocFsciPacket(gBleL2capCmdDisconnectLeCbChannelOpCode_c,
                                                sizeof(uint16_t) + fsciBleGetDeviceIdBufferSize(&deviceId));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(channelId, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleL2capCancelConnectionCmdMonitor(uint16_t lePsm,
                                            deviceId_t deviceId,
                                            l2caLeCbConnectionRequestResult_t refuseReason)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleL2capAllocFsciPacket(
        gBleL2capCmdCancelConnectionOpCode_c,
        sizeof(uint16_t) + fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(l2caLeCbConnectionRequestResult_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint16Value(lePsm, pBuffer);
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromEnumValue(refuseReason, pBuffer, l2caLeCbConnectionRequestResult_t);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleL2capSendLeCbDataCmdMonitor(deviceId_t deviceId,
                                        uint16_t channelId,
                                        uint8_t *pPacket,
                                        uint16_t packetLength)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleL2capAllocFsciPacket(
        gBleL2capCmdSendLeCbDataOpCode_c,
        sizeof(uint16_t) + fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint16_t) + packetLength);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(channelId, pBuffer);
    fsciBleGetBufferFromUint16Value(packetLength, pBuffer);
    fsciBleGetBufferFromArray(pPacket, pBuffer, packetLength);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

static void fsciBleL2capLeCbDataCallback(deviceId_t deviceId, uint16_t lePsm, uint8_t *pPacket, uint16_t packetLength)
{
}

static void fsciBleL2capLeCbControlCallback(l2capControlMessageType_t messageType, void *pMessage)
{
}

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
