/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSCI_COMMUNICATION_H_
#define _FSCI_COMMUNICATION_H_

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "EmbeddedTypes.h"
#include "SerialManager.h"
#include "FsciInterface.h"

#include "fsl_os_abstraction.h"

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
typedef enum
{
    PACKET_IS_VALID,
    PACKET_IS_TO_SHORT,
    FRAMING_ERROR,
    INTERNAL_ERROR
} fsci_packetStatus_t;

typedef struct fsciComm_tag
{
    clientPacket_t *pPacketFromClient;
    clientPacketHdr_t pktHeader;
    uint16_t bytesReceived;
#if gFsciHostSupport_c
    osaMutexId_t syncHostMutexId;
#endif
#if gFsciRxAck_c
    osaMutexId_t syncTxRxAckMutexId;
    osaEventId_t ackEvent;
    uint8_t txRetryCnt;
    volatile bool_t ackReceived;
#endif
#if gFsciRxTimeout_c
    osaTimerId_t rxRestartTmr;
    bool_t rxOngoing;
    volatile bool_t rxTmrExpired;
#endif
} fsciComm_t;

typedef enum
{
    gFSCIHost_RspReady_c      = (1 << 0),
    gFSCIHost_RspParamReady_c = (1 << 1),
} fsciHostEventType_t;

typedef enum
{
    gFSCIAckReady_c = (1 << 0),
} fsciAckEventType_t;

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
extern uint8_t gFsciSerialInterfaces[];

/*! *********************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
********************************************************************************** */
#define gFSCI_StartMarker_c 0x02
#define gFSCI_EndMarker_c   0x03
#define gFSCI_EscapeChar_c  0x7F

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */
void FSCI_commInit(void *initStruct);
void FSCI_receivePacket(void *param);
uint32_t FSCI_GetVirtualInterface(uint32_t fsciInterface);
uint32_t FSCI_GetFsciInterface(uint32_t hwInterface, uint32_t virtualInterface);
uint32_t FSCI_encodeEscapeSeq(uint8_t *pDataIn, uint32_t len, uint8_t *pDataOut);
void FSCI_decodeEscapeSeq(uint8_t *pData, uint32_t len);
uint8_t FSCI_computeChecksum(void *pBuffer, uint16_t size);

#if gFsciHostSupport_c
void FSCI_HostSyncLock(uint32_t fsciInstance, opGroup_t OG, opCode_t OC, bool_t OP);
void FSCI_HostSyncUnlock(uint32_t fsciInstance);
#endif

#endif /* _FSCI_COMMUNICATION_H_ */
