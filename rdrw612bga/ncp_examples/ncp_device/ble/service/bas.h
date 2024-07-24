/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_BAS_H_
#define ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_BAS_H_

/**
 * @brief Battery Service (BAS)
 * @defgroup bt_bas Battery Service (BAS)
 * @ingroup bluetooth
 * @{
 *
 * [Experimental] Users should note that the APIs can change
 * as a part of ongoing development.
 */

#ifdef __cplusplus
extern "C" {
#endif
void bas_disconnect(void *args);

void bas_connect(void *args);

/* register battery service */
void peripheral_bas_task(void *args);

void init_bas_service(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_BAS_H_ */
