/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DSP_IPC_H__
#define __DSP_IPC_H__

#include "message.h"
#include "dsp_config.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief Initializes IPC
 */
void dsp_ipc_init();

/*!
 * @brief Deinitializes IPC
 */
void dsp_ipc_deinit();

/*!
 * @brief Send message to DSP (synchronous mode)
 *
 * @param msg           pointer to the message to be sent
 */
void dsp_ipc_send_sync(message_t *msg);

/*!
 * @brief Receive message from DSP (synchronous mode)
 *
 * @param msg           pointer to the message to be filled
 */
void dsp_ipc_recv_sync(message_t *msg);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif // __DSP_IPC_H__
