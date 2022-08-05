/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RTP_BUFFER_H__
#define __RTP_BUFFER_H__

#include "main_cm33.h"
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void rtp_buffer_init(app_handle_t *app);

uint8_t *rtp_buffer_get(app_handle_t *app);

void rtp_buffer_put(app_handle_t *app, uint8_t *buffer);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __RTP_BUFFER_H__ */
