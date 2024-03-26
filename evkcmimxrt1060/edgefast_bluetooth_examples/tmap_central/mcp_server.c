/** @file
 *  @brief Bluetooth Media Control Profile (MCP) Server role.
 *
 *  Copyright 2023 NXP
 *
 *  SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <stdio.h>

#include <bluetooth/conn.h>
#include <bluetooth/audio/media_proxy.h>

int mcp_server_init(void)
{
	int err;

	err = media_proxy_pl_init();

	return err;
}
