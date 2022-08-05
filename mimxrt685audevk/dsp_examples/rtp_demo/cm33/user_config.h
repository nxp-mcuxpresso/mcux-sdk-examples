/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "lwip/debug.h"
#include "task.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Wi-Fi network to join.
 * Set this to your network name.
 */
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif /* WIFI_SSID */

/*!
 * @brief Password needed to join Wi-Fi network.
 * If you are using WPA, set this to your network password.
 */
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif /* WIFI_PASSWORD */

/*!
 * @brief RTP stream multicast address as IPv4 address string.
 * Set to NULL if you want to receive unicast only.
 */
#ifndef RTP_RECV_MCAST_ADDRESS
#define RTP_RECV_MCAST_ADDRESS "232.0.232.232"
#endif /* RTP_RECV_MCAST_ADDRESS */

/*!
 * @brief UDP port number to listen to incoming RTP stream on.
 */
#ifndef RTP_RECV_PORT
#define RTP_RECV_PORT 5004
#endif /* RTP_RECV_PORT */

/*!
 * @brief RTP_DEBUG: Enable lwIP debugging for RTP.
 * Set to LWIP_DBG_ON or LWIP_DBG_OFF.
 */
#ifndef RTP_DEBUG
#define RTP_DEBUG LWIP_DBG_OFF
#endif /* RTP_DEBUG */

/*!
 * @brief DSP IPC task stack size in bytes.
 */
#ifndef APP_DSP_IPC_TASK_STACK_SIZE
#define APP_DSP_IPC_TASK_STACK_SIZE (2 * 1024)
#endif /* APP_DSP_IPC_TASK_STACK_SIZE */

/*!
 * @brief DSP IPC task priority.
 */
#ifndef APP_DSP_IPC_TASK_PRIORITY
#define APP_DSP_IPC_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#endif /* APP_DSP_IPC_TASK_PRIORITY */

/*!
 * @brief Wi-Fi client task stack size in bytes.
 */
#ifndef WIFI_CLIENT_TASK_STACK_SIZE
#define WIFI_CLIENT_TASK_STACK_SIZE (2 * 1024)
#endif /* WIFI_CLIENT_TASK_STACK_SIZE */

/*!
 * @brief Wi-Fi client task priority.
 */
#ifndef WIFI_CLIENT_TASK_PRIORITY
#define WIFI_CLIENT_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#endif /* WIFI_CLIENT_TASK_PRIORITY */

/*!
 * @brief RTP receiver task stack size in bytes.
 */
#ifndef RTP_RECEIVER_TASK_STACK_SIZE
#define RTP_RECEIVER_TASK_STACK_SIZE (2 * 1024)
#endif /* RTP_RECEIVER_TASK_STACK_SIZE */

/*!
 * @brief RTP receiver task priority.
 */
#ifndef RTP_RECEIVER_TASK_PRIORITY
#define RTP_RECEIVER_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#endif /* RTP_RECEIVER_TASK_PRIORITY */

#endif /* __USER_CONFIG_H__ */
