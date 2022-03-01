/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "dsp_ipc.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "rpmsg_lite.h"
#include "rpmsg_ns.h"
#include "rpmsg_queue.h"
#include "srtm_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/**
 * @brief dsp structure used for inter processor communication
 */
typedef struct _dsp_ipc
{
    /* Core communication with the DSP based on RPMSG*/
    volatile unsigned long remote_addr;
    struct rpmsg_lite_endpoint *my_ept;
    rpmsg_queue_handle my_queue;
    struct rpmsg_lite_instance *my_rpmsg;

    /* Mutex used synchronize the communication with the DSP */
    SemaphoreHandle_t mutex;

    /* Used to enqueue the messages for the DSP */
    QueueHandle_t queue;
} dsp_ipc_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
static dsp_ipc_t dsp_ipc;

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_DSP_IPC_Init()
{
    dsp_ipc.my_rpmsg =
        rpmsg_lite_master_init((void *)RPMSG_LITE_SHMEM_BASE, RPMSG_LITE_SHMEM_SIZE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS);
    dsp_ipc.my_queue    = rpmsg_queue_create(dsp_ipc.my_rpmsg);
    dsp_ipc.my_ept      = rpmsg_lite_create_ept(dsp_ipc.my_rpmsg, MCU_EPT_ADDR, rpmsg_queue_rx_cb, dsp_ipc.my_queue);
    dsp_ipc.remote_addr = DSP_EPT_ADDR;
}

void BOARD_DSP_IPCAsync_Init(int max_length)
{
    dsp_ipc.mutex = xSemaphoreCreateMutex();
    dsp_ipc.queue = xQueueCreate(max_length, sizeof(srtm_message_async));
    vQueueAddToRegistry(dsp_ipc.queue, "dsp_ipc.queue");
}

void BOARD_DSP_IPC_Deinit()
{
    rpmsg_lite_destroy_ept(dsp_ipc.my_rpmsg, dsp_ipc.my_ept);
    dsp_ipc.my_ept = NULL;
    rpmsg_queue_destroy(dsp_ipc.my_rpmsg, dsp_ipc.my_queue);
    dsp_ipc.my_queue = NULL;
    rpmsg_lite_deinit(dsp_ipc.my_rpmsg);
}

void BOARD_DSP_IPCAsync_Deinit()
{
    BOARD_DSP_IPC_Deinit();
    vQueueUnregisterQueue(dsp_ipc.queue);
    vQueueDelete(dsp_ipc.queue);
    vSemaphoreDelete(dsp_ipc.mutex);
}

static void inline dsp_ipc_lock()
{
    (void)xSemaphoreTake(dsp_ipc.mutex, portMAX_DELAY);
}

static void inline dsp_ipc_unlock()
{
    xSemaphoreGive(dsp_ipc.mutex);
}

void inline dsp_ipc_send_sync(srtm_message *msg)
{
    rpmsg_lite_send(dsp_ipc.my_rpmsg, dsp_ipc.my_ept, dsp_ipc.remote_addr, (char *)msg, sizeof(srtm_message), RL_BLOCK);
}

void inline dsp_ipc_recv_sync(srtm_message *msg)
{
    rpmsg_queue_recv(dsp_ipc.my_rpmsg, dsp_ipc.my_queue, (uint32_t *)&(dsp_ipc.remote_addr), (char *)msg,
                     sizeof(srtm_message), RL_NULL, RL_BLOCK);
}

void inline dsp_ipc_send_async(srtm_message_async *msg)
{
    xQueueSend(dsp_ipc.queue, msg, portMAX_DELAY);
}

void dsp_ipc_queue_worker_task()
{
    srtm_message_async msg_async;
    BaseType_t rc;

    while (1)
    {
        rc = xQueueReceive(dsp_ipc.queue, &msg_async, portMAX_DELAY);
        if (rc == pdTRUE)
        {
            dsp_ipc_lock();
            dsp_ipc_send_sync((srtm_message *)&msg_async);
            dsp_ipc_recv_sync((srtm_message *)&msg_async);
            dsp_ipc_unlock();
            msg_async.cb(msg_async.params, (srtm_message *)&msg_async);
        }
    }
}
