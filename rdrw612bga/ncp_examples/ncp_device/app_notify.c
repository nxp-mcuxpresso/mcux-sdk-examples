/* @file app_notify.c
 *
 *  @brief This file contains declaration of the API functions.
 *
 *  Copyright 2008-2023 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */

#include <wm_os.h>
#include "wlan.h"
#include "wifi.h"

#include "app_notify.h"
#include "ncp_glue_wifi.h"
#include "ncp_config.h"
#include "mdns_service.h"
#include "fsl_os_abstraction.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_NOTIFY_MAX_EVENTS 20

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern int wifi_ncp_send_response(uint8_t *pbuf);
uint8_t suspend_notify_flag = 0;

/*******************************************************************************
 * Variables
 ******************************************************************************/

static os_queue_pool_define(app_notify_event_queue_data, APP_NOTIFY_MAX_EVENTS * sizeof(app_notify_msg_t));
static os_queue_t app_notify_event_queue; /* app notify event queue */

static os_thread_t app_notify_event_thread;                  /* app notify event processing task */
static os_thread_stack_define(app_notify_event_stack, 2048); /* app notify event processing task stack*/

extern uint32_t current_cmd;
extern OSA_SEMAPHORE_HANDLE_DEFINE(wifi_ncp_lock);
/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * This function is used to send event to app_notify_event_task through the
 * message queue.
 */
int app_notify_event(uint16_t event, int result, void *data, int len)
{
    int ret;
    app_notify_msg_t msg;
    if (!app_notify_event_queue)
    {
        app_e("app_notify_event_queue has no create, event %d", event);
        return -WM_FAIL;
    }

    memset(&msg, 0, sizeof(msg));
    msg.event    = event;
    msg.reason   = result;
    msg.data_len = len;
    msg.data     = data;

    ret = os_queue_send(&app_notify_event_queue, &msg, OS_NO_WAIT);
    if (ret != WM_SUCCESS)
    {
        app_e("failed to send event(%d) on queue", event);
        /* Release bridge lock */
        OSA_SemaphorePost(wifi_ncp_lock);
        app_d("put bridge lock");
    }

    return ret;
}

/**
 * This function handles wifi_ncp_lock release for asynchronous commands
 * and sends response to uart.
 */
static void app_notify_event_handler(void *argv)
{
    int ret = WM_SUCCESS;
    app_notify_msg_t msg;
    uint8_t *event_buf = NULL;

    while (1)
    {
        /* Receive message on queue */
        ret = os_queue_recv(&app_notify_event_queue, &msg, OS_WAIT_FOREVER);
        if (ret != WM_SUCCESS)
        {
            app_e("failed to get message from queue [%d]", ret);
            continue;
        }

        app_d("got notify message: %d %d %p", msg.event, msg.reason, msg.data);

        switch (msg.event)
        {
            case APP_EVT_SCAN_RESULT:
                if (msg.reason == APP_EVT_REASON_SUCCESS && msg.data != NULL)
                {
                    app_d("got scan result");
                    wlan_bridge_prepare_scan_result(msg.data);
                    os_mem_free(msg.data);
                }
                else
                {
                    wlan_bridge_prepare_status(NCP_BRIDGE_CMD_WLAN_STA_SCAN, msg.reason);
                }
                break;
            case APP_EVT_USER_DISCONNECT:
                if(current_cmd == NCP_BRIDGE_CMD_WLAN_STA_CONNECT)
                {
                    app_d("current network connect fail");
                    wlan_bridge_prepare_status(NCP_BRIDGE_CMD_WLAN_STA_CONNECT, APP_EVT_REASON_FAILURE);
                }
                else
                {
                    app_d("disconnect from the current network");
                    wlan_bridge_prepare_status(NCP_BRIDGE_CMD_WLAN_STA_DISCONNECT, msg.reason);
                }
                break;
            case APP_EVT_UAP_PROV_START:
                app_d("got uap_prov_start result");
                wlan_bridge_prepare_status(NCP_BRIDGE_CMD_WLAN_BASIC_WLAN_UAP_PROV_START, msg.reason);
                break;
            case APP_EVT_USER_CONNECT:
                if (msg.reason == APP_EVT_REASON_SUCCESS && msg.data != NULL)
                {
                    app_d("connected to network");
                    wlan_bridge_prepare_connect_result(msg.data);
                    os_mem_free(msg.data);
                }
                else
                {
                    wlan_bridge_prepare_status(NCP_BRIDGE_CMD_WLAN_STA_CONNECT, msg.reason);
                }
                break;
            case APP_EVT_USER_START_NETWORK:
                if (msg.reason == APP_EVT_REASON_SUCCESS && msg.data != NULL)
                {
                    app_d("network started");
                    wlan_bridge_prepare_start_network_result(msg.data);
                    os_mem_free(msg.data);
                }
                else
                {
                    wlan_bridge_prepare_status(NCP_BRIDGE_CMD_WLAN_NETWORK_START, msg.reason);
                }
                break;
            case APP_EVT_USER_STOP_NETWORK:
                app_d("got stop network result");
                wlan_bridge_prepare_status(NCP_BRIDGE_CMD_WLAN_NETWORK_STOP, msg.reason);
                break;
            case APP_EVT_WPS_DONE:
                app_d("got network of WPS session");
                if (is_nvm_enabled() && msg.data)
                    wifi_set_network((struct wlan_network *)(msg.data));
                break;
            case APP_EVT_HS_CONFIG:
                app_d("got MCU sleep config result");
                wlan_bridge_prepare_status(NCP_BRIDGE_CMD_WLAN_POWERMGMT_MCU_SLEEP, msg.reason);
                break;
            case APP_EVT_SUSPEND:
                app_d("got suspend command result");
                wlan_bridge_prepare_status(NCP_BRIDGE_CMD_WLAN_POWERMGMT_SUSPEND, msg.reason);
                break;
            case APP_EVT_MCU_SLEEP_ENTER:
                app_d("got MCU sleep enter report");
                event_buf = wlan_bridge_evt_status(NCP_BRIDGE_EVENT_MCU_SLEEP_ENTER, &msg);
                if (!event_buf)
                    ret = -WM_FAIL;
                break;
            case APP_EVT_MCU_SLEEP_EXIT:
#ifdef CONFIG_NCP_USB
                /* Wait for USB re-init done */
                OSA_TimeDelay(500);
#endif
                app_d("got MCU sleep exit report");
                event_buf = wlan_bridge_evt_status(NCP_BRIDGE_EVENT_MCU_SLEEP_EXIT, &msg);
                if (!event_buf)
                    ret = -WM_FAIL;
                break;
            case APP_EVT_MDNS_SEARCH_RESULT:
                app_d("got mdns search result");
                if (msg.reason == APP_EVT_REASON_SUCCESS && msg.data != NULL)
                {
                    wlan_bridge_prepare_mdns_result(msg.data);
                    continue;
                }
                else
                {
                    event_buf = wlan_bridge_evt_status(NCP_BRIDGE_EVENT_MDNS_QUERY_RESULT, &msg);
                    if (!event_buf)
                        ret = -WM_FAIL;
                }
                break;
            case APP_EVT_MDNS_RESOLVE_DOMAIN_NAME:
                app_d("got the resolved server address");
                if (msg.reason == APP_EVT_REASON_SUCCESS && msg.data != NULL)
                {
                    event_buf = wlan_bridge_prepare_mdns_resolve_result(msg.data);
                }
                else
                {
                    event_buf = wlan_bridge_evt_status(NCP_BRIDGE_EVENT_MDNS_RESOLVE_DOMAIN, &msg);
                }
                if (!event_buf)
                    ret = -WM_FAIL;
                break;
            case APP_EVT_INVALID_CMD:
                app_d("got invalid command");
                wlan_bridge_prepare_status(NCP_BRIDGE_CMD_INVALID_CMD, msg.reason);
                break;
            case APP_EVT_CSI_DATA:
                app_d("got csi data report");
                event_buf = wlan_bridge_evt_status(NCP_BRIDGE_EVENT_CSI_DATA, &msg);
                if (!event_buf)
                    ret = -WM_FAIL;
                break;
            default:
                app_d("no matching case");
                ret = -WM_FAIL;
                break;
        }

        if (ret == WM_SUCCESS)
        {
            if (event_buf)
                wifi_ncp_send_response(event_buf);
            else
                wifi_ncp_send_response((uint8_t *)ncp_bridge_get_response_buffer());
        }

        if(msg.event == APP_EVT_SUSPEND)
            suspend_notify_flag &= (~APP_NOTIFY_SUSPEND_CMDRESP);
        else if (msg.event == APP_EVT_MCU_SLEEP_ENTER)
            suspend_notify_flag &= (~APP_NOTIFY_SUSPEND_EVT);

        if (event_buf)
        {
            os_mem_free(event_buf);
            event_buf = NULL;
        }
    }
}

int app_notify_init(void)
{
    int ret;

    ret = os_queue_create(&app_notify_event_queue, "app_notify_event_queue", sizeof(app_notify_msg_t),
                          &app_notify_event_queue_data);
    if (ret != WM_SUCCESS)
    {
        app_e("failed to create app notify event queue: %d", ret);
        return -WM_FAIL;
    }

    ret = os_thread_create(&app_notify_event_thread, "app_notify_event_task", app_notify_event_handler, 0,
                           &app_notify_event_stack, OS_PRIO_2);
    if (ret != WM_SUCCESS)
    {
        app_e("failed to create app notify event thread: %d", ret);
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}
