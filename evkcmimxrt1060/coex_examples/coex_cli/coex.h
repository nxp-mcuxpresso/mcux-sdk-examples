/**
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

#ifndef COEX_H
#define COEX_H
#define WLAN_CMD_MAX_LEN 512
#define PROFILE_NAME_LEN 30
#define ARGS_SIZE        20

#include "dhcp-server.h"
#include <wm_net.h>
#include "cli.h"
#include "wifi_ping.h"
#include "iperf.h"
#include "controller.h"
#include "fsl_adapter_uart.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

int coex_cli_init(void);
int pollChar(void);
void coex_menuPrint(void);
void coex_menuAction(int ch);
void coex_controller_init(void);

int wlan_event_callback(enum wlan_event_reason reason, void *data);
#endif
