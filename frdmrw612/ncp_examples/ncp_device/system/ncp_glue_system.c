/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#include "board.h"
#include "ncp_debug.h"
#include "ncp_config.h"
#include "ncp_cmd_system.h"
#include "ncp_glue_system.h"
#include "ncp_lpm.h"
#include "host_sleep.h"
#include "fsl_pm_device.h"
#include "fsl_pm_core.h"
#include "app_notify.h"
#if CONFIG_NCP_WIFI
#include "wlan.h"
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern power_cfg_t global_power_config;
#if !CONFIG_NCP_BLE
extern OSA_TIMER_HANDLE_DEFINE(wake_timer);
#endif
int current_PM_mode;
extern uint64_t rtc_timeout;
extern uint8_t suspend_notify_flag;
#if CONFIG_NCP_WIFI
extern int is_hs_handshake_done;
#endif
extern OSA_SEMAPHORE_HANDLE_DEFINE(hs_cfm);
extern uint16_t g_cmd_seqno;
extern uint8_t cmd_buf[NCP_INBUF_SIZE];
uint8_t sys_res_buf[NCP_SYS_INBUF_SIZE];

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define HOST_SLEEP_DEF_WAKE_TIME 5000

/*******************************************************************************
 * Code
 ******************************************************************************/

NCPCmd_DS_SYS_COMMAND *ncp_sys_get_resp_buf()
{
    ncp_get_sys_resp_buf_lock();
    return (NCPCmd_DS_SYS_COMMAND *)(sys_res_buf);
}

/** Prepare TLV command response */
int ncp_sys_prepare_status(uint32_t cmd, uint16_t result)
{
    NCPCmd_DS_SYS_COMMAND *cmd_res = ncp_sys_get_resp_buf();
    cmd_res->header.cmd            = cmd;

    cmd_res->header.size     = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum   = 0x00;
    cmd_res->header.result   = result;
    return NCP_SUCCESS;
}

uint8_t *ncp_sys_evt_status(uint32_t evt_id, void *msg)
{
    uint8_t *event_buf        = NULL;
    app_notify_msg_t *message = (app_notify_msg_t *)msg;
    int total_len             = 0;

    total_len = message->data_len + NCP_CMD_HEADER_LEN;
    event_buf = (uint8_t *)OSA_MemoryAllocate(total_len);
    if (event_buf == NULL)
    {
        ncp_e("failed to allocate memory for event");
        return NULL;
    }

    NCP_COMMAND *evt_hdr = (NCP_COMMAND *)event_buf;
    evt_hdr->cmd                = evt_id;
    evt_hdr->size               = total_len;
    evt_hdr->seqnum             = 0x00;
    evt_hdr->result             = message->reason;
    if (message->data_len)
        memcpy(event_buf + NCP_CMD_HEADER_LEN, message->data, message->data_len);

    return event_buf;
}

static int ncp_sys_set_config(void *tlv)
{
    NCP_CMD_SYSTEM_CFG *sys_cfg = (NCP_CMD_SYSTEM_CFG *)tlv;
    int ret                     = NCP_SUCCESS;
    const char *mod_name = NULL, *var_name = NULL, *value = NULL;

    if ((*sys_cfg->module_name == '\0') || (*sys_cfg->variable_name == '\0') || (*sys_cfg->value == '\0'))
    {
        ncp_e("invalid params");
        ret = -NCP_FAIL;
        goto done;
    }

    mod_name = sys_cfg->module_name;
    var_name = sys_cfg->variable_name;
    value    = sys_cfg->value;

    ret = ncp_set_conf(mod_name, var_name, value);

done:
    ncp_sys_prepare_status(NCP_RSP_SYSTEM_CONFIG_SET, ret);

    return ret;
}

static int ncp_sys_get_config(void *tlv)
{
    NCP_CMD_SYSTEM_CFG *sys_cfg = (NCP_CMD_SYSTEM_CFG *)tlv;
    int ret                     = NCP_SUCCESS;
    const char *mod_name, *var_name;
    char value[CONFIG_VALUE_MAX_LEN] = {0};

    if ((*sys_cfg->module_name == '\0') || (*sys_cfg->variable_name == '\0'))
    {
        ncp_e("invalid params");
        ncp_sys_prepare_status(NCP_RSP_SYSTEM_CONFIG_GET, NCP_CMD_RESULT_ERROR);
        return -NCP_FAIL;
    }

    mod_name = sys_cfg->module_name;
    var_name = sys_cfg->variable_name;

    ret = ncp_get_conf(mod_name, var_name, value, sizeof(value));
    if (ret != WM_SUCCESS)
    {
        ncp_sys_prepare_status(NCP_RSP_SYSTEM_CONFIG_GET, NCP_CMD_RESULT_ERROR);
        return ret;
    }

    NCPCmd_DS_SYS_COMMAND *cmd_res = ncp_sys_get_resp_buf();
    cmd_res->header.cmd            = NCP_RSP_SYSTEM_CONFIG_GET;
    cmd_res->header.size           = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum         = 0x00;
    cmd_res->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_SYSTEM_CFG *sys_cfg_res = (NCP_CMD_SYSTEM_CFG *)&cmd_res->params.system_cfg;
    (void)memcpy(sys_cfg_res->module_name, sys_cfg->module_name, sizeof(sys_cfg_res->module_name));
    (void)memcpy(sys_cfg_res->variable_name, sys_cfg->variable_name, sizeof(sys_cfg_res->variable_name));
    (void)memcpy(sys_cfg_res->value, value, sizeof(sys_cfg_res->value));

    cmd_res->header.size += sizeof(NCP_CMD_SYSTEM_CFG);

    return ret;
}

static int ncp_sys_wake_cfg(void *tlv)
{
    osa_status_t status;
    int ret = 0;
    NCP_CMD_POWERMGMT_WAKE_CFG *wake_config = (NCP_CMD_POWERMGMT_WAKE_CFG *)tlv;

    if(strcmp(BOARD_NAME, "RD-RW61X-BGA") == 0 && wake_config->wake_mode == WAKE_MODE_WIFI_NB)
    {
        ncp_e("Invalid wake mode. The WIFI-NB mode is for FRDMRW612 only.");
        ret = -WM_E_INVAL;
        goto out;
    }
    global_power_config.wake_mode     = wake_config->wake_mode;
    global_power_config.subscribe_evt = wake_config->subscribe_evt;
#if !CONFIG_NCP_BLE
    global_power_config.wake_duration = wake_config->wake_duration;
    if (global_power_config.wake_duration > 0)
    {
        status = OSA_TimerChange((osa_timer_handle_t)wake_timer, global_power_config.wake_duration * 1000, 0);
        if (status != KOSA_StatusSuccess)
        {
            ret = -WM_FAIL;
            goto out;
        }
    }
#endif
out:
    if (ret)
        ncp_sys_prepare_status(NCP_RSP_SYSTEM_POWERMGMT_WAKE_CFG, NCP_CMD_RESULT_ERROR);
    else
        ncp_sys_prepare_status(NCP_RSP_SYSTEM_POWERMGMT_WAKE_CFG, NCP_CMD_RESULT_OK);

    return WM_SUCCESS;
}

static int ncp_sys_mcu_sleep(void *tlv)
{
    NCP_CMD_POWERMGMT_MCU_SLEEP *mcu_sleep_config = (NCP_CMD_POWERMGMT_MCU_SLEEP *)tlv;

    global_power_config.enable = mcu_sleep_config->enable;
    /* MCU sleep is disabled */
    if (global_power_config.enable == 0)
    {
        memset(&global_power_config, 0x0, sizeof(global_power_config));
#if CONFIG_NCP_WIFI
        wlan_cancel_host_sleep();
        wlan_clear_host_sleep_config();
#endif

#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
        /* Release current constrations of Power Manager */
        LPM_ConfigureNextLowPowerMode(0, 0);
#else
        if (current_PM_mode == PM_LP_STATE_PM2)
        {
            PM_ReleaseConstraints(PM_LP_STATE_PM2, APP_PM2_CONSTRAINTS);
        }
        else if (current_PM_mode == PM_LP_STATE_PM3)
        {
            PM_ReleaseConstraints(PM_LP_STATE_PM3, APP_PM3_CONSTRAINTS);
        }
        current_PM_mode = PM_LP_STATE_PM0;
#endif
        goto out;
    }
    else
    {
#if CONFIG_NCP_WIFI
        global_power_config.is_manual = mcu_sleep_config->is_manual;
#endif
        /* No wake_mode configuration. Use default GPIO mode */
        if (global_power_config.wake_mode == 0)
        {
            global_power_config.wake_mode     = WAKE_MODE_GPIO;
            global_power_config.subscribe_evt = 1;
            global_power_config.wake_duration = HOST_SLEEP_DEF_WAKE_TIME;
        }
#if CONFIG_NCP_WIFI
        if (global_power_config.is_manual)
            global_power_config.is_periodic = 0;
        else
        {
#endif
            if (global_power_config.wake_mode == WAKE_MODE_INTF ||
                (!strcmp(BOARD_NAME, "FRDM-RW612") && global_power_config.wake_mode == WAKE_MODE_GPIO))
            {
#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
                /* Set current PM constraints to PM2 */
#if !(CONFIG_NCP_USB)
                LPM_ConfigureNextLowPowerMode(2, mcu_sleep_config->rtc_timeout);
#endif
#else
                if (current_PM_mode == PM_LP_STATE_PM3)
                    PM_ReleaseConstraints(PM_LP_STATE_PM3, APP_PM3_CONSTRAINTS);
                PM_SetConstraints(PM_LP_STATE_PM2, APP_PM2_CONSTRAINTS);
                current_PM_mode = PM_LP_STATE_PM2;
#endif
            }
            else
            {
                global_power_config.subscribe_evt = 1;
#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
                /* Set current PM constraints to PM3 */
                LPM_ConfigureNextLowPowerMode(3, mcu_sleep_config->rtc_timeout);
#else
                if (current_PM_mode == PM_LP_STATE_PM2)
                    PM_ReleaseConstraints(PM_LP_STATE_PM2, APP_PM2_CONSTRAINTS);
                PM_SetConstraints(PM_LP_STATE_PM3, APP_PM3_CONSTRAINTS);
                current_PM_mode = PM_LP_STATE_PM3;
#endif
            }
            global_power_config.rtc_timeout = mcu_sleep_config->rtc_timeout;
            rtc_timeout                     = global_power_config.rtc_timeout * 1000000;
            global_power_config.is_periodic = 1;
#if CONFIG_NCP_WIFI
        }
#endif
    }
#if CONFIG_NCP_WIFI
    wlan_config_host_sleep(global_power_config.is_manual, global_power_config.is_periodic);
#endif
out:
    ncp_sys_prepare_status(NCP_RSP_SYSTEM_POWERMGMT_MCU_SLEEP, NCP_CMD_RESULT_OK);

    return WM_SUCCESS;
}

static int ncp_sys_wakeup_host(void *tlv)
{
    NCP_CMD_POWERMGMT_WAKEUP_HOST *wake_host_ctrl = (NCP_CMD_POWERMGMT_WAKEUP_HOST *)tlv;

    global_power_config.wakeup_host = wake_host_ctrl->enable;
    ncp_sys_prepare_status(NCP_RSP_SYSTEM_POWERMGMT_WAKEUP_HOST, NCP_CMD_RESULT_OK);

    return WM_SUCCESS;
}

static int ncp_sys_mcu_sleep_cfm(void *tlv)
{
    suspend_notify_flag &= (~APP_NOTIFY_SUSPEND_EVT);
    suspend_notify_flag |= APP_NOTIFY_SUSPEND_CFM;
    if(global_power_config.is_manual == false)
    {
#if CONFIG_NCP_WIFI
        is_hs_handshake_done = WLAN_HOSTSLEEP_SUCCESS;
#endif
        lpm_setHandshakeState(NCP_LMP_HANDSHAKE_FINISH);

        OSA_SemaphorePost((osa_semaphore_handle_t)hs_cfm);
    }

    return WM_SUCCESS;
}

struct cmd_t system_cmd_config[] = {
    {NCP_CMD_SYSTEM_CONFIG_SET, "ncp-set", ncp_sys_set_config, CMD_SYNC},
    {NCP_CMD_SYSTEM_CONFIG_GET, "ncp-get", ncp_sys_get_config, CMD_SYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t system_cmd_powermgmt[] = {
    {NCP_CMD_SYSTEM_POWERMGMT_WAKE_CFG, "ncp-wake-cfg", ncp_sys_wake_cfg, CMD_SYNC},
    {NCP_CMD_SYSTEM_POWERMGMT_MCU_SLEEP, "ncp-mcu-sleep", ncp_sys_mcu_sleep, CMD_SYNC},
    {NCP_CMD_SYSTEM_POWERMGMT_WAKEUP_HOST, "ncp-wakeup-host", ncp_sys_wakeup_host, CMD_SYNC},
    {NCP_CMD_SYSTEM_POWERMGMT_MCU_SLEEP_CFM, NULL, ncp_sys_mcu_sleep_cfm, CMD_ASYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_subclass_t cmd_subclass_system[] = {
    {NCP_CMD_SYSTEM_CONFIG, system_cmd_config},
    {NCP_CMD_SYSTEM_POWERMGMT, system_cmd_powermgmt},
    {NCP_CMD_INVALID, NULL},
};
