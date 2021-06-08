/*! *********************************************************************************
 * \addtogroup L2CA
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

#ifndef _L2CA_TYPES_H
#define _L2CA_TYPES_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "ble_general.h"

/************************************************************************************
*************************************************************************************
* Public constants & macros
*************************************************************************************
************************************************************************************/
/*
    CID name space on LE-U logical link

    CID             Description
    -------------   ---------------------
    0X0000          Null identifier (Not Allowed)
    0x0001-0x0003   Reserved
    0x0004          Attribute Protocol
    0x0005          Low Energy L2CAP Signalling channel
    0x0006          Security Manager Protocol
    0x0007-0x001F   Reserved
    0x0020-0x003E   Assigned Numbers
    0x003F          Reserved
    0x0040-0x007F   Dynamically allocated
    0x0080-0xFFFF   Reserved
*/
#define gL2capCidNull_c      (0x0000)
#define gL2capCidAtt_c       (0x0004)
#define gL2capCidSignaling_c (0x0005)
#define gL2capCidSmp_c       (0x0006)

#define gL2capCidSigAssignedFirst_c (0x0020)
#define gL2capCidSigAssignedLast_c  (0x003E)

#define gL2capCidLePsmDynamicFirst_c (0x0040)
#define gL2capCidLePsmDynamicLast_c  (0x007F)

#define gL2capCidNotApplicable_c (0xFFFF)

/*
    LE Protocol/Service Multiplexer

    Range           Description
    -------------   ---------------------
    0x0001-0x007F   Fixed, SIG assigned. LE_PSM is fixed for all implementations
    0x0080-0x00FF   Dynamic LE_PSM may be fixed for a given implementation or
                    may be assigned at the time the service is registered in GATT
    0x0100-0xFFFF   Reserved Not applicable
*/
#define gL2caLePsmSigAssignedFirst_c (0x0001)
#define gL2caLePsmSigAssignedLast_c  (0x007F)

#define gL2caLePsmDynamicFirst_c (0x0080)
#define gL2caLePsmDynamicLast_c  (0x00FF)

#define gL2capDefaultMtu_c (23)
#define gL2capDefaultMps_c (23)
#define gL2capMaximumMps_c (65533)

#define gExpandAsEnum_m(a, b, c)  b = c,
#define gExpandAsTable_m(a, b, c) c,

/*
 *   Add here all Bluetooth SIG Assigned Numbers for L2CAP LE_PSM
 */
#define gLePsmSigAssignedNumbersTable_m(entry) entry(0, gL2ca_Le_Psm_Ipsp_c, 0x0023)

typedef enum
{
    gLePsmSigAssignedNumbersTable_m(gExpandAsEnum_m)
} l2caLePsmSigAssignedType_t;

#define gL2caLePsmSigAssignedCount_c (1)
#define gL2caLePsmDynamicCount_c     (0)

#define gL2caMaxLePsmSupported_c (gL2caLePsmSigAssignedCount_c + gL2caLePsmDynamicCount_c)

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
typedef enum
{
    gCommandNotUnderstood_c = 0x00,
    gSignalingMtuExceeded_c = 0x01,
    gInvalidCidInRequestd_c = 0x02
} l2caCommandRejectReasonType_t;

typedef enum
{
    gL2ca_LePsmConnectRequest_c,
    gL2ca_LePsmConnectionComplete_c,
    gL2ca_LePsmDisconnectNotification_c,
    gL2ca_NoPeerCredits_c,
    gL2ca_LocalCreditsNotification_c,
} l2capControlMessageType_t;

/************************************************************************************
*************************************************************************************
* Interface callback type definitions
*************************************************************************************
************************************************************************************/
typedef void (*l2caGenericCallback_t)(deviceId_t deviceId, uint8_t *pPacket, uint16_t packetLength);

typedef void (*l2caControlCallback_t)(l2capControlMessageType_t messageType, void *pMessage);

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* _L2CA_TYPES_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
