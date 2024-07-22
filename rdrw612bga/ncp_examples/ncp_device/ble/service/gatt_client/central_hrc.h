/*
 * Copyright 2021 NXP
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef INCLUDE_BLUETOOTH_HRC_H_
#define INCLUDE_BLUETOOTH_HRC_H_
/**
 * @brief Health Rate (HRC)
 * @defgroup bt_hrc Health Rate (HRC)
 * @ingroup bluetooth
 * @{
 *
 * [Experimental] Users should note that the APIs can change
 * as a part of ongoing development.
 */

#ifdef __cplusplus
extern "C" {
#endif


void central_hrc_connect(void *args);

void central_hrc_disconnect(void *args);

bool hrc_adv_report_processed(struct adv_report_data *data, void *user_addr);

void central_hrc_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* INCLUDE_BLUETOOTH_HTC_H_ */