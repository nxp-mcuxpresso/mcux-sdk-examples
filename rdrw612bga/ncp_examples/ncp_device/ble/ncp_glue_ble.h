/*
 * Copyright (c) 2015-2016 Intel Corporation
 * Copyright (c) 2022 Codecoup
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __NCP_GLUE_BLE_H__
#define __NCP_GLUE_BLE_H__

#include "ncp_cmd_ble.h"

/** Prepare TLV command response */
int ble_bridge_prepare_status(uint32_t cmd,
    uint8_t status,
    uint8_t *data,
    size_t len);

NCPCmd_DS_COMMAND *ncp_bridge_get_ble_response_buffer();

#endif /* __NCP_GLUE_BLE_H__ */
