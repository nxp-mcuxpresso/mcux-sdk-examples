/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSCI_BLE_L2CAP_H
#define _FSCI_BLE_L2CAP_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "fsci_ble_l2cap_types.h"

/************************************************************************************
*************************************************************************************
* Public constants & macros
*************************************************************************************
************************************************************************************/

/*! FSCI operation group for GATT Database (application) */
#define gFsciBleL2capOpcodeGroup_c 0x41

/*! Macro used for monitoring commands */
#define FsciL2capCmdMonitor(function, ...) fsciBleL2cap##function##CmdMonitor(__VA_ARGS__)

/*! *********************************************************************************
 * \brief  Allocates a FSCI packet for L2CAP.
 *
 * \param[in]    opCode      FSCI L2CAP operation code.
 * \param[in]    dataSize    Size of the payload.
 *
 * \return The allocated FSCI packet
 *
 ********************************************************************************** */
#define fsciBleL2capAllocFsciPacket(opCode, dataSize) \
    fsciBleAllocFsciPacket(gFsciBleL2capOpcodeGroup_c, (opCode), (dataSize))

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/*! FSCI operation codes for L2CAP */
typedef enum
{
    gBleL2capCmdRegisterLePsmOpCode_c = 0x07,  /*! L2ca_RegisterLePsm command operation code */
    gBleL2capCmdDeregisterLePsmOpCode_c,       /*! L2ca_DeregisterLePsm command operation code */
    gBleL2capCmdConnectLePsmOpCode_c,          /*! L2ca_ConnectLePsm command operation code */
    gBleL2capCmdDisconnectLeCbChannelOpCode_c, /*! L2ca_DisconnectLeCbChannel command operation code */
    gBleL2capCmdCancelConnectionOpCode_c,      /*! L2ca_CancelConnection command operation code */
    gBleL2capCmdSendLeCbDataOpCode_c,          /*! L2ca_SendLeCbData command operation code */
    gBleL2capCmdSendLeCreditOpCode_c,          /*! L2ca_SendLeCredit command operation code */

    gBleL2capStatusOpCode_c = 0x80,            /*! L2CAP status operation code */

    /* Events */
    gBleL2capEvtLePsmConnectRequestOpCode_c =
        0x83, /*! l2caLeCbControlCallback event (messageType == gL2ca_LePsmConnectRequest_c) operation code */
    gBleL2capEvtLePsmConnectionCompleteOpCode_c,     /*! l2caLeCbControlCallback event (messageType ==
                                                        gL2ca_LePsmConnectionComplete_c) operation code */
    gBleL2capEvtLePsmDisconnectNotificationOpCode_c, /*! l2caLeCbControlCallback event (messageType ==
                                                        gL2ca_LePsmDisconnectNotification_c) operation code */
    gBleL2capEvtNoPeerCreditsOpCode_c, /*! l2caLeCbControlCallback event (messageType == gL2ca_NoPeerCredits_c)
                                          operation code */
    gBleL2capEvtLocalCreditsNotificationOpCode_c, /*! l2caLeCbControlCallback event (messageType ==
                                                     gL2ca_LocalCreditsNotification_c) operation code */
    gBleL2capEvtLeCbDataOpCode_c                  /*! l2caLeCbDataCallback event operation code */
} fsciBleL2capOpCode_t;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*! *********************************************************************************
* \brief  Calls the L@CAP function associated with the operation code received over UART.
*         The L2CAP function parameters are extracted from the received FSCI payload.
*
* \param[in]    pData               Packet (containing FSCI header and FSCI.
                                    payload) received over UART.
* \param[in]    param               Pointer given when this function is registered in
                                    FSCI.
* \param[in]    fsciInterfaceId     FSCI interface on which the packet was received.
*
********************************************************************************** */
void fsciBleL2capHandler(void *pData, void *param, uint32_t fsciInterface);

/*! *********************************************************************************
 * \brief  L2ca_ConnectLePsm commands monitoring function.
 *
 * \param[in]    lePsm               Bluetooth SIG or Vendor LE_PSM.
 * \param[in]    deviceId            The DeviceID for which the command is intended.
 * \param[in]    initialCredits      Initial credits.
 *
 ********************************************************************************** */
void fsciBleL2capConnectLePsmCmdMonitor(uint16_t lePsm, deviceId_t deviceId, uint16_t initialCredits);

/*! *********************************************************************************
 * \brief  L2ca_SendLeCredit commands monitoring function.
 *
 * \param[in]    deviceId            The DeviceID for which the command is intended.
 * \param[in]    channelId           The L2CAP Channel Id assigned on the initiator.
 * \param[in]    credits             Initial credits or number of credits to be given.
 *
 ********************************************************************************** */
void fsciBleL2capSendLeCreditCmdMonitor(deviceId_t deviceId, uint16_t channelId, uint16_t credits);

/*! *********************************************************************************
 * \brief  L2ca_RegisterLePsm command monitoring function.
 *
 * \param[in]    lePsm               Bluetooth SIG or Vendor LE_PSM.
 * \param[in]    lePsmMtu            MTU value.
 *
 ********************************************************************************** */
void fsciBleL2capRegisterLePsmCmdMonitor(uint16_t lePsm, uint16_t lePsmMtu);

/*! *********************************************************************************
 * \brief  L2ca_DeregisterLePsm command monitoring function.
 *
 * \param[in]    lePsm               Bluetooth SIG or Vendor LE_PSM.
 *
 ********************************************************************************** */
void fsciBleL2capDeregisterLePsmCmdMonitor(uint16_t lePsm);

/*! *********************************************************************************
 * \brief  L2ca_DisconnectLeCbChannel command monitoring function.
 *
 * \param[in]    deviceId            The DeviceID for which the command is intended.
 * \param[in]    channelId           The L2CAP Channel Id assigned on the initiator
 *
 ********************************************************************************** */
void fsciBleL2capDisconnectLeCbChannelCmdMonitor(deviceId_t deviceId, uint16_t channelId);

/*! *********************************************************************************
 * \brief  L2ca_CancelConnection command monitoring function.
 *
 * \param[in]    lePsm               Bluetooth SIG or Vendor LE_PSM
 * \param[in]    deviceId            The DeviceID for which the command is intended.
 * \param[in]    refuseReason        Connection refusal reason
 *
 ********************************************************************************** */
void fsciBleL2capCancelConnectionCmdMonitor(uint16_t lePsm,
                                            deviceId_t deviceId,
                                            l2caLeCbConnectionRequestResult_t refuseReason);

/*! *********************************************************************************
 * \brief  L2ca_SendLeCbData command monitoring function.
 *
 * \param[in]    deviceId            The DeviceID for which the command is intended.
 * \param[in]    channelId           The L2CAP Channel Id assigned on the initiator.
 * \param[in]    pPacket             Data buffer to be transmitted.
 * \param[in]    packetLength        Length of the data buffer.
 *
 ********************************************************************************** */
void fsciBleL2capSendLeCbDataCmdMonitor(deviceId_t deviceId,
                                        uint16_t channelId,
                                        uint8_t *pPacket,
                                        uint16_t packetLength);

/*! *********************************************************************************
 * \brief  fsciBleL2capLeCbDataEvtMonitor event monitoring macro.
 *
 * \param[in]    deviceId            The DeviceID from which data is received.
 * \param[in]    srcCid              Source Channel ID
 * \param[in]    pPacket             Received data buffer.
 * \param[in]    packetLength        Length of the data buffer.
 *
 ********************************************************************************** */
void fsciBleL2capLeCbDataEvtMonitor(deviceId_t deviceId, uint16_t srcCid, uint8_t *pPacket, uint16_t packetLength);

/*! *********************************************************************************
 * \brief  l2caLeCbControlCallback events monitoring function.
 *
 * \param[in]    messageType     Message type.
 * \param[in]    pMessage        Message data (can be NULL).
 *
 ********************************************************************************** */
void fsciBleL2capLeCbControlEvtMonitor(l2capControlMessageType_t messageType, void *pMessage);

/*! *********************************************************************************
 * \brief   This function sets the callback function for data plane messages.
 *
 * \param[in]    dataCallback        The function desired by the application to be
 *                                   used as data plane messages callback, or NULL.
 *                                   If NULL, the FSCI will use an empty function.
 *
 ********************************************************************************** */
void fsciBleSetL2capLeCbDataCallback(l2caLeCbDataCallback_t dataCallback);

/*! *********************************************************************************
 * \brief   This function sets the callback function for control plane messages.
 *
 * \param[in]    controlCallback     The function desired by the application to be
 *                                   used as control plane messages callback, or NULL.
 *                                   If NULL, the FSCI will use an empty function.
 *
 ********************************************************************************** */
void fsciBleSetL2capLeCbControlCallback(l2caLeCbControlCallback_t controlCallback);

#ifdef __cplusplus
}
#endif

#endif /* _FSCI_BLE_L2CAP_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
