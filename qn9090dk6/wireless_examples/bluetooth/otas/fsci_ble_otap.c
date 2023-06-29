/*! *********************************************************************************
* \addtogroup FSCI BLE OTAP
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the BLE OTAP FSCI module
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "Panic.h"
#include "FsciInterface.h"
#include "MemManager.h"
#include "FunctionLib.h"

#include "fsci_ble_otap.h"



/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

#define FsciBleOtap_IsInterfaceValid(interface)     ((interface) < gFsciMaxInterfaces_c? TRUE : FALSE)

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

static uint32_t mFsciBleOtapInterfaceId = mFsciInvalidInterface_c;

/************************************************************************************
*************************************************************************************
* External memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* External functions prototypes
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

/*! *********************************************************************************
* \details  Register a FSCI interface for the BLE OTAP FSCI module.
*
********************************************************************************** */
void FsciBleOtap_Register (uint32_t fsciInterfaceId)
{
#if gFsciBleOtapEnabled_d
    if (gFsciSuccess_c != FSCI_RegisterOpGroup (gFsciBleOtapOpcodeGroup_c, 
                                                gFsciMonitorMode_c, 
                                                FsciBleOtap_Handler, 
                                                NULL,
                                                fsciInterfaceId))
    {
        panic(0, (uint32_t)FsciBleOtap_Register, 0, 0);
    }
    else
    {
        mFsciBleOtapInterfaceId = fsciInterfaceId;
    }
#endif /* gFsciBleOtapEnabled_d */
}


#if gFsciBleOtapEnabled_d
/*! *********************************************************************************
* \details  BLE OTAP FSCI messages handler.
*
********************************************************************************** */
void FsciBleOtap_Handler (void*     pData, 
                          void*     param,
                          uint32_t  fsciInterface)
{
    BleApp_HandleFsciBleOtapCommand ((clientPacket_t*)pData,
                                     fsciInterface);
    
    (void)MEM_BufferFree (pData);
}
#endif /* gFsciBleOtapEnabled_d */


#if gFsciBleOtapEnabled_d
/*! *********************************************************************************
* \details  Allocates a FSCI packet for the BLE OTAP FSCI module using the
*           set up memory allocator and retruns a pointer to theat packet.
*           Allocated length: FSCI header size + data size + FSCI packet CRC size.
*
********************************************************************************** */
clientPacketStructured_t* FsciBleOtap_AllocatePacket (opCode_t  opCode, 
                                                      uint16_t  dataSize)
{
    clientPacketStructured_t* pClientPacket;
    
    pClientPacket = (clientPacketStructured_t*)MEM_BufferAlloc (sizeof(clientPacketHdr_t) + 
                                                                (uint32_t)dataSize + 
                                                                sizeof(uint8_t));
    
    if(NULL == pClientPacket)
    {
        /* If the buffer can not be allocated try to send a FSCI error if the
         * BLE OTAP FSCI interface was initialized. */
        if (FsciBleOtap_IsInterfaceValid (mFsciBleOtapInterfaceId))
        {
            FSCI_Error((uint8_t)gFsciOutOfMessages_c, mFsciBleOtapInterfaceId);
        }
        return NULL;
    }
    
    /* Fill in the FSCI packet header components */
    pClientPacket->header.opGroup   = gFsciBleOtapOpcodeGroup_c;
    pClientPacket->header.opCode    = opCode;
    pClientPacket->header.len       = dataSize;
    
    /* Return the allocated FSCI packet */
    return pClientPacket;
}
#endif /* gFsciBleOtapEnabled_d */

#if gFsciBleOtapEnabled_d
/*! *********************************************************************************
* \details  Allocates a FSCI BLE OTAP packet with the specified opcode, payload
*           and payload length and sends it over the FSCI interface.
*
********************************************************************************** */
void FsciBleOtap_SendPkt (opCode_t* pOpCode,
                          uint8_t*  pPayload,
                          uint16_t  payloadLen)
{
    union
    {
        clientPacketStructured_t*   pFsciOtapPktTemp;
        clientPacket_t*             clientPacketTemp;
    }clientPacketVars;
    
    clientPacketStructured_t*   pFsciOtapPkt;
    
    pFsciOtapPkt = FsciBleOtap_AllocatePacket (*pOpCode,
                                               payloadLen);
    
    if (NULL != pFsciOtapPkt)
    {
        /* If memory allocation for the packet was succesful then fill in the 
         * payload and try to send the packet. */
        FLib_MemCpy (pFsciOtapPkt->payload,
                     pPayload,
                     payloadLen);
        
        if (FsciBleOtap_IsInterfaceValid (mFsciBleOtapInterfaceId))
        {
            clientPacketVars.pFsciOtapPktTemp = pFsciOtapPkt;
            FSCI_transmitFormatedPacket (clientPacketVars.clientPacketTemp, mFsciBleOtapInterfaceId);
        }
    }
    else
    {
        /* The FsciBleOtap_AllocatePacket() function tries to send a FSCI Error
         * if the packet cannot be allocated so there is noting to do here
         * at this moment.*/
    }
}
#endif /* gFsciBleOtapEnabled_d */

/*! *********************************************************************************
* @}
********************************************************************************** */
