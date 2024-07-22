/** @file ncp_host_command_system.c
 *
 *  @brief  This file provides API functions to build tlv commands and process tlv responses.
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "ncp_debug.h"
#include "ncp_cmd_system.h"
#include "ncp_host_command.h"
#include <string.h>

power_cfg_t global_power_config;
extern uint8_t mcu_device_status;
extern OSA_SEMAPHORE_HANDLE_DEFINE(gpio_wakelock);
#if CONFIG_NCP_USB
extern uint8_t usb_enter_pm2;
#endif

#if CONFIG_NCP_SPI
extern AT_NONCACHEABLE_SECTION_INIT(uint8_t mcu_tlv_command_buff[NCP_HOST_COMMAND_LEN]);
#else
extern uint8_t mcu_tlv_command_buff[NCP_HOST_COMMAND_LEN];
#endif

extern int ncp_host_cli_register_commands(const struct ncp_host_cli_command *commands, int num_commands);

MCU_NCPCmd_DS_SYS_COMMAND *ncp_host_get_cmd_buffer_sys()
{
    return (MCU_NCPCmd_DS_SYS_COMMAND *)(mcu_tlv_command_buff);
}

int ncp_process_set_cfg_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_SYS_COMMAND *cmd_res = (MCU_NCPCmd_DS_SYS_COMMAND *)res;
    ret                                = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_ERROR)
    {
        ncp_e("Failed to set system configuration!");
        return -NCP_FAIL;
    }

    (void)PRINTF("Set system configuration successfully!\r\n");

    return NCP_SUCCESS;
}

int ncp_process_get_cfg_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_SYS_COMMAND *cmd_res = (MCU_NCPCmd_DS_SYS_COMMAND *)res;
    ret                                = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_ERROR)
    {
        ncp_e("Failed to set system configuration!");
        return -NCP_FAIL;
    }

    NCP_CMD_SYSTEM_CFG *sys_cfg = (NCP_CMD_SYSTEM_CFG *)&cmd_res->params.system_cfg;
    (void)PRINTF("%s = %s\r\n", sys_cfg->variable_name, sys_cfg->value);

    return NCP_SUCCESS;
}

/**
 * @brief      This function processes ncp device test loopback response from ncp host
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int ncp_process_test_loopback_response(uint8_t *res)
{
    int ret;

    NCPCmd_DS_SYS_COMMAND *cmd_res = (NCPCmd_DS_SYS_COMMAND *)res;
    ret                            = cmd_res->header.result;
    if (ret == NCP_CMD_RESULT_ERROR)
    {
        ncp_e("%s: test loopback failed!", __FUNCTION__);
        return -NCP_FAIL;
    }

    (void)PRINTF("Response test-loopback cmd size=0x%x:\r\n", cmd_res->header.size);
    ncp_dump_hex(cmd_res, cmd_res->header.size);

    return NCP_SUCCESS;
}

int ncp_set_command(int argc, char **argv)
{
    const char *mod, *var, *val;
    MCU_NCPCmd_DS_SYS_COMMAND *sys_cfg_command = ncp_host_get_cmd_buffer_sys();
    (void)memset((uint8_t *)sys_cfg_command, 0, NCP_HOST_COMMAND_LEN);

    if (argc < 4)
    {
        ncp_e("Invalid parameter number!");
        return -NCP_FAIL;
    }

    /* module name */
    mod = argv[1];
    if (*mod == '\0')
    {
        ncp_e("Module name is invalid params!");
        return -NCP_FAIL;
    }
    /* variable name */
    var = argv[2];
    if (*var == '\0')
    {
        ncp_e("Variable name is invalid params!");
        return -NCP_FAIL;
    }
    /* variable value */
    val = argv[3];
    if (*val == '\0')
    {
        ncp_e("Variable value is invalid params!");
        return -NCP_FAIL;
    }

    sys_cfg_command->header.cmd      = NCP_CMD_SYSTEM_CONFIG_SET;
    sys_cfg_command->header.size     = NCP_CMD_HEADER_LEN;
    sys_cfg_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_SYSTEM_CFG *sys_cfg = (NCP_CMD_SYSTEM_CFG *)&sys_cfg_command->params.system_cfg;
    strcpy(sys_cfg->module_name, mod);
    strcpy(sys_cfg->variable_name, var);
    strcpy(sys_cfg->value, val);

    sys_cfg_command->header.size += sizeof(NCP_CMD_SYSTEM_CFG);

    return NCP_SUCCESS;
}

int ncp_get_command(int argc, char **argv)
{
    const char *mod, *var;
    MCU_NCPCmd_DS_SYS_COMMAND *sys_cfg_command = ncp_host_get_cmd_buffer_sys();

    if (argc < 3)
    {
        (void)PRINTF("Error: Invalid parameter number!\r\n");
        return -NCP_FAIL;
    }

    /* module name */
    mod = argv[1];
    if (*mod == '\0')
    {
        ncp_e("Module name is invalid params!");
        return -NCP_FAIL;
    }
    /* variable name */
    var = argv[2];
    if (*var == '\0')
    {
        ncp_e("Variable name is invalid params!");
        return -NCP_FAIL;
    }

    sys_cfg_command->header.cmd      = NCP_CMD_SYSTEM_CONFIG_GET;
    sys_cfg_command->header.size     = NCP_CMD_HEADER_LEN;
    sys_cfg_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_SYSTEM_CFG *sys_cfg = (NCP_CMD_SYSTEM_CFG *)&sys_cfg_command->params.system_cfg;
    strcpy(sys_cfg->module_name, mod);
    strcpy(sys_cfg->variable_name, var);
    strcpy(sys_cfg->value, "");

    sys_cfg_command->header.size += sizeof(NCP_CMD_SYSTEM_CFG);

    return NCP_SUCCESS;
}

int ncp_wake_cfg_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_SYS_COMMAND *wake_cfg_cmd = ncp_host_get_cmd_buffer_sys();
    uint8_t wake_mode                       = 0;
    uint8_t subscribe_evt                   = 0;
    uint32_t wake_duration                  = 0;

    if (argc != 4)
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF("    %s <wake_mode> <subscribe_evt> <wake_duration>\r\n", argv[0]);
        (void)PRINTF("    wake_mode    : GPIO -- GPIO wakeup\r\n");
        (void)PRINTF("                   INTF -- Interface wakeup\r\n");
        (void)PRINTF("    subscribe_evt: 0 -- unsubscribe MCU device sleep status events\r\n");
        (void)PRINTF("                   1 -- subscribe MCU device sleep status events\r\n");
        (void)PRINTF("                   For GPIO mode, sleep status event is forced to be subscribed\r\n");
        (void)PRINTF("    wake_duration: Within the wake_duration, MCU device will keep active mode\r\n");
        (void)PRINTF("                   Unit is second\r\n");
        (void)PRINTF("Example:\r\n");
        (void)PRINTF("    ncp-wake-cfg INTF 0 5\r\n");
        (void)PRINTF("    ncp-wake-cfg GPIO 1 60\r\n");
        return -NCP_FAIL;
    }
    subscribe_evt = (uint8_t)atoi(argv[2]);
    if ((subscribe_evt != 0) && (subscribe_evt != 1))
    {
        (void)PRINTF("Invalid value of parameter subscribe_evt\r\n");
        return -NCP_FAIL;
    }
    if (!strncmp(argv[1], "INTF", 4))
    {
        wake_mode = WAKE_MODE_INTF;
    }
    else if (!strncmp(argv[1], "GPIO", 4))
    {
        wake_mode     = WAKE_MODE_GPIO;
        subscribe_evt = 1;
    }
    else
    {
        (void)PRINTF("Invalid input of wake_mode\r\n");
        return -NCP_FAIL;
    }
    wake_duration                           = atoi(argv[3]);
    wake_cfg_cmd->header.cmd                = NCP_CMD_SYSTEM_POWERMGMT_WAKE_CFG;
    wake_cfg_cmd->header.size               = NCP_CMD_HEADER_LEN;
    wake_cfg_cmd->header.result             = NCP_CMD_RESULT_OK;
    NCP_CMD_POWERMGMT_WAKE_CFG *wake_config = (NCP_CMD_POWERMGMT_WAKE_CFG *)&wake_cfg_cmd->params.wake_config;
    wake_config->wake_mode                  = wake_mode;
    wake_config->subscribe_evt              = subscribe_evt;
    wake_config->wake_duration              = wake_duration;
    wake_cfg_cmd->header.size += sizeof(NCP_CMD_POWERMGMT_WAKE_CFG);

    global_power_config.wake_mode     = wake_mode;
    global_power_config.subscribe_evt = subscribe_evt;
    global_power_config.wake_duration = wake_duration;

    return NCP_SUCCESS;
}

int ncp_process_wake_cfg_response(uint8_t *res)
{
    MCU_NCPCmd_DS_SYS_COMMAND *cmd_res = (MCU_NCPCmd_DS_SYS_COMMAND *)res;
    uint16_t result                    = cmd_res->header.result;

    if (result == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Wake mode cfg is successful!\r\n");
        (void)PRINTF("Please issue ncp-mcu-sleep command to enable new configs\r\n");
    }
    else
        (void)PRINTF("Wake mode cfg is failed!\r\n");

    return NCP_SUCCESS;
}

int ncp_mcu_sleep_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_SYS_COMMAND *mcu_sleep_command = ncp_host_get_cmd_buffer_sys();
    uint8_t enable                               = 0;
#if CONFIG_NCP_WIFI
    uint8_t is_manual                            = false;
#endif
    int rtc_timeout_s                            = 0;

    if (argc < 2 || argc > 4)
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF("    %s <enable> <mode> <rtc_timeout>\r\n", argv[0]);
        (void)PRINTF("    enable     : enable/disable mcu sleep\r\n");
        (void)PRINTF("                 0 - disable mcu sleep\r\n");
        (void)PRINTF("                 1 - enable mcu sleep\r\n");
        (void)PRINTF("    mode       : Mode of how host enter low power.\r\n");
#if CONFIG_NCP_WIFI
        (void)PRINTF("                 manual - Manual mode. Need to use wlan-suspend command to enter low power.\r\n");
#endif
        (void)PRINTF("                 pm     - Power Manager.\r\n");
        (void)PRINTF("    rtc_timeout: RTC timer value. Unit is second. For Power Manager only!\r\n");
        (void)PRINTF("Examples:\r\n");
        (void)PRINTF("    ncp-mcu-sleep 1 pm 5\r\n");
#if CONFIG_NCP_WIFI
        (void)PRINTF("    ncp-mcu-sleep 1 manual\r\n");
#endif
        (void)PRINTF("    ncp-mcu-sleep 0\r\n");
        return -NCP_FAIL;
    }
    enable = (uint8_t)atoi(argv[1]);
    if (enable != 0 && enable != 1)
    {
        (void)PRINTF("Invalid value of parameter enable\r\n");
        return -NCP_FAIL;
    }
    if (enable)
    {
        if (argc < 3)
        {
            (void)PRINTF("Invalid number of input!\r\n");
            (void)PRINTF("Usage:\r\n");
            (void)PRINTF("    ncp-mcu-sleep <enable> <mode> <rtc_timer>\r\n");
            return -NCP_FAIL;
        }
#if CONFIG_NCP_WIFI
        if (!strncmp(argv[2], "manual", 6))
            is_manual = true;
        else
#endif
        if (!strncmp(argv[2], "pm", 2))
        {
            if (argc != 4)
            {
                (void)PRINTF("Error!Invalid number of inputs! Need to specify both <rtc_timeout> and <periodic>\r\n");
                return -NCP_FAIL;
            }
            rtc_timeout_s = atoi(argv[3]);
            if (rtc_timeout_s == 0)
            {
                (void)PRINTF("Error!Invalid value of <rtc_timeout>!\r\n");
                return -NCP_FAIL;
            }
        }
        else
        {
            (void)PRINTF("Invalid input!\r\n");
            (void)PRINTF("Usage:\r\n");
            (void)PRINTF("    wlan-mcu-sleep <enable> <mode> <rtc_timer>\r\n");
            return -NCP_FAIL;
        }
    }
    mcu_sleep_command->header.cmd      = NCP_CMD_SYSTEM_POWERMGMT_MCU_SLEEP;
    mcu_sleep_command->header.size     = NCP_CMD_HEADER_LEN;
    mcu_sleep_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_POWERMGMT_MCU_SLEEP *mcu_sleep_config =
        (NCP_CMD_POWERMGMT_MCU_SLEEP *)&mcu_sleep_command->params.mcu_sleep_config;
    mcu_sleep_config->enable       = enable;
#if CONFIG_NCP_WIFI
    mcu_sleep_config->is_manual    = is_manual;
#endif
    mcu_sleep_config->rtc_timeout  = rtc_timeout_s;
    mcu_sleep_command->header.size += sizeof(NCP_CMD_POWERMGMT_MCU_SLEEP);

    global_power_config.enable      = enable;
#if CONFIG_NCP_WIFI
    global_power_config.is_manual   = is_manual;
#endif
    global_power_config.rtc_timeout = rtc_timeout_s;
    if (global_power_config.wake_mode == 0)
    {
        global_power_config.wake_mode     = WAKE_MODE_GPIO;
        global_power_config.subscribe_evt = 1;
        global_power_config.wake_duration = 5;
    }
    return NCP_SUCCESS;
}

int ncp_process_mcu_sleep_response(uint8_t *res)
{
    MCU_NCPCmd_DS_SYS_COMMAND *cmd_res = (MCU_NCPCmd_DS_SYS_COMMAND *)res;
    uint16_t result                    = cmd_res->header.result;

    if (result == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("MCU sleep cfg is success!\r\n");
        /* Clear previous power configs if mcu sleep is disabled */
        if (global_power_config.enable == 0)
            (void)memset(&global_power_config, 0x0, sizeof(global_power_config));
    }
    else
        (void)PRINTF("MCU sleep cfg is fail!\r\n");

    return NCP_SUCCESS;
}

int ncp_wakeup_host_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_SYS_COMMAND *wake_host_cmd = ncp_host_get_cmd_buffer_sys();
    uint8_t enable                           = 0;

    if (argc != 2)
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF("    %s <0/1>\r\n", argv[0]);
        (void)PRINTF("    0-disable  1-enable\r\n");
        (void)PRINTF("Make sure to configure wowlan conditions before enabling host wakeup\r\n");
        (void)PRINTF("Once enabled, MCU only wakes up host if MCU is wokenup by WLAN\r\n");
        return -NCP_FAIL;
    }
    enable = (uint8_t)atoi(argv[1]);
    if (enable == 1)
    {
        if (!global_power_config.is_mef && !global_power_config.wake_up_conds)
        {
            (void)PRINTF("Not configure wowlan conditions yet\r\n");
            (void)PRINTF("Please configure wowlan conditions first\r\n");
            return -NCP_FAIL;
        }
    }

    wake_host_cmd->header.cmd      = NCP_CMD_SYSTEM_POWERMGMT_WAKEUP_HOST;
    wake_host_cmd->header.size     = NCP_CMD_HEADER_LEN;
    wake_host_cmd->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_POWERMGMT_WAKEUP_HOST *host_wakeup_ctrl =
        (NCP_CMD_POWERMGMT_WAKEUP_HOST *)&wake_host_cmd->params.host_wakeup_ctrl;
    host_wakeup_ctrl->enable        = enable;
    wake_host_cmd->header.size      += sizeof(NCP_CMD_POWERMGMT_WAKEUP_HOST);
    global_power_config.wakeup_host = enable;

    return NCP_SUCCESS;
}

int ncp_process_wakeup_host_response(uint8_t *res)
{
    MCU_NCPCmd_DS_SYS_COMMAND *cmd_res = (MCU_NCPCmd_DS_SYS_COMMAND *)res;
    uint16_t result                    = cmd_res->header.result;

    if (result == NCP_CMD_RESULT_ERROR)
        (void)PRINTF("wakeup host command is failed\r\n");
    else
        (void)PRINTF("wakeup host command is successful\r\n");
    return NCP_SUCCESS;
}

int wlan_get_mcu_sleep_conf_command(int argc, char **argv)
{
    PRINTF("MCU sleep: %s\r\n", global_power_config.enable ? "enabled" : "disabled");
    if (global_power_config.wake_mode == 0)
    {
        global_power_config.wake_mode     = WAKE_MODE_GPIO;
        global_power_config.subscribe_evt = 1;
        global_power_config.wake_duration = 5;
    }
    PRINTF("Wake mode: %s\r\n", global_power_config.wake_mode == WAKE_MODE_GPIO ? "GPIO" : "UART");
    PRINTF("Subscribe event: %s\r\n", global_power_config.subscribe_evt ? "enabled" : "disabled");
    PRINTF("Wake duration: %ds\r\n", global_power_config.wake_duration);
#if CONFIG_NCP_WIFI
    PRINTF("Wake up method: %s\r\n", global_power_config.is_mef ? "MEF" : "wowlan");
    if (!global_power_config.is_mef)
        PRINTF("Wakeup bitmap: 0x%x\r\n", global_power_config.wake_up_conds);
    PRINTF("MCU sleep method: %s\r\n", global_power_config.is_manual ? "Manual" : "Power Manager");
#else
    PRINTF("MCU sleep method: Power Manager\r\n");
#endif
    PRINTF("MCU rtc timeout: %ds\r\n", global_power_config.rtc_timeout);
    PRINTF("Wakeup host: %s\r\n", global_power_config.wakeup_host ? "Enabled" : "Disabled");
    return NCP_SUCCESS;
}

#if CONFIG_NCP_USB
/* 1:enter pm2
 * 2:exit to pm0
 * 0:defualt value to keep status and no action
 */
int usb_pm_cfg(int argc, char **argv)
{
    if (argc < 2)
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        return -NCP_FAIL;
    }
    usb_enter_pm2 = atoi(argv[1]);

    return NCP_SUCCESS;
}
#endif

/* Display the usage of test-loopback */
static void display_test_loopback_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("\ttest-loopback <string1> <string2> ... <stringN>\r\n");
}

int ncp_test_loopback_command(int argc, char **argv)
{
    int ret = NCP_SUCCESS;
    uint8_t *pos = 0;
    uint32_t copy_size = 0;
    int i = 1;
    MCU_NCPCmd_DS_SYS_COMMAND *cmd = ncp_host_get_cmd_buffer_sys();
    (void)memset((uint8_t *)cmd, 0, NCP_HOST_COMMAND_LEN);
    cmd->header.cmd      = NCP_CMD_SYSTEM_TEST_LOOPBACK;
    cmd->header.size     = NCP_CMD_HEADER_LEN;
    cmd->header.result   = NCP_CMD_RESULT_OK;

    /* If number of arguments is not even then print error */
    if (argc < 2)
    {
        ret = -NCP_FAIL;
        goto end;
    }

    pos = (uint8_t *)cmd + sizeof(NCP_COMMAND);
    while ((i < argc) && (cmd->header.size < NCP_HOST_COMMAND_LEN))
    {
        copy_size = MIN(strlen(argv[i]), NCP_HOST_COMMAND_LEN - cmd->header.size);
        memcpy(pos, argv[i], copy_size);
        pos += copy_size;
        cmd->header.size += copy_size;
        //(void)PRINTF("%s: copy_size=0x%x cmd->header.size=0x%x\r\n", __FUNCTION__, copy_size, cmd->header.size);
        //(void)PRINTF("%s: argv[%d]=%s\r\n", __FUNCTION__, i, argv[i]);
        i++;
    }

    return NCP_SUCCESS;

end:
    ncp_e("Incorrect usage");
    display_test_loopback_usage();

    return ret;
}

void ncp_mcu_sleep_cfm(NCP_HOST_COMMAND *header)
{
    header->cmd      = NCP_CMD_SYSTEM_POWERMGMT_MCU_SLEEP_CFM;
    header->size     = NCP_CMD_HEADER_LEN;
    header->result   = NCP_CMD_RESULT_OK;
}

int ncp_process_sleep_status(uint8_t *res)
{
    MCU_NCPCmd_DS_SYS_COMMAND *event = (MCU_NCPCmd_DS_SYS_COMMAND *)res;
    int status                       = 0;

    if (event->header.cmd == NCP_EVENT_MCU_SLEEP_ENTER)
    {
        NCP_HOST_COMMAND sleep_cfm;

        if(global_power_config.subscribe_evt)
            (void)PRINTF("MCU device enters sleep mode\r\n");
        memset(&sleep_cfm, 0x0, sizeof(sleep_cfm));
        ncp_mcu_sleep_cfm(&sleep_cfm);
        status = (int)ncp_tlv_send((void *)&sleep_cfm, sleep_cfm.size);
        if(status != NCP_SUCCESS)
            (void)PRINTF("Failed to send mcu sleep cfm\r\n");
        OSA_TimeDelay(100);
        mcu_device_status = MCU_DEVICE_STATUS_SLEEP;
        if (global_power_config.wake_mode == WAKE_MODE_GPIO)
        {
            status = OSA_SemaphoreWait((osa_semaphore_handle_t)gpio_wakelock, osaWaitNone_c);
            if (status != NCP_SUCCESS)
                (void)PRINTF("Failed to get gpio_wakelock\r\n");
        }
    }
    else
    {
        if(global_power_config.subscribe_evt)
            (void)PRINTF("MCU device exits sleep mode\r\n");
        mcu_device_status = MCU_DEVICE_STATUS_ACTIVE;
        if (global_power_config.wake_mode == WAKE_MODE_GPIO)
        {
            status = OSA_SemaphorePost((osa_semaphore_handle_t)gpio_wakelock);
            if (status != NCP_SUCCESS)
                (void)PRINTF("Failed to put gpio_wakelock\r\n");
        }
    }

    return NCP_SUCCESS;
}

int system_process_event(uint8_t *res)
{
    int ret                        = -NCP_FAIL;
    MCU_NCPCmd_DS_SYS_COMMAND *evt = (MCU_NCPCmd_DS_SYS_COMMAND *)res;

    switch (evt->header.cmd)
    {
        case NCP_EVENT_MCU_SLEEP_ENTER:
        case NCP_EVENT_MCU_SLEEP_EXIT:
            ret = ncp_process_sleep_status(res);
            break;
        default:
            PRINTF("Invaild event!\r\n");
            break;
    }
    return ret;
}

/**
 * @brief       This function processes response from ncp device
 *
 * @param res   A pointer to uint8_t
 * @return      Status returned
 */
int system_process_response(uint8_t *res)
{
    int ret                            = -NCP_FAIL;
    MCU_NCPCmd_DS_SYS_COMMAND *cmd_res = (MCU_NCPCmd_DS_SYS_COMMAND *)res;
    switch (cmd_res->header.cmd)
    {
        case NCP_RSP_SYSTEM_CONFIG_SET:
            ret = ncp_process_set_cfg_response(res);
            break;
        case NCP_RSP_SYSTEM_CONFIG_GET:
            ret = ncp_process_get_cfg_response(res);
            break;
        case NCP_CMD_SYSTEM_TEST_LOOPBACK:
            ret = ncp_process_test_loopback_response(res);
            break;
        case NCP_RSP_SYSTEM_POWERMGMT_WAKE_CFG:
            ret = ncp_process_wake_cfg_response(res);
            break;
        case NCP_RSP_SYSTEM_POWERMGMT_MCU_SLEEP:
            ret = ncp_process_mcu_sleep_response(res);
            break;
        case NCP_RSP_SYSTEM_POWERMGMT_WAKEUP_HOST:
            ret = ncp_process_wakeup_host_response(res);
            break;
        default:
            ncp_e("Invaild response cmd!");
            break;
    }
    return ret;
}

/**
 * @brief      command list
 *
 */
static struct ncp_host_cli_command ncp_host_app_cli_commands_system[] = {
#if !(COFNIG_NCP_SDIO_TEST_LOOPBACK)
    {"ncp-set", "<module_name> <variable_name> <value>", ncp_set_command},
    {"ncp-get", "<module_name> <variable_name>", ncp_get_command},
#else
    {"test-loopback", NULL, ncp_test_loopback_command},
#endif
    {"ncp-wake-cfg", NULL, ncp_wake_cfg_command},
    {"ncp-mcu-sleep", NULL, ncp_mcu_sleep_command},
    {"ncp-wakeup-host", NULL, ncp_wakeup_host_command},
    {"ncp-get-mcu-sleep-config", NULL, wlan_get_mcu_sleep_conf_command},
#if CONFIG_NCP_USB
    {"ncp-usb-pm-cfg", "<1/2>", usb_pm_cfg},
#endif
};

/**
 * @brief      Register ncp_host_cli commands
 *
 * @return     Status returned
 */
int ncp_host_system_command_init()
{
    if (ncp_host_cli_register_commands(ncp_host_app_cli_commands_system,
                                       sizeof(ncp_host_app_cli_commands_system) / sizeof(struct ncp_host_cli_command)) != 0)
        return -NCP_FAIL;

    return NCP_SUCCESS;
}
