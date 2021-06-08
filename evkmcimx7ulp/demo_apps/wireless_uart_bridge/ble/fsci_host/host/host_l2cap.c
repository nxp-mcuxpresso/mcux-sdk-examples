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

#include "fsci_ble_l2cap.h"
#include "host_ble.h"

#if gFsciIncluded_c
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

bleResult_t L2ca_RegisterLeCbCallbacks(l2caLeCbDataCallback_t pCallback, l2caLeCbControlCallback_t pCtrlCallback)
{
    fsciBleSetL2capLeCbDataCallback(pCallback);
    fsciBleSetL2capLeCbControlCallback(pCtrlCallback);

    return gBleSuccess_c;
}

bleResult_t L2ca_RegisterLePsm(uint16_t lePsm, uint16_t lePsmMtu)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleL2capOpcodeGroup_c, gBleL2capStatusOpCode_c, FALSE);
    FsciL2capCmdMonitor(RegisterLePsm, lePsm, lePsmMtu);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t L2ca_DeregisterLePsm(uint16_t lePsm)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleL2capOpcodeGroup_c, gBleL2capStatusOpCode_c, FALSE);
    FsciL2capCmdMonitor(DeregisterLePsm, lePsm);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t L2ca_ConnectLePsm(uint16_t lePsm, deviceId_t deviceId, uint16_t initialCredits)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleL2capOpcodeGroup_c, gBleL2capStatusOpCode_c, FALSE);
    FsciL2capCmdMonitor(ConnectLePsm, lePsm, deviceId, initialCredits);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t L2ca_DisconnectLeCbChannel(deviceId_t deviceId, uint16_t channelId)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleL2capOpcodeGroup_c, gBleL2capStatusOpCode_c, FALSE);
    FsciL2capCmdMonitor(DisconnectLeCbChannel, deviceId, channelId);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t L2ca_CancelConnection(uint16_t lePsm, deviceId_t deviceId, l2caLeCbConnectionRequestResult_t refuseReason)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleL2capOpcodeGroup_c, gBleL2capStatusOpCode_c, FALSE);
    FsciL2capCmdMonitor(CancelConnection, lePsm, deviceId, refuseReason);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t L2ca_SendLeCbData(deviceId_t deviceId, uint16_t channelId, uint8_t *pPacket, uint16_t packetLength)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleL2capOpcodeGroup_c, gBleL2capStatusOpCode_c, FALSE);
    FsciL2capCmdMonitor(SendLeCbData, deviceId, channelId, pPacket, packetLength);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t L2ca_SendLeCredit(deviceId_t deviceId, uint16_t channelId, uint16_t credits)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleL2capOpcodeGroup_c, gBleL2capStatusOpCode_c, FALSE);
    FsciL2capCmdMonitor(SendLeCredit, deviceId, channelId, credits);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

/*************************************************************************************
 *************************************************************************************
 * Private functions
 *************************************************************************************
 ************************************************************************************/

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
