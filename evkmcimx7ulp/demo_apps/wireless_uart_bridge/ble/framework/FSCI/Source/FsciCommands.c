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

#include "EmbeddedTypes.h"
#include "FsciInterface.h"
#include "FsciCommands.h"
#include "FsciCommunication.h"
#include "FunctionLib.h"
#include "MemManager.h"

#if gFsciIncluded_c
/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
/* Set to TRUE when FSCI_Error() is called. */
uint8_t mFsciErrorReported;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* FSCI Error message */
static gFsciErrorMsg_t mFsciErrorMsg = {{
                                            gFSCI_StartMarker_c,
                                            gFSCI_CnfOpcodeGroup_c,
                                            mFsciMsgError_c,
                                            sizeof(clientPacketStatus_t),
                                        },
                                        gFsciSuccess_c,
                                        0,
                                        0};

/* FSCI Ack message */
#if gFsciTxAck_c
static gFsciAckMsg_t mFsciAckMsg = {
    {gFSCI_StartMarker_c, gFSCI_CnfOpcodeGroup_c, mFsciMsgAck_c, sizeof(uint8_t)}, 0, 0, 0};
#endif

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * \brief   This is the handler function for the FSCI OpGroup.
 *          It calls the handler function for the received OpCode.
 *
 * \param[in] pData pointer to location of the received data
 * \param[in] fsciInterface the interface on which the packet was received
 *
 ********************************************************************************** */
void fsciMsgHandler(void *pData, uint32_t fsciInterface)
{
    /* I'm the BLE client and won't response BLE request */
    MEM_BufferFree(pData);
    FSCI_Error(gFsciUnknownOpcode_c, fsciInterface);
}

/*! *********************************************************************************
 * \brief  Send an error message back to the external client.
 *         This function should not block even if there is no more dynamic memory available
 *
 * \param[in] errorCode the erros encountered
 * \param[in] fsciInterface the interface on which the packet was received
 *
 *
 ********************************************************************************** */
void FSCI_Error(uint8_t errorCode, uint32_t fsciInterface)
{
    uint32_t virtInterface = FSCI_GetVirtualInterface(fsciInterface);
    uint8_t size           = sizeof(mFsciErrorMsg) - 1;

    /* Don't cascade error messages. */
    if (!mFsciErrorReported)
    {
        mFsciErrorMsg.status   = errorCode;
        mFsciErrorMsg.checksum = FSCI_computeChecksum(((uint8_t *)&mFsciErrorMsg) + 1, size - 2);

        if (virtInterface)
        {
#if gFsciMaxVirtualInterfaces_c
            mFsciErrorMsg.checksum2 = mFsciErrorMsg.checksum;
            mFsciErrorMsg.checksum += virtInterface;
            mFsciErrorMsg.checksum2 ^= mFsciErrorMsg.checksum;
            size++;
#else
            (void)virtInterface;
#endif
        }

        Serial_SyncWrite(gFsciSerialInterfaces[fsciInterface], (uint8_t *)&mFsciErrorMsg, size);

        mFsciErrorReported = TRUE;
    }
}

#if gFsciTxAck_c
/*! *********************************************************************************
 * \brief  Send an ack message back to the external client.
 *
 * \param[in] checksum of the packet received
 * \param[in] fsciInterface the interface on which the packet was received
 *
 *
 ********************************************************************************** */
void FSCI_Ack(uint8_t checksum, uint32_t fsciInterface)
{
    uint32_t virtInterface = FSCI_GetVirtualInterface(fsciInterface);
    uint8_t size           = sizeof(mFsciAckMsg) - 1;

    mFsciAckMsg.checksumPacketReceived = checksum;
    mFsciAckMsg.checksum               = FSCI_computeChecksum(((uint8_t *)&mFsciAckMsg) + 1, size - 2);

    if (virtInterface)
    {
#if gFsciMaxVirtualInterfaces_c
        mFsciAckMsg.checksum2 = mFsciAckMsg.checksum;
        mFsciAckMsg.checksum += virtInterface;
        mFsciAckMsg.checksum2 ^= mFsciAckMsg.checksum;
        size++;
#else
        (void)virtInterface;
#endif
    }

    Serial_SyncWrite(gFsciSerialInterfaces[fsciInterface], (uint8_t *)&mFsciAckMsg, size);
}
#endif

#if gFsciHostSupport_c
/*! *********************************************************************************
 * \brief  Send a CPU-Reset Request to a FSCI black box
 *
 * \param[in] fsciInterface the interface to send the packet on
 *
 ********************************************************************************** */
gFsciStatus_t FSCI_ResetReq(uint32_t fsciInterface)
{
    clientPacket_t *pFsciPacket = MEM_BufferAlloc(sizeof(clientPacketHdr_t) + 2);
    gFsciStatus_t status        = gFsciSuccess_c;
    uint8_t checksum            = 0;
    uint8_t size                = 0;

    if (NULL == pFsciPacket)
    {
        status = gFsciOutOfMessages_c;
    }
    else
    {
        pFsciPacket->structured.header.startMarker = gFSCI_StartMarker_c;
        pFsciPacket->structured.header.opGroup     = gFSCI_ReqOpcodeGroup_c;
        pFsciPacket->structured.header.opCode      = mFsciMsgResetCPUReq_c;
        pFsciPacket->structured.header.len         = 0;
        size                                       = sizeof(clientPacketHdr_t) + 1;
        checksum                                   = FSCI_computeChecksum(pFsciPacket->raw + 1, size - 2);
        pFsciPacket->structured.payload[0]         = checksum;
        Serial_SyncWrite(gFsciSerialInterfaces[fsciInterface], pFsciPacket->raw, size);
        MEM_BufferFree(pFsciPacket);
    }

    return status;
}
#endif

#endif /* gFsciIncluded_c */
