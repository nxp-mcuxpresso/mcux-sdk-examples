/* eddystone.h - eddystone sample */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#if defined(EDDYSTONE) && (EDDYSTONE == 1)

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
void beacon_task(void *pvParameters);
#ifdef __cplusplus
}
#endif

#endif /* EDDYSTONE */
