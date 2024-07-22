/**@file ncp_host_command_ble.h
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */
#if CONFIG_NCP_BLE

#ifndef __NCP_HOST_COMMAND_BLE_H_
#define __NCP_HOST_COMMAND_BLE_H_

#include "ncp_adapter.h"
#include "ncp_host_app.h"
#include "ncp_host_command.h"
#include "ncp_cmd_ble.h"

int ble_process_response(uint8_t *res);

int ble_process_ncp_event(uint8_t *res);

void write_charateristic_command_local(NCP_SET_VALUE_CMD *param);

MCU_NCPCmd_DS_COMMAND *ncp_host_get_cmd_buffer_ble();

#endif /*__NCP_HOST_COMMAND_BLE_H_*/

#endif /* CONFIG_NCP_BLE */