/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_GLUE_SYSTEM_H__
#define __NCP_GLUE_SYSTEM_H__

#include "ncp_cmd_system.h"

uint8_t *ncp_sys_evt_status(uint32_t evt_id, void *msg);
void ncp_get_sys_resp_buf_lock();
void ncp_put_sys_resp_buf_lock();

#endif /* __NCP_GLUE_SYSTEM_H__ */
