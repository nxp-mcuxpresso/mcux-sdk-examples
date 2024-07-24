/*
 * Copyright (c) 2015-2016 Intel Corporation
 * Copyright (c) 2022 Codecoup
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#if CONFIG_NCP_BLE

#ifndef __NCP_GLUE_BLE_H__
#define __NCP_GLUE_BLE_H__

#include "ncp_cmd_ble.h"

/** Prepare TLV command response */
int ble_prepare_status(uint32_t cmd,
    uint8_t status,
    uint8_t *data,
    size_t len);

NCPCmd_DS_COMMAND *ncp__get_ble_response_buffer();

void ncp_get_ble_resp_buf_lock();
void ncp_put_ble_resp_buf_lock();

#endif /* __NCP_GLUE_BLE_H__ */

#endif
