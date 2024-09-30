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

#ifndef _NCP_CONFIG_H_
#define _NCP_CONFIG_H_

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "lfs.h"
#include "osa.h"
#include "wmlog.h"

#include "wlan.h"

#include "fsl_usart_freertos.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/* wifi flash table mgmt structure */
typedef struct
{
    char name[32];
    int offset;
    int len;
} wifi_flash_table_type_t;

/* wifi config file path */
#define CONFIG_FILE_DIR           "etc"
#define SYS_CONFIG_FILE_PATH      "/etc/sys_conf"
#define PROV_CONFIG_FILE_PATH     "/etc/prov_conf"
#define WLAN_BSS1_CONFIG_FILE_PATH "/etc/wlan_bssA_conf"
#define WLAN_BSS2_CONFIG_FILE_PATH "/etc/wlan_bssB_conf"
#define WLAN_BSS3_CONFIG_FILE_PATH "/etc/wlan_bssC_conf"
#define WLAN_BSS4_CONFIG_FILE_PATH "/etc/wlan_bssD_conf"
#define WLAN_BSS5_CONFIG_FILE_PATH "/etc/wlan_bssE_conf"

#define INITIAL_OFFSET 0

#define WLAN_BSS_MAX_NUM             5
#define WLAN_BSS_STATUS_NONAVAILABLE 0
#define WLAN_BSS_STATUS_AVAILABLE    1

typedef enum
{
    WIFI_CONFIG_SYS = 0,
    WIFI_CONFIG_PROV,
    WIFI_CONFIG_WLAN_STA,
    WIFI_CONFIG_WLAN_UAP,
    WIFI_CONFIG_MAX
} wifi_flash_config_type_e;

/* system config */
typedef enum
{
    SYS_PM_EXT_GPIO = 0,
    SYS_PM_EXT_GPIO_LEVEL,
    SYS_PROTO_UART_BAUDRATE,
    SYS_PROTO_UART_PARITY,
    SYS_PROTO_UART_STOPBITS,
    SYS_PROTO_UART_FLOW_CONTROL,
    SYS_PROTO_UART_ID,
    SYS_DEBUG_UART_ID,
    SYS_IEEE_LISTEN_INTERVAL,
    SYS_HOST_WAKEUP_GPIO_NO,
    SYS_HOST_WAKEUP_GPIO_LEVEL,
    SYS_HOST_WAKEUP_DELAY,
    SYS_CONFIG_NVM,
    SYS_MAX_TYPE
} wifi_flash_sys_type_e;

/** Awake indicator GPIO number
 *
 *  content: "0" - "255"
 *  value: 0-255 GPIO NO
 */
#define SYS_PM_EXT_GPIO_OFT      INITIAL_OFFSET
#define SYS_PM_EXT_GPIO_NAME     "pm_ext_gpio="
#define SYS_PM_EXT_GPIO_NAME_LEN 12
#define SYS_PM_EXT_GPIO_MAX_LEN  4
#define SYS_PM_EXT_GPIO_DEF      "10"
#define SYS_PM_EXT_GPIO_END      (SYS_PM_EXT_GPIO_OFT + SYS_PM_EXT_GPIO_NAME_LEN + SYS_PM_EXT_GPIO_MAX_LEN)

/** Level of the awake indicator pin to indicate active state
 *
 *  content: "0", "1"
 *  value: 0 - active low, 1 - active high
 */
#define SYS_PM_EXT_GPIO_LEVEL_OFT      SYS_PM_EXT_GPIO_END
#define SYS_PM_EXT_GPIO_LEVEL_NAME     "pm_ext_gpio_level="
#define SYS_PM_EXT_GPIO_LEVEL_NAME_LEN 18
#define SYS_PM_EXT_GPIO_LEVEL_MAX_LEN  2
#define SYS_PM_EXT_GPIO_LEVEL_DEF      "1"
#define SYS_PM_EXT_GPIO_LEVEL_END \
    (SYS_PM_EXT_GPIO_LEVEL_OFT + SYS_PM_EXT_GPIO_LEVEL_NAME_LEN + SYS_PM_EXT_GPIO_LEVEL_MAX_LEN)

/** Baudrate for serial communication
 *
 *  content: "4800", "9600", "115200", "3000000", ...
 *  value: 4800, 9600, 115200, 3000000, ...
 */
#define SYS_PROTO_UART_BAUDRATE_OFT      SYS_PM_EXT_GPIO_LEVEL_END
#define SYS_PROTO_UART_BAUDRATE_NAME     "proto_uart_baudrate="
#define SYS_PROTO_UART_BAUDRATE_NAME_LEN 20
#define SYS_PROTO_UART_BAUDRATE_MAX_LEN  8
#define SYS_PROTO_UART_BAUDRATE_DEF      "115200"
#define SYS_PROTO_UART_BAUDRATE_END \
    (SYS_PROTO_UART_BAUDRATE_OFT + SYS_PROTO_UART_BAUDRATE_NAME_LEN + SYS_PROTO_UART_BAUDRATE_MAX_LEN)

/** Parity bit for serial communication
 *
 *  content: "none","odd","even"
 *  value: "none" - disable parity, "odd" - odd parity, "even" - even parity
 */
#define SYS_PROTO_UART_PARITY_OFT      SYS_PROTO_UART_BAUDRATE_END
#define SYS_PROTO_UART_PARITY_NAME     "proto_uart_parity="
#define SYS_PROTO_UART_PARITY_NAME_LEN 18
#define SYS_PROTO_UART_PARITY_MAX_LEN  5
#define SYS_PROTO_UART_PARITY_DEF      "none"
#define SYS_PROTO_UART_PARITY_END \
    (SYS_PROTO_UART_PARITY_OFT + SYS_PROTO_UART_PARITY_NAME_LEN + SYS_PROTO_UART_PARITY_MAX_LEN)

/** Stopbits for serial communication
 *
 *  content: "1","1.5","2"
 *  value: 1, 1.5, 2
 */
#define SYS_PROTO_UART_STOPBITS_OFT      SYS_PROTO_UART_PARITY_END
#define SYS_PROTO_UART_STOPBITS_NAME     "proto_uart_stopbits="
#define SYS_PROTO_UART_STOPBITS_NAME_LEN 20
#define SYS_PROTO_UART_STOPBITS_MAX_LEN  4
#define SYS_PROTO_UART_STOPBITS_DEF      "1"
#define SYS_PROTO_UART_STOPBITS_END \
    (SYS_PROTO_UART_STOPBITS_OFT + SYS_PROTO_UART_STOPBITS_NAME_LEN + SYS_PROTO_UART_STOPBITS_MAX_LEN)

/** Flow control support to manage data rate between the host and the device to avoid data loss
 *
 *  content: "none","software","hardware"
 *  value: "none","software","hardware"
 */
#define SYS_PROTO_UART_FLOW_CONTROL_OFT      SYS_PROTO_UART_STOPBITS_END
#define SYS_PROTO_UART_FLOW_CONTROL_NAME     "proto_uart_flow_ctrl="
#define SYS_PROTO_UART_FLOW_CONTROL_NAME_LEN 21
#define SYS_PROTO_UART_FLOW_CONTROL_MAX_LEN  9
#define SYS_PROTO_UART_FLOW_CONTROL_DEF      "none"
#define SYS_PROTO_UART_FLOW_CONTROL_END \
    (SYS_PROTO_UART_FLOW_CONTROL_OFT + SYS_PROTO_UART_FLOW_CONTROL_NAME_LEN + SYS_PROTO_UART_FLOW_CONTROL_MAX_LEN)

/** Specifies the UART Identifier for NCP protocol
 *
 *  content: "0", "1", "2", "3"
 *  value: 0 - UART0, 1 - UART1, 2 - UART2, 3 - UART3
 */
#define SYS_PROTO_UART_ID_OFT      SYS_PROTO_UART_FLOW_CONTROL_END
#define SYS_PROTO_UART_ID_NAME     "proto_uart_id="
#define SYS_PROTO_UART_ID_NAME_LEN 14
#define SYS_PROTO_UART_ID_MAX_LEN  2
#define SYS_PROTO_UART_ID_DEF      "0"
#define SYS_PROTO_UART_ID_END      (SYS_PROTO_UART_ID_OFT + SYS_PROTO_UART_ID_NAME_LEN + SYS_PROTO_UART_ID_MAX_LEN)

/** Specifies the UART Identifier for NCP debug console
 *
 *  content: "0", "1", "2", "3"
 *  value: 0 - UART0, 1 - UART1, 2 - UART2, 3 - UART3
 */
#define SYS_DEBUG_UART_ID_OFT      SYS_PROTO_UART_ID_END
#define SYS_DEBUG_UART_ID_NAME     "debug_uart_id="
#define SYS_DEBUG_UART_ID_NAME_LEN 14
#define SYS_DEBUG_UART_ID_MAX_LEN  2
#define SYS_DEBUG_UART_ID_DEF      "3"
#define SYS_DEBUG_UART_ID_END      (SYS_DEBUG_UART_ID_OFT + SYS_DEBUG_UART_ID_NAME_LEN + SYS_DEBUG_UART_ID_MAX_LEN)

/** With power save enabled, when the device is connected, it goes uses IEEEPS power save.
 *  This parameter is default interval in milliseconds after which
 *  it checks the availability of the data from the connected access point.
 *
 *  content: "100" - "65535"
 *  value: 100-65535
 */
#define SYS_IEEE_LISTEN_INTERVAL_OFT      SYS_DEBUG_UART_ID_END
#define SYS_IEEE_LISTEN_INTERVAL_NAME     "ieee_listen_interval="
#define SYS_IEEE_LISTEN_INTERVAL_NAME_LEN 21
#define SYS_IEEE_LISTEN_INTERVAL_MAX_LEN  6
#define SYS_IEEE_LISTEN_INTERVAL_DEF      "200"
#define SYS_IEEE_LISTEN_INTERVAL_END \
    (SYS_IEEE_LISTEN_INTERVAL_OFT + SYS_IEEE_LISTEN_INTERVAL_NAME_LEN + SYS_IEEE_LISTEN_INTERVAL_MAX_LEN)

/** With asynchronous events mode enabled, this GPIO is used to wake up the host
 *  before sending any asynchronous events.
 *
 *  content: "0" - "255"
 *  value: 0-255 GPIO NO
 */
#define SYS_HOST_WAKEUP_GPIO_NO_OFT      SYS_IEEE_LISTEN_INTERVAL_END
#define SYS_HOST_WAKEUP_GPIO_NO_NAME     "host_wakeup_gpio_no="
#define SYS_HOST_WAKEUP_GPIO_NO_NAME_LEN 20
#define SYS_HOST_WAKEUP_GPIO_NO_MAX_LEN  4
#define SYS_HOST_WAKEUP_GPIO_NO_DEF      "11"
#define SYS_HOST_WAKEUP_GPIO_NO_END \
    (SYS_HOST_WAKEUP_GPIO_NO_OFT + SYS_HOST_WAKEUP_GPIO_NO_NAME_LEN + SYS_HOST_WAKEUP_GPIO_NO_MAX_LEN)

/** Level of the host wakeup GPIO to indicate to the host
 *  that it should expect an asynchronous event from the device
 *
 *  content: "0", "1"
 *  value: 0 - active low, 1 - active high
 */
#define SYS_HOST_WAKEUP_GPIO_LEVEL_OFT      SYS_HOST_WAKEUP_GPIO_NO_END
#define SYS_HOST_WAKEUP_GPIO_LEVEL_NAME     "host_wakeup_gpio_level="
#define SYS_HOST_WAKEUP_GPIO_LEVEL_NAME_LEN 23
#define SYS_HOST_WAKEUP_GPIO_LEVEL_MAX_LEN  2
#define SYS_HOST_WAKEUP_GPIO_LEVEL_DEF      "0"
#define SYS_HOST_WAKEUP_GPIO_LEVEL_END \
    (SYS_HOST_WAKEUP_GPIO_LEVEL_OFT + SYS_HOST_WAKEUP_GPIO_LEVEL_NAME_LEN + SYS_HOST_WAKEUP_GPIO_LEVEL_MAX_LEN)

/** With asynchronous events mode enabled, the device sends the asynchronous event
 *  to the host after this delay. This delay is in milliseconds.
 *
 *  content: "0" - "255"
 *  value: 0-255 in ms
 */
#define SYS_HOST_WAKEUP_DELAY_OFT      SYS_HOST_WAKEUP_GPIO_LEVEL_END
#define SYS_HOST_WAKEUP_DELAY_NAME     "host_wakeup_delay="
#define SYS_HOST_WAKEUP_DELAY_NAME_LEN 18
#define SYS_HOST_WAKEUP_DELAY_MAX_LEN  4
#define SYS_HOST_WAKEUP_DELAY_DEF      "5"
#define SYS_HOST_WAKEUP_DELAY_END \
    (SYS_HOST_WAKEUP_DELAY_OFT + SYS_HOST_WAKEUP_DELAY_NAME_LEN + SYS_HOST_WAKEUP_DELAY_MAX_LEN)

/** Enabling NVM means using LittleFS to save configuration information.
 *
 *  content: "0" - disable, "1" - enable
 *  value: 0, 1
 */
#define SYS_CONFIG_NVM_OFT      SYS_HOST_WAKEUP_DELAY_END
#define SYS_CONFIG_NVM_NAME     "nvm="
#define SYS_CONFIG_NVM_NAME_LEN 4
#define SYS_CONFIG_NVM_MAX_LEN  2
#define SYS_CONFIG_NVM_DEF      "1"
#define SYS_CONFIG_NVM_END      (SYS_CONFIG_NVM_OFT + SYS_CONFIG_NVM_NAME_LEN + SYS_CONFIG_NVM_MAX_LEN)

#define SYS_CONFIG_FILE_MAX_SIZE SYS_CONFIG_NVM_END

/* provisioning config */
typedef enum
{
    PROV_SSID = 0,
    PROV_SECURITY,
    PROV_PSK,
    PROV_PASSPHRASE,
    PROV_MDNS_ENABLED,
    PROV_HOST_NAME,
    PROV_MAX_TYPE
} wifi_flash_prov_type_e;

/** SSID of the WLAN network hosted by NCP for provisioning.
 *  xxxx are the last 4 digits of the MAC address of the WLAN interface.
 *
 *  content: SSID string
 *  value: SSID string
 */
#define PROV_SSID_OFT      INITIAL_OFFSET
#define PROV_SSID_NAME     "ssid="
#define PROV_SSID_NAME_LEN 5
#define PROV_SSID_MAX_LEN  33
#define PROV_SSID_DEF      "nxpdemo-FE01"
#define PROV_SSID_END      (PROV_SSID_OFT + PROV_SSID_NAME_LEN + PROV_SSID_MAX_LEN)

/** WLAN security mode for provisioning network
 *
 *  content: "0", "3", "4", "9" "10"
 *  value: 0 - open, 3 - WPA-PSK, 4 - WPA2-PSK, 9 - WPA3_SAE, 10 WPA2/WAP3 MIXED
 */
#define PROV_SECURITY_OFT      PROV_SSID_END
#define PROV_SECURITY_NAME     "security="
#define PROV_SECURITY_NAME_LEN 9
#define PROV_SECURITY_MAX_LEN  3
#define PROV_SECURITY_DEF      "4"
#define PROV_SECURITY_END      (PROV_SECURITY_OFT + PROV_SECURITY_NAME_LEN + PROV_SECURITY_MAX_LEN)

/** The WPA/WPA2 Pre-shared key (network password) for WLAN network
 *
 *  content: passphrase string
 *  value: passphrase string
 */
#define PROV_PSK_OFT      PROV_SECURITY_END
#define PROV_PSK_NAME     "psk="
#define PROV_PSK_NAME_LEN 4
#define PROV_PSK_MAX_LEN  65
#define PROV_PSK_DEF      "12345678"
#define PROV_PSK_END      (PROV_PSK_OFT + PROV_PSK_NAME_LEN + PROV_PSK_MAX_LEN)

/** The WPA3 SAE passphrase for WLAN network. This field is ignored if the security is 0.
 *
 *  content: passphrase string
 *  value: passphrase string
 */
#define PROV_PASSPHRASE_OFT      PROV_PSK_END
#define PROV_PASSPHRASE_NAME     "passphrase="
#define PROV_PASSPHRASE_NAME_LEN 11
#define PROV_PASSPHRASE_MAX_LEN  256
#define PROV_PASSPHRASE_DEF      "12345678"
#define PROV_PASSPHRASE_END      (PROV_PASSPHRASE_OFT + PROV_PASSPHRASE_NAME_LEN + PROV_PASSPHRASE_MAX_LEN)

/** This field indicates if the provisioning should announce an mDNS hostname
 *
 *  content: "0", "1"
 *  value: 0 - disabled, 1 - enabled
 */
#define PROV_MDNS_ENABLED_OFT      PROV_PASSPHRASE_END
#define PROV_MDNS_ENABLED_NAME     "mdns_enabled="
#define PROV_MDNS_ENABLED_NAME_LEN 13
#define PROV_MDNS_ENABLED_MAX_LEN  2
#define PROV_MDNS_ENABLED_DEF      "1"
#define PROV_MDNS_ENABLED_END      (PROV_MDNS_ENABLED_OFT + PROV_MDNS_ENABLED_NAME_LEN + PROV_MDNS_ENABLED_MAX_LEN)

/** Hostname of the NCP which can be resolved from the mDNS capable browser.
 *  "xxxx" are the last four bytes of the WLAN MAC address of the module.
 *
 *  content: host name string
 *  value: host name string
 */
#define PROV_HOST_NAME_OFT      PROV_MDNS_ENABLED_END
#define PROV_HOST_NAME_NAME     "hostname="
#define PROV_HOST_NAME_NAME_LEN 9
#define PROV_HOST_NAME_MAX_LEN  33
#define PROV_HOST_NAME_DEF      "nxpdemo-FE01"
#define PROV_HOST_NAME_END      (PROV_HOST_NAME_OFT + PROV_HOST_NAME_NAME_LEN + PROV_HOST_NAME_MAX_LEN)

#define PROV_CONFIG_FILE_MAX_SIZE PROV_HOST_NAME_END

/* wlan STA config */
typedef enum
{
    WLAN_MAC = 0,
    WLAN_CONFIGURED,
    WLAN_SSID,
    WLAN_BSSID,
    WLAN_CHANNEL,
    WLAN_SECURITY,
    WLAN_PSK,
    WLAN_PASSPHRASE,
    WLAN_MFPC,
    WLAN_MFPR,
    WLAN_PWE,
    WLAN_ANONYMOUS_IDENTITY,
    WLAN_CLIENT_KEY_PASSWD,
    WLAN_IP_ADDR_TYPE,
    WLAN_IP_ADDR,
    WLAN_NETMASK,
    WLAN_GATEWAY,
    WLAN_DNS1,
    WLAN_DNS2,
    WLAN_RECONNECT_ATTEMPTS,
    WLAN_RECONNECT_DELAY,
    WLAN_CHK_SERVER_CERT,
    WLAN_REGION_CODE,
    WLAN_PROFILE,
    WLAN_STATUS,
    WLAN_ROLE,
    WLAN_STA_MAX_TYPE,
} wifi_flash_wlan_sta_type_e;

/** This configuration parameter returns the MAC address of the WLAN interface.
 *  This is read-only parameter and can't be modified.
 *
 *  content: "00:50:43:02:fe:01"
 *  value: {0x00, 0x50, 0x43, 0x02, 0xfe, 0x01}
 */
#define WLAN_MAC_OFT      INITIAL_OFFSET
#define WLAN_MAC_NAME     "mac="
#define WLAN_MAC_NAME_LEN 4
#define WLAN_MAC_MAX_LEN  18
#define WLAN_MAC_DEF      "00:50:43:02:fe:01"
#define WLAN_MAC_END      (WLAN_MAC_OFT + WLAN_MAC_NAME_LEN + WLAN_MAC_MAX_LEN)

/** This field indicates if the NCP is configured (provisioned) with home network settings.
 *  This field is set to 1 automatically when the user provisions the NCP.
 *
 *  content: "0", "1"
 *  value: 0,1
 */
#define WLAN_CONFIGURED_OFT      WLAN_MAC_END
#define WLAN_CONFIGURED_NAME     "configured="
#define WLAN_CONFIGURED_NAME_LEN 11
#define WLAN_CONFIGURED_MAX_LEN  2
#define WLAN_CONFIGURED_DEF      "0"
#define WLAN_CONFIGURED_END      (WLAN_CONFIGURED_OFT + WLAN_CONFIGURED_NAME_LEN + WLAN_CONFIGURED_MAX_LEN)

/** SSID of the home WLAN network.
 *  This parameter is modified automatically once the NCP is provisioned.
 *
 *  content: SSID string
 *  value: SSID string
 */
#define WLAN_SSID_OFT      WLAN_CONFIGURED_END
#define WLAN_SSID_NAME     "ssid="
#define WLAN_SSID_NAME_LEN 5
#define WLAN_SSID_MAX_LEN  33
#define WLAN_SSID_DEF      "ssid"
#define WLAN_SSID_END      (WLAN_SSID_OFT + WLAN_SSID_NAME_LEN + WLAN_SSID_MAX_LEN)

/** BSSID of the home WLAN network.
 *  This parameter is modified automatically once the NCP is provisioned.
 *
 *  content: BSSID address unsigned char[6]
 *  value: BSSID address unsigned char[6]
 */
#define WLAN_BSSID_OFT      WLAN_SSID_END
#define WLAN_BSSID_NAME     "bssid="
#define WLAN_BSSID_NAME_LEN 6
#define WLAN_BSSID_MAX_LEN  18
#define WLAN_BSSID_DEF      "00:00:00:00:00:00"
#define WLAN_BSSID_END      (WLAN_BSSID_OFT + WLAN_BSSID_NAME_LEN + WLAN_BSSID_MAX_LEN)

/** Channel of the home WLAN network.
 *  This parameter is modified automatically once the NCP is provisioned.
 *
 *  content: channel
 *  value: channel
 */
#define WLAN_CHANNEL_OFT      WLAN_BSSID_END
#define WLAN_CHANNEL_NAME     "channel="
#define WLAN_CHANNEL_NAME_LEN 8
#define WLAN_CHANNEL_MAX_LEN  4
#define WLAN_CHANNEL_DEF      "0"
#define WLAN_CHANNEL_END      (WLAN_CHANNEL_OFT + WLAN_CHANNEL_NAME_LEN + WLAN_CHANNEL_MAX_LEN)

/** Security mode of the home WLAN connection
 *
 *  content: "0", "1", "2", "3", "4", "9", "10"
 *  value: 0 - open, 1 - WEP(open mode), 2 - WEP(shared mode), 3 - WPA-PSK, 4 - WPA2-PSK, 9 - WPA3_SAE, 10 - WPA2/WAP3
 * MIXED
 */
#define WLAN_SECURITY_OFT      WLAN_CHANNEL_END
#define WLAN_SECURITY_NAME     "security="
#define WLAN_SECURITY_NAME_LEN 9
#define WLAN_SECURITY_MAX_LEN  3
#define WLAN_SECURITY_DEF      "4"
#define WLAN_SECURITY_END      (WLAN_SECURITY_OFT + WLAN_SECURITY_NAME_LEN + WLAN_SECURITY_MAX_LEN)

/** The WPA/WPA2 Pre-shared key (network password) for home network
 *
 *  content: passphrase string
 *  value: passphrase string
 */
#define WLAN_PSK_OFT      WLAN_SECURITY_END
#define WLAN_PSK_NAME     "psk="
#define WLAN_PSK_NAME_LEN 4
#define WLAN_PSK_MAX_LEN  65
#define WLAN_PSK_DEF      "12345678"
#define WLAN_PSK_END      (WLAN_PSK_OFT + WLAN_PSK_NAME_LEN + WLAN_PSK_MAX_LEN)

/** The WPA3 SAE password for home network
 *
 *  content: passphrase string
 *  value: passphrase string
 */
#define WLAN_PASSPHRASE_OFT      WLAN_PSK_END
#define WLAN_PASSPHRASE_NAME     "passphrase="
#define WLAN_PASSPHRASE_NAME_LEN 11
#define WLAN_PASSPHRASE_MAX_LEN  256
#define WLAN_PASSPHRASE_DEF      ""
#define WLAN_PASSPHRASE_END      (WLAN_PASSPHRASE_OFT + WLAN_PASSPHRASE_NAME_LEN + WLAN_PASSPHRASE_MAX_LEN)

/** Management Frame Protection Capable (MFPC)
 *
 *  content: "0", "1"
 *  value: 0 - disable,1 - enable
 */
#define WLAN_MFPC_OFT      WLAN_PASSPHRASE_END
#define WLAN_MFPC_NAME     "mfpc="
#define WLAN_MFPC_NAME_LEN 5
#define WLAN_MFPC_MAX_LEN  2
#define WLAN_MFPC_DEF      "0"
#define WLAN_MFPC_END      (WLAN_MFPC_OFT + WLAN_MFPC_NAME_LEN + WLAN_MFPC_MAX_LEN)

/** Management Frame Protection Required (MFPR)
 *
 *  content: "0", "1"
 *  value: 0 - disable,1 - enable
 */
#define WLAN_MFPR_OFT      WLAN_MFPC_END
#define WLAN_MFPR_NAME     "mfpr="
#define WLAN_MFPR_NAME_LEN 5
#define WLAN_MFPR_MAX_LEN  2
#define WLAN_MFPR_DEF      "0"
#define WLAN_MFPR_END      (WLAN_MFPR_OFT + WLAN_MFPR_NAME_LEN + WLAN_MFPR_MAX_LEN)

/**  PWE derivation
 *   content:
 *      0 = hunting-and-pecking loop only (default without password identifier)
 *      1 = hash-to-element only (default with password identifier)
 *      2 = both hunting-and-pecking loop and hash-to-element enabled
 *  value: 0, 1, 2
 */
#define WLAN_PWE_OFT      WLAN_MFPR_END
#define WLAN_PWE_NAME     "pwe="
#define WLAN_PWE_NAME_LEN 4
#define WLAN_PWE_MAX_LEN  2
#define WLAN_PWE_DEF      "0"
#define WLAN_PWE_END      (WLAN_PWE_OFT + WLAN_PWE_NAME_LEN + WLAN_PWE_MAX_LEN)

/** Anonymous identity string for EAP
 *
 *  content: Anonymous identity string
 *
 */
#define WLAN_ANONYMOUS_IDENTITY_OFT      WLAN_PWE_END
#define WLAN_ANONYMOUS_IDENTITY_NAME     "anonymous_identity="
#define WLAN_ANONYMOUS_IDENTITY_NAME_LEN 19
#define WLAN_ANONYMOUS_IDENTITY_MAX_LEN  64
#define WLAN_ANONYMOUS_IDENTITY_DEF      "0"
#define WLAN_ANONYMOUS_IDENTITY_END      (WLAN_ANONYMOUS_IDENTITY_OFT + WLAN_ANONYMOUS_IDENTITY_NAME_LEN + WLAN_ANONYMOUS_IDENTITY_MAX_LEN)

/** Client key password
 *
 *  content: Client key password
 *
 */
#define WLAN_CLIENT_KEY_PASSWD_OFT      WLAN_ANONYMOUS_IDENTITY_END
#define WLAN_CLIENT_KEY_PASSWD_NAME     "client_key_passwd="
#define WLAN_CLIENT_KEY_PASSWD_NAME_LEN 18
#define WLAN_CLIENT_KEY_PASSWD_MAX_LEN  128
#define WLAN_CLIENT_KEY_PASSWD_DEF      "0"
#define WLAN_CLIENT_KEY_PASSWD_END      (WLAN_CLIENT_KEY_PASSWD_OFT + WLAN_CLIENT_KEY_PASSWD_NAME_LEN + WLAN_CLIENT_KEY_PASSWD_MAX_LEN)

/** IP address type
 *
 *  content: "0", "1"
 *  value: 0 - static,1 - DHCP
 */
#define WLAN_IP_ADDR_TYPE_OFT      WLAN_CLIENT_KEY_PASSWD_END
#define WLAN_IP_ADDR_TYPE_NAME     "ipaddr_type="
#define WLAN_IP_ADDR_TYPE_NAME_LEN 12
#define WLAN_IP_ADDR_TYPE_MAX_LEN  2
#define WLAN_IP_ADDR_TYPE_DEF      "0"
#define WLAN_IP_ADDR_TYPE_END      (WLAN_IP_ADDR_TYPE_OFT + WLAN_IP_ADDR_TYPE_NAME_LEN + WLAN_IP_ADDR_TYPE_MAX_LEN)

/** IP address. If the IP address type is DHCP, this value is ignored.
 *
 *  value: ip "0.0.0.0"
 */
#define WLAN_IP_ADDR_OFT      WLAN_IP_ADDR_TYPE_END
#define WLAN_IP_ADDR_NAME     "ipaddr="
#define WLAN_IP_ADDR_NAME_LEN 7
#define WLAN_IP_ADDR_MAX_LEN  17
#define WLAN_IP_ADDR_DEF      "0.0.0.0"
#define WLAN_IP_ADDR_END      (WLAN_IP_ADDR_OFT + WLAN_IP_ADDR_NAME_LEN + WLAN_IP_ADDR_MAX_LEN)

/** Netmask for the home network. If the IP address type is DHCP, this value is ignored.
 *
 *  value: netmask "255.255.255.255"
 */
#define WLAN_NETMASK_OFT      WLAN_IP_ADDR_END
#define WLAN_NETMASK_NAME     "netmask="
#define WLAN_NETMASK_NAME_LEN 8
#define WLAN_NETMASK_MAX_LEN  17
#define WLAN_NETMASK_DEF      "255.255.255.255"
#define WLAN_NETMASK_END      (WLAN_NETMASK_OFT + WLAN_NETMASK_NAME_LEN + WLAN_NETMASK_MAX_LEN)

/** Default gateway IP address. If the IP address type is DHCP, this value is ignored.
 *
 *  value: gateway "0.0.0.0"
 */
#define WLAN_GATEWAY_OFT      WLAN_NETMASK_END
#define WLAN_GATEWAY_NAME     "gateway="
#define WLAN_GATEWAY_NAME_LEN 8
#define WLAN_GATEWAY_MAX_LEN  17
#define WLAN_GATEWAY_DEF      "0.0.0.0"
#define WLAN_GATEWAY_END      (WLAN_GATEWAY_OFT + WLAN_GATEWAY_NAME_LEN + WLAN_GATEWAY_MAX_LEN)

/** First DNS server IP address. If the IP address type is DHCP, this value is ignored.
 *
 *  value: DNS1 "0.0.0.0"
 */
#define WLAN_DNS1_OFT      WLAN_GATEWAY_END
#define WLAN_DNS1_NAME     "dns1="
#define WLAN_DNS1_NAME_LEN 5
#define WLAN_DNS1_MAX_LEN  17
#define WLAN_DNS1_DEF      "0.0.0.0"
#define WLAN_DNS1_END      (WLAN_DNS1_OFT + WLAN_DNS1_NAME_LEN + WLAN_DNS1_MAX_LEN)

/** Second DNS server IP address. If the IP address type is DHCP, this value is ignored.
 *
 *  value: DNS1 "0.0.0.0"
 */
#define WLAN_DNS2_OFT      WLAN_DNS1_END
#define WLAN_DNS2_NAME     "dns2="
#define WLAN_DNS2_NAME_LEN 5
#define WLAN_DNS2_MAX_LEN  17
#define WLAN_DNS2_DEF      "0.0.0.0"
#define WLAN_DNS2_END      (WLAN_DNS2_OFT + WLAN_DNS2_NAME_LEN + WLAN_DNS2_MAX_LEN)

/** How many times should the NCP attempt reconnection in case of link loss.
 *  If this value is 0, NCP does not try reconnection on connection fail or link loss.
 *
 *  content: "0" - "5"
 *  value: 0-5
 */
#define WLAN_RECONNECT_ATTEMPTS_OFT      WLAN_DNS2_END
#define WLAN_RECONNECT_ATTEMPTS_NAME     "reconnect_attempts="
#define WLAN_RECONNECT_ATTEMPTS_NAME_LEN 19
#define WLAN_RECONNECT_ATTEMPTS_MAX_LEN  2
#define WLAN_RECONNECT_ATTEMPTS_DEF      "5"
#define WLAN_RECONNECT_ATTEMPTS_END \
    (WLAN_RECONNECT_ATTEMPTS_OFT + WLAN_RECONNECT_ATTEMPTS_NAME_LEN + WLAN_RECONNECT_ATTEMPTS_MAX_LEN)

/** A delay in seconds for consecutive reconnection attempts.
 *
 *  content: 0, yet not supported and ignored
 *  value: 0
 */
#define WLAN_RECONNECT_DELAY_OFT      WLAN_RECONNECT_ATTEMPTS_END
#define WLAN_RECONNECT_DELAY_NAME     "reconnect_delay="
#define WLAN_RECONNECT_DELAY_NAME_LEN 16
#define WLAN_RECONNECT_DELAY_MAX_LEN  2
#define WLAN_RECONNECT_DELAY_DEF      "0"
#define WLAN_RECONNECT_DELAY_END \
    (WLAN_RECONNECT_DELAY_OFT + WLAN_RECONNECT_DELAY_NAME_LEN + WLAN_RECONNECT_DELAY_MAX_LEN)

/** Validate HTTPS server's certificate for TLS connection
 *
 *  content: "0", "1"
 *  value: 0 - do not validate, 1 - should validate
 */
#define WLAN_CHK_SERVER_CERT_OFT      WLAN_RECONNECT_DELAY_END
#define WLAN_CHK_SERVER_CERT_NAME     "chk_server_cert="
#define WLAN_CHK_SERVER_CERT_NAME_LEN 16
#define WLAN_CHK_SERVER_CERT_MAX_LEN  2
#define WLAN_CHK_SERVER_CERT_DEF      "0"
#define WLAN_CHK_SERVER_CERT_END \
    (WLAN_CHK_SERVER_CERT_OFT + WLAN_CHK_SERVER_CERT_NAME_LEN + WLAN_CHK_SERVER_CERT_MAX_LEN)

/** Region code, default is US 0x10
 *
 *  content: "0", "16"
 *  value: 0 - do not validate, 16 - should validate
 */
#define WLAN_REGION_CODE_OFT      WLAN_CHK_SERVER_CERT_END
#define WLAN_REGION_CODE_NAME     "region_code="
#define WLAN_REGION_CODE_NAME_LEN 12
#define WLAN_REGION_CODE_MAX_LEN  3
#define WLAN_REGION_CODE_DEF      "16"
#define WLAN_REGION_CODE_END      (WLAN_REGION_CODE_OFT + WLAN_REGION_CODE_NAME_LEN + WLAN_REGION_CODE_MAX_LEN)

/** The name of this network profile.
 *
 *  content: profile name string
 *  value: profile name string
 */
#define WLAN_PROFILE_NAME_OFT     WLAN_REGION_CODE_END
#define WLAN_PROFILE_NAME         "profile_name="
#define WLAN_PROFILE_NAME_LEN     13
#define WLAN_PROFILE_NAME_MAX_LEN 32
#define WLAN_PROFILE_NAME_DEF     "nxp_sta"
#define WLAN_PROFILE_NAME_END     (WLAN_PROFILE_NAME_OFT + WLAN_PROFILE_NAME_LEN + WLAN_PROFILE_NAME_MAX_LEN)

/** The status of this network.
 *
 *  content: "0","1","2" 
 *  value: 0 - unavailable, 1 - available, 2 - removed
 */
#define WLAN_STATUS_OFT       WLAN_PROFILE_NAME_END
#define WLAN_STATUS_NAME      "status="
#define WLAN_STATUS_NAME_LEN  7
#define WLAN_STATUS_MAX_LEN   2
#define WLAN_STATUS_DEF       "0"
#define WLAN_STATUS_END       (WLAN_STATUS_OFT + WLAN_STATUS_NAME_LEN + WLAN_STATUS_MAX_LEN)

/** Network wireless BSS Role
 *
 *  content:"0", "1", "255"
 *  value: 0 - Station, 1 - uAP, 0xff - Any
 */
#define WLAN_ROLE_OFT      WLAN_STATUS_END
#define WLAN_ROLE_NAME     "role="
#define WLAN_ROLE_NAME_LEN 5
#define WLAN_ROLE_MAX_LEN  4
#define WLAN_ROLE_DEF      "0"
#define WLAN_ROLE_END      (WLAN_ROLE_OFT + WLAN_ROLE_NAME_LEN + WLAN_ROLE_MAX_LEN)

#define WLAN_STA_CONFIG_FILE_MAX_SIZE WLAN_ROLE_END

/* wlan uAP additional config */
typedef enum
{
    WLAN_CAPA = WLAN_STA_MAX_TYPE,
    WLAN_DTIM,
    WLAN_ACS_BAND,
    WLAN_UAP_MAX_TYPE,
} wifi_flash_wlan_uap_type_e;

/** Wireless capalibity
 *
 *  content:"1", "3", "7", "15"
 *  value: 1 - legacy, 3 - 11n, 7 - 11ac, 15 -11ax
 */
#define WLAN_CAPA_OFT      WLAN_ROLE_END
#define WLAN_CAPA_NAME     "capa="
#define WLAN_CAPA_NAME_LEN 5
#define WLAN_CAPA_MAX_LEN  3
#define WLAN_CAPA_DEF      "15"
#define WLAN_CAPA_END      (WLAN_CAPA_OFT + WLAN_CAPA_NAME_LEN + WLAN_CAPA_MAX_LEN)

/** DTIM Period
 *
 *  content: "1 - 255"
 *  value: default 10
 */
#define WLAN_DTIM_OFT      WLAN_CAPA_END
#define WLAN_DTIM_NAME     "dtim="
#define WLAN_DTIM_NAME_LEN 5
#define WLAN_DTIM_MAX_LEN  4
#define WLAN_DTIM_DEF      "10"
#define WLAN_DTIM_END      (WLAN_DTIM_OFT + WLAN_DTIM_NAME_LEN + WLAN_DTIM_MAX_LEN)

/** ACS mode
 *
 *  content: "0" - 2.4GHz channel, "1" - 5GHz channel
 *  value: 0, 1
 */
#define WLAN_ACS_BAND_OFT      WLAN_DTIM_END
#define WLAN_ACS_BAND_NAME     "acs_band="
#define WLAN_ACS_BAND_NAME_LEN 9
#define WLAN_ACS_BAND_MAX_LEN  2
#define WLAN_ACS_BAND_DEF      "0"
#define WLAN_ACS_BAND_END      (WLAN_ACS_BAND_OFT + WLAN_ACS_BAND_NAME_LEN + WLAN_ACS_BAND_MAX_LEN)

#define WLAN_UAP_CONFIG_FILE_MAX_SIZE WLAN_ACS_BAND_END

/* log */
#define flash_log_e(...) wmlog_e("flash", ##__VA_ARGS__)
#define flash_log_w(...) wmlog_w("flash", ##__VA_ARGS__)

#if CONFIG_WIFI_IO_DEBUG
#define flash_log_d(...) wmlog("flash", ##__VA_ARGS__)
#else
#define flash_log_d(...)
#endif /* ! CONFIG_WIFI_IO_DEBUG */

#define WLAN_NETWORK_NOT_PROVISIONED 0
#define WLAN_NETWORK_PROVISIONED     1

#define FULL_PATH_NAME_SIZE    32
#define FULL_VAR_NAME_SIZE     32
#define FULL_CONFIG_VALUE_SIZE 256

/** Structure for registering LittleFS config file */
struct ncp_conf_t
{
    /** The name of the config file */
    const char *name;
    /** wifi flash table mgmt structure */
    const wifi_flash_table_type_t *flash_table;
    /** MAX Type */
    uint8_t max_type;
};

typedef struct
{
    char *config_path; // ncp lfs bss config path
    char network_name[33]; // network name
    int flag;  //write flag.
}wifi_bss_config;

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * API functions
 **********************************************************************************************************************/
/* LittleFS init */
int ncp_config_init(void);
/* get LittleFS handler */
lfs_t *wifi_get_lfs_handler(void);

/* LittleFS API save/load system/provision/wlan config */
int wifi_save_system_config(lfs_file_t *file, int type, void *buf, int len);
int wifi_load_system_config(lfs_file_t *file, int type, void *buf, int len);
int wifi_save_provision_config(lfs_file_t *file, int type, void *buf, int len);
int wifi_load_provision_config(lfs_file_t *file, int type, void *buf, int len);
int wifi_save_wlan_sta_config(lfs_file_t *file, int type, void *buf, int len);
int wifi_load_wlan_sta_config(lfs_file_t *file, int type, void *buf, int len);
int wifi_save_wlan_uap_config(lfs_file_t *file, int type, void *buf, int len);
int wifi_load_wlan_uap_config(lfs_file_t *file, int type, void *buf, int len);

typedef int (*wifi_save_wlan_config_fn_t)(lfs_file_t *file, int type, void *buf, int len);
typedef int (*wifi_load_wlan_config_fn_t)(lfs_file_t *file, int type, void *buf, int len);

/* LittleFS API save/load raw file */
int wifi_save_file(char *path, void *buf, int len);
int wifi_load_file(char *path, void *buf, int len);

/* reset flash content to factory */
void ncp_config_reset_factory(void);

/* set/get network configs */
int wifi_set_network(struct wlan_network *network);
int wifi_get_network(struct wlan_network *network, enum wlan_bss_role bss_role, char*net_name);
/*set overwrite flag when a network is removed.*/
int wifi_overwrite_network(char *removed_network);
/* Judge whether network is added or not.*/
bool ncp_network_is_added(char *network);

int ncp_set_conf(const char *mod_name, const char *var_name, const char *value);
int ncp_get_conf(const char *mod_name, const char *var_name, char *value, uint32_t max_len);

/* set/get uart configs */
int ncp_get_uart_conf(struct rtos_usart_config *usart_cfg);

bool is_nvm_enabled(void);

#if defined(__cplusplus)
}
#endif

#endif /* _NCP_CONFIG_H_ */
