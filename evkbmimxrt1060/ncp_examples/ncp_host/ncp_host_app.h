/**@file ncp_host_app.h
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _NCP_HOST_APP_H_
#define _NCP_HOST_APP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NCP_HOST_CMD_SIZE_BIT1     4
#define NCP_HOST_CMD_SIZE_BIT2     5
#define NCP_HOST_CMD_SEQUENCE_BIT1 6
#define NCP_HOST_CMD_SEQUENCE_BIT2 7

#define TLV_CMD_BUF          200

int ncp_host_app_init();

int mcu_get_command_resp_sem();

int mcu_put_command_resp_sem();

int mcu_get_command_lock();

int mcu_put_command_lock();

int ncp_host_send_tlv_command();

#endif /*_NCP_HOST_APP_H_*/
