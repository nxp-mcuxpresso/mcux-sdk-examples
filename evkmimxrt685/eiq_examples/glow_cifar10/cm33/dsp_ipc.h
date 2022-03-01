/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DSP_IPC_H__
#define __DSP_IPC_H__

#include "srtm_config.h"
#include "dsp_config.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief Initializes hifi
 */
void BOARD_DSP_IPC_Init();

/*!
 * @brief Initializes hifi's asynchronous primitives
 *
 * @param max_length       maximum length of the queue of jobs
 */
void BOARD_DSP_IPCAsync_Init(int max_length);

/*!
 * @brief Deinitializes hifi
 */
void BOARD_DSP_IPC_Deinit();

/*!
 * @brief Deinitializes hifi's asynchronous primitives
 */
void BOARD_DSP_IPCAsync_Deinit();

/*!
 * @brief Send message to hifi (synchronous mode)
 *
 * @param msg           pointer to the message to be send
 */
void dsp_ipc_send_sync(srtm_message *msg);

/*!
 * @brief Receive message from hifi (synchronous mode)
 *
 * @param msg           pointer to the message to be filled
 */
void dsp_ipc_recv_sync(srtm_message *msg);

/*!
 * @brief Send message to hifi (asynchronous mode)
 * This function puts the message to be sent to hifi in a queue. A worker task
 * will then take the message and send it to the hifi. After it is processed by
 * the hifi, the assigned callback will be called.
 *
 * @param msg           pointer to the message to be send
 */
void dsp_ipc_send_async(srtm_message_async *msg);

/*!
 * @brief Worker task which processes the tasks from the waiting queue
 * This function gets the message to be sent to hifi from the queue. and sends it to the dsp.
 * After it is processed by the dsp, the assigned callback will be called.
 */
void dsp_ipc_queue_worker_task();

#endif // __DSP_IPC_H__
