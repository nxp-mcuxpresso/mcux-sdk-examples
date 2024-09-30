/*
 *  Copyright 2020-2021 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#if defined(MBEDTLS_MCUX_ELS_PKC_API)
// this macro is defined because we don't use ksdk for rw61x, but edgefast include mbedtls_config_client.h as default, so not use it
#define KSDK_MBEDTLS_CONFIG_H

#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_STD_CALLOC pvPortCalloc
#define MBEDTLS_PLATFORM_STD_FREE vPortFree
#define MBEDTLS_THREADING_C
#define MBEDTLS_THREADING_ALT
#endif /* defined(MBEDTLS_MCUX_ELS_PKC_API) */

#include "usb_host_config.h"

#define CONTROLLER_ID                               kUSB_ControllerEhci0

/* Logs */
#define CONFIG_ENABLE_ERROR_LOGS                    1
#define CONFIG_ENABLE_WARNING_LOGS                  1

/* Task priority */
#if (CONFIG_NCP_BLE)
#undef CONFIG_WIFI_MAX_PRIO
#define CONFIG_WIFI_MAX_PRIO (configMAX_PRIORITIES - 6)
#endif

#define CONFIG_BT_HCI_TX_PRIO                       7

/* bluetooth feature */
#undef CONFIG_BT_BREDR
#define CONFIG_BT_CENTRAL                           1
#define CONFIG_BT_PERIPHERAL                        1
#define CONFIG_BT_SMP                               1
#define CONFIG_BT_SMP_ALLOW_UNAUTH_OVERWRITE        1
#define CONFIG_BT_SMP_APP_PAIRING_ACCEPT            1
#define CONFIG_BT_SIGNING                           1
#define CONFIG_BT_BONDABLE                          1
#define CONFIG_BT_ATT_PREPARE_COUNT                 12
#define CONFIG_BT_GATT_CLIENT                       1
#define CONFIG_BT_L2CAP_DYNAMIC_CHANNEL             1
#define CONFIG_BT_DEVICE_NAME                       "NCP_BLE"
#define CONFIG_BT_DEVICE_NAME_MAX                   32
#define CONFIG_BT_DEVICE_NAME_DYNAMIC               1
#define CONFIG_BT_DEVICE_NAME_GATT_WRITABLE         1
#define CONFIG_BT_FILTER_ACCEPT_LIST                1
#define CONFIG_BT_MAX_CONN                          1
#define CONFIG_BT_MAX_PAIRED                        1
#define CONFIG_BT_GATT_NOTIFY_MULTIPLE              1
#define CONFIG_BT_ATT_RETRY_ON_SEC_ERR              0
#define CONFIG_BT_GATT_DYNAMIC_DB                   1
#define CONFIG_BT_BUF_ACL_RX_SIZE                   100
#define CONFIG_BT_DIS_MODEL                         "NCP BLE Demo"
#define CONFIG_BT_DIS_MANUF                         "NXP"
#define CONFIG_BT_SETTINGS                          1
#define CONFIG_BT_HOST_CRYPTO                       1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST             1

#define CONFIG_BT_BROADCASTER                       1
#define CONFIG_BT_OBSERVER                          1
#define CONFIG_BT_CONN                              1

#define CONFIG_BT_PHY_UPDATE                        1
#define CONFIG_BT_AUTO_PHY_UPDATE                   1
#define CONFIG_BT_DATA_LEN_UPDATE                   1
#define CONFIG_BT_AUTO_DATA_LEN_UPDATE              1
#define CONFIG_BT_USER_PHY_UPDATE                   1
#define CONFIG_BT_USER_DATA_LEN_UPDATE              1
/*
 * Bluetooth buffer configuration
 */
#define CONFIG_BT_BUF_ACL_TX_SIZE                   27
#define CONFIG_BT_BUF_ACL_TX_COUNT                  3
#define CONFIG_BT_BUF_ACL_RX_COUNT                  3
#define CONFIG_BT_BUF_EVT_RX_SIZE                   255
#define CONFIG_BT_BUF_EVT_RX_COUNT                  3
#define CONFIG_BT_BUF_EVT_DISCARDABLE_SIZE          43
#define CONFIG_BT_BUF_EVT_DISCARDABLE_COUNT         20
#define CONFIG_BT_BUF_CMD_TX_SIZE                   65
#define CONFIG_BT_BUF_CMD_TX_COUNT                  2

#define CONFIG_BT_RPA                               1
#define CONFIG_BT_HCI_HOST                          1
#define CONFIG_BT_HCI_TX_STACK_SIZE                 2048
#define CONFIG_BT_HCI_ECC_STACK_SIZE                1140
#define CONFIG_BT_RECV_BLOCKING                     1
#define CONFIG_BT_RX_STACK_SIZE                     2500
#define CONFIG_BT_DRIVER_RX_HIGH_PRIO               6
#define CONFIG_BT_LIM_ADV_TIMEOUT                   30
#define CONFIG_BT_CONN_TX_MAX                       3

/*
 * L2CAP Options
 */
#define CONFIG_BT_L2CAP_TX_BUF_COUNT                3
#define CONFIG_BT_L2CAP_TX_FRAG_COUNT               2
#define CONFIG_BT_L2CAP_TX_MTU                      69

/*
 * ATT and GATT Options
 */
#define CONFIG_BT_ATT_ENFORCE_FLOW                  1
#define CONFIG_BT_EATT_SEC_LEVEL                    1
#define CONFIG_BT_GATT_AUTO_SEC_REQ                 1
#define CONFIG_BT_GATT_SERVICE_CHANGED              1
#define CONFIG_BT_GATT_CACHING                      1
#define CONFIG_BT_GATT_READ_MULTIPLE                1

#define CONFIG_BT_GAP_AUTO_UPDATE_CONN_PARAMS       1
#define CONFIG_BT_GAP_PERIPHERAL_PREF_PARAMS        1
#define CONFIG_BT_PERIPHERAL_PREF_MIN_INT           24
#define CONFIG_BT_PERIPHERAL_PREF_MAX_INT           40
#define CONFIG_BT_PERIPHERAL_PREF_LATENCY           0
#define CONFIG_BT_PERIPHERAL_PREF_TIMEOUT           42
#define CONFIG_DEVICE_NAME_GATT_WRITABLE_ENCRYPT    1
#define CONFIG_BT_CREATE_CONN_TIMEOUT               3
#define CONFIG_BT_CONN_PARAM_UPDATE_TIMEOUT         5000
#define CONFIG_BT_BACKGROUND_SCAN_INTERVAL          2048
#define CONFIG_BT_BACKGROUND_SCAN_WINDOW            18


#define CONFIG_BT_DEVICE_APPEARANCE                 0
#define CONFIG_BT_ID_MAX                            1
#define CONFIG_BT_ECC                               1
#define CONFIG_BT_HOST_CCM                          1

#define CONFIG_WORK_QUEUE_TASK_STACK_SIZE           4096
#define CONFIG_BT_CONN_PARAM_RETRY_COUNT            3
#define CONFIG_BT_CONN_PARAM_RETRY_TIMEOUT          5000


#define DEBUG_CONSOLE_RX_ENABLE                     0

//#define SDK_DEBUGCONSOLE                            DEBUGCONSOLE_DISABLE


#define OSA_USED                                    1
//#define CONFIG_BT_SNOOP 1

#define CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT          1

#include "edgefast_bluetooth_config.h"

#include "edgefast_bluetooth_debug_config.h"
