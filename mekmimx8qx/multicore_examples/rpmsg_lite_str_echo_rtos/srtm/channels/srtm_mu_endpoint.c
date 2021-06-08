/*
 * Copyright 2019, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include "srtm_heap.h"
#include "srtm_dispatcher.h"
#include "srtm_dispatcher_struct.h"
#include "srtm_peercore.h"
#include "srtm_peercore_struct.h"
#include "srtm_channel.h"
#include "srtm_channel_struct.h"
#include "srtm_mu_endpoint.h"
#include "srtm_message.h"
#include "srtm_message_struct.h"
#include "fsl_mu.h"
#include "fsl_irqsteer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef SRTM_DEBUG_COMMUNICATION
#define SRTM_DEBUG_COMMUNICATION (0)
#endif

#define SRTM_MU_NOTIFY_CHANNEL 1
#define SRTM_MU_IRQ_CHANNEL    3
/* Get the NVIC IRQn of given IRQSTEER IRQn */
#define GET_IRQSTEER_MASTER_IRQn(IRQn) \
    (IRQn_Type)(IRQSTEER_0_IRQn + (IRQn - FSL_FEATURE_IRQSTEER_IRQ_START_INDEX) / 64U)

typedef struct _srtm_mu_endpoint
{
    struct _srtm_channel channel;
    srtm_mu_endpoint_config_t config;
    uint32_t index;
    uint32_t *payload;
    bool started;
} * srtm_mu_endpoint_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static srtm_mu_endpoint_t handle;
/*******************************************************************************
 * Code
 ******************************************************************************/
static void SRTM_MUEndpoint_NotifyPeerCore(srtm_channel_t channel)
{
    srtm_mu_endpoint_t handle = (srtm_mu_endpoint_t)(void *)channel;
    MU_Type *targetMu         = handle->config.mu;
    MU_SendMsgNonBlocking(targetMu, SRTM_MU_NOTIFY_CHANNEL, SRTM_MU_MSG_READY_B);
    /*
     * U-Boot will poll every other channel
     */
    MU_SendMsgNonBlocking(targetMu, 0, 0);
    MU_SendMsgNonBlocking(targetMu, 2, 0);
    MU_SendMsgNonBlocking(targetMu, 3, 0);
}

void SRTM_MUEndpoint_Handler(void)
{
    uint32_t type;
    void *payload;
    uint32_t payload_len;
    uint32_t index;
    MU_Type *targetMu;
    srtm_message_type_t msgType;

    targetMu    = handle->config.mu;
    index       = MU_ReceiveMsgNonBlocking(targetMu, 0);
    type        = MU_ReceiveMsgNonBlocking(targetMu, 1);
    payload     = (void *)MU_ReceiveMsgNonBlocking(targetMu, 2);
    payload_len = MU_ReceiveMsgNonBlocking(targetMu, 3);

#if SRTM_DEBUG_COMMUNICATION
    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: MU indicate payload: \r\n\t", __func__);
    for (uint32_t i = 0U; i < payload_len; i++)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%x ", ((uint8_t *)payload)[i]);
    }
    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "\r\n");
#endif

    msgType = (srtm_message_type_t)((uint8_t *)payload)[3];
    if (handle->started)
    {
        if (SRTM_MU_MSG_READY_A == type)
        {
            SRTM_MUEndpoint_NotifyPeerCore(&handle->channel);
        }
        else if (SRTM_MU_MSG_REQ == type)
        {
            if (msgType != SRTM_MessageTypeRequest)
            {
                assert(false);
            }
            handle->index   = index;
            handle->payload = payload;
            assert(handle->channel.core);
            assert(handle->channel.core->dispatcher);
            SRTM_Dispatcher_PostRecvData(handle->channel.core->dispatcher, &handle->channel, payload, payload_len);
        }
    }
    else
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: Get data before channel start\r\n", __func__);
    }
}

static srtm_status_t SRTM_MUEndpoint_Start(srtm_channel_t channel)
{
    srtm_mu_endpoint_t handle = (srtm_mu_endpoint_t)(void *)channel;
    srtm_status_t status      = SRTM_Status_Success;

    assert(handle != NULL);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle->started = true;

    return status;
}

static srtm_status_t SRTM_MUEndpoint_Stop(srtm_channel_t channel)
{
    srtm_mu_endpoint_t handle = (srtm_mu_endpoint_t)(void *)channel;
    srtm_status_t status      = SRTM_Status_Success;

    assert(handle != NULL);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle->started = false;

    return status;
}

static srtm_status_t srtm_mu_sendmessage(srtm_mu_endpoint_t handle, void *data, uint32_t len)
{
    MU_Type *targetMu = handle->config.mu;
    uint32_t index    = handle->index;

    memcpy(handle->payload, data, len);
    /*
     * 0 : index
     * 1 : type
     * 2 : payload address
     * 3 : payload length
     * TR3 will trigger interrupt to peer
     */
    MU_SendMsgNonBlocking(targetMu, 0, index);
    MU_SendMsgNonBlocking(targetMu, 1, SRTM_MU_MSG_RESP); // Response
    MU_SendMsgNonBlocking(targetMu, 2, (uint32_t)handle->payload);
    MU_SendMsgNonBlocking(targetMu, 3, len);

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_MUEndpoint_SendData(srtm_channel_t channel, void *data, uint32_t len)
{
    srtm_mu_endpoint_t handle = (srtm_mu_endpoint_t)(void *)channel;
    srtm_status_t status      = SRTM_Status_InvalidState;

    assert(handle != NULL);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_DEBUG, "%s: len %d\r\n", __func__, len);

    if (handle->started)
    {
#if SRTM_DEBUG_COMMUNICATION
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: MU send: \r\n\t", __func__);
        for (uint32_t i = 0; i < len; i++)
        {
            SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%x ", ((uint8_t *)data)[i]);
        }
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "\r\n");
#endif
        if (SRTM_Status_Success == srtm_mu_sendmessage(handle, data, len))
        {
            status = SRTM_Status_Success;
        }
        else
        {
            SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: MU send failed\r\n", __func__);
            status = SRTM_Status_TransferFailed;
        }
    }
    else
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: channel not started\r\n", __func__);
    }

    return status;
}

srtm_channel_t SRTM_MUEndpoint_Create(srtm_mu_endpoint_config_t *config)
{
    assert(config != NULL);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_mu_endpoint_t)SRTM_Heap_Malloc(sizeof(struct _srtm_mu_endpoint));
    assert(handle != NULL);

    handle->started = false;
    handle->payload = NULL;

    memcpy(&handle->config, config, sizeof(struct _srtm_mu_endpoint_config));

    SRTM_List_Init(&handle->channel.node);
    handle->channel.core     = NULL;
    handle->channel.destroy  = SRTM_MUEndpoint_Destroy;
    handle->channel.start    = SRTM_MUEndpoint_Start;
    handle->channel.stop     = SRTM_MUEndpoint_Stop;
    handle->channel.sendData = SRTM_MUEndpoint_SendData;

    /*
     * Hardware Initialization
     */
    MU_Init(handle->config.mu);
    MU_EnableInterrupts(handle->config.mu, (1U << 27U) >> SRTM_MU_IRQ_CHANNEL);
    NVIC_EnableIRQ(GET_IRQSTEER_MASTER_IRQn(handle->config.mu_nvic_irq));

    return &handle->channel;
}

void SRTM_MUEndpoint_Destroy(srtm_channel_t channel)
{
    srtm_mu_endpoint_t handle = (srtm_mu_endpoint_t)(void *)channel;

    assert(channel != NULL);
    assert(channel->core == NULL);             /* Channel must be removed from core before destroy */
    assert(SRTM_List_IsEmpty(&channel->node)); /* Channel must not on any list */

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    /*
     * Hardware Deinitialization
     */
    NVIC_DisableIRQ(GET_IRQSTEER_MASTER_IRQn(handle->config.mu_nvic_irq));
    MU_Deinit(handle->config.mu);

    SRTM_Heap_Free(handle);
}
