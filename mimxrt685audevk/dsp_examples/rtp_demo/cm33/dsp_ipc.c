/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rpmsg_lite.h"
#include "dsp_ipc.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "rpmsg_ns.h"
#include "rpmsg_queue.h"
#include "message.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @brief dsp structure used for inter processor communication
 */
typedef struct _dsp_ipc
{
    /* Core communication with the DSP based on RPMsg */
    volatile unsigned long remote_addr;
    struct rpmsg_lite_endpoint *my_ept;
    rpmsg_queue_handle my_queue;
    struct rpmsg_lite_instance *my_rpmsg;
} dsp_ipc_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

static dsp_ipc_t dsp_ipc;

/*******************************************************************************
 * Code
 ******************************************************************************/

void dsp_ipc_init()
{
    dsp_ipc.my_rpmsg =
        rpmsg_lite_master_init((void *)RPMSG_LITE_SHMEM_BASE, RPMSG_LITE_SHMEM_SIZE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS);
    dsp_ipc.my_queue    = rpmsg_queue_create(dsp_ipc.my_rpmsg);
    dsp_ipc.my_ept      = rpmsg_lite_create_ept(dsp_ipc.my_rpmsg, MCU_EPT_ADDR, rpmsg_queue_rx_cb, dsp_ipc.my_queue);
    dsp_ipc.remote_addr = DSP_EPT_ADDR;
}

void dsp_ipc_deinit()
{
    rpmsg_lite_destroy_ept(dsp_ipc.my_rpmsg, dsp_ipc.my_ept);
    dsp_ipc.my_ept = NULL;
    rpmsg_queue_destroy(dsp_ipc.my_rpmsg, dsp_ipc.my_queue);
    dsp_ipc.my_queue = NULL;
    rpmsg_lite_deinit(dsp_ipc.my_rpmsg);
}

void dsp_ipc_send_sync(message_t *msg)
{
    rpmsg_lite_send(dsp_ipc.my_rpmsg, dsp_ipc.my_ept, dsp_ipc.remote_addr, (char *)msg, sizeof(message_t), RL_BLOCK);
}

void inline dsp_ipc_recv_sync(message_t *msg)
{
    rpmsg_queue_recv(dsp_ipc.my_rpmsg, dsp_ipc.my_queue, (uint32_t *)&(dsp_ipc.remote_addr), (char *)msg,
                     sizeof(message_t), RL_NULL, RL_BLOCK);
}
