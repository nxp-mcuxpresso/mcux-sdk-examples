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
#include "FsciCommunication.h"
#include "FsciCommands.h"
#include "FunctionLib.h"
#include "MemManager.h"
#include "SerialManager.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if gFsciIncluded_c
/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

#define gFsciTextLogSize_c (gFsciMaxPayloadLen_c - sizeof(clientPacketHdr_t) - gFsciTimestampSize_c - 1)
#define gFsciFileLogSize_c 220

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

extern uint8_t gFsciTxDisable;

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/******************************************************************************
To save RAM, use a dynamically allocated message buffer to construct
the message. If no buffer is available, send an error buffer and give up.
Write as much of the requested data as possible to the buffer, but there
is no guarantee there the buffer will be big enough.
Concatenate the source data to the print buffer, then
******************************************************************************/

/*! *********************************************************************************
 * \brief   To save RAM, use a dynamically allocated message buffer to construct
 * the message. If no buffer is available, send an error buffer and give up.
 * Write as much of the requested data as possible to the buffer, but there
 * is no guarantee there the buffer will be big enough.
 * Concatenate the source data to the print buffer
 *
 * \param[in] readyToSend - if TRUE, send the buffer to the external client
 * \param[in] pSrc - pointer to the data location
 * \param[in] len - length of the data
 *
 * \remarks FSCI's version of printf() debugging.
 *
 ********************************************************************************** */
void FSCI_Print(uint8_t readyToSend, void *pSrc, fsciLen_t len)
{
    static uint8_t *pData    = NULL;
    static uint16_t totalLen = 0;

    if (!gFsciTxDisable && (gFsciLoggingInterface_c < gFsciMaxInterfaces_c))
    {
        if (NULL == pData)
        {
            pData = MEM_BufferAlloc(sizeof(clientPacket_t));
            if (pData)
            {
                ((clientPacket_t *)pData)->structured.header.opGroup = gFSCI_CnfOpcodeGroup_c;
                ((clientPacket_t *)pData)->structured.header.opCode  = mFsciMsgDebugPrint_c;
                totalLen                                             = sizeof(clientPacketHdr_t);
            }
        }

        /* Check for overflow */
        if (pData && (totalLen + len <= gFsciMaxPayloadLen_c))
        {
            FLib_MemCpy(&pData[totalLen], pSrc, len);
            totalLen += len;

            if (readyToSend)
            {
                ((clientPacket_t *)pData)->structured.header.len = totalLen - sizeof(clientPacketHdr_t);
                FSCI_transmitFormatedPacket((clientPacket_t *)pData, gFsciLoggingInterface_c);
                pData    = NULL;
                totalLen = 0;
            }
        }
    }
}

/*! *********************************************************************************
 * \brief   Sends a formated text string to the host.
 *
 * \param[in] const char *fmt - The string and format specifiers to output to the datalog.
 * \param[in] ... - The variable number of parameters to output to the datalog.
 *
 * \remarks Behaves like printf
 *
 ********************************************************************************** */
#if gFsciUseFmtLog_c
void FSCI_LogFormatedText(const char *fmt, ...)
{
    clientPacket_t *pFsciData;
    uint16_t length;
    va_list argp;

    if (gFsciTxDisable)
        return;

    pFsciData = MEM_BufferAlloc(sizeof(clientPacket_t));
    if (NULL == pFsciData)
    {
        FSCI_Error(gFsciOutOfMessages_c, gFsciLoggingInterface_c);
        return;
    }

    pFsciData->structured.header.opGroup = gFSCI_LoggingOpcodeGroup_c;
    pFsciData->structured.header.opCode  = 0x01;

#if gFsciTimestampSize_c
    *((uint64_t *)pFsciData->structured.payload) = TimerGetAbsoluteTime();
#endif

    OSA_InterruptDisable();
    va_start(argp, fmt);
    length = vsnprintf((char *)(&pFsciData->structured.payload[gFsciTimestampSize_c]), gFsciTextLogSize_c, fmt, argp);
    va_end(argp);
    OSA_InterruptEnable();

    if (length >= gFsciTextLogSize_c)
    {
        pFsciData->structured.payload[gFsciTextLogSize_c - 6] = '.';
        pFsciData->structured.payload[gFsciTextLogSize_c - 5] = '.';
        pFsciData->structured.payload[gFsciTextLogSize_c - 4] = '.';
        pFsciData->structured.payload[gFsciTextLogSize_c - 3] = '\n';
        pFsciData->structured.payload[gFsciTextLogSize_c - 2] = '\r';
        pFsciData->structured.payload[gFsciTextLogSize_c - 1] = '\0';
        length                                                = gFsciTextLogSize_c - 1;
    }

    /* Compute total payload len */
    pFsciData->structured.header.len = length + gFsciTimestampSize_c;

    FSCI_transmitFormatedPacket(pFsciData, gFsciLoggingInterface_c);
}
#endif /* gFsciUseFmtLog_c */

/*! *********************************************************************************
 * \brief   Sends binary data to a specific file.
 *
 * \param[in] char *fileName - The name of the file in which the data will be stored.
 * \param[in] mode - The mode in which the file will be accessed.
 * \param[in] pData - Pointer to the data to be written.
 * \param[in] dataSize - The size of the data to be written.
 *
 ********************************************************************************** */
#if gFsciUseFileDataLog_c
void FSCI_LogToFile(char *fileName, uint8_t *pData, uint16_t dataSize, uint8_t mode)
{
    uint8_t fileNameSize = strlen(fileName);
    uint16_t idx;
    uint8_t checksum;
    clientPacket_t *pFsciData;
#if gFsciUseEscapeSeq_c
    uint16_t size;
#endif

    pFsciData = MEM_BufferAlloc(sizeof(clientPacket_t));
    if (NULL == pFsciData)
    {
        FSCI_Error(gFsciOutOfMessages_c, gFsciLoggingInterface_c);
        return;
    }

    // Disable text logging!
    gFsciTxDisable = 1;

    // Fill the message Header
    pFsciData->structured.header.opGroup = gFSCI_LoggingOpcodeGroup_c;
    pFsciData->structured.header.opCode  = mode;

#if gFsciTimestampSize_c
    *((uint64_t *)pFsciData->structured.payload) = TimerGetAbsoluteTime();
#endif

    idx                                  = gFsciTimestampSize_c;
    pFsciData->structured.payload[idx++] = fileNameSize;

    // Compute checksum
    checksum = FSCI_computeChecksum(&pFsciData->raw[1], sizeof(clientPacketHdr_t) + idx - 1);
    checksum ^= FSCI_computeChecksum(fileName, fileNameSize);
    checksum ^= FSCI_computeChecksum(pData, dataSize);

    // Transmit data
#if gFsciUseEscapeSeq_c
    pFsciData->structured.header.len = gFsciTimestampSize_c + sizeof(fileNameSize) + fileNameSize + dataSize;
    size                             = FSCI_encodeEscapeSeq(buffer, idx, &buffer[idx]);
    Serial_SyncWrite(gFsciSerialInterfaces[gFsciLoggingInterface_c], &buffer[idx], size);

    size = FSCI_encodeEscapeSeq((uint8_t *)fileName, fileNameSize, buffer);
    Serial_SyncWrite(gFsciSerialInterfaces[gFsciLoggingInterface_c], buffer, size);

    idx = sizeof(clientPacketStructured_t) / 2;
    while (dataSize)
    {
        if (dataSize > idx)
        {
            size = FSCI_encodeEscapeSeq(pData, idx, buffer);
            pData += idx;
            dataSize -= idx;
        }
        else
        {
            size     = FSCI_encodeEscapeSeq(pData, dataSize, buffer);
            dataSize = 0;
        }

        Serial_SyncWrite(gFsciSerialInterfaces[gFsciLoggingInterface_c], buffer, size);
    }

    size           = FSCI_encodeEscapeSeq(&checksum, sizeof(checksum), buffer);
    buffer[size++] = gFSCI_EndMarker_c;
    Serial_SyncWrite(gFsciSerialInterfaces[gFsciLoggingInterface_c], buffer, size);

#else /* gFsciUseEscapeSeq_c */
    pFsciData->structured.header.len = gFsciTimestampSize_c + sizeof(fileNameSize) + fileNameSize + dataSize;
    Serial_SyncWrite(gFsciSerialInterfaces[gFsciLoggingInterface_c], pFsciData->raw, sizeof(clientPacketHdr_t) + idx);
    Serial_SyncWrite(gFsciSerialInterfaces[gFsciLoggingInterface_c], (uint8_t *)fileName, fileNameSize);
    Serial_SyncWrite(gFsciSerialInterfaces[gFsciLoggingInterface_c], pData, dataSize);
    Serial_SyncWrite(gFsciSerialInterfaces[gFsciLoggingInterface_c], &checksum, sizeof(checksum));
#endif

    gFsciTxDisable = 0;
    MEM_BufferFree(pFsciData);
}
#endif /* gFsciUseFileDataLog_c */

#endif /* gFsciIncluded_c */
