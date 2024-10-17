/** @file ncp_intf_usb_host_cdc.h
 *
 *  @brief main file
 *
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#ifndef _NCP_INTF_USB_HOST_CDC_H_
#define _NCP_INTF_USB_HOST_CDC_H_

#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_host_cdc.h"
#include "ncp_usb_host_cdc.h"
#include "ncp_tlv_adapter.h"
   
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
int ncp_usb_host_send(uint8_t *data, size_t data_len, tlv_send_callback_t cb);
int ncp_usb_host_init(void* argv);
int ncp_usb_host_deinit(void* argv);
void ncp_usb_get_rx_sem(void);
void ncp_usb_host_recv_cb(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status);
void ncp_usb_check_bus(void);

#endif
