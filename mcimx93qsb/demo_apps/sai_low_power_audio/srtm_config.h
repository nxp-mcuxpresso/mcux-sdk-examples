/*
 * Copyright 2023 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SRTM_CONFIG_H
#define _SRTM_CONFIG_H

#ifdef SDK_OS_FREE_RTOS
#include "FreeRTOS.h"
#include "rpmsg_config.h"
#include "fsl_debug_console.h"

#ifdef __cplusplus
extern "C" {
#endif
#if configSUPPORT_STATIC_ALLOCATION
#define SRTM_STATIC_API 1
typedef StaticSemaphore_t srtm_sem_buf_t;
typedef StaticSemaphore_t srtm_mutex_buf_t;
#endif

#if !(defined(RL_ALLOW_CUSTOM_SHMEM_CONFIG) && (RL_ALLOW_CUSTOM_SHMEM_CONFIG == 1))
#define SRTM_DISPATCHER_MSG_MAX_LEN (RL_BUFFER_PAYLOAD_SIZE)
#else
#define SRTM_DISPATCHER_MSG_MAX_LEN (RL_BUFFER_PAYLOAD_SIZE(0))
#endif
#define SRTM_DISPATCHER_CONFIG_RX_MSG_MAX_LEN (SRTM_DISPATCHER_MSG_MAX_LEN)

/* debug macros */
/*
 * If want to debug SRTM, should define SRTM_DEBUG_MESSAGE_FUNC to proper printf function,
 * also need define SRTM_DEBUG_VERBOSE_LEVEL.
 *
 * Debug levels as follows(components/srtm/include/srtm_defs.h):
 * SRTM_DEBUG_VERBOSE_NONE
 * SRTM_DEBUG_VERBOSE_ERROR
 * SRTM_DEBUG_VERBOSE_WARN
 * SRTM_DEBUG_VERBOSE_INFO
 * SRTM_DEBUG_VERBOSE_DEBUG
 */
#define SRTM_DEBUG_MESSAGE_FUNC  DbgConsole_Printf
#define SRTM_DEBUG_VERBOSE_LEVEL SRTM_DEBUG_VERBOSE_WARN

#define SRTM_DEBUG_COMMUNICATION (0)

/* Switch to disable XXXXX service debugging messages. */
#define SRTM_I2C_SERVICE_DEBUG_OFF    (0)
#define SRTM_AUDIO_SERVICE_DEBUG_OFF  (0)
#define SRTM_IO_SERVICE_DEBUG_OFF     (0)
#define SRTM_KEYPAD_SERVICE_DEBUG_OFF (0)
#define SRTM_LFCL_SERVICE_DEBUG_OFF   (0)
#define SRTM_PWM_SERVICE_DEBUG_OFF    (0)
#define SRTM_RTC_SERVICE_DEBUG_OFF    (0)
#define SRTM_SENSOR_SERVICE_DEBUG_OFF (0)
#define SRTM_AUDIO_SERVICE_DEBUG_OFF  (0)
#define SRTM_PMIC_SERVICE_DEBUG_OFF   (0)

#define SRTM_SAI_EDMA_LOCAL_BUF_ENABLE (1)

#endif /* SDK_OS_FREE_RTOS */

#ifdef __cplusplus
}
#endif
#endif /* _SRTM_CONFIG_H */
