/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _NCP_BLE_H_
#define _NCP_BLE_H_

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <porting.h>

#include <bluetooth/bluetooth.h>

#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
extern bool is_create_conn_cmd;

/*******************************************************************************
 * API
 ******************************************************************************/
void ncp_ble_task(void *pvParameters);

uint8_t bt_init(void);
int ble_ncp_init(void);

int ble_ncp_send_response(uint8_t *pbuf);

/*******************************************************************************
 * Code
 ******************************************************************************/
static inline void ncp_ble_set_bit(uint8_t *addr, unsigned int bit)
{
    uint8_t *p = addr + (bit / 8U);

    *p |= BIT(bit % 8);
}

static inline uint8_t ncp_ble_test_bit(const uint8_t *addr, unsigned int bit)
{
    const uint8_t *p = addr + (bit / 8U);

    return *p & BIT(bit % 8);
}

#ifdef __cplusplus
}
#endif

#endif //_NCP_BLE_H_


