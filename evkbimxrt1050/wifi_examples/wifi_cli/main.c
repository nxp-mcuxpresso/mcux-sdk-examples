/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2020 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

// SDK Included Files
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "wlan_bt_fw.h"
#include "wlan.h"
#include "wifi.h"
#include "wm_net.h"
#include <wm_os.h>
#include "dhcp-server.h"
#include "cli.h"
#include "wifi_ping.h"
#include "iperf.h"
#ifndef RW610
#include "wifi_bt_config.h"
#else
#include "fsl_power.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"
#include "fsl_rtc.h"
#endif
#include "cli_utils.h"
#ifdef CONFIG_HOST_SLEEP
#include "host_sleep.h"
#endif

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
int wlan_driver_init(void);
int wlan_driver_deinit(void);
int wlan_driver_reset(void);
int wlan_reset_cli_init(void);

/*******************************************************************************
 * Code
 ******************************************************************************/

const int TASK_MAIN_PRIO       = OS_PRIO_3;
#ifdef CONFIG_WPS2
const int TASK_MAIN_STACK_SIZE = 1500;
#else
const int TASK_MAIN_STACK_SIZE = 800;
#endif

portSTACK_TYPE *task_main_stack = NULL;
TaskHandle_t task_main_task_handler;
#ifdef RW610
#ifdef CONFIG_POWER_MANAGER
/* Global power manager handle */
AT_ALWAYS_ON_DATA(pm_handle_t pm_handle);
AT_ALWAYS_ON_DATA(pm_wakeup_source_t wlanWakeupSource);
AT_ALWAYS_ON_DATA(pm_wakeup_source_t rtcWakeupSource);
AT_ALWAYS_ON_DATA(pm_wakeup_source_t pin1WakeupSource);
extern pm_notify_element_t wlan_notify;
extern bool is_wakeup_cond_set;
#define APP_PM2_CONSTRAINTS \
    6U, PM_RESC_SRAM_0K_384K_STANDBY, PM_RESC_SRAM_384K_448K_STANDBY, PM_RESC_SRAM_448K_512K_STANDBY, \
    PM_RESC_SRAM_512K_640K_STANDBY, PM_RESC_SRAM_640K_896K_STANDBY, PM_RESC_SRAM_896K_1216K_STANDBY
#define APP_PM3_CONSTRAINTS \
    6U, PM_RESC_SRAM_0K_384K_RETENTION, PM_RESC_SRAM_384K_448K_RETENTION, PM_RESC_SRAM_448K_512K_RETENTION, \
    PM_RESC_SRAM_512K_640K_RETENTION, PM_RESC_SRAM_640K_896K_RETENTION, PM_RESC_SRAM_896K_1216K_RETENTION
#if defined(configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
#ifndef POWER_MANAGER_RTC_PIN1_PRIORITY
#define POWER_MANAGER_RTC_PIN1_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)
#endif
#else
#ifndef POWER_MANAGER_RTC_PIN1_PRIORITY
#define POWER_MANAGER_RTC_PIN1_PRIORITY (3U)
#endif
#endif
#endif
#endif

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

static struct wlan_network sta_network;
static struct wlan_network uap_network;

/* Callback Function passed to WLAN Connection Manager. The callback function
 * gets called when there are WLAN Events that need to be handled by the
 * application.
 */
int wlan_event_callback(enum wlan_event_reason reason, void *data)
{
    int ret;
    struct wlan_ip_config addr;
    char ip[16];
    static int auth_fail                      = 0;
    wlan_uap_client_disassoc_t *disassoc_resp = data;

    printSeparator();
    PRINTF("app_cb: WLAN: received event %d\r\n", reason);
    printSeparator();

    switch (reason)
    {
        case WLAN_REASON_INITIALIZED:
            PRINTF("app_cb: WLAN initialized\r\n");
            printSeparator();

            ret = wlan_basic_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize BASIC WLAN CLIs\r\n");
                return 0;
            }

            ret = wlan_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize WLAN CLIs\r\n");
                return 0;
            }
            PRINTF("WLAN CLIs are initialized\r\n");
            printSeparator();
#ifdef RW610
            ret = wlan_enhanced_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize WLAN CLIs\r\n");
                return 0;
            }
            PRINTF("ENHANCED WLAN CLIs are initialized\r\n");
            printSeparator();
#endif
            ret = ping_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize PING CLI\r\n");
                return 0;
            }

            ret = iperf_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize IPERF CLI\r\n");
                return 0;
            }

            ret = dhcpd_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize DHCP Server CLI\r\n");
                return 0;
            }

            PRINTF("CLIs Available:\r\n");
            printSeparator();
            help_command(0, NULL);
            printSeparator();
            break;
        case WLAN_REASON_INITIALIZATION_FAILED:
            PRINTF("app_cb: WLAN: initialization failed\r\n");
            break;
        case WLAN_REASON_AUTH_SUCCESS:
            PRINTF("app_cb: WLAN: authenticated to network\r\n");
            break;
        case WLAN_REASON_SUCCESS:
            PRINTF("app_cb: WLAN: connected to network\r\n");
            ret = wlan_get_address(&addr);
            if (ret != WM_SUCCESS)
            {
                PRINTF("failed to get IP address\r\n");
                return 0;
            }

            net_inet_ntoa(addr.ipv4.address, ip);

            ret = wlan_get_current_network(&sta_network);
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to get External AP network\r\n");
                return 0;
            }

            PRINTF("Connected to following BSS:\r\n");
            PRINTF("SSID = [%s]\r\n", sta_network.ssid);
            if (addr.ipv4.address != 0U)
            {
                PRINTF("IPv4 Address: [%s]\r\n", ip);
            }
#ifdef CONFIG_IPV6
            int i;
            for (i = 0; i < CONFIG_MAX_IPV6_ADDRESSES; i++)
            {
                if (ip6_addr_isvalid(addr.ipv6[i].addr_state))
                {
                    (void)PRINTF("IPv6 Address: %-13s:\t%s (%s)\r\n", ipv6_addr_type_to_desc(&addr.ipv6[i]),
                                 inet6_ntoa(addr.ipv6[i].address), ipv6_addr_state_to_desc(addr.ipv6[i].addr_state));
                }
            }
            (void)PRINTF("\r\n");
#endif
            auth_fail = 0;
            break;
        case WLAN_REASON_CONNECT_FAILED:
            PRINTF("app_cb: WLAN: connect failed\r\n");
            break;
        case WLAN_REASON_NETWORK_NOT_FOUND:
            PRINTF("app_cb: WLAN: network not found\r\n");
            break;
        case WLAN_REASON_NETWORK_AUTH_FAILED:
            PRINTF("app_cb: WLAN: network authentication failed\r\n");
            auth_fail++;
            if (auth_fail >= 3)
            {
                PRINTF("Authentication Failed. Disconnecting ... \r\n");
                wlan_disconnect();
                auth_fail = 0;
            }
            break;
        case WLAN_REASON_ADDRESS_SUCCESS:
            PRINTF("network mgr: DHCP new lease\r\n");
            break;
        case WLAN_REASON_ADDRESS_FAILED:
            PRINTF("app_cb: failed to obtain an IP address\r\n");
            break;
        case WLAN_REASON_USER_DISCONNECT:
            PRINTF("app_cb: disconnected\r\n");
            auth_fail = 0;
            break;
        case WLAN_REASON_LINK_LOST:
            PRINTF("app_cb: WLAN: link lost\r\n");
            break;
        case WLAN_REASON_CHAN_SWITCH:
            PRINTF("app_cb: WLAN: channel switch\r\n");
            break;
        case WLAN_REASON_UAP_SUCCESS:
            PRINTF("app_cb: WLAN: UAP Started\r\n");
            ret = wlan_get_current_uap_network(&uap_network);

            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to get Soft AP network\r\n");
                return 0;
            }

            printSeparator();
            PRINTF("Soft AP \"%s\" started successfully\r\n", uap_network.ssid);
            printSeparator();
            if (dhcp_server_start(net_get_uap_handle()))
                PRINTF("Error in starting dhcp server\r\n");

            PRINTF("DHCP Server started successfully\r\n");
            printSeparator();
            break;
        case WLAN_REASON_UAP_CLIENT_ASSOC:
            PRINTF("app_cb: WLAN: UAP a Client Associated\r\n");
            printSeparator();
            PRINTF("Client => ");
            print_mac((const char *)data);
            PRINTF("Associated with Soft AP\r\n");
            printSeparator();
            break;
        case WLAN_REASON_UAP_CLIENT_CONN:
            PRINTF("app_cb: WLAN: UAP a Client Connected\r\n");
            printSeparator();
            PRINTF("Client => ");
            print_mac((const char *)data);
            PRINTF("Connected with Soft AP\r\n");
            printSeparator();
            break;
        case WLAN_REASON_UAP_CLIENT_DISSOC:
            printSeparator();
            PRINTF("app_cb: WLAN: UAP a Client Dissociated:");
            PRINTF(" Client MAC => ");
            print_mac((const char *)(disassoc_resp->sta_addr));
            PRINTF(" Reason code => ");
            PRINTF("%d\r\n", disassoc_resp->reason_code);
            printSeparator();
            break;
        case WLAN_REASON_UAP_STOPPED:
            PRINTF("app_cb: WLAN: UAP Stopped\r\n");
            printSeparator();
            PRINTF("Soft AP \"%s\" stopped successfully\r\n", uap_network.ssid);
            printSeparator();

            dhcp_server_stop();

            PRINTF("DHCP Server stopped successfully\r\n");
            printSeparator();
            break;
        case WLAN_REASON_PS_ENTER:
            PRINTF("app_cb: WLAN: PS_ENTER\r\n");
            break;
        case WLAN_REASON_PS_EXIT:
            PRINTF("app_cb: WLAN: PS EXIT\r\n");
            break;
#ifdef CONFIG_SUBSCRIBE_EVENT_SUPPORT
        case WLAN_REASON_RSSI_HIGH:
        case WLAN_REASON_SNR_LOW:
        case WLAN_REASON_SNR_HIGH:
        case WLAN_REASON_MAX_FAIL:
        case WLAN_REASON_BEACON_MISSED:
        case WLAN_REASON_DATA_RSSI_LOW:
        case WLAN_REASON_DATA_RSSI_HIGH:
        case WLAN_REASON_DATA_SNR_LOW:
        case WLAN_REASON_DATA_SNR_HIGH:
        case WLAN_REASON_LINK_QUALITY:
        case WLAN_REASON_PRE_BEACON_LOST:
            break;
#endif
        default:
            PRINTF("app_cb: WLAN: Unknown Event: %d\r\n", reason);
    }
    return 0;
}

int wlan_driver_init(void)
{
    int result = 0;

    /* Initialize WIFI Driver */
    result = wlan_init(wlan_fw_bin, wlan_fw_bin_len);

    assert(0 == result);

    result = wlan_start(wlan_event_callback);

    assert(0 == result);

    return result;
}

#ifndef RW610
int wlan_driver_deinit(void)
{
    int result = 0;

    result = wlan_stop();
    assert(0 == result);
    wlan_deinit(0);

    return result;
}

static void wlan_hw_reset(void)
{
    BOARD_WIFI_BT_Enable(false);
    os_thread_sleep(1);
    BOARD_WIFI_BT_Enable(true);
}

int wlan_driver_reset(void)
{
    int result = 0;

    result = wlan_driver_deinit();
    assert(0 == result);

    wlan_hw_reset();

    result = wlan_driver_init();
    assert(0 == result);

    return result;
}

static void test_wlan_reset(int argc, char **argv)
{
    (void)wlan_driver_reset();
}

#ifdef CONFIG_HOST_SLEEP
static void test_mcu_system_wait(int argc, char **argv)
{
    (void)lpm_SystemWait();
}
static void test_mcu_low_pwr_idle(int argc, char **argv)
{
    (void)lpm_LowPwrIdle();
}
static void test_mcu_suspend(int argc, char **argv)
{
    (void)lpm_Suspend();
}
static void test_mcu_shutdown(int argc, char **argv)
{
    (void)lpm_Shutdown();
}
#endif

static struct cli_command reset_commands[] = {
    {"wlan-reset", NULL, test_wlan_reset},
#ifdef CONFIG_HOST_SLEEP
    {"mcu-system-wait", NULL, test_mcu_system_wait},
    {"mcu-low-pwr-idle", NULL, test_mcu_low_pwr_idle},
    {"mcu-suspend", NULL, test_mcu_suspend},
    {"mcu-shutdown", NULL, test_mcu_shutdown},
#endif
};

int wlan_reset_cli_init(void)
{
    unsigned int i;

    for (i = 0; i < sizeof(reset_commands) / sizeof(struct cli_command); i++)
    {
        if (cli_register_command(&reset_commands[i]) != 0)
        {
            return -1;
        }
    }

    return 0;
}
#endif

#ifdef RW610
#ifdef CONFIG_POWER_MANAGER
void powerManager_StartRtcTimer(uint64_t timeOutUs)
{
    uint32_t currSeconds;

    PM_EnableWakeupSource(&rtcWakeupSource);
    /* Read the RTC seconds register to get current time in seconds */
    currSeconds = RTC_GetSecondsTimerCount(RTC);
    /* Add alarm seconds to current time */
    currSeconds += (timeOutUs + 999999U) / 1000000U;
    /* Set alarm time in seconds */
    RTC_SetSecondsTimerMatch(RTC, currSeconds);
}

void powerManager_StopRtcTimer()
{
    RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
    PM_DisableWakeupSource(&rtcWakeupSource);
}

void RTC_IRQHandler()
{
    if (RTC_GetStatusFlags(RTC) & kRTC_AlarmFlag)
    {
        RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
        PM_DisableWakeupSource(&rtcWakeupSource);
        wakeup_by = WAKEUP_BY_RTC;
    }
}

void PIN1_INT_IRQHandler()
{
    POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeHigh);
    NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
    DisableIRQ(PIN1_INT_IRQn);
    POWER_ClearWakeupStatus(PIN1_INT_IRQn);
    POWER_DisableWakeup(PIN1_INT_IRQn);
    wakeup_by = WAKEUP_BY_PIN1;
}

void powerManager_RTC_Init()
{
    DisableIRQ(RTC_IRQn);
    POWER_ClearWakeupStatus(RTC_IRQn);
    POWER_DisableWakeup(RTC_IRQn);
    RTC_Init(RTC);
    /* Enable wakeup in PD mode */
    RTC_EnableAlarmTimerInterruptFromDPD(RTC, true);
    /* Start RTC */
    RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
    RTC_StartTimer(RTC);
    /* Register RTC timer callbacks in power manager */
    PM_RegisterTimerController(&pm_handle, powerManager_StartRtcTimer, powerManager_StopRtcTimer, NULL, NULL);
}

void powerManager_Wakeupsource_Init()
{
    memset(&wlanWakeupSource, 0x0, sizeof(pm_wakeup_source_t));
    memset(&rtcWakeupSource, 0x0, sizeof(pm_wakeup_source_t));
    memset(&pin1WakeupSource, 0x0, sizeof(pm_wakeup_source_t));
    /* Init WLAN wakeup source. Power manager API PM_InitWakeupSource()
     * can't be called to init WLAN wakeup source since RW610 use IMU
     * interrupt to wakeup host and can't be disabled here.
     */
    wlanWakeupSource.wsId    = WL_MCI_WAKEUP0_IRQn;
    wlanWakeupSource.service = NULL;
    wlanWakeupSource.enabled = false;
    wlanWakeupSource.active  = false;
    POWER_ClearWakeupStatus(WL_MCI_WAKEUP0_IRQn);
    POWER_DisableWakeup(WL_MCI_WAKEUP0_IRQn);
    /* Init other wakeup sources. Corresponding IRQ numbers act as wsId here. */
    PM_InitWakeupSource(&rtcWakeupSource, RTC_IRQn, NULL, false);
    PM_InitWakeupSource(&pin1WakeupSource, PM_WSID_WAKEUP_PIN1_LOW_LEVEL, NULL, false);
}

void powerManager_WakeupSourceDump()
{
    if(wakeup_by == 0x1)
        PRINTF("Woken up by WLAN\r\n");
    if(wakeup_by == 0x2)
        PRINTF("Woken up by RTC\r\n");
    if(wakeup_by == 0x4)
        PRINTF("Woken up by PIN1\r\n");
}

void powerManager_EnterLowPower()
{
    /* Check is_wakeup_cond_set first, as wakelcok will be deleted in wlan-reset 0 */
    if(is_wakeup_cond_set && pm_handle.enable && !wakelock_isheld())
    {
#ifdef CONFIG_RW610_A1
        PM_SetConstraints(PM_LP_STATE_PM3, APP_PM3_CONSTRAINTS);
#else
        PM_SetConstraints(PM_LP_STATE_PM2, APP_PM2_CONSTRAINTS);
#endif
        /* Enable PIN1 as wakeup sources */
        PM_EnableWakeupSource(&pin1WakeupSource);
        /* duration unit is us here */
        PM_EnterLowPower(60000000);
        powerManager_WakeupSourceDump();
        wakeup_by = 0;
        /* Exit low power and reset constraints */
#ifdef CONFIG_RW610_A1
        PM_ReleaseConstraints(PM_LP_STATE_PM3, APP_PM3_CONSTRAINTS);
#else
        PM_ReleaseConstraints(PM_LP_STATE_PM2, APP_PM2_CONSTRAINTS);
#endif
    }
}

void powerManager_Init()
{
    uint32_t resetSrc;
    power_init_config_t initCfg =
    {
        /* VCORE AVDD18 supplied from iBuck on RD board. */
        .iBuck         = true,
        /* CAU_SOC_SLP_REF_CLK not needed. */
        .gateCauRefClk = true,
    };
    POWER_InitPowerConfig(&initCfg);
    resetSrc = POWER_GetResetCause();
    PRINTF("\r\nMCU wakeup source 0x%x...\r\n", resetSrc);
    /* In case PM3/PM4 wakeup, the wakeup config and status need to be cleared */
    POWER_ClearResetCause(resetSrc);

    PM_CreateHandle(&pm_handle);
    /* Init and start RTC time counter */
    powerManager_RTC_Init();
    /* Set priority of RTC and PIN1 interrupt */
    NVIC_SetPriority(RTC_IRQn, POWER_MANAGER_RTC_PIN1_PRIORITY);
    NVIC_SetPriority(PIN1_INT_IRQn, POWER_MANAGER_RTC_PIN1_PRIORITY);
    /* Register WLAN notifier */
    PM_RegisterNotify(kPM_NotifyGroup0, &wlan_notify);
    /* Init WLAN wakeup source */
    powerManager_Wakeupsource_Init();
    PM_EnablePowerManager(true);
    os_setup_idle_function(powerManager_EnterLowPower);
    wakeup_by = 0;
}
#endif
#endif

void task_main(void *param)
{
    int32_t result = 0;
    (void)result;

    PRINTF("Initialize CLI\r\n");
    printSeparator();

    result = cli_init();

    assert(WM_SUCCESS == result);

#ifdef RW610
#ifdef CONFIG_POWER_MANAGER
    PRINTF("Initialize Power Manager\r\n");
    powerManager_Init();
    printSeparator();
#endif
#endif

    PRINTF("Initialize WLAN Driver\r\n");
    printSeparator();

    /* Initialize WIFI Driver */
    result = wlan_driver_init();

    assert(WM_SUCCESS == result);

#ifndef RW610
    result = wlan_reset_cli_init();

    assert(WM_SUCCESS == result);
#endif

#ifdef CONFIG_HOST_SLEEP
    hostsleep_init();
#endif

    while (1)
    {
        /* wait for interface up */
        os_thread_sleep(os_msec_to_ticks(5000));
    }
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int main(void)
{
    BaseType_t result = 0;
    (void)result;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
#ifdef RW610
    POWER_PowerOffBle();
#endif

    printSeparator();
    PRINTF("wifi cli demo\r\n");
    printSeparator();

    result =
        xTaskCreate(task_main, "main", TASK_MAIN_STACK_SIZE, task_main_stack, TASK_MAIN_PRIO, &task_main_task_handler);
    assert(pdPASS == result);

    vTaskStartScheduler();
    for (;;)
        ;
}
