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

void coex_cli_init(void);
int pollChar();
void coex_menuPrint(void);
void coex_menuAction(int ch);

int wlan_event_callback(enum wlan_event_reason reason, void *data);
#endif
