/*! *********************************************************************************
 * \defgroup BLE OTAP Client ATT
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2019, 2023 NXP
*
*
* \file
*
* This file is the interface file for the BLE OTAP Client ATT application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef _OTAP_CLIENT_H_
#define _OTAP_CLIENT_H_

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*! *********************************************************************************
* \brief        Configures the OTAP service.
*
*\return        TRUE - Successfully configured, FALSE - an error occurred
********************************************************************************** */
bool_t OtapClient_Config(void);

/*! *********************************************************************************
* \brief        Callback for ATT MTU changed event.
*
* \param[in]    deviceId        The device ID of the connected peer.
* \param[in]    negotiatedMtu   Negotiated MTU length
********************************************************************************** */
void OtapClient_AttMtuChanged (deviceId_t deviceId, uint16_t negotiatedMtu);

/*! *********************************************************************************
* \brief        Handles the CCCD Written GATT event
*
* \param[in]    deviceId    The device ID of the connected peer
* \param[in]    handle      The handle of the written characteristic
* \param[in]    cccd        Flags for the value of the CCCD
********************************************************************************** */
void OtapClient_CccdWritten (deviceId_t deviceId, uint16_t handle, gattCccdFlags_t cccd);

/*! *********************************************************************************
* \brief        Handles the Attribute Written GATT event
*
* \param[in]    deviceId    The device ID of the connected peer
* \param[in]    handle      The handle of the written attribute
* \param[in]    length      Length of the value to be written
* \param[in]    pValue      Pointer to attribute's value
********************************************************************************** */
void OtapClient_AttributeWritten (deviceId_t deviceId, uint16_t handle, uint16_t length, uint8_t* pValue);

/*! *********************************************************************************
* \brief        Handles the Attribute Written Without Response GATT event
*
* \param[in]    deviceId    The device ID of the connected peer
* \param[in]    handle      The handle of the written attribute
* \param[in]    length      Length of the value to be written
* \param[in]    pValue      Pointer to attribute's value
********************************************************************************** */
void OtapClient_AttributeWrittenWithoutResponse (deviceId_t deviceId, uint16_t handle, uint16_t length, uint8_t* pValue);

/*! *********************************************************************************
* \brief        Handles the Value Confirmation GATT event
*
* \param[in]    deviceId    The device ID of the connected peer
********************************************************************************** */
void OtapClient_HandleValueConfirmation (deviceId_t deviceId);

/*! *********************************************************************************
* \brief        Handles the Connection Complete event
*
* \param[in]    deviceId   Device ID of the connected peer
********************************************************************************** */
void OtapClient_HandleConnectionEvent (deviceId_t deviceId);

/*! *********************************************************************************
* \brief        Handles the Disconnection event
*
* \param[in]    deviceId   Device ID of the disconnected peer
********************************************************************************** */
void OtapClient_HandleDisconnectionEvent (deviceId_t deviceId);

/*! *********************************************************************************
* \brief        Handles the Encryption Changed event
*
* \param[in]    deviceId   Device ID of the connected peer
********************************************************************************** */
void OtapClient_HandleEncryptionChangedEvent (deviceId_t deviceId);

#ifdef __cplusplus
}
#endif


#endif /* _OTAP_CLIENT_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
