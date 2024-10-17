/* @file app_notify.c
 *
 *  @brief This file contains declaration of the API functions.
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#include <osa.h>
#include "app_notify.h"
#include "wlan.h"
#include "wifi.h"
#include "ncp_glue_wifi.h"
#include "ncp_glue_system.h"
#include "ncp_config.h"
#include "mdns_service.h"
#include "fsl_os_abstraction.h"
#if CONFIG_NCP_USB
#include "ncp_intf_usb_device_cdc.h"
#endif
#include "ncp_system.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_NOTIFY_MAX_EVENTS 20

#ifndef OT_NCP_CMD_HANDLING
#define OT_NCP_CMD_HANDLING 4
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern int wifi_ncp_send_response(uint8_t *pbuf);
uint8_t suspend_notify_flag = 0;
extern uint8_t wifi_res_buf[NCP_INBUF_SIZE];

#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
extern void APP_SetTicklessIdle(bool enable);
#endif

#if CONFIG_NCP_SPI
extern int ncp_spi_txrx_is_finish(void);
#endif

#if CONFIG_NCP_OT
extern volatile uint8_t OtNcpDataHandle;
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

static OSA_MSGQ_HANDLE_DEFINE(app_notify_event_queue, APP_NOTIFY_MAX_EVENTS, sizeof(app_notify_msg_t)); /* app notify event queue */

static void app_notify_event_handler(void *argv);
static OSA_TASK_HANDLE_DEFINE(app_notify_event_thread);                                  /* app notify event processing task */
static OSA_TASK_DEFINE(app_notify_event_handler, PRIORITY_RTOS_TO_OSA(2), 1, 2048, 0); /* app notify event processing task stack*/

extern uint32_t current_cmd;
extern OSA_SEMAPHORE_HANDLE_DEFINE(wifi_ncp_lock);

#if CONFIG_NCP_USB
extern usb_cdc_vcom_struct_t s_cdcVcom;
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * This function is used to send event to app_notify_event_task through the
 * message queue.
 */
int app_notify_event(uint16_t event, int result, void *data, int len)
{
    app_notify_msg_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.event    = event;
    msg.reason   = result;
    msg.data_len = len;
    msg.data     = data;

    if (OSA_MsgQPut((osa_msgq_handle_t)app_notify_event_queue, &msg) != KOSA_StatusSuccess)
    {
        app_e("failed to send event(%d) on queue", event);
        /* Release lock */
        OSA_SemaphorePost(wifi_ncp_lock);
        app_d("put lock");
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}

/**
 * This function handles wifi_ncp_lock release for asynchronous commands
 * and sends response to uart.
 */
static void app_notify_event_handler(void *argv)
{
    int ret = WM_SUCCESS;
    osa_status_t status;
    app_notify_msg_t msg;
    uint8_t *event_buf = NULL;
#if CONFIG_NCP_USB
    int lpm_usb_retry_cnt = 20;
#endif
#if CONFIG_NCP_SPI
    uint8_t spi_chk_finish_cnt = 10;
#endif
#if CONFIG_NCP_OT
    uint8_t ot_chk_rsp_cnt = 10;
#endif

    while (1)
    {
        /* Receive message on queue */
        status = OSA_MsgQGet((osa_msgq_handle_t)app_notify_event_queue, &msg, osaWaitForever_c);
        if (status != KOSA_StatusSuccess)
        {
            app_e("failed to get message from queue [%d]", status);
            continue;
        }

        app_d("got notify message: %d %d %p", msg.event, msg.reason, msg.data);

        switch (msg.event)
        {
            case APP_EVT_SCAN_RESULT:
                if (msg.reason == APP_EVT_REASON_SUCCESS && msg.data != NULL)
                {
                    app_d("got scan result");
                    wlan_ncp_prepare_scan_result(msg.data);
                    OSA_MemoryFree(msg.data);
                }
                else
                {
                    wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_SCAN, msg.reason);
                }
                break;
            case APP_EVT_USER_DISCONNECT:
                if(current_cmd == NCP_CMD_WLAN_STA_CONNECT)
                {
                    app_d("current network connect fail");
                    wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_CONNECT, APP_EVT_REASON_FAILURE);
                }
                else if(current_cmd == NCP_CMD_WLAN_STA_DISCONNECT)
                {
                    app_d("disconnect from the current network");
                    wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_DISCONNECT, msg.reason);
                }
                else
                {
                    app_d("disconnect from the current network");
                    event_buf = wlan_ncp_evt_status(NCP_EVENT_WLAN_STA_DISCONNECT, &msg);
                    if (!event_buf)
                        ret = -WM_FAIL;
                }
                break;
            case APP_EVT_UAP_PROV_START:
                app_d("got uap_prov_start result");
                wlan_ncp_prepare_status(NCP_RSP_WLAN_BASIC_WLAN_UAP_PROV_START, msg.reason);
                break;
            case APP_EVT_USER_CONNECT:
                if(current_cmd == NCP_CMD_WLAN_STA_CONNECT)
                {
                    if (msg.reason == APP_EVT_REASON_SUCCESS && msg.data != NULL)
                    {
                        app_d("connected to network");
                        wlan_ncp_prepare_connect_result(msg.data);
                        OSA_MemoryFree(msg.data);
                    }
                    else
                    {
                        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_CONNECT, msg.reason);
                    }

                }
                else
                {
                    app_d("connect with the current network");
                    event_buf = wlan_ncp_evt_status(NCP_EVENT_WLAN_STA_CONNECT, &msg);
                    if (!event_buf)
                        ret = -WM_FAIL;
                }
                break;
            case APP_EVT_USER_START_NETWORK:
                if(current_cmd == NCP_CMD_WLAN_NETWORK_START)
                {
                    if (msg.reason == APP_EVT_REASON_SUCCESS && msg.data != NULL)
                    {
                        app_d("network started");
                        wlan_ncp_prepare_start_network_result(msg.data);
                        OSA_MemoryFree(msg.data);
                    }
                    else
                    {
                        wlan_ncp_prepare_status(NCP_RSP_WLAN_NETWORK_START, msg.reason);
                    }
                }
                else
                {
                    app_d("the current network started");
                }
                break;
            case APP_EVT_USER_STOP_NETWORK:
                app_d("got stop network result");
                if(current_cmd == NCP_CMD_WLAN_NETWORK_STOP)
                {
                    wlan_ncp_prepare_status(NCP_RSP_WLAN_NETWORK_STOP, msg.reason);
                }
                else
                {
                    event_buf = wlan_ncp_evt_status(NCP_EVENT_WLAN_STOP_NETWORK, &msg);
                    if (!event_buf)
                        ret = -WM_FAIL;
                }
                break;
            case APP_EVT_WPS_DONE:
                app_d("got network of WPS session");
                if (is_nvm_enabled() && msg.data)
                    wifi_set_network((struct wlan_network *)(msg.data));
                break;
            case APP_EVT_HS_CONFIG:
                app_d("got MCU sleep config result");
                wlan_ncp_prepare_status(NCP_RSP_SYSTEM_POWERMGMT_MCU_SLEEP, msg.reason);
                break;
            case APP_EVT_SUSPEND:
                app_d("got suspend command result");
                wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_SUSPEND, msg.reason);
                break;
            case APP_EVT_MCU_SLEEP_ENTER:
#if CONFIG_NCP_SPI
                /* For ot and ble PM3 mode, the point at which spi starts receiving data may be earlier
                 * than the point at which spi finish is checked in enter sleep notify in tickless, resulting
                 * in spi just starting to receive data when sleep handshake starts, repeat check here
                 * for avoid that
                 * */
                spi_chk_finish_cnt = 10;
                while((spi_chk_finish_cnt > 0) && (ncp_spi_txrx_is_finish() == 0))
                {
                    OSA_TimeDelay(50);
                    spi_chk_finish_cnt--;
                }

                if (spi_chk_finish_cnt == 0)
                {
                    app_e("spi txrx was not finish yet");
                }
#endif
#if CONFIG_NCP_OT
                /* it should be ensured that the OT data has been sent first and then
                 * start sleep handshake.
                 * */
                ot_chk_rsp_cnt = 10;
                while((ot_chk_rsp_cnt > 0) && (OtNcpDataHandle == OT_NCP_CMD_HANDLING))
                {
                    OSA_TimeDelay(50);
                    ot_chk_rsp_cnt--;
                }

                if (ot_chk_rsp_cnt == 0)
                {
                    app_e("ot has not been transmitted yet");
                }
#endif
                app_d("got MCU sleep enter report");
                event_buf = ncp_sys_evt_status(NCP_EVENT_MCU_SLEEP_ENTER, &msg);
                if (!event_buf)
                    ret = -WM_FAIL;
                break;
            case APP_EVT_MCU_SLEEP_EXIT:
#if CONFIG_NCP_USB
#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
                APP_SetTicklessIdle(false);
#endif
                /* Wait for USB re-init done */
                lpm_usb_retry_cnt = 20;
                while(lpm_usb_retry_cnt > 0 && 1 != s_cdcVcom.attach)
                {
                    OSA_TimeDelay(50);
                    lpm_usb_retry_cnt--;
                }

                if(0 == lpm_usb_retry_cnt)
                {
                    app_e("usb enum failed from LPM");
                }
#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
                APP_SetTicklessIdle(true);
#endif
#endif
#if CONFIG_NCP_SDIO
#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
                APP_SetTicklessIdle(false);
#endif
                /* Wait for SDIO re-init done */
                OSA_TimeDelay(800);
#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
                APP_SetTicklessIdle(true);
#endif
#endif
                app_d("got MCU sleep exit report");
                event_buf = ncp_sys_evt_status(NCP_EVENT_MCU_SLEEP_EXIT, &msg);
                if (!event_buf)
                    ret = -WM_FAIL;
                break;
            case APP_EVT_MDNS_SEARCH_RESULT:
                app_d("got mdns search result");
                if (msg.reason == APP_EVT_REASON_SUCCESS && msg.data != NULL)
                {
                    wlan_ncp_prepare_mdns_result(msg.data);
                    continue;
                }
                else
                {
                    event_buf = wlan_ncp_evt_status(NCP_EVENT_MDNS_QUERY_RESULT, &msg);
                    if (!event_buf)
                        ret = -WM_FAIL;
                }
                break;
            case APP_EVT_MDNS_RESOLVE_DOMAIN_NAME:
                app_d("got the resolved server address");
                if (msg.reason == APP_EVT_REASON_SUCCESS && msg.data != NULL)
                {
                    event_buf = wlan_ncp_prepare_mdns_resolve_result(msg.data);
                }
                else
                {
                    event_buf = wlan_ncp_evt_status(NCP_EVENT_MDNS_RESOLVE_DOMAIN, &msg);
                }
                if (!event_buf)
                    ret = -WM_FAIL;
                break;
            case APP_EVT_INVALID_CMD:
                app_d("got invalid command");
                wlan_ncp_prepare_status(NCP_RSP_INVALID_CMD, msg.reason);
                break;
            case APP_EVT_CSI_DATA:
                app_d("got csi data report");
                event_buf = wlan_ncp_evt_status(NCP_EVENT_CSI_DATA, &msg);
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
            {
                if(msg.event == APP_EVT_MCU_SLEEP_ENTER || msg.event == APP_EVT_MCU_SLEEP_EXIT)
                {
                    system_ncp_send_response(event_buf);
                }
                else
                    wifi_ncp_send_response(event_buf);
            }
            else
                wifi_ncp_send_response((uint8_t *)wifi_res_buf);
        }

        if (event_buf)
        {
            OSA_MemoryFree(event_buf);
            event_buf = NULL;
        }
    }
}

int app_notify_init(void)
{
    osa_status_t status;

    status = OSA_MsgQCreate((osa_msgq_handle_t)app_notify_event_queue, APP_NOTIFY_MAX_EVENTS, sizeof(app_notify_msg_t));
    if (status != KOSA_StatusSuccess)
    {
        app_e("failed to create app notify event queue: %d", status);
        return -WM_FAIL;
    }

    status = OSA_TaskCreate((osa_task_handle_t)app_notify_event_thread, OSA_TASK(app_notify_event_handler), NULL);
    if (status != KOSA_StatusSuccess)
    {
        app_e("failed to create app notify event thread: %d", status);
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}
