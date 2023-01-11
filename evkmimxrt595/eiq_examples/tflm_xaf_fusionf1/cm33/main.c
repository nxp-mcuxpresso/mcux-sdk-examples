/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board_init.h"
#include "dsp_support.h"
#include "message.h"
#include "rpmsg_lite.h"
#include "rpmsg_ns.h"
#include "rpmsg_env_specific.h"
#include "fsl_debug_console.h"

/**
 * @brief dsp structure used for inter-processor communication
 */
typedef struct _dsp_ipc
{
    /* Core communication with the DSP based on RPMsg */
    volatile unsigned long remote_addr;
    struct rpmsg_lite_endpoint *local_ept;
    struct rpmsg_lite_instance *local_rpmsg;
    bool msg_received;
    message_t msg;
} dsp_ipc_t;

const char* kCategoryLabels[] = {
    "silence",
    "unknown",
    "yes",
    "no",
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

static volatile dsp_ipc_t dsp_ipc;

/*******************************************************************************
 * Code
 ******************************************************************************/

static int32_t rpmsg_callback(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    if (payload_len == sizeof(message_t))
    {
        dsp_ipc.msg_received = 1;
        memcpy((void*)&dsp_ipc.msg, payload, payload_len);
    }

    return 0;
}

void dsp_ipc_init()
{
    static struct rpmsg_lite_instance context;
    static struct rpmsg_lite_ept_static_context ept_context;

    dsp_ipc.local_rpmsg =
        rpmsg_lite_master_init((void *)RPMSG_LITE_SHMEM_BASE, RPMSG_LITE_SHMEM_SIZE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS, &context);
    dsp_ipc.local_ept   = rpmsg_lite_create_ept(dsp_ipc.local_rpmsg, MCU_EPT_ADDR, &rpmsg_callback, NULL, &ept_context);
    dsp_ipc.remote_addr = DSP_EPT_ADDR;
}

void dsp_ipc_deinit()
{
    rpmsg_lite_destroy_ept(dsp_ipc.local_rpmsg, dsp_ipc.local_ept);
    dsp_ipc.local_ept = NULL;
    rpmsg_lite_deinit(dsp_ipc.local_rpmsg);
}

void dsp_ipc_send_sync(message_t *msg)
{
    rpmsg_lite_send(dsp_ipc.local_rpmsg, dsp_ipc.local_ept, dsp_ipc.remote_addr, (char *)msg, sizeof(message_t), RL_BLOCK);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialize standard SDK demo application pins */
    BOARD_Init();

    /* Print the initial banner */
    PRINTF("\r\nStarting Xtensa example from Cortex-M33 core\r\n");

    dsp_ipc_init();

    /* Copy DSP image to RAM and start DSP core. */
    BOARD_DSP_Init();

    while (true)
    {
        /* Blocking receive IPC message from DSP. */
        if (dsp_ipc.msg_received)
        {
            dsp_ipc.msg_received = 0;
            int is_new_command = dsp_ipc.msg.params[0];
            int category = dsp_ipc.msg.params[1];
            int score = dsp_ipc.msg.params[2];

            if (is_new_command)
            {
                PRINTF("Detected: %s (%d%%)\r\n", kCategoryLabels[category], (score * 100) / 256);
            }
        }
    }
}
