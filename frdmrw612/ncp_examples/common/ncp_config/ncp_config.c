/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 *
 * This file is used to provide flash save/load API for wifi in little endian, now by Little FS.
 * There are four ways to access flash.
 * 1. Add system/wlan/prov config to access common config files.
 * 2. Use wifi_save_file/wifi_load_file APIs to save/load a new file.
 *    Please be noted that wifi_save_file will overrwrite old file.
 * 3. Implement new API using lfs_save_file/lfs_load_file APIs.
 * 4. Implement new API using opensource LittleFS system APIs.
 *
 * Flash operations will affect performance.
 * Please do not put it in performance sensitive places.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#if !(defined(__ARMCC_VERSION) || defined(__ICCARM__))
#include <strings.h>
#endif
#include <wm_net.h>
#include "ncp_config.h"
#include "littlefs_adapter.h"
#include "wlan.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
const wifi_flash_table_type_t g_wifi_flash_table_system[SYS_MAX_TYPE] = {
    {SYS_PM_EXT_GPIO_NAME, SYS_PM_EXT_GPIO_OFT, SYS_PM_EXT_GPIO_MAX_LEN},
    {SYS_PM_EXT_GPIO_LEVEL_NAME, SYS_PM_EXT_GPIO_LEVEL_OFT, SYS_PM_EXT_GPIO_LEVEL_MAX_LEN},
    {SYS_PROTO_UART_BAUDRATE_NAME, SYS_PROTO_UART_BAUDRATE_OFT, SYS_PROTO_UART_BAUDRATE_MAX_LEN},
    {SYS_PROTO_UART_PARITY_NAME, SYS_PROTO_UART_PARITY_OFT, SYS_PROTO_UART_PARITY_MAX_LEN},
    {SYS_PROTO_UART_STOPBITS_NAME, SYS_PROTO_UART_STOPBITS_OFT, SYS_PROTO_UART_STOPBITS_MAX_LEN},
    {SYS_PROTO_UART_FLOW_CONTROL_NAME, SYS_PROTO_UART_FLOW_CONTROL_OFT, SYS_PROTO_UART_FLOW_CONTROL_MAX_LEN},
    {SYS_PROTO_UART_ID_NAME, SYS_PROTO_UART_ID_OFT, SYS_PROTO_UART_ID_MAX_LEN},
    {SYS_DEBUG_UART_ID_NAME, SYS_DEBUG_UART_ID_OFT, SYS_DEBUG_UART_ID_MAX_LEN},
    {SYS_IEEE_LISTEN_INTERVAL_NAME, SYS_IEEE_LISTEN_INTERVAL_OFT, SYS_IEEE_LISTEN_INTERVAL_MAX_LEN},
    {SYS_HOST_WAKEUP_GPIO_NO_NAME, SYS_HOST_WAKEUP_GPIO_NO_OFT, SYS_HOST_WAKEUP_GPIO_NO_MAX_LEN},
    {SYS_HOST_WAKEUP_GPIO_LEVEL_NAME, SYS_HOST_WAKEUP_GPIO_LEVEL_OFT, SYS_HOST_WAKEUP_GPIO_LEVEL_MAX_LEN},
    {SYS_HOST_WAKEUP_DELAY_NAME, SYS_HOST_WAKEUP_DELAY_OFT, SYS_HOST_WAKEUP_DELAY_MAX_LEN},
    {SYS_CONFIG_NVM_NAME, SYS_CONFIG_NVM_OFT, SYS_CONFIG_NVM_MAX_LEN}};

const wifi_flash_table_type_t g_wifi_flash_table_provision[PROV_MAX_TYPE] = {
    {PROV_SSID_NAME, PROV_SSID_OFT, PROV_SSID_MAX_LEN},
    {PROV_SECURITY_NAME, PROV_SECURITY_OFT, PROV_SECURITY_MAX_LEN},
    {PROV_PSK_NAME, PROV_PSK_OFT, PROV_PSK_MAX_LEN},
    {PROV_PASSPHRASE_NAME, PROV_PASSPHRASE_OFT, PROV_PASSPHRASE_MAX_LEN},
    {PROV_MDNS_ENABLED_NAME, PROV_MDNS_ENABLED_OFT, PROV_MDNS_ENABLED_MAX_LEN},
    {PROV_HOST_NAME_NAME, PROV_HOST_NAME_OFT, PROV_HOST_NAME_MAX_LEN}};

const wifi_flash_table_type_t g_wifi_flash_table_wlan_sta[WLAN_STA_MAX_TYPE] = {
    {WLAN_MAC_NAME, WLAN_MAC_OFT, WLAN_MAC_MAX_LEN},
    {WLAN_CONFIGURED_NAME, WLAN_CONFIGURED_OFT, WLAN_CONFIGURED_MAX_LEN},
    {WLAN_SSID_NAME, WLAN_SSID_OFT, WLAN_SSID_MAX_LEN},
    {WLAN_BSSID_NAME, WLAN_BSSID_OFT, WLAN_BSSID_MAX_LEN},
    {WLAN_CHANNEL_NAME, WLAN_CHANNEL_OFT, WLAN_CHANNEL_MAX_LEN},
    {WLAN_SECURITY_NAME, WLAN_SECURITY_OFT, WLAN_SECURITY_MAX_LEN},
    {WLAN_PSK_NAME, WLAN_PSK_OFT, WLAN_PSK_MAX_LEN},
    {WLAN_PASSPHRASE_NAME, WLAN_PASSPHRASE_OFT, WLAN_PASSPHRASE_MAX_LEN},
    {WLAN_MFPC_NAME, WLAN_MFPC_OFT, WLAN_MFPC_MAX_LEN},
    {WLAN_MFPR_NAME, WLAN_MFPR_OFT, WLAN_MFPR_MAX_LEN},
    {WLAN_PWE_NAME, WLAN_PWE_OFT, WLAN_PWE_MAX_LEN},
    {WLAN_ANONYMOUS_IDENTITY_NAME, WLAN_ANONYMOUS_IDENTITY_OFT, WLAN_ANONYMOUS_IDENTITY_MAX_LEN},
    {WLAN_CLIENT_KEY_PASSWD_NAME, WLAN_CLIENT_KEY_PASSWD_OFT, WLAN_CLIENT_KEY_PASSWD_MAX_LEN},
    {WLAN_IP_ADDR_TYPE_NAME, WLAN_IP_ADDR_TYPE_OFT, WLAN_IP_ADDR_TYPE_MAX_LEN},
    {WLAN_IP_ADDR_NAME, WLAN_IP_ADDR_OFT, WLAN_IP_ADDR_MAX_LEN},
    {WLAN_NETMASK_NAME, WLAN_NETMASK_OFT, WLAN_NETMASK_MAX_LEN},
    {WLAN_GATEWAY_NAME, WLAN_GATEWAY_OFT, WLAN_GATEWAY_MAX_LEN},
    {WLAN_DNS1_NAME, WLAN_DNS1_OFT, WLAN_DNS1_MAX_LEN},
    {WLAN_DNS2_NAME, WLAN_DNS2_OFT, WLAN_DNS2_MAX_LEN},
    {WLAN_RECONNECT_ATTEMPTS_NAME, WLAN_RECONNECT_ATTEMPTS_OFT, WLAN_RECONNECT_ATTEMPTS_MAX_LEN},
    {WLAN_RECONNECT_DELAY_NAME, WLAN_RECONNECT_DELAY_OFT, WLAN_RECONNECT_DELAY_MAX_LEN},
    {WLAN_CHK_SERVER_CERT_NAME, WLAN_CHK_SERVER_CERT_OFT, WLAN_CHK_SERVER_CERT_MAX_LEN},
    {WLAN_REGION_CODE_NAME, WLAN_REGION_CODE_OFT, WLAN_REGION_CODE_MAX_LEN},
    {WLAN_PROFILE_NAME, WLAN_PROFILE_NAME_OFT, WLAN_PROFILE_NAME_MAX_LEN},
    {WLAN_STATUS_NAME, WLAN_STATUS_OFT, WLAN_STATUS_MAX_LEN},
    {WLAN_ROLE_NAME, WLAN_ROLE_OFT, WLAN_ROLE_MAX_LEN}};

const wifi_flash_table_type_t g_wifi_flash_table_wlan_uap[WLAN_UAP_MAX_TYPE] = {
    {WLAN_MAC_NAME, WLAN_MAC_OFT, WLAN_MAC_MAX_LEN},
    {WLAN_CONFIGURED_NAME, WLAN_CONFIGURED_OFT, WLAN_CONFIGURED_MAX_LEN},
    {WLAN_SSID_NAME, WLAN_SSID_OFT, WLAN_SSID_MAX_LEN},
    {WLAN_BSSID_NAME, WLAN_BSSID_OFT, WLAN_BSSID_MAX_LEN},
    {WLAN_CHANNEL_NAME, WLAN_CHANNEL_OFT, WLAN_CHANNEL_MAX_LEN},
    {WLAN_SECURITY_NAME, WLAN_SECURITY_OFT, WLAN_SECURITY_MAX_LEN},
    {WLAN_PSK_NAME, WLAN_PSK_OFT, WLAN_PSK_MAX_LEN},
    {WLAN_PASSPHRASE_NAME, WLAN_PASSPHRASE_OFT, WLAN_PASSPHRASE_MAX_LEN},
    {WLAN_MFPC_NAME, WLAN_MFPC_OFT, WLAN_MFPC_MAX_LEN},
    {WLAN_MFPR_NAME, WLAN_MFPR_OFT, WLAN_MFPR_MAX_LEN},
    {WLAN_PWE_NAME, WLAN_PWE_OFT, WLAN_PWE_MAX_LEN},
    {WLAN_ANONYMOUS_IDENTITY_NAME, WLAN_ANONYMOUS_IDENTITY_OFT, WLAN_ANONYMOUS_IDENTITY_MAX_LEN},
    {WLAN_CLIENT_KEY_PASSWD_NAME, WLAN_CLIENT_KEY_PASSWD_OFT, WLAN_CLIENT_KEY_PASSWD_MAX_LEN},
    {WLAN_IP_ADDR_TYPE_NAME, WLAN_IP_ADDR_TYPE_OFT, WLAN_IP_ADDR_TYPE_MAX_LEN},
    {WLAN_IP_ADDR_NAME, WLAN_IP_ADDR_OFT, WLAN_IP_ADDR_MAX_LEN},
    {WLAN_NETMASK_NAME, WLAN_NETMASK_OFT, WLAN_NETMASK_MAX_LEN},
    {WLAN_GATEWAY_NAME, WLAN_GATEWAY_OFT, WLAN_GATEWAY_MAX_LEN},
    {WLAN_DNS1_NAME, WLAN_DNS1_OFT, WLAN_DNS1_MAX_LEN},
    {WLAN_DNS2_NAME, WLAN_DNS2_OFT, WLAN_DNS2_MAX_LEN},
    {WLAN_RECONNECT_ATTEMPTS_NAME, WLAN_RECONNECT_ATTEMPTS_OFT, WLAN_RECONNECT_ATTEMPTS_MAX_LEN},
    {WLAN_RECONNECT_DELAY_NAME, WLAN_RECONNECT_DELAY_OFT, WLAN_RECONNECT_DELAY_MAX_LEN},
    {WLAN_CHK_SERVER_CERT_NAME, WLAN_CHK_SERVER_CERT_OFT, WLAN_CHK_SERVER_CERT_MAX_LEN},
    {WLAN_REGION_CODE_NAME, WLAN_REGION_CODE_OFT, WLAN_REGION_CODE_MAX_LEN},
    {WLAN_PROFILE_NAME, WLAN_PROFILE_NAME_OFT, WLAN_PROFILE_NAME_MAX_LEN},
    {WLAN_STATUS_NAME, WLAN_STATUS_OFT, WLAN_STATUS_MAX_LEN},
    {WLAN_ROLE_NAME, WLAN_ROLE_OFT, WLAN_ROLE_MAX_LEN},
    {WLAN_CAPA_NAME, WLAN_CAPA_OFT, WLAN_CAPA_MAX_LEN},
    {WLAN_DTIM_NAME, WLAN_DTIM_OFT, WLAN_DTIM_MAX_LEN},
    {WLAN_ACS_BAND_NAME, WLAN_ACS_BAND_OFT, WLAN_ACS_BAND_MAX_LEN}};

static struct ncp_conf_t ncp_conf_list[] = {{"sys", g_wifi_flash_table_system, SYS_MAX_TYPE},
                                            {"prov", g_wifi_flash_table_provision, PROV_MAX_TYPE},
                                            {"wlan_bssA", g_wifi_flash_table_wlan_uap, WLAN_UAP_MAX_TYPE},
                                            {"wlan_bssB", g_wifi_flash_table_wlan_uap, WLAN_UAP_MAX_TYPE},
                                            {"wlan_bssC", g_wifi_flash_table_wlan_uap, WLAN_UAP_MAX_TYPE},
                                            {"wlan_bssD", g_wifi_flash_table_wlan_uap, WLAN_UAP_MAX_TYPE},
                                            {"wlan_bssE", g_wifi_flash_table_wlan_uap, WLAN_UAP_MAX_TYPE},
                                            {NULL, NULL, 0}};

extern int lfs_mounted;
extern lfs_t lfs;

#define wifi_flash_check_rw_ret(val)                                           \
    do                                                                         \
    {                                                                          \
        if (val < 0)                                                           \
        {                                                                      \
            flash_log_e("Line:%d read write file fail res %d", __LINE__, val); \
            goto done;                                                         \
        }                                                                      \
    } while (0)

#define ENABLE_NVM  1
#define DISABLE_NVM 0

/* Enabling NVM means using LittleFS to save configuration information. */
static uint8_t is_nvm_enable = ENABLE_NVM;

char *wifi_bss_config_file[5] = {WLAN_BSS1_CONFIG_FILE_PATH, WLAN_BSS2_CONFIG_FILE_PATH, WLAN_BSS3_CONFIG_FILE_PATH, WLAN_BSS4_CONFIG_FILE_PATH, WLAN_BSS5_CONFIG_FILE_PATH};

wifi_bss_config wifi_lfs_bss_config[5];

/*******************************************************************************
 * Code
 ******************************************************************************/
static struct ncp_conf_t *lookup_conf(struct ncp_conf_t *conf, const char *conf_name)
{
    int i;
    for (i = 0; conf[i].flash_table && conf[i].max_type; i++)
    {
        if (!strcasecmp(conf_name, conf[i].name))
        {
            return &conf[i];
        }
    }
    return NULL;
}

/* wifi save specific config to file, check config name first
 * return WM_SUCCESS on success, other num on fail
 */
static int wifi_save_config(const wifi_flash_table_type_t *flash_type, lfs_file_t *file, const uint8_t *buf, int len)
{
    int res;
    char rd_name[32] = {0};
    int name_len;

    if (file == NULL || (file->flags & LFS_O_RDWR) != LFS_O_RDWR)
    {
        flash_log_e("%s null file or permission not read write", __func__);
        return -WM_E_INVAL;
    }

    res = lfs_file_seek(&lfs, file, flash_type->offset, LFS_SEEK_SET);
    if (res != flash_type->offset)
    {
        flash_log_e("%s seek file fail offset %d, res %d", __func__, flash_type->offset, res);
        return -WM_E_NOSPC;
    }

    name_len = strlen(flash_type->name);
    res      = lfs_file_read(&lfs, file, rd_name, name_len);
    if (res != name_len)
    {
        flash_log_e("%s read file fail offset %d, res %d", __func__, flash_type->offset, res);
        return -WM_E_NOSPC;
    }

    if (memcmp(rd_name, flash_type->name, name_len))
    {
        flash_log_e("%s config offset %d not match, file read[%s], should be[%s]", __func__, flash_type->offset,
                    rd_name, flash_type->name);
        return -WM_FAIL;
    }

    res = lfs_file_write(&lfs, file, buf, len);
    if (res != len)
    {
        flash_log_e("%s config offset %d write bytes not match, write %d, len %d", __func__, flash_type->offset, res,
                    len);
        return -WM_E_NOSPC;
    }

    return WM_SUCCESS;
}

/* wifi load specific config from file, check config name first
 * return WM_SUCCESS on success, other num on fail
 */
static int wifi_load_config(const wifi_flash_table_type_t *flash_type, lfs_file_t *file, uint8_t *buf, int len)
{
    int res;
    char rd_name[32] = {0};
    int name_len;

    if (file == NULL)
    {
        flash_log_e("%s null file", __func__);
        return -WM_E_INVAL;
    }

    res = lfs_file_seek(&lfs, file, flash_type->offset, LFS_SEEK_SET);
    if (res != flash_type->offset)
    {
        flash_log_e("%s seek file %s fail offset %d, res %d", __func__, flash_type->offset, res);
        return -WM_FAIL;
    }

    name_len = strlen(flash_type->name);
    res      = lfs_file_read(&lfs, file, rd_name, name_len);
    if (res != name_len)
    {
        flash_log_e("%s read file %s fail offset %d, res %d", __func__, flash_type->offset, res);
        return -WM_FAIL;
    }

    if (memcmp(rd_name, flash_type->name, name_len))
    {
        flash_log_e("%s config offset %d not match, file read[%s], should be[%s]", __func__, flash_type->offset,
                    rd_name, flash_type->name);
        return -WM_FAIL;
    }

    res = lfs_file_read(&lfs, file, buf, len);
    if (res < 0)
    {
        flash_log_e("%s config offset %d read fail res %d", __func__, flash_type->offset, res, len);
        return -WM_E_NOSPC;
    }

    return WM_SUCCESS;
}

/* wifi save system config
 * return WM_SUCCESS on success, other num on fail
 */
int wifi_save_system_config(lfs_file_t *file, int type, void *buf, int len)
{
    int res;
    const wifi_flash_table_type_t *flash_type = NULL;

    if (type < 0 || type >= SYS_MAX_TYPE)
    {
        flash_log_e("%s invalid type %d", __func__, type);
        return -WM_E_INVAL;
    }

    if (buf == NULL)
    {
        flash_log_e("%s null buff", __func__);
        return -WM_E_INVAL;
    }

    flash_type = &g_wifi_flash_table_system[type];
    if (flash_type->len < len)
        len = flash_type->len;

    res = wifi_save_config(flash_type, file, buf, len);
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s save config type %d fail res %d", __func__, type, res);
        return res;
    }
    return WM_SUCCESS;
}

/* wifi load system config
 * return WM_SUCCESS on success, other num on fail
 */
int wifi_load_system_config(lfs_file_t *file, int type, void *buf, int len)
{
    int res;
    const wifi_flash_table_type_t *flash_type = NULL;

    if (type < 0 || type >= SYS_MAX_TYPE)
    {
        flash_log_e("%s invalid type %d", __func__, type);
        return -WM_E_INVAL;
    }

    if (buf == NULL)
    {
        flash_log_e("%s null buff", __func__);
        return -WM_E_INVAL;
    }

    flash_type = &g_wifi_flash_table_system[type];
    if (flash_type->len < len)
        len = flash_type->len;

    res = wifi_load_config(flash_type, file, buf, len);
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s load config type %d fail res %d", __func__, type, res);
        return res;
    }
    return WM_SUCCESS;
}

/* wifi save provision config
 * return WM_SUCCESS on success, other num on fail
 */
int wifi_save_provision_config(lfs_file_t *file, int type, void *buf, int len)
{
    int res;
    const wifi_flash_table_type_t *flash_type = NULL;

    if (type < 0 || type >= PROV_MAX_TYPE)
    {
        flash_log_e("%s invalid type %d", __func__, type);
        return -WM_E_INVAL;
    }

    if (buf == NULL)
    {
        flash_log_e("%s null buff", __func__);
        return -WM_E_INVAL;
    }

    flash_type = &g_wifi_flash_table_provision[type];
    if (flash_type->len < len)
        len = flash_type->len;

    res = wifi_save_config(flash_type, file, buf, len);
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s save config type %d fail res %d", __func__, type, res);
        return res;
    }
    return WM_SUCCESS;
}

/* wifi load provision config
 * return WM_SUCCESS on success, other num on fail
 */
int wifi_load_provision_config(lfs_file_t *file, int type, void *buf, int len)
{
    int res;
    const wifi_flash_table_type_t *flash_type = NULL;

    if (type < 0 || type >= PROV_MAX_TYPE)
    {
        flash_log_e("%s invalid type %d", __func__, type);
        return -WM_E_INVAL;
    }

    if (buf == NULL)
    {
        flash_log_e("%s null buff", __func__);
        return -WM_E_INVAL;
    }

    flash_type = &g_wifi_flash_table_provision[type];
    if (flash_type->len < len)
        len = flash_type->len;

    res = wifi_load_config(flash_type, file, buf, len);
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s load config type %d fail res %d", __func__, type, res);
        return res;
    }
    return WM_SUCCESS;
}

/* wifi save wlan station config
 * return WM_SUCCESS on success, other num on fail
 */
int wifi_save_wlan_sta_config(lfs_file_t *file, int type, void *buf, int len)
{
    int res;
    const wifi_flash_table_type_t *flash_type = NULL;

    if (type < 0 || type >= WLAN_STA_MAX_TYPE)
    {
        flash_log_e("%s invalid type %d", __func__, type);
        return -WM_E_INVAL;
    }

    if (buf == NULL)
    {
        flash_log_e("%s null buff", __func__);
        return -WM_E_INVAL;
    }

    flash_type = &g_wifi_flash_table_wlan_sta[type];
    if (flash_type->len < len)
        len = flash_type->len;

    res = wifi_save_config(flash_type, file, buf, len);
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s save config type %d fail res %d", __func__, type, res);
        return res;
    }
    return WM_SUCCESS;
}

/* wifi load wlan station config
 * return WM_SUCCESS on success, other num on fail
 */
int wifi_load_wlan_sta_config(lfs_file_t *file, int type, void *buf, int len)
{
    int res;
    const wifi_flash_table_type_t *flash_type = NULL;

    if (type < 0 || type >= WLAN_STA_MAX_TYPE)
    {
        flash_log_e("%s invalid type %d", __func__, type);
        return -WM_E_INVAL;
    }

    if (buf == NULL)
    {
        flash_log_e("%s null buff", __func__);
        return -WM_E_INVAL;
    }

    flash_type = &g_wifi_flash_table_wlan_sta[type];
    if (flash_type->len < len)
        len = flash_type->len;

    res = wifi_load_config(flash_type, file, buf, len);
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s load config type %d fail res %d", __func__, type, res);
        return res;
    }
    return WM_SUCCESS;
}

/* wifi save wlan uap config
 * return WM_SUCCESS on success, other num on fail
 */
int wifi_save_wlan_uap_config(lfs_file_t *file, int type, void *buf, int len)
{
    int res;
    const wifi_flash_table_type_t *flash_type = NULL;

    if (type < 0 || type >= WLAN_UAP_MAX_TYPE)
    {
        flash_log_e("%s invalid type %d", __func__, type);
        return -WM_E_INVAL;
    }

    if (buf == NULL)
    {
        flash_log_e("%s null buff", __func__);
        return -WM_E_INVAL;
    }

    flash_type = &g_wifi_flash_table_wlan_uap[type];
    if (flash_type->len < len)
        len = flash_type->len;

    res = wifi_save_config(flash_type, file, buf, len);
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s save config type %d fail res %d", __func__, type, res);
        return res;
    }
    return WM_SUCCESS;
}

/* wifi load wlan uAP config
 * return WM_SUCCESS on success, other num on fail
 */
int wifi_load_wlan_uap_config(lfs_file_t *file, int type, void *buf, int len)
{
    int res;
    const wifi_flash_table_type_t *flash_type = NULL;

    if (type < 0 || type >= WLAN_UAP_MAX_TYPE)
    {
        flash_log_e("%s invalid type %d", __func__, type);
        return -WM_E_INVAL;
    }

    if (buf == NULL)
    {
        flash_log_e("%s null buff", __func__);
        return -WM_E_INVAL;
    }

    flash_type = &g_wifi_flash_table_wlan_uap[type];
    if (flash_type->len < len)
        len = flash_type->len;

    res = wifi_load_config(flash_type, file, buf, len);
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s load config type %d fail res %d", __func__, type, res);
        return res;
    }
    return WM_SUCCESS;
}

/* Example of wifi overwrite raw file, create one if not exist.
 * Will overwrite old file!
 * return actual write bytes on success, less than 0 on fail
 */
int wifi_save_file(char *path, void *buf, int len)
{
    int res;

    if (path == NULL || buf == NULL || len <= 0)
    {
        flash_log_e("%s invalid argument", __func__);
        return -WM_E_INVAL;
    }

    res = lfs_save_file(path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC, 0, buf, len);
    if (res < 0)
    {
        flash_log_e("%s lfs_save_file fail res %d", __func__, res);
        return -WM_FAIL;
    }

    return res;
}

/* Example of wifi load raw file
 * return actual read bytes on success, less than 0 on fail
 */
int wifi_load_file(char *path, void *buf, int len)
{
    int res;

    if (path == NULL || buf == NULL || len <= 0)
    {
        flash_log_e("%s invalid argument", __func__);
        return -WM_E_INVAL;
    }

    res = lfs_load_file(path, LFS_O_RDONLY, 0, buf, len);
    if (res < 0)
    {
        flash_log_e("%s lfs_load_file fail res %d", __func__, res);
        return -WM_FAIL;
    }

    return res;
}

/* try to read file "/etc/sys_conf", if not exist, create one and set default config */
static int wifi_save_default_config_sys(void)
{
    int res;
    int ret = -WM_FAIL;
    lfs_file_t file;
    uint8_t buf[5] = {0};

    res = lfs_file_open(&lfs, &file, SYS_CONFIG_FILE_PATH, LFS_O_RDONLY);
    if (res == 0)
    {
        res = lfs_file_close(&lfs, &file);
        if (res != 0)
        {
            flash_log_e("%s close file %s fail res %d", __func__, SYS_CONFIG_FILE_PATH, res);
            return -WM_FAIL;
        }
        return WM_SUCCESS;
    }

    flash_log_w("%s %s not exist, one-shot initialize default config", __func__, SYS_CONFIG_FILE_PATH);

    res = lfs_file_open(&lfs, &file, SYS_CONFIG_FILE_PATH, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (res != 0)
    {
        flash_log_e("%s open file %s fail res %d", __func__, SYS_CONFIG_FILE_PATH, res);
        return -WM_E_NOENT;
    }

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, SYS_PM_EXT_GPIO_NAME, SYS_PM_EXT_GPIO_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_PM_EXT_GPIO_DEF, strlen(SYS_PM_EXT_GPIO_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, SYS_PM_EXT_GPIO_MAX_LEN - strlen(SYS_PM_EXT_GPIO_DEF));
    wifi_flash_check_rw_ret(res);

    res = lfs_file_write(&lfs, &file, SYS_PM_EXT_GPIO_LEVEL_NAME, SYS_PM_EXT_GPIO_LEVEL_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_PM_EXT_GPIO_LEVEL_DEF, SYS_PM_EXT_GPIO_LEVEL_MAX_LEN);
    wifi_flash_check_rw_ret(res);

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, SYS_PROTO_UART_BAUDRATE_NAME, SYS_PROTO_UART_BAUDRATE_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_PROTO_UART_BAUDRATE_DEF, strlen(SYS_PROTO_UART_BAUDRATE_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, SYS_PROTO_UART_BAUDRATE_MAX_LEN - strlen(SYS_PROTO_UART_BAUDRATE_DEF));
    wifi_flash_check_rw_ret(res);

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, SYS_PROTO_UART_PARITY_NAME, SYS_PROTO_UART_PARITY_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_PROTO_UART_PARITY_DEF, strlen(SYS_PROTO_UART_PARITY_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, SYS_PROTO_UART_PARITY_MAX_LEN - strlen(SYS_PROTO_UART_PARITY_DEF));
    wifi_flash_check_rw_ret(res);

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, SYS_PROTO_UART_STOPBITS_NAME, SYS_PROTO_UART_STOPBITS_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_PROTO_UART_STOPBITS_DEF, strlen(SYS_PROTO_UART_STOPBITS_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, SYS_PROTO_UART_STOPBITS_MAX_LEN - strlen(SYS_PROTO_UART_STOPBITS_DEF));
    wifi_flash_check_rw_ret(res);

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, SYS_PROTO_UART_FLOW_CONTROL_NAME, SYS_PROTO_UART_FLOW_CONTROL_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_PROTO_UART_FLOW_CONTROL_DEF, strlen(SYS_PROTO_UART_FLOW_CONTROL_DEF));
    wifi_flash_check_rw_ret(res);
    res =
        lfs_file_write(&lfs, &file, buf, SYS_PROTO_UART_FLOW_CONTROL_MAX_LEN - strlen(SYS_PROTO_UART_FLOW_CONTROL_DEF));
    wifi_flash_check_rw_ret(res);

    res = lfs_file_write(&lfs, &file, SYS_PROTO_UART_ID_NAME, SYS_PROTO_UART_ID_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_PROTO_UART_ID_DEF, SYS_PROTO_UART_ID_MAX_LEN);
    wifi_flash_check_rw_ret(res);

    res = lfs_file_write(&lfs, &file, SYS_DEBUG_UART_ID_NAME, SYS_DEBUG_UART_ID_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_DEBUG_UART_ID_DEF, SYS_DEBUG_UART_ID_MAX_LEN);
    wifi_flash_check_rw_ret(res);

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, SYS_IEEE_LISTEN_INTERVAL_NAME, SYS_IEEE_LISTEN_INTERVAL_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_IEEE_LISTEN_INTERVAL_DEF, strlen(SYS_IEEE_LISTEN_INTERVAL_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, SYS_IEEE_LISTEN_INTERVAL_MAX_LEN - strlen(SYS_IEEE_LISTEN_INTERVAL_DEF));
    wifi_flash_check_rw_ret(res);

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, SYS_HOST_WAKEUP_GPIO_NO_NAME, SYS_HOST_WAKEUP_GPIO_NO_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_HOST_WAKEUP_GPIO_NO_DEF, strlen(SYS_HOST_WAKEUP_GPIO_NO_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, SYS_HOST_WAKEUP_GPIO_NO_MAX_LEN - strlen(SYS_HOST_WAKEUP_GPIO_NO_DEF));
    wifi_flash_check_rw_ret(res);

    res = lfs_file_write(&lfs, &file, SYS_HOST_WAKEUP_GPIO_LEVEL_NAME, SYS_HOST_WAKEUP_GPIO_LEVEL_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_HOST_WAKEUP_GPIO_LEVEL_DEF, SYS_HOST_WAKEUP_GPIO_LEVEL_MAX_LEN);
    wifi_flash_check_rw_ret(res);

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, SYS_HOST_WAKEUP_DELAY_NAME, SYS_HOST_WAKEUP_DELAY_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_HOST_WAKEUP_DELAY_DEF, strlen(SYS_HOST_WAKEUP_DELAY_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, SYS_HOST_WAKEUP_DELAY_MAX_LEN - strlen(SYS_HOST_WAKEUP_DELAY_DEF));
    wifi_flash_check_rw_ret(res);

    res = lfs_file_write(&lfs, &file, SYS_CONFIG_NVM_NAME, SYS_CONFIG_NVM_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, SYS_CONFIG_NVM_DEF, SYS_CONFIG_NVM_MAX_LEN);
    wifi_flash_check_rw_ret(res);

    ret = WM_SUCCESS;
done:
    res = lfs_file_close(&lfs, &file);
    if (res != 0)
    {
        flash_log_e("%s close file %s fail res %d", __func__, SYS_CONFIG_FILE_PATH, res);
        return -WM_FAIL;
    }
    return ret;
}

/* try to read file "/etc/prov_conf", if not exist, create one and set default config */
static int wifi_save_default_config_prov(void)
{
    int res;
    int ret = -WM_FAIL;
    lfs_file_t file;
    uint8_t buf[256] = {0};

    res = lfs_file_open(&lfs, &file, PROV_CONFIG_FILE_PATH, LFS_O_RDONLY);
    if (res == 0)
    {
        res = lfs_file_close(&lfs, &file);
        if (res != 0)
        {
            flash_log_e("%s close file %s fail res %d", __func__, PROV_CONFIG_FILE_PATH, res);
            return -WM_FAIL;
        }
        return WM_SUCCESS;
    }

    flash_log_w("%s %s not exist, one-shot initialize default config", __func__, PROV_CONFIG_FILE_PATH);

    res = lfs_file_open(&lfs, &file, PROV_CONFIG_FILE_PATH, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (res != 0)
    {
        flash_log_e("%s open file %s fail res %d", __func__, PROV_CONFIG_FILE_PATH, res);
        return -WM_E_NOENT;
    }

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, PROV_SSID_NAME, PROV_SSID_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, PROV_SSID_DEF, strlen(PROV_SSID_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, PROV_SSID_MAX_LEN - strlen(PROV_SSID_DEF));
    wifi_flash_check_rw_ret(res);

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, PROV_SECURITY_NAME, PROV_SECURITY_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, PROV_SECURITY_DEF, strlen(PROV_SECURITY_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, PROV_SECURITY_MAX_LEN - strlen(PROV_SECURITY_DEF));
    wifi_flash_check_rw_ret(res);

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, PROV_PSK_NAME, PROV_PSK_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, PROV_PSK_DEF, strlen(PROV_PSK_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, PROV_PSK_MAX_LEN - strlen(PROV_PSK_DEF));
    wifi_flash_check_rw_ret(res);

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, PROV_PASSPHRASE_NAME, PROV_PASSPHRASE_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, PROV_PASSPHRASE_DEF, strlen(PROV_PASSPHRASE_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, PROV_PASSPHRASE_MAX_LEN - strlen(PROV_PASSPHRASE_DEF));
    wifi_flash_check_rw_ret(res);

    res = lfs_file_write(&lfs, &file, PROV_MDNS_ENABLED_NAME, PROV_MDNS_ENABLED_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, PROV_MDNS_ENABLED_DEF, PROV_MDNS_ENABLED_MAX_LEN);
    wifi_flash_check_rw_ret(res);

    /* default string value length less than its max length, padding with 0 */
    res = lfs_file_write(&lfs, &file, PROV_HOST_NAME_NAME, PROV_HOST_NAME_NAME_LEN);
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, PROV_HOST_NAME_DEF, strlen(PROV_HOST_NAME_DEF));
    wifi_flash_check_rw_ret(res);
    res = lfs_file_write(&lfs, &file, buf, PROV_HOST_NAME_MAX_LEN - strlen(PROV_HOST_NAME_DEF));
    wifi_flash_check_rw_ret(res);

    ret = WM_SUCCESS;
done:
    res = lfs_file_close(&lfs, &file);
    if (res != 0)
    {
        flash_log_e("%s close file %s fail res %d", __func__, PROV_CONFIG_FILE_PATH, res);
        return -WM_FAIL;
    }
    return ret;
}

/* There are 5 bss profiles which are used to save bss information.
 * try to read file "/etc/wlan_bss*_conf", if not exist, create one and set default config.
 * Initialize bss profile to uap network profile.
*/
static int wifi_save_default_config_wlan_bss(void)
{
    int res, index;
    lfs_file_t file;
    uint8_t buf[256] = {0};
    char * path;

    for(index = 0; index < 5; index++)
    {
        path = wifi_bss_config_file[index];
        res = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
        if (res == 0)
        {
            res = lfs_file_close(&lfs, &file);
            if (res != 0)
            {
                flash_log_e("%s close file %s fail res %d", __func__, path, res);
                return -WM_FAIL;
            }
            continue;
        }

        flash_log_w("%s %s not exist, one-shot initialize default config", __func__, path);

        res = lfs_file_open(&lfs, &file, path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
        if (res != 0)
        {
            flash_log_e("%s open file %s fail res %d", __func__, path, res);
            return -WM_E_NOENT;
        }

        res = lfs_file_write(&lfs, &file, WLAN_MAC_NAME, WLAN_MAC_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_MAC_DEF, WLAN_MAC_MAX_LEN);
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_CONFIGURED_NAME, WLAN_CONFIGURED_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_CONFIGURED_DEF, WLAN_CONFIGURED_MAX_LEN);
        wifi_flash_check_rw_ret(res);

        /* default string value length less than its max length, padding with 0 */
        res = lfs_file_write(&lfs, &file, WLAN_SSID_NAME, WLAN_SSID_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_SSID_DEF, strlen(WLAN_SSID_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_SSID_MAX_LEN - strlen(WLAN_SSID_DEF));
        wifi_flash_check_rw_ret(res);

        /* default bssid 0:0:0:0:0:0 */
        res = lfs_file_write(&lfs, &file, WLAN_BSSID_NAME, WLAN_BSSID_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_BSSID_DEF, WLAN_BSSID_MAX_LEN);
        wifi_flash_check_rw_ret(res);

        /* default string value length less than its max length, padding with 0 */
        res = lfs_file_write(&lfs, &file, WLAN_CHANNEL_NAME, WLAN_CHANNEL_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_CHANNEL_DEF, strlen(WLAN_CHANNEL_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_CHANNEL_MAX_LEN - strlen(WLAN_CHANNEL_DEF));
        wifi_flash_check_rw_ret(res);

        /* default string value length less than its max length, padding with 0 */
        res = lfs_file_write(&lfs, &file, WLAN_SECURITY_NAME, WLAN_SECURITY_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_SECURITY_DEF, strlen(WLAN_SECURITY_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_SECURITY_MAX_LEN - strlen(WLAN_SECURITY_DEF));
        wifi_flash_check_rw_ret(res);

        /* default string value length less than its max length, padding with 0 */
        res = lfs_file_write(&lfs, &file, WLAN_PSK_NAME, WLAN_PSK_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_PSK_DEF, strlen(WLAN_PSK_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_PSK_MAX_LEN - strlen(WLAN_PSK_DEF));
        wifi_flash_check_rw_ret(res);

        /* default string value length less than its max length, padding with 0 */
        res = lfs_file_write(&lfs, &file, WLAN_PASSPHRASE_NAME, WLAN_PASSPHRASE_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_PASSPHRASE_DEF, strlen(WLAN_PASSPHRASE_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_PASSPHRASE_MAX_LEN - strlen(WLAN_PASSPHRASE_DEF));
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_MFPC_NAME, WLAN_MFPC_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_MFPC_DEF, WLAN_MFPC_MAX_LEN);
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_MFPR_NAME, WLAN_MFPR_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_MFPR_DEF, WLAN_MFPR_MAX_LEN);
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_PWE_NAME, WLAN_PWE_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_PWE_DEF, strlen(WLAN_PWE_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_PWE_MAX_LEN - strlen(WLAN_PWE_DEF));
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_ANONYMOUS_IDENTITY_NAME, WLAN_ANONYMOUS_IDENTITY_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_ANONYMOUS_IDENTITY_DEF, strlen(WLAN_ANONYMOUS_IDENTITY_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_ANONYMOUS_IDENTITY_MAX_LEN - strlen(WLAN_ANONYMOUS_IDENTITY_DEF));
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_CLIENT_KEY_PASSWD_NAME, WLAN_CLIENT_KEY_PASSWD_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_CLIENT_KEY_PASSWD_DEF, strlen(WLAN_CLIENT_KEY_PASSWD_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_CLIENT_KEY_PASSWD_MAX_LEN - strlen(WLAN_CLIENT_KEY_PASSWD_DEF));
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_IP_ADDR_TYPE_NAME, WLAN_IP_ADDR_TYPE_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_IP_ADDR_TYPE_DEF, WLAN_IP_ADDR_TYPE_MAX_LEN);
        wifi_flash_check_rw_ret(res);

        /* default string value length less than its max length, padding with 0 */
        res = lfs_file_write(&lfs, &file, WLAN_IP_ADDR_NAME, WLAN_IP_ADDR_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_IP_ADDR_DEF, strlen(WLAN_IP_ADDR_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_IP_ADDR_MAX_LEN - strlen(WLAN_IP_ADDR_DEF));
        wifi_flash_check_rw_ret(res);

        /* default string value length less than its max length, padding with 0 */
        res = lfs_file_write(&lfs, &file, WLAN_NETMASK_NAME, WLAN_NETMASK_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_NETMASK_DEF, strlen(WLAN_NETMASK_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_NETMASK_MAX_LEN - strlen(WLAN_NETMASK_DEF));
        wifi_flash_check_rw_ret(res);

        /* default string value length less than its max length, padding with 0 */
        res = lfs_file_write(&lfs, &file, WLAN_GATEWAY_NAME, WLAN_GATEWAY_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_GATEWAY_DEF, strlen(WLAN_GATEWAY_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_GATEWAY_MAX_LEN - strlen(WLAN_GATEWAY_DEF));
        wifi_flash_check_rw_ret(res);

        /* default string value length less than its max length, padding with 0 */
        res = lfs_file_write(&lfs, &file, WLAN_DNS1_NAME, WLAN_DNS1_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_DNS1_DEF, strlen(WLAN_DNS1_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_DNS1_MAX_LEN - strlen(WLAN_DNS1_DEF));
        wifi_flash_check_rw_ret(res);

        /* default string value length less than its max length, padding with 0 */
        res = lfs_file_write(&lfs, &file, WLAN_DNS2_NAME, WLAN_DNS2_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_DNS2_DEF, strlen(WLAN_DNS2_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_DNS2_MAX_LEN - strlen(WLAN_DNS2_DEF));
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_RECONNECT_ATTEMPTS_NAME, WLAN_RECONNECT_ATTEMPTS_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_RECONNECT_ATTEMPTS_DEF, WLAN_RECONNECT_ATTEMPTS_MAX_LEN);
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_RECONNECT_DELAY_NAME, WLAN_RECONNECT_DELAY_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_RECONNECT_DELAY_DEF, WLAN_RECONNECT_DELAY_MAX_LEN);
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_CHK_SERVER_CERT_NAME, WLAN_CHK_SERVER_CERT_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_CHK_SERVER_CERT_DEF, WLAN_CHK_SERVER_CERT_MAX_LEN);
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_REGION_CODE_NAME, WLAN_REGION_CODE_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_REGION_CODE_DEF, WLAN_REGION_CODE_MAX_LEN);
        wifi_flash_check_rw_ret(res);

        /* default string value length less than its max length, padding with 0 */
        res = lfs_file_write(&lfs, &file, WLAN_PROFILE_NAME, WLAN_PROFILE_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_PROFILE_NAME_DEF, strlen(WLAN_PROFILE_NAME_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_PROFILE_NAME_MAX_LEN - strlen(WLAN_PROFILE_NAME_DEF));
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_STATUS_NAME, WLAN_STATUS_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_STATUS_DEF, strlen(WLAN_STATUS_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_STATUS_MAX_LEN - strlen(WLAN_STATUS_DEF));
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_ROLE_NAME, WLAN_ROLE_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_ROLE_DEF, strlen(WLAN_ROLE_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_ROLE_MAX_LEN - strlen(WLAN_ROLE_DEF));
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_CAPA_NAME, WLAN_CAPA_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_CAPA_DEF, WLAN_CAPA_MAX_LEN);
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_DTIM_NAME, WLAN_DTIM_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_DTIM_DEF, strlen(WLAN_DTIM_DEF));
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, buf, WLAN_DTIM_MAX_LEN - strlen(WLAN_DTIM_DEF));
        wifi_flash_check_rw_ret(res);

        res = lfs_file_write(&lfs, &file, WLAN_ACS_BAND_NAME, WLAN_ACS_BAND_NAME_LEN);
        wifi_flash_check_rw_ret(res);
        res = lfs_file_write(&lfs, &file, WLAN_ACS_BAND_DEF, WLAN_ACS_BAND_MAX_LEN);
        wifi_flash_check_rw_ret(res);
done:
        res = lfs_file_close(&lfs, &file);
        if (res != 0)
        {
            flash_log_e("%s close file %s fail res %d", __func__, path, res);
            return -WM_FAIL;
        }
    }

    return WM_SUCCESS;
}

static int wifi_init_lfs_config()
{
    int res, index;
    lfs_file_t file;
    char * path, status[2];

    for(index = 0; index < WLAN_BSS_MAX_NUM; index++)
    {
        wifi_lfs_bss_config[index].config_path = wifi_bss_config_file[index];
        memset(wifi_lfs_bss_config[index].network_name, 0, 33);

        path = wifi_bss_config_file[index];
        res = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
        if (res != 0)
        {
            flash_log_w("open file %s fail res %d", path, res);
            wifi_lfs_bss_config[index].flag = WLAN_BSS_STATUS_NONAVAILABLE;
            continue;
        }

        res = wifi_load_wlan_sta_config(&file, WLAN_PROFILE, wifi_lfs_bss_config[index].network_name, 32);
        if(res != 0)
        {
            flash_log_e("file %s load profile fail res %d", path, res);
        }

        res = wifi_load_wlan_sta_config(&file, WLAN_STATUS, status, sizeof(status));
        if(res != 0)
        {
            flash_log_e("file %s load status fail res %d", path, res);
        }
        wifi_lfs_bss_config[index].flag = atoi(status);

        res = lfs_file_close(&lfs, &file);
        if (res != 0)
        {
            flash_log_e("%s close file %s fail res %d", __func__, path, res);
            return -WM_FAIL;
        }
    }

    return WM_SUCCESS;
}

/* If a file doesn't exist in a sta config file path, create a new file in this path to save network information.
 * When add a new network, find a file with the same profile name and overwrite it.
 * When add a new network, find the first unavailable network profile and overwrite it.
 */
int wifi_set_network(struct wlan_network *network)
{
    int res, i, index = 0, bss_status;
    int ret = -WM_FAIL;
    char channel[4], security_type[3], mfpc[2], mfpr[2], addr_type[2], pwe[2];
    struct in_addr ip, gw, nm, dns1, dns2;
    char ip_addr_temp[17], configured[2], role[2], status[2];
    char uap_prov_sta_name[] = "uap_prov_sta_label";
    lfs_file_t file;
    char *path;
    wifi_save_wlan_config_fn_t save_config;
    flash_log_d("%s: network->name = %s\r\n",network->name);

    if(network->role == WLAN_BSS_ROLE_STA)
    {
        save_config = wifi_save_wlan_sta_config;
    }
    else
    {
        save_config = wifi_save_wlan_uap_config;
    }

    /*uap prov station network is always saved in */
    if(strcmp(network->name, uap_prov_sta_name) == 0)
    {
        path = wifi_lfs_bss_config[0].config_path;

        res = lfs_file_open(&lfs, &file, path, LFS_O_RDWR | LFS_O_CREAT);
        if (res != 0)
        {
            flash_log_e("create file %s fail res %d", path, res);
            return -WM_FAIL;
        }
        index = 0;
        goto ncp_set_bss;
    }

    /*Network alredy exist in little file system, overwrite it.*/
    for(i = 0; i < WLAN_BSS_MAX_NUM; i++)
    {
        if(strcmp(wifi_lfs_bss_config[i].network_name, network->name) == 0)
        {
            index = i;
            path = wifi_lfs_bss_config[i].config_path;
            flash_log_d("%s: path = %s\r\n",__func__, path);
            res = lfs_file_open(&lfs, &file, path, LFS_O_RDWR);
            if (res != 0)
            {
                flash_log_e("open file %s fail res %d", path, res);
                return -WM_FAIL;
            }
            goto ncp_set_bss;
        }
    }

    /*If there is empty file path, create file in this path.*/
    for(i = 0; i < WLAN_BSS_MAX_NUM; i++)
    {
        path = wifi_lfs_bss_config[i].config_path;
        /* If file exists, can open it with only read and write flag*/
        res = lfs_file_open(&lfs, &file, path, LFS_O_RDWR);
        if (res != 0)
        {
            flash_log_w("open file %s fail res %d", path, res);
            /*Create new file in path to save network*/
            res = lfs_file_open(&lfs, &file, path, LFS_O_RDWR | LFS_O_CREAT);
            if (res != 0)
            {
                flash_log_e("create file %s fail res %d", path, res);
                return -WM_FAIL;
            }
            index = i;
            goto ncp_set_bss;
        }

        flash_log_d("%s: path = %s\r\n",__func__, path);
        res = lfs_file_close(&lfs, &file);
        if (res != 0)
        {
            flash_log_e("%s close file %s fail res %d", __func__, path, res);
            return -WM_FAIL;
        }
    }

    /*Find unavailable network, overwrite it.*/
    for(i = 0; i < WLAN_BSS_MAX_NUM; i++)
    {
        if(wifi_lfs_bss_config[i].flag == WLAN_BSS_STATUS_NONAVAILABLE)
        {
            index = i;
            path = wifi_lfs_bss_config[i].config_path;
            flash_log_d("%s: path = %s\r\n",__func__, path);
            res = lfs_file_open(&lfs, &file, path, LFS_O_RDWR);
            if (res != 0)
            {
                flash_log_e("open file %s fail res %d", path, res);
                return -WM_FAIL;
            }
            break;
        }
    }

    if(i == WLAN_BSS_MAX_NUM)
    {
        flash_log_e("can't find unavailable bss profile to overwrite it.");
        return -WM_FAIL;
    }

ncp_set_bss:
    res = save_config(&file, WLAN_PROFILE, network->name, sizeof(network->name));
    res += save_config(&file, WLAN_SSID, network->ssid, sizeof(network->ssid));
    res += save_config(&file, WLAN_BSSID, network->bssid, sizeof(network->bssid));
    snprintf(channel, sizeof(channel), "%u", network->channel);
    res += save_config(&file, WLAN_CHANNEL, channel, sizeof(channel));
    snprintf(security_type, sizeof(security_type), "%u", network->security.type);
    res += save_config(&file, WLAN_SECURITY, security_type, sizeof(security_type));
    res += save_config(&file, WLAN_PSK, network->security.psk, sizeof(network->security.psk));
    res += save_config(&file, WLAN_PASSPHRASE, network->security.password, sizeof(network->security.password));
    snprintf(mfpc, sizeof(mfpc), "%u", network->security.mfpc);
    res += save_config(&file, WLAN_MFPC, mfpc, sizeof(mfpc));
    snprintf(mfpr, sizeof(mfpr), "%u", network->security.mfpr);
    res += save_config(&file, WLAN_MFPR, mfpr, sizeof(mfpr));
    snprintf(pwe, sizeof(pwe), "%u", network->security.pwe_derivation);
    res += save_config(&file, WLAN_PWE, pwe, sizeof(network->security.pwe_derivation));
#if CONFIG_EAP_TLS
    res += save_config(&file, WLAN_ANONYMOUS_IDENTITY, network->security.anonymous_identity, sizeof(network->security.anonymous_identity));
    res += save_config(&file, WLAN_CLIENT_KEY_PASSWD, network->security.client_key_passwd, sizeof(network->security.client_key_passwd));
#endif

    if (network->ip.ipv4.addr_type == ADDR_TYPE_STATIC)
    {
        snprintf(addr_type, sizeof(addr_type), "%u", ADDR_TYPE_STATIC);
        res += save_config(&file, WLAN_IP_ADDR_TYPE, addr_type, sizeof(addr_type));

        ip.s_addr = network->ip.ipv4.address;
        res += save_config(&file, WLAN_IP_ADDR, inet_ntoa(ip), sizeof(ip_addr_temp));

        gw.s_addr = network->ip.ipv4.gw;
        res += save_config(&file, WLAN_GATEWAY, inet_ntoa(gw), sizeof(ip_addr_temp));

        nm.s_addr = network->ip.ipv4.netmask;
        res += save_config(&file, WLAN_NETMASK, inet_ntoa(nm), sizeof(ip_addr_temp));

        dns1.s_addr = network->ip.ipv4.dns1;
        res += save_config(&file, WLAN_DNS1, inet_ntoa(dns1), sizeof(ip_addr_temp));

        dns2.s_addr = network->ip.ipv4.dns2;
        res += save_config(&file, WLAN_DNS2, inet_ntoa(dns2), sizeof(ip_addr_temp));
    }
    else
    {
        if (network->ip.ipv4.addr_type == ADDR_TYPE_LLA)
        {
            snprintf(addr_type, sizeof(addr_type), "%u", ADDR_TYPE_LLA);
            res += save_config(&file, WLAN_IP_ADDR_TYPE, addr_type, sizeof(addr_type));
        }
        else
        {
            snprintf(addr_type, sizeof(addr_type), "%u", ADDR_TYPE_DHCP);
            res += save_config(&file, WLAN_IP_ADDR_TYPE, addr_type, sizeof(addr_type));
        }

        ip.s_addr = 0;
        res += save_config(&file, WLAN_IP_ADDR, inet_ntoa(ip), sizeof(ip_addr_temp));

        gw.s_addr = 0;
        res += save_config(&file, WLAN_GATEWAY, inet_ntoa(gw), sizeof(ip_addr_temp));

        nm.s_addr = 0;
        res += save_config(&file, WLAN_NETMASK, inet_ntoa(nm), sizeof(ip_addr_temp));

        dns1.s_addr = 0;
        res += save_config(&file, WLAN_DNS1, inet_ntoa(dns1), sizeof(ip_addr_temp));

        dns2.s_addr = 0;
        res += save_config(&file, WLAN_DNS2, inet_ntoa(dns2), sizeof(ip_addr_temp));
    }

    snprintf(configured, sizeof(configured), "%u", WLAN_NETWORK_PROVISIONED);
    res += save_config(&file, WLAN_CONFIGURED, configured, sizeof(configured));
    bss_status = WLAN_BSS_STATUS_AVAILABLE;
    snprintf(status, sizeof(status), "%u", bss_status);
    res += save_config(&file, WLAN_STATUS, status, sizeof(status));
    snprintf(role, sizeof(role), "%u", network->role);
    res += save_config(&file, WLAN_ROLE, role, sizeof(role));

    /* If the network role is uAP, the following information also needs to be stored. */
    if (network->role == WLAN_BSS_ROLE_UAP)
    {
        char capa[WLAN_CAPA_MAX_LEN], dtim[WLAN_DTIM_MAX_LEN], acs_band[WLAN_ACS_BAND_MAX_LEN];

        snprintf(capa, sizeof(capa), "%u", network->wlan_capa);
        ret += save_config(&file, WLAN_CAPA, capa, sizeof(capa));

        snprintf(dtim, sizeof(dtim), "%u", network->dtim_period);
        ret += save_config(&file, WLAN_DTIM, dtim, sizeof(dtim));

        snprintf(acs_band, sizeof(acs_band), "%u", network->acs_band);
        ret += save_config(&file, WLAN_ACS_BAND, acs_band, sizeof(acs_band));
    }

    if (res != 0)
    {
        flash_log_e("write wlan network config fail");
        ret = -WM_FAIL;
    }
    else
    {
        ret = WM_SUCCESS;
    }

    wifi_lfs_bss_config[index].flag = WLAN_BSS_STATUS_AVAILABLE;
    memcpy(wifi_lfs_bss_config[index].network_name, network->name, sizeof(network->name));

    res = lfs_file_close(&lfs, &file);
    if (res != 0)
    {
        flash_log_e("%s close file %s fail res %d", __func__, path, res);
        ret = -WM_FAIL;
    }

    return ret;
}

/* If net_name is null, find the first available network according to bss_role.
 * If net_name isn't null, find the available network which name is same as net_name.
 */

int wifi_get_network(struct wlan_network *network, enum wlan_bss_role bss_role, char *net_name)
{
    int res, i;
    int ret = -WM_FAIL;
    char channel[4], security_type[3], mfpc[2], mfpr[2], addr_type[2], pwe[2];
    char ip_addr_temp[17], role[2];
    lfs_file_t file;
    char *path;
    wifi_load_wlan_config_fn_t load_config = wifi_load_wlan_sta_config;

    if(net_name == NULL)
    {
        if(bss_role == WLAN_BSS_ROLE_UAP)
        {
            load_config = wifi_load_wlan_uap_config;
        }

        for (i = 0; i < WLAN_BSS_MAX_NUM; i++)
        {
            path = wifi_lfs_bss_config[i].config_path;
            res = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
            if (res != 0)
            {
                flash_log_e("open file %s fail res %d", path, res);
                return -WM_FAIL;
            }
            res = load_config(&file, WLAN_ROLE, role, sizeof(role));
            network->role = (enum wlan_bss_role)atoi(role);
            if(network->role == bss_role)
                break;
            
            res = lfs_file_close(&lfs, &file);
        }
        if( i == WLAN_BSS_MAX_NUM)
        {
            flash_log_e("can't find %s bss in lfs file", bss_role == 0 ? "sta" : "uap");
            return -WM_FAIL;
        }

        res = load_config(&file, WLAN_PROFILE, network->name, 32);
    }
    else
    {
        if(bss_role == WLAN_BSS_ROLE_UAP)
        {
            load_config = wifi_load_wlan_uap_config;
        }

        for (i = 0; i < WLAN_BSS_MAX_NUM; i++)
        {
            path = wifi_lfs_bss_config[i].config_path;

            if(wifi_lfs_bss_config[i].flag != WLAN_BSS_STATUS_AVAILABLE)
            {
                continue;
            }
            
            res = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
            if (res != 0)
            {
                flash_log_e("open file %s fail res %d", path, res);
                return -WM_FAIL;
            }
            
            res = load_config(&file, WLAN_PROFILE, network->name, sizeof(network->name));
            if(res != 0)
            {
                flash_log_e("read file profile %s fail res %d", path, res);
                return -WM_FAIL;
            }
            flash_log_d("%s: network->name = %s\r\n", __func__, network->name);
            if(strcmp(network->name, net_name) == 0)
                goto ncp_get_bss;

            res = lfs_file_close(&lfs, &file);
            if (res != 0)
            {
                flash_log_e("%s close file %s fail res %d", __func__, path, res);
                return -WM_FAIL;
            }
            memset(network, 0, sizeof(struct wlan_network));
        }

        if(i == WLAN_BSS_MAX_NUM)  // Can't find specified network, retrurn flase.
        {
            flash_log_e("%s can't find specified network. net_name = %s", __func__, net_name);
            return -WM_FAIL;
        }
    }

ncp_get_bss:
    res += load_config(&file, WLAN_SSID, network->ssid, sizeof(network->ssid));
    res += load_config(&file, WLAN_BSSID, network->bssid, sizeof(network->bssid));
    res += load_config(&file, WLAN_CHANNEL, channel, sizeof(channel));
    network->channel = atoi(channel);
    res += load_config(&file, WLAN_SECURITY, security_type, sizeof(security_type));
    network->security.type = (enum wlan_security_type)atoi(security_type);
    res += load_config(&file, WLAN_PSK, network->security.psk, sizeof(network->security.psk));
    res += load_config(&file, WLAN_PASSPHRASE, network->security.password, sizeof(network->security.password));

    if (network->security.type != WLAN_SECURITY_NONE)
    {
        if (network->security.type != WLAN_SECURITY_WEP_OPEN && network->security.type != WLAN_SECURITY_WEP_SHARED)
            network->security.psk_len = strlen(network->security.psk);
        else
            network->security.psk_len = sizeof(network->security.psk);

        network->security.password_len = strlen(network->security.password);
    }
    else
    {
        network->security.psk_len      = 0;
        network->security.password_len = 0;
    }

    res += load_config(&file, WLAN_MFPC, mfpc, sizeof(mfpc));
    network->security.mfpc = atoi(mfpc);
    res += load_config(&file, WLAN_MFPR, mfpr, sizeof(mfpr));
    network->security.mfpr = atoi(mfpr);
    res += load_config(&file, WLAN_PWE, pwe, sizeof(network->security.pwe_derivation));
    network->security.pwe_derivation = atoi(pwe);
#if CONFIG_EAP_TLS
    res += load_config(&file, WLAN_ANONYMOUS_IDENTITY, network->security.anonymous_identity, sizeof(network->security.anonymous_identity));
    res += load_config(&file, WLAN_CLIENT_KEY_PASSWD, network->security.client_key_passwd, sizeof(network->security.client_key_passwd));
#endif

    res += load_config(&file, WLAN_IP_ADDR_TYPE, addr_type, sizeof(addr_type));
    network->ip.ipv4.addr_type = (enum address_types)atoi(addr_type);
    if (res == 0 && network->ip.ipv4.addr_type == ADDR_TYPE_STATIC)
    {
        res += load_config(&file, WLAN_IP_ADDR, ip_addr_temp, sizeof(ip_addr_temp));
        network->ip.ipv4.address = net_inet_aton(ip_addr_temp);
        res += load_config(&file, WLAN_NETMASK, ip_addr_temp, sizeof(ip_addr_temp));
        network->ip.ipv4.netmask = net_inet_aton(ip_addr_temp);
        res += load_config(&file, WLAN_GATEWAY, ip_addr_temp, sizeof(ip_addr_temp));
        network->ip.ipv4.gw = net_inet_aton(ip_addr_temp);
        res += load_config(&file, WLAN_DNS1, ip_addr_temp, sizeof(ip_addr_temp));
        network->ip.ipv4.dns1 = net_inet_aton(ip_addr_temp);
        res += load_config(&file, WLAN_DNS2, ip_addr_temp, sizeof(ip_addr_temp));
        network->ip.ipv4.dns2 = net_inet_aton(ip_addr_temp);
    }

    res += load_config(&file, WLAN_ROLE, role, sizeof(role));
    network->role = (enum wlan_bss_role)atoi(role);

    network->ssid_specific = 1;

    /* If the network role is uAP, the following information also needs to be loaded. */
    if (bss_role == WLAN_BSS_ROLE_UAP)
    {
        char capa[WLAN_CAPA_MAX_LEN], dtim[WLAN_DTIM_MAX_LEN], acs_band[WLAN_ACS_BAND_MAX_LEN];

        ret += load_config(&file, WLAN_CAPA, capa, sizeof(capa));
        network->wlan_capa = atoi(capa);

        ret += load_config(&file, WLAN_DTIM, dtim, sizeof(dtim));
        network->dtim_period = atoi(dtim);

        ret += load_config(&file, WLAN_ACS_BAND, acs_band, sizeof(acs_band));
        network->acs_band = atoi(acs_band);
    }

    if (res != 0)
    {
        flash_log_e("read wlan network config fail");
        ret = -WM_FAIL;
    }
    else
    {
        ret = WM_SUCCESS;
    }

    res = lfs_file_close(&lfs, &file);
    if (res != 0)
    {
        flash_log_e("%s close file %s fail res %d", __func__, path, res);
        return -WM_FAIL;
    }
    return ret;
}

/* If remove a network or want to reset it, call this function to set flag of
 * the network profile to WLAN_BSS_STATUS_NONAVAILABLE.
 */
int wifi_overwrite_network(char *removed_network)
{
    char *path = NULL;
    char status[2];
    int res, i, bss_status;
    int ret = WM_SUCCESS;
    lfs_file_t file;

    for (i = 0; i < WLAN_BSS_MAX_NUM; i++)
    {
        if(strcmp(wifi_lfs_bss_config[i].network_name, removed_network) != 0)
        {
            continue;
        }

        path = wifi_lfs_bss_config[i].config_path;
            
        res = lfs_file_open(&lfs, &file, path, LFS_O_RDWR);
        if (res != 0)
        {
            flash_log_e("open file %s fail res %d", path, res);
            ret = -WM_FAIL;
            goto done;
        }

        /*The network has been removed, this file can store new network.*/
        wifi_lfs_bss_config[i].flag = WLAN_BSS_STATUS_NONAVAILABLE;
        bss_status = WLAN_BSS_STATUS_NONAVAILABLE;
        snprintf(status, sizeof(status), "%u", bss_status);
        res = wifi_save_wlan_sta_config(&file, WLAN_STATUS, status, sizeof(status));
        if (res != 0)
        {
            flash_log_e("file %s, save status fail res %d", path, res);
            ret = -WM_FAIL;
            goto done;
        }
        flash_log_d("%s: Set %s flag to 0.\r\n",__func__, removed_network);
        break;
    }
    if(i == WLAN_BSS_MAX_NUM) // Can't find specified network, retrurn flase.
    {
        flash_log_e("Can't find removed %s network", removed_network);
        ret = -WM_FAIL;
        return ret;
    }

done:
    res = lfs_file_close(&lfs, &file);
    if(res != 0)
    {
        flash_log_e("close file fail res %d", res);
        ret = -WM_FAIL;
    }

    return ret;
}

bool ncp_network_is_added(char *network)
{
    int  i;
    bool flag = false;

    for (i = 0; i < WLAN_BSS_MAX_NUM; i++)
    {
        if(strcmp(wifi_lfs_bss_config[i].network_name, network) != 0)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    if(i == WLAN_BSS_MAX_NUM)
        flag = false;
    else
        flag = true;

    return flag;
}


/* Search for a matching entry in the flash_table.
 * return WM_SUCCESS on success, other num on fail
 */
static int search_active_object(const char *mod_name, const char *variable, wifi_flash_table_type_t *flash_object)
{
    int ret                     = -WM_FAIL;
    struct ncp_conf_t *cur_conf = NULL;
    int i                       = 0;

    /* find the matching config file */
    cur_conf = lookup_conf(ncp_conf_list, mod_name);
    if (cur_conf == NULL)
    {
        flash_log_e("%s invaild module name: %s", __func__, mod_name);
        return -WM_FAIL;
    }

    /* find the matching flash table */
    for (i = 0; i < cur_conf->max_type; i++)
    {
        if (!strcmp(cur_conf->flash_table[i].name, variable))
        {
            memcpy(flash_object, &cur_conf->flash_table[i], sizeof(wifi_flash_table_type_t));
            ret = WM_SUCCESS;
            break;
        }
    }

    return ret;
}

/* ncp set variable
 * return WM_SUCCESS on success, other num on fail
 */
static int ncp_set_variable(
    lfs_file_t *file, const char *mod_name, const char *variable, IN const void *value, uint32_t len)
{
    int ret = WM_SUCCESS;
    wifi_flash_table_type_t flash_object;

    /* Search for matching entries in the matching flash table */
    if (!search_active_object(mod_name, variable, &flash_object))
    {
        flash_log_d("flash table matched for: %s", flash_object.name);
    }
    else
    {
        flash_log_e("%s matching failed in flash table: %s", __func__, flash_object.name);
        return -WM_FAIL;
    }

    if (len > flash_object.len)
        len = flash_object.len;

    ret = wifi_save_config(&flash_object, file, value, len);
    if (ret != WM_SUCCESS)
    {
        flash_log_e("%s save config fail, ret %d", __func__, ret);
    }

    return ret;
}

/* ncp set variable
 * return WM_SUCCESS on success, other num on fail
 */
static int ncp_get_variable(
    lfs_file_t *file, const char *mod_name, const char *variable, OUT void *value, uint32_t max_len)
{
    int ret = WM_SUCCESS;
    wifi_flash_table_type_t flash_object;

    /* Search for matching entries in the matching flash table */
    if (!search_active_object(mod_name, variable, &flash_object))
    {
        flash_log_d("flash table matched for: %s", flash_object.name);
    }
    else
    {
        flash_log_e("%s matching failed in flash table: %s", __func__, flash_object.name);
        return -WM_FAIL;
    }

    if (flash_object.len > max_len)
    {
        flash_log_e("%s Variable value %d will not fit in given buffer %d", __func__, flash_object.len, ret);
        return -WM_FAIL;
    }
    ret = wifi_load_config(&flash_object, file, value, flash_object.len);
    if (ret != WM_SUCCESS)
    {
        flash_log_e("%s save config fail, ret %d", __func__, ret);
    }

    return ret;
}

int ncp_set_conf(const char *mod_name, const char *var_name, IN const char *value)
{
    int ret = WM_SUCCESS;
    lfs_file_t file;
    char file_path[FULL_PATH_NAME_SIZE];
    char variable[FULL_VAR_NAME_SIZE];

    flash_log_d("file_conf: %s, variable_name: %s, value: %s", mod_name, var_name, value);
    if (!mod_name || !var_name || !value)
    {
        flash_log_e("Invalid arguments");
        return -WM_FAIL;
    }

    snprintf(file_path, FULL_PATH_NAME_SIZE, "/etc/%s_conf", mod_name);
    snprintf(variable, FULL_VAR_NAME_SIZE, "%s=", var_name);

    if (!lfs_mounted)
    {
        flash_log_e("LFS not mounted");
        return -WM_FAIL;
    }

    ret = lfs_file_open(&lfs, &file, file_path, LFS_O_RDWR | LFS_O_CREAT);
    if (ret != 0)
    {
        flash_log_e("open file: %s, fail ret %d", file_path, ret);
        return -WM_FAIL;
    }

    ret = ncp_set_variable(&file, mod_name, variable, value, strlen(value) + 1);

    ret = lfs_file_close(&lfs, &file);
    if (ret != 0)
    {
        flash_log_e("%s close file: %s, fail ret %d", __func__, file_path, ret);
        return -WM_FAIL;
    }

    return ret;
}

int ncp_get_conf(const char *mod_name, const char *var_name, OUT char *value, uint32_t max_len)
{
    int ret = WM_SUCCESS;
    lfs_file_t file;
    char file_path[FULL_PATH_NAME_SIZE];
    char variable[FULL_VAR_NAME_SIZE];

    flash_log_d("file_conf: %s, variable_name: %s", mod_name, var_name);
    if (!mod_name || !var_name || !value)
    {
        flash_log_e("Invalid arguments");
        return -WM_FAIL;
    }

    snprintf(file_path, FULL_PATH_NAME_SIZE, "/etc/%s_conf", mod_name);
    snprintf(variable, FULL_VAR_NAME_SIZE, "%s=", var_name);

    if (!lfs_mounted)
    {
        flash_log_e("LFS not mounted");
        return -WM_FAIL;
    }

    ret = lfs_file_open(&lfs, &file, file_path, LFS_O_RDWR | LFS_O_CREAT);
    if (ret != 0)
    {
        flash_log_e("open file: %s, fail ret %d", file_path, ret);
        return -WM_FAIL;
    }

    ret = ncp_get_variable(&file, mod_name, variable, value, max_len);

    ret = lfs_file_close(&lfs, &file);
    if (ret != 0)
    {
        flash_log_e("%s close file: %s, fail ret %d", __func__, file_path, ret);
        return -WM_FAIL;
    }

    return ret;
}

int ncp_get_uart_conf(struct rtos_usart_config *usart_cfg)
{
    int res;
    int ret = WM_SUCCESS;
    char baud[8];
    char stopbits[4];
    char parity[5];
    lfs_file_t file;

    res = lfs_file_open(&lfs, &file, SYS_CONFIG_FILE_PATH, LFS_O_RDONLY);
    if (res != 0)
    {
        flash_log_e("open file %s fail res %d", SYS_CONFIG_FILE_PATH, res);
        return -WM_FAIL;
    }

    /* Get UART Configuration from LittleFS */
    res = wifi_load_system_config(&file, SYS_PROTO_UART_BAUDRATE, baud, sizeof(baud));
    res += wifi_load_system_config(&file, SYS_PROTO_UART_STOPBITS, stopbits, sizeof(stopbits));
    res += wifi_load_system_config(&file, SYS_PROTO_UART_PARITY, parity, sizeof(parity));
    if (res != 0)
    {
        flash_log_e("get protocol uart config fail");
        ret = -WM_FAIL;
        goto done;
    }

    usart_cfg->baudrate = atoi(baud);
    if (strncmp(parity, "even", sizeof(parity)) == 0)
    {
        flash_log_d("Even parity");
        usart_cfg->parity = kUSART_ParityEven;
    }
    else if (strncmp(parity, "odd", sizeof(parity)) == 0)
    {
        flash_log_d("Odd parity");
        usart_cfg->parity = kUSART_ParityOdd;
    }
    else
    {
        flash_log_d("None parity");
        usart_cfg->parity = kUSART_ParityDisabled;
    }

    if (strncmp(stopbits, "2", sizeof(stopbits)) == 0)
    {
        flash_log_d("2 stopbits");
        usart_cfg->stopbits = kUSART_TwoStopBit;
    }
    else
    {
        flash_log_d("1 stopbits");
        usart_cfg->stopbits = kUSART_OneStopBit;
    }

done:
    res = lfs_file_close(&lfs, &file);
    if (res != 0)
    {
        flash_log_e("%s close file %s fail res %d", __func__, SYS_CONFIG_FILE_PATH, res);
        ret = -WM_FAIL;
    }

    return ret;
}

lfs_t *wifi_get_lfs_handler(void)
{
    return &lfs;
}

int ncp_config_init(void)
{
    int res;
    struct lfs_info info;
    char nvm[2];

    res = littlefs_init();
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s init little fs fail res %d", __func__, res);
        return -WM_FAIL;
    }

    /* try to read dir "/etc/", if not exist, create one */
    res = lfs_stat(&lfs, CONFIG_FILE_DIR, &info);
    if (res != 0 || info.type != LFS_TYPE_DIR)
    {
        flash_log_w("%s wifi config directory not exist, one-shot initialize default config", __func__);
        res = lfs_mkdir(&lfs, CONFIG_FILE_DIR);
        if (res != 0)
        {
            flash_log_e("%s init littlefs wifi config directory fail res %d", __func__, res);
            return -WM_FAIL;
        }
    }

    res = wifi_save_default_config_sys();
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s set wifi default system config fail res %d", __func__, res);
        return -WM_FAIL;
    }

    res = wifi_save_default_config_prov();
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s set wifi default provisioning config fail res %d\r\n", __func__, res);
        return -WM_FAIL;
    }

    res = wifi_save_default_config_wlan_bss();
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s set wifi default bss config fail res %d\r\n", __func__, res);
        return -WM_FAIL;
    }

    res = wifi_init_lfs_config();
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s Init lfs config fail res %d\r\n", __func__, res);
        return -WM_FAIL;
    }

    res = ncp_get_conf("sys", "nvm", nvm, sizeof(nvm));
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s, failed to read NVM value from LittleFS, res %d\r\n", __func__, res);
        return -WM_FAIL;
    }
    is_nvm_enable = atoi(nvm);

    return WM_SUCCESS;
}

void ncp_config_reset_factory(void)
{
    int res;

    if (lfs_mounted)
        lfs_unmount(&lfs);

    res = lfs_format(&lfs, lfs.cfg);
    if (res != 0)
        flash_log_e("reset factory fail res %d", res);
    else
        flash_log_w("reset factory success");
}

bool is_nvm_enabled(void)
{
    int res;
    char nvm[2];

    res = ncp_get_conf("sys", "nvm", nvm, sizeof(nvm));
    if (res != WM_SUCCESS)
    {
        flash_log_e("%s, failed to read NVM value from LittleFS, res %d\r\n", __func__, res);
        return false;
    }
    is_nvm_enable = atoi(nvm);

    return (is_nvm_enable == ENABLE_NVM) ? true : false;
}
