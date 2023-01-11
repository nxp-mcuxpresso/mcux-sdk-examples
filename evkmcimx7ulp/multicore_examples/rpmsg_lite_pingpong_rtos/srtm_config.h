/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SRTM_CONFIG_H
#define _SRTM_CONFIG_H

#ifdef SDK_OS_FREE_RTOS
#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif
#if configSUPPORT_STATIC_ALLOCATION
#define SRTM_STATIC_API 1
typedef StaticSemaphore_t srtm_sem_buf_t;
typedef StaticSemaphore_t srtm_mutex_buf_t;
#endif

#endif /* SDK_OS_FREE_RTOS */

#ifdef __cplusplus
}
#endif
#endif /* _SRTM_CONFIG_H */
