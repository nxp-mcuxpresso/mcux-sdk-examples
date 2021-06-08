/*! *********************************************************************************
 * \defgroup L2CA L2CAP
 * @{
 ********************************************************************************** */
/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _L2CA_INTERFACE_H
#define _L2CA_INTERFACE_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "ble_general.h"

#include "fsl_os_abstraction.h"

#include "l2ca_types.h"

/************************************************************************************
*************************************************************************************
* Public constants & macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
typedef struct l2caConfigStruct_tag
{
    /* The list of the supported LE features for the Controller */
    uint32_t leFeatures;

    /* Maximum length (in octets) of the data portion of each HCI ACL Data Packet
        that the Controller is able to accept. */
    uint32_t hciLeBufferSize;

    /* The maximum size of payload data in octets that the L2CAP layer entity is
    capable of accepting. The MPS corresponds to the maximum PDU payload size. */
    uint16_t maxPduPayloadSize;
} l2caConfigStruct_t;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Interface callback type definitions
*************************************************************************************
************************************************************************************/
typedef l2caGenericCallback_t l2caAttChannelCallback_t;
typedef l2caGenericCallback_t l2caSmpChannelCallback_t;

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*
 * L2CAP Interface Primitives
 */

bleResult_t L2ca_Init(void);

/**********************************************************************************
 * \brief
 *
 * \param[in]
 *
 * \param[out]
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t L2ca_Config(l2caConfigStruct_t *pConfigStruct);

/**********************************************************************************
 * \brief        Sends a data packet through ATT Channel
 *
 * \param[in]    deviceId            The DeviceID for which the command is intended
 * \param[in]    pPacket             Data buffer to be transmitted
 * \param[in]    packetLength        Length of the data buffer
 *
 * \return       Result of the operation
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t L2ca_SendAttData(deviceId_t deviceId, uint8_t *pPacket, uint16_t packetLength);

/**********************************************************************************
 * \brief        Sends a data packet through SM Channel
 *
 * \param[in]    deviceId            The DeviceID for which the command is intended
 * \param[in]    pPacket             Data buffer to be transmitted
 * \param[in]    packetLength        Length of the data buffer
 *
 * \return       Result of the operation
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t L2ca_SendSmpData(deviceId_t deviceId, uint8_t *pPacket, uint16_t packetLength);

/**********************************************************************************
 * \brief
 *
 * \param[in]
 *
 * \return       Result of the operation
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t L2ca_RegisterAttCallback(l2caAttChannelCallback_t pCallback);

/**********************************************************************************
 * \brief
 *
 * \param[in]
 *
 * \return       Result of the operation
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t L2ca_RegisterSmpCallback(l2caSmpChannelCallback_t pCallback);

void L2ca_NotifyConnection(deviceId_t deviceId);

void L2ca_NotifyDisconnection(deviceId_t deviceId);

#ifdef __cplusplus
}
#endif

#endif /* _L2CA_INTERFACE_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
