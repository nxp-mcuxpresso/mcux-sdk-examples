/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_HRS_H_
#define ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_HRS_H_

#include <stddef.h>

/**
 * @brief Heart Rate Service (HRS)
 * @defgroup bt_hrs Heart Rate Service (HRS)
 * @ingroup bluetooth
 * @{
 *
 * [Experimental] Users should note that the APIs can change
 * as a part of ongoing development.
 */

#ifdef __cplusplus
extern "C" {
#endif

// register hrs service
void init_hrs_service(void);

int bt_hrs_notify(uint16_t heartrate);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_HRS_H_ */
