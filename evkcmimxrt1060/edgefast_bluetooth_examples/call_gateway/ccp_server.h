/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CCP_SERVER_H__
#define __CCP_SERVER_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
int call_server_init(shell_handle_t shellHandle);

int call_server_enable_command(void);
int call_server_disable_command(void);

#endif /* __CCP_SERVER_H__*/