/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "wlan.h"

int wifi_is_ready(void)
{
    int err;
    enum wlan_connection_state wifiState;

    err = wlan_get_connection_state(&wifiState);

    (void)wifiState;
    return (WM_SUCCESS == err) ? 0 : -1;
}

