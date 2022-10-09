/*! *********************************************************************************
 * \defgroup FSCI BLE OTAP
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* This file is the interface file for the BLE OTAP FSCI module
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef FSCI_BLE_OTAP_H
#define FSCI_BLE_OTAP_H

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/

#define gFsciBleOtapOpcodeGroup_c       0x4F

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/*! This is the application function which handles commands received for the
*   BLE OTAP module via the FSCI interface.
*/
extern void BleApp_HandleFsciBleOtapCommand (clientPacket_t*    pData,
                                             uint32_t           fsciInterface);

/*! *********************************************************************************
* \brief  Registers an interface to be used with the BLE OTAP FSCI module.
*
* \param[in]    fsciInterfaceId     Interface to be registered.
*
********************************************************************************** */
void FsciBleOtap_Register (uint32_t fsciInterfaceId);

/*! *********************************************************************************
* \brief  Handles BLE OTAP FSCI messages received on the FSCI interface
*         and associated with the BLE OTAP FACI Opcode Group.
*
* \param[in]    pData               Packet (containing FSCI header and FSCI 
*                                   payload) received over UART   
* \param[in]    param               Pointer given when this function is registered
*                                   in FSCI
* \param[in]    fsciInterfaceId     FSCI interface on which the packet was received  
*
********************************************************************************** */
void FsciBleOtap_Handler (void*     pData, 
                          void*     param,
                          uint32_t  fsciInterface);

/*! *********************************************************************************
* \brief  Allocates a FSCI packet for the BLE OTAP FSCI module.
*
* \param[in]    opCode      FSCI BLE OTAP operation code.
* \param[in]    dataSize    Size of the payload.
*
* \return       Returns a pointer to the allocated packet or NULL if it could
*               not be allocated.
*
********************************************************************************** */
clientPacketStructured_t* FsciBleOtap_AllocatePacket (opCode_t  opCode, 
                                                      uint16_t  dataSize);
                                                      
/*! *********************************************************************************
* \details  Allocates a FSCI BLE OTAP packet with the specified opcode, payload
*           and payload length and sends it over the FSCI interface.
*
********************************************************************************** */
/*! *********************************************************************************
* \brief  Allocates and send a FSCI packet for the BLE OTAP FSCI module
*         having the specified opcode and the given payload and length.
*
* \param[in]    pOpCode     Pointer to the FSCI BLE OTAP operation code.
* \param[in]    pPayload    Pointer to the packet payload.
* \param[in]    payloadLen  Payload length.
*
********************************************************************************** */
void FsciBleOtap_SendPkt (opCode_t* pOpCode,
                          uint8_t*  pPayload,
                          uint16_t  payloadLen);

#ifdef __cplusplus
}
#endif 


#endif /* FSCI_BLE_OTAP_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
