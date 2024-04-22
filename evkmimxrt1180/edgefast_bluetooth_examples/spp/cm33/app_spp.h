/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APP_SPP_H__
#define __APP_SPP_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
/** @brief Init spp appl
 *
 *  Init spp appl.
 *
 */
void spp_appl_init(void);

/** @brief Print spp appl handle info
 *
 *  Print all active spp appl handle info.
 *
 */
void spp_appl_handle_info(void);

/** @brief Select spp appl handle
 *
 *  Select spp sppl handle.
 *
 *  @param handle Spp appl handle wanted to be selected.
 */
void spp_appl_handle_select(uint8_t handle);

/** @brief SPP appl server channel register
 *
 *  Register spp server channel.
 *
 *  @param channel Server channel wanted to be registered.
 */
void spp_appl_server_register(uint8_t channel);

/** @brief SPP appl connect
 *
 *  Conn spp appl.
 *
 *  @param conn    BR conn.
 *  @param channel Channel wanted to be connect.
 */
void spp_appl_connect(struct bt_conn *conn, uint8_t channel);

/** @brief SPP appl disconnect
 *
 *  Disconnect current active spp appl.
 *
 */
void spp_appl_disconnect(void);

/** @brief SPP appl send data
 *
 *  Send spp appl data.
 *
 *  @param index String index needed to be sent.
 *
 *  Notice:
 *  If index is 1, AT+CIND=?\\r\n\ will be sent.
 *  If index is 2, AT+CIND?\\r\n\  will be sent.
 *  If index is 3, ATEP\\r\n\      will be sent.
 *  If index is 4, AT+CKPD=E\\r\n\ will be sent.
 */
void spp_appl_send(uint8_t index);

/** @brief Get SPP Server Port Setting
 *
 *  This API enables the user to get current device's port setting in server channel.
 *
 *  @param channel Channel.
 *
 *  Notice: This API can only be called after a spp connection is created.
 */
void spp_appl_get_server_port(uint8_t channel);

/** @brief Get SPP Client Port Setting
 *
 *  This API enables the user to get current device's port setting in client channel.
 *
 *  @param channel Channel.
 *
 *  Notice: This API can be called before or after a spp connection is created.
 */
void spp_appl_get_client_port(uint8_t channel);

/** @brief Set SPP Server Port Setting
 *
 *  This API enables the user to set current device's port setting in server channel.
 *
 *  @param channel Channel.
 *
 *  Notice: This API can only be called after a spp connection is created.
 */
void spp_appl_set_server_port(uint8_t channel);

/** @brief Set SPP Client Port Setting
 *
 *  This API enables the user to set current device's port setting in client channel.
 *
 *  @param channel Channel.
 *
 *  Notice: This API can be called before or after a spp connection is created.
 */
void spp_appl_set_client_port(uint8_t channel);

/** @brief Send Local SPP Parameter in Server
 *
 *  This API enables the user to send current device's parameter in server channel.
 *
 *  @param channel Channel.
 *
 */
void spp_appl_set_server_pn(uint8_t channel);

/** @brief Send Local SPP Parameter in Client
 *
 *  This API enables the user to send current device's parameter in client channel.
 *
 *  @param channel Channel.
 *
 */
void spp_appl_set_client_pn(uint8_t channel);

/** @brief Get Local SPP Server Parameter
 *
 *  This API enables the user to get current device's parameter in server channel.
 *
 *  @param channel Channel.
 *
 *  Notice: This API can only be called after a spp connection is created.
 */
void spp_appl_get_local_server_pn(uint8_t channel);

/** @brief Get Local SPP Client Parameter
 *
 *  This API enables the user to get current device's parameter in client channel.
 *
 *  @param channel Channel.
 *
 *  Notice: This API can only be called after a spp connection is created.
 */
void spp_appl_get_local_client_pn(uint8_t channel);

/** @brief Send Line Status
 *
 *  This API enables the user to send current device's line status.
 *
 */
void spp_appl_send_rls(void);

/** @brief Send Modum Status
 *
 *  This API enables the user to send current device's modum status.
 *
 */
void spp_appl_send_msc(void);
#endif /* __APP_SPP_H__ */