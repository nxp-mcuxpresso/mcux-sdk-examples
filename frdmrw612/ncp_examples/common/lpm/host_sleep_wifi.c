/** @file host_sleep_wifi.c
 *
 *  @brief Host sleep WIFI file
 *
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#if CONFIG_HOST_SLEEP
#if CONFIG_NCP_WIFI

#include "host_sleep_wifi.h"
#include "wlan.h"
#include "wifi.h"
#include "fsl_power.h"
#include "ncp_cmd_wifi.h"

extern int wakeup_by;
extern int wakeup_reason;

int ncp_wifi_lowpower_allowed(void)
{
   return wakelock_isheld();
}

void ncp_enable_wlan_wakeup(bool enable)
{
    if(enable == true)
    {
        POWER_ClearWakeupStatus(WL_MCI_WAKEUP0_IRQn);
        POWER_EnableWakeup(WL_MCI_WAKEUP0_IRQn);
    }
    else
        POWER_ClearWakeupStatus(WL_MCI_WAKEUP0_IRQn);
}

void ncp_print_wlan_wakeup(void)
{
    wifi_print_wakeup_reason(0);
}

int ncp_cancel_wlan_wakeup(void)
{
    if (wlan_is_started())
    {
        if(wlan_hs_send_event(HOST_SLEEP_EXIT, NULL) != 0)
            return -1;
    }
    return 0;
}

void ncp_clear_wlan_wakeup(void)
{
    wifi_clear_wakeup_reason();
    wakeup_by = 0;
}

void ncp_check_wlan_wakeup(void)
{
    if (wakeup_by == WAKEUP_BY_WLAN)
        wakeup_reason = WAKEUP_BY_WLAN;
}

#endif /* CONFIG_NCP_WIFI */
#endif /* CONFIG_HOST_SLEEP */
