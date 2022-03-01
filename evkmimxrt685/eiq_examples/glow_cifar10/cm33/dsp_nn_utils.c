/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "dsp_nn_utils.h"
#include "dsp_ipc.h"
#include "srtm_config.h"

unsigned int xa_nn_echo()
{
    srtm_message msg = {0};

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_GENERAL;
    msg.head.command      = SRTM_Command_ECHO;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (unsigned int)(msg.param[6]);
}

void *xa_nn_malloc(unsigned int size)
{
    srtm_message msg      = {0};
    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_GENERAL;
    msg.head.command      = SRTM_Command_xa_nn_malloc;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(size);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (void *)(msg.param[0]);
}

void xa_nn_free(void *p)
{
    srtm_message msg = {0};

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_GENERAL;
    msg.head.command      = SRTM_Command_xa_nn_free;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);
}
