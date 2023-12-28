/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_CONFIG_H__
#define __SRTM_CONFIG_H__

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "srtm_config_audio.h"
#include "srtm_config_nn.h"

/*!
 * @addtogroup srtm
 * @{
 */

/*******************************************************************************
 * Definitions, version for initial SRTM on RT500
 ******************************************************************************/
/*! @brief Defines SRTM major version */
#define SRTM_VERSION_MAJOR (0x01U)
/*! @brief Defines SRTM minor version */
#define SRTM_VERSION_MINOR (0x02U)
/*! @brief Defines SRTM bugfix version */
#define SRTM_VERSION_BUGFIX (0x00U)

/*! @brief SRTM version definition */
#define SRTM_MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))

/* There is no specific packing definition here for cross-compiler compatibility*/
/* There is no specific DEBUG MESSAGE Function here */

/**
 * @brief Timeout definition: infinite wait that never timeout
 */
#define SRTM_WAIT_FOR_EVER (0xFFFFFFFFU)

/**
 * @brief Timeout definition: no wait that return immediately
 */
#define SRTM_NO_WAIT (0x0U)

/*! @brief SRTM error code */
typedef enum _srtm_status
{
    SRTM_Status_Success = 0x00U,    /*!< Success */
    SRTM_Status_Error,              /*!< Failed */

    SRTM_Status_InvalidParameter,   /*!< Invalid parameter */
    SRTM_Status_InvalidMessage,     /*!< Invalid message */
    SRTM_Status_InvalidState,       /*!< Operate in invalid state */
    SRTM_Status_OutOfMemory,        /*!< Memory allocation failed */
    SRTM_Status_Timeout,            /*!< Timeout when waiting for an event */
    SRTM_Status_ListAddFailed,      /*!< Cannot add to list as node already in another list */
    SRTM_Status_ListRemoveFailed,   /*!< Cannot remove from list as node not in list */

    SRTM_Status_TransferTimeout,    /*!< Transfer timeout */
    SRTM_Status_TransferNotAvail,   /*!< Transfer failed due to peer core not ready */
    SRTM_Status_TransferFailed,     /*!< Transfer failed due to communication failure */

    SRTM_Status_ServiceNotFound,    /*!< Cannot find service for a request/notification */
    SRTM_Status_ServiceVerMismatch, /*!< Service version cannot support the request/notification */
} srtm_status_t;

/**
 * @brief SRTM message type fields
 */
typedef enum _srtm_message_type
{
    SRTM_MessageTypeRequest = 0x00U,  /*!< Request message */
    SRTM_MessageTypeResponse,         /*!< Response message for certain Request */
    SRTM_MessageTypeNotification,     /*!< Notification message that doesn't require response */
    SRTM_MessageTypeCommLast,         /*!< Last value of communication message */

    SRTM_MessageTypeProcedure = 0x40, /*!< Local procedure */
    SRTM_MessageTypeRawData   = 0x41, /*!< Raw data message */
} srtm_message_type_t;

/**
 * @brief SRTM message direction fields
 */
typedef enum _srtm_message_direct
{
    SRTM_MessageDirectNone = 0x00U, /*!< Local procedure message has no direction */
    SRTM_MessageDirectRx,           /*!< Received message */
    SRTM_MessageDirectTx,           /*!< Transfer message */
} srtm_message_direct_t;

/**
 * @brief SRTM communication packet head
 * Do NOT use any typedef enums for any shared structure, the size will be different cross different platforms!
 * ONLY use basic data types for consistant structure size!
 */
typedef struct _srtm_packet_head
{
    uint8_t category;
    uint8_t majorVersion;
    uint8_t minorVersion;
    uint8_t type;
    uint8_t command;
    uint8_t priority;
    uint8_t reserved[4U];
} srtm_packet_head_t;

/**
 * @brief SRTM list fields
 */
typedef struct _srtm_list
{
    struct _srtm_list *prev; /*!< previous list node */
    struct _srtm_list *next; /*!< next list node */
} srtm_list_t;

/**
 * @brief SRTM message structure
 * Do NOT use any typedef enums for any shared structure, the size will be different cross different platforms!
 * ONLY use basic data types for consistant structure size!
 */
#define SRTM_CMD_PARAMS_MAX 32
typedef struct _srtm_message
{
    srtm_list_t node;                    /*!< SRTM message list node to link to a list */
    srtm_packet_head_t head;             /*!< SRTM raw data, including header and payload for CommMessage */
    uint32_t error;                      /*!< SRTM message error status */
    uint32_t param[SRTM_CMD_PARAMS_MAX]; /*!< SRTM user defined message params */
} srtm_message;

/**
 * @brief async SRTM message structure
 */
#define SRTM_CMD_PARAMS_MAX 32
typedef struct _srtm_message_async
{
    srtm_message msg;
    void (*cb)(void *, srtm_message *msg);
    void *params;
} srtm_message_async;

/**
 * @brief SRTM message category fields
 */
typedef enum _srtm_message_category
{
    SRTM_MessageCategory_GENERAL = 0x00U,
    SRTM_MessageCategory_AUDIO,
    SRTM_MessageCategory_NN,
} srtm_message_category_t;

/**
 * SRTM command fields
 */

/**
 * @brief SRTM general command fields
 */
typedef enum _srtm_rt600_general_command
{
    SRTM_Command_ECHO = 0x00U, /*!< ECHO */
    SRTM_Command_xa_nn_malloc,
    SRTM_Command_xa_nn_free,
    SRTM_Command_xa_nn_empty,
    SRTM_Command_SYST, /*!< For System Test ONLY */
    SRTM_Command_GENERAL_MAX,
} srtm_general_command_t;

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief Get SRTM version.
 *
 * @return SRTM version.
 */
static inline uint32_t SRTM_GetVersion(void)
{
    return SRTM_MAKE_VERSION(SRTM_VERSION_MAJOR, SRTM_VERSION_MINOR, SRTM_VERSION_BUGFIX);
}

/*! @} */

#endif /* __SRTM_CONFIG_H__ */
