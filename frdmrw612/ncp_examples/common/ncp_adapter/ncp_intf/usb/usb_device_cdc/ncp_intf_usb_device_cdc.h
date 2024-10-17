/** @file ncp_intf_usb_device_cdc.h
 *
 *  @brief main file
 *
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#ifndef _NCP_INTF_USB_DEVICE_CDC_H_
#define _NCP_INTF_USB_DEVICE_CDC_H_

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "usb_device_cdc_app.h"
#include "usb_device_config.h"
#include "usb_device_cdc_acm.h"
#include "ncp_tlv_adapter.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
void ncp_usb_device_recv(uint8_t *recv_data, uint32_t packet_len);
int ncp_usb_device_send(uint8_t *data, size_t data_len, tlv_send_callback_t cb);
int ncp_usb_device_init();
void ncp_usb_put_tx_sem(void);
#endif
