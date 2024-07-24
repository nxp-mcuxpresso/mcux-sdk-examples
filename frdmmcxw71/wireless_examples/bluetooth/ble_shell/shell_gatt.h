/*! *********************************************************************************
 * \defgroup SHELL GATT
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2021, 2023 NXP
*
*
* \file
*
* This file is the interface file for the GATT Shell module
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef SHELL_GATT_H
#define SHELL_GATT_H

/*************************************************************************************
**************************************************************************************
* Public types and macros
**************************************************************************************
*************************************************************************************/

typedef struct gattUuidNames_tag
{
    uint16_t    uuid;
    char*       name;
}gattUuidNames_t;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
extern const gattUuidNames_t mGattServices[];
extern const gattUuidNames_t mGattChars[];
extern const gattUuidNames_t mGattDescriptors[];
/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*! *********************************************************************************
 * \brief        Returns the name of a service identified by its UUID.
 *
 * \param[in]    uuid16                  UUID of the service
 * \return       char*                   Pointer to service's name
 ********************************************************************************** */
const char* ShellGatt_GetServiceName
(
    uint16_t uuid16
);

/*! *********************************************************************************
 * \brief        Returns the name of a given characteristic identified by its UUID.
 *
 * \param[in]    uuid16                  UUID of the characteristic
 * \return       char*                   Pointer to characteristic's name
 ********************************************************************************** */
const char* ShellGatt_GetCharacteristicName
(
    uint16_t uuid16
);

/*! *********************************************************************************
 * \brief        Returns the name of a given descriptor identified by its UUID.
 *
 * \param[in]    uuid16                  UUID of the descriptor
 * \return       char*                   Pointer to descriptor's name
 ********************************************************************************** */
const char* ShellGatt_GetDescriptorName
(
    uint16_t uuid16
);

/*! *********************************************************************************
 * \brief        Entry point for "gatt" shell command.
 *
 * \param[in]    argc           Number of arguments
 * \param[in]    argv           Array of argument's values
 *
 * \return       int8_t         Command status
 ********************************************************************************** */
shell_status_t ShellGatt_Command(shell_handle_t shellHandle, int32_t argc, char **argv);

/* SHELL GATT Events Callback */
/*! *********************************************************************************
 * \brief        Handles GATT server callback from host stack.
 *
 * \param[in]    deviceId        Client peer device ID.
 * \param[in]    pServerEvent    Pointer to gattServerEvent_t.
 ********************************************************************************** */
void ShellGatt_ServerCallback
(
   deviceId_t deviceId,
   gattServerEvent_t* pServerEvent
);

/*! *********************************************************************************
 * \brief        Handles GATT client callback from host stack.
 *
 * \param[in]    serverDeviceId      Server peer device ID.
 * \param[in]    procedureType       GATT procedure type.
 * \param[in]    procedureType       GATT procedure result.
 * \param[in]    error               Result.
 ********************************************************************************** */
void ShellGatt_ClientCallback
(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
);

/*! *********************************************************************************
 * \brief        Handles GATT notification message.
 *
 * \param[in]    serverDeviceId               Server peer device ID.
 * \param[in]    characteristicValueHandle    Handle of the notified characteristic
 * \param[in]    aValue                       Pointer to characteristic's value
 * \param[in]    valueLength                  Length of characteristic's value
 ********************************************************************************** */
void ShellGatt_NotificationCallback
(
    deviceId_t          serverDeviceId,
    uint16_t            characteristicValueHandle,
    uint8_t*            aValue,
    uint16_t            valueLength
);

/*! *********************************************************************************
 * \brief        Handles GATT indication message.
 *
 * \param[in]    serverDeviceId               Server peer device ID.
 * \param[in]    characteristicValueHandle    Handle of the indicated characteristic
 * \param[in]    aValue                       Pointer to characteristic's value
 * \param[in]    valueLength                  Length of characteristic's value
 ********************************************************************************** */
void ShellGatt_IndicationCallback
(
    deviceId_t          serverDeviceId,
    uint16_t            characteristicValueHandle,
    uint8_t*            aValue,
    uint16_t            valueLength
);

#ifdef __cplusplus
}
#endif


#endif /* SHELL_GATT_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
