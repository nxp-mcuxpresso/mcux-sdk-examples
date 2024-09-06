/** @file host_sleep_wifi.h
 *
 *  @brief Host sleep wifi file
 *
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#ifndef _HOST_SLEEP_WIFI_H_
#define _HOST_SLEEP_WIFI_H_

#include "host_sleep.h"
#include "wlan.h"

void ncp_enable_wlan_wakeup(bool enable);
void ncp_print_wlan_wakeup(void);
int ncp_cancel_wlan_wakeup(void);
void ncp_clear_wlan_wakeup(void);
void ncp_check_wlan_wakeup(void);

#endif /* _HOST_SLEEP_WIFI_H_ */
