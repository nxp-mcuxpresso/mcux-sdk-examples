/*
 * Copyright 2021 NXP
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef INCLUDE_BLUETOOTH_HTC_H_
#define INCLUDE_BLUETOOTH_HTC_H_
/**
 * @brief Health Thermometer (HTC)
 * @defgroup bt_htc Health Thermometer (HTC)
 * @ingroup bluetooth
 * @{
 *
 * [Experimental] Users should note that the APIs can change
 * as a part of ongoing development.
 */

#ifdef __cplusplus
extern "C" {
#endif


void central_htc_connect(void *args);

void central_htc_disconnect(void *args);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* INCLUDE_BLUETOOTH_HTC_H_ */