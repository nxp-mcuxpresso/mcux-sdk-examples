/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _IEEE_11073_AGENT_H_
#define _IEEE_11073_AGENT_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief agent disable/enable full feature macro */
#define AGENT_SUPPORT_FULL_FEATURE (0x00U)

/*! @brief agent send data QoS value */
#define AGENT_SEND_DATA_QOS (0x08U)

/*! @brief agent states */
#define AGENT_STATE_DISCONNECTED (0x00U)
#define AGENT_STATE_CON_UNASSOCIATED (0x01U)
#define AGENT_STATE_CON_ASSOCIATING (0x02U)
#define AGENT_STATE_CON_ASSOC_CFG_SENDING_CONFIG (0x03U)
#define AGENT_STATE_CON_ASSOC_CFG_WAITING_APPROVAL (0x04U)
#define AGENT_STATE_CON_ASSOC_OPERATING (0x05U)
#define AGENT_STATE_CON_DISASSOCIATING (0x06U)

/*! @brief agent event */
#define AGENT_EVENT_CONNECTED (0x00U)
#define AGENT_EVENT_ACCEPTED_AARQ (0x01U)
#define AGENT_EVENT_ACCEPTED_UNKNOWN_CONFIG_AARQ (0x02U)
#define AGENT_EVENT_REJECTED_AARQ (0x03U)
#define AGENT_EVENT_ACCEPTED_CONFIG (0x04U)
#define AGENT_EVENT_UNSUPPORTED_CONFIG (0x05U)
#define AGENT_EVENT_RECV_ROIV_CMIP_GET (0x06U)
#define AGENT_EVENT_RORS_CMIP_GET_SENT (0x07U)
#define AGENT_EVENT_MEASUREMENT_SENT (0x08U)

/*! @brief APDU header size */
#define APDU_HEADER_SIZE (0x04U)
#define ASSOC_RSP_HEADER_SIZE (0x02U)
#define ABRT_DATA_LENGTH (0x02U)
#define RLRE_DATA_LENGTH (0x02U)
#define RLRQ_DATA_LENGTH (0x02U)
#define PRST_HEADER_LENGTH (0x08U)
#define ANY_HEADER_SIZE (0x02U)
#define EVT_REPORT_RESULT_SIMPLE_HEADER_SIZE (0x0AU)
#define APDU_MAX_BUFFER_SIZE (0xFFU)

/*! @brief timeout definitions */
#define TO_ASSOC (10000U)
#define RC_ASSOC (3U)
#define TO_CONFIG (10000U)
#define TO_RELEASE (3000U)
#define TO_CA (3000U)
#define TO_GET (3000U)
#define TO_CS (3000U)

#if ((defined IEEE_MAX_TIMER_OBJECTS) && (IEEE_MAX_TIMER_OBJECTS > 0))
/*! @brief timer struct */
typedef struct _ieee11073_timer_struct
{
    uint8_t timerId;                      /*!< Timer identify */
    ieee11073_timer_object_t timerObject; /*!< Timer Object */
} ieee11073_timer_struct_t;
#endif

/*! @brief agent structure */
typedef struct _agent_struct
{
    void *agentHandle;  /*!< the agent handle */
    uint8_t agentState; /*!< the agent state */
#if IEEE_MAX_TIMER_OBJECTS
    ieee11073_timer_struct_t agentTimer[2U]; /*!< timer to implement timeout functionalities */
#endif
    uint16_t invokeId;                           /*!< the invokeId */
    uint8_t isSendingRorsCmipGet;                /*!< sending remote operation response | Get checking */
    uint8_t agentTxBuffer[APDU_MAX_BUFFER_SIZE]; /*!< the buffer to send */
} agent_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief initialize the agent device.
 *
 * This function initializes the agent device and starts transporting.
 *
 * @param handle            the agent handle.
 */
void AGENT_Init(void *handle);

/*!
 * @brief medical callback.
 * This function handles the callback of medical system.
 *
 * @param handle        the handle points to agent handle.
 * @param eventType     the event type.
 * @param data          the pointer to callback data.
 *
 * @return None.
 */
void AGENT_MedicalCallback(void *handle, uint8_t eventType, uint8_t *data);

/*!
 * @brief send association request.
 *
 * This function gets the association request data from application and sends
 * it to the host using PHDC transport channel, this request is to establish a
 * connection between the device and the host.
 *
 * @param handle        the agent handle.
 * @param associationData     the association request data.
 * @param size          the association request data size.
 */
void AGENT_SendAssociationRequest(void *handle, uint8_t *associationData, uint32_t size);

#if AGENT_SUPPORT_FULL_FEATURE
/*!
 * @brief send association abort request.
 *
 * This function implements association abort request, it builds abort data
 * and sends it out using PHDC transport channel.
 *
 * @param handle        the agent handle.
 * @param abortReason   the abort reason.
 */
void AGENT_SendAssociationAbortRequest(void *handle, abort_reason_t abortReason);

/*!
 * @brief send association release request.
 *
 * This function implements association release request, it builds release data
 * and sends it out using PHDC transport channel.
 *
 * @param handle            the agent handle.
 * @param releaseReason     the release reason.
 */
void AGENT_SendAssociationRleaseRequest(void *handle, release_request_reason_t releaseReason);
#endif

/*!
 * @brief set agent state.
 *
 * This function returns the state of device corresponding with specified agent.
 *
 * @param handle            the agent handle.
 * @param state             the state of Agent.
 */
void AGENT_SetAgentState(void *handle, uint8_t state);

/*!
 * @brief send agent configuration.
 *
 * This function gets the agent configuration from application and sends it to
 * the host using PHDC transport channel.
 *
 * @param handle        the agent handle.
 * @param config    the agent configuration data.
 * @param size          the agent configuration data size.
 */
void AGENT_SendConfig(void *handle, uint8_t *config, uint32_t size);
#if defined(__cplusplus)
}
#endif
#endif
/* _IEEE_11073_AGENT_H_ */
