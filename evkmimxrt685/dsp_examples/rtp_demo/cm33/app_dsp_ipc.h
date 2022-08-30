/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_DSP_IPC_H__
#define __APP_DSP_IPC_H__

#include "main_cm33.h"
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void app_dsp_ipc_init(app_handle_t *app);

void app_dsp_ipc_packet_ready(uint8_t *buffer, size_t size);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __APP_DSP_IPC_H__ */
