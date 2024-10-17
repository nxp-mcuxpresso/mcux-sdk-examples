/*
 * Copyright 2020 - 2021, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_CONNECT_H__
#define __APP_CONNECT_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define AUTO_CONNECT_USE_BOND_INFO (1U)
extern struct bt_conn *default_conn;

/*******************************************************************************
 * API
 ******************************************************************************/

void app_connect_init(void);
void app_a2dp_snk_auto_connect(void);
#if !((defined AUTO_CONNECT_USE_BOND_INFO) && (AUTO_CONNECT_USE_BOND_INFO))
int app_auto_connect_save_addr(bt_addr_t const *addr);
#endif

#endif /* __APP_CONNECT_H__ */