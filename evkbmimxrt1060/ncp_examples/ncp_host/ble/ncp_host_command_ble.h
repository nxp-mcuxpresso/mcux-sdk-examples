/*
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 * 
 */

/*!\file ncp_host_command_ble.h
 *\brief NCP Bluetooth LE host command interfaces.
 */

#if CONFIG_NCP_BLE

#ifndef __NCP_HOST_COMMAND_BLE_H_
#define __NCP_HOST_COMMAND_BLE_H_

#include "ncp_adapter.h"
#include "ncp_host_app.h"
#include "ncp_host_command.h"
#include "ncp_cmd_ble.h"

/**
 * NCP Bluetooth LE process command response
 * 
 * \param[in,out] res A pointer to command response buffer
 * 
 * \return NCP_STATUS_SUCCESS if successful 
 */
int ble_process_response(uint8_t *res);

/**
 * NCP Bluetooth LE process event
 * 
 * \param[in,out] res A pointer to command event buffer
 * 
 * \return NCP_STATUS_SUCCESS if successful 
 */
int ble_process_ncp_event(uint8_t *res);

/**
 * Write Local Characteristic command
 * 
 * \param[in,out] A pointer to NCP_SET_VALUE_CMD
 * 
 * \return void
 * 
 */
void write_charateristic_command_local(NCP_SET_VALUE_CMD *param);

/**
 * 
 * Get NCP Bluetooth LE command buffer
 * 
 * \return A ponter to MCU_NCPCmd_DS_COMMAND
 * 
 */
MCU_NCPCmd_DS_COMMAND *ncp_host_get_cmd_buffer_ble();

#endif /*__NCP_HOST_COMMAND_BLE_H_*/

#endif /* CONFIG_NCP_BLE */