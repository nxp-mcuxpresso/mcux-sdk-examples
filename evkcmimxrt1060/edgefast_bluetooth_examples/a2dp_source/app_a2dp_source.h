/*
 * Copyright 2020 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_A2DP_SOURCE_H__
#define __APP_A2DP_SOURCE_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

void app_a2dp_source_task(void *param);

void app_sdp_discover_a2dp_sink(void);

#endif /* __APP_A2DP_SOURCE_H__ */