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
#include "fsci_ble_l2cap.h"
#include "fsci_ble_gatt.h"
#include "fsci_ble_gatt_db_app.h"
#include "fsci_ble_gap.h"

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

uint32_t fsciBleInterfaceId = 0xFF; /* Indicates the FSCI interface that
                                    will be used for monitoring */

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

void fsciBleRegister(uint32_t fsciInterfaceId)
{
    /* Register L2CAP command handler */
    if (FSCI_RegisterOpGroup(gFsciBleL2capOpcodeGroup_c, gFsciMonitorMode_c, fsciBleL2capHandler, NULL,
                             fsciInterfaceId) != gFsciSuccess_c)
    {
        panic(0, (uint32_t)fsciBleRegister, 0, 0);
    }

    /* Register GATT command handler */
    if (FSCI_RegisterOpGroup(gFsciBleGattOpcodeGroup_c, gFsciMonitorMode_c, fsciBleGattHandler, NULL,
                             fsciInterfaceId) != gFsciSuccess_c)
    {
        panic(0, (uint32_t)fsciBleRegister, 0, 0);
    }

    /* Register GATT Database (application) command handler */
    if (FSCI_RegisterOpGroup(gFsciBleGattDbAppOpcodeGroup_c, gFsciMonitorMode_c, fsciBleGattDbAppHandler, NULL,
                             fsciInterfaceId) != gFsciSuccess_c)
    {
        panic(0, (uint32_t)fsciBleRegister, 0, 0);
    }

    /* Register GAP command handler */
    if (FSCI_RegisterOpGroup(gFsciBleGapOpcodeGroup_c, gFsciMonitorMode_c, fsciBleGapHandler, NULL, fsciInterfaceId) !=
        gFsciSuccess_c)
    {
        panic(0, (uint32_t)fsciBleRegister, 0, 0);
    }

    /* Save FSCI interface to be used for monitoring */
    fsciBleInterfaceId = fsciInterfaceId;
}

clientPacketStructured_t *fsciBleAllocFsciPacket(opGroup_t opCodeGroup, uint8_t opCode, uint16_t dataSize)
{
    /* Allocate buffer for the FSCI packet (header, data, and CRC) */
    clientPacketStructured_t *pClientPacket =
        (clientPacketStructured_t *)MEM_BufferAlloc(sizeof(clientPacketHdr_t) + dataSize + sizeof(uint8_t));

    if (NULL == pClientPacket)
    {
        /* Buffer can not be allocated */
        FSCI_Error(gFsciOutOfMessages_c, fsciBleInterfaceId);
        return NULL;
    }

    /* Create FSCI packet header */
    pClientPacket->header.opGroup = opCodeGroup;
    pClientPacket->header.opCode  = opCode;
    pClientPacket->header.len     = dataSize;

    /* Return the allocated FSCI packet */
    return pClientPacket;
}

void fsciBleNoParamCmdOrEvtMonitor(opGroup_t opCodeGroup, uint8_t opCode)
{
    clientPacketStructured_t *pClientPacket;

    /* Allocate the FSCI packet to be transmitted over UART (with FSCI header added) */
    pClientPacket = fsciBleAllocFsciPacket(opCodeGroup, opCode, 0);

    if (NULL == pClientPacket)
    {
        /* FSCI packet can not be allocated */
        return;
    }

    /* Transmit FSCI packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
