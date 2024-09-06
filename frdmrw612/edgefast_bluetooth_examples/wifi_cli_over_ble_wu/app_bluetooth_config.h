/*
 *  Copyright 2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#define USB_HOST_CONFIG_EHCI (1U)
#define CONTROLLER_ID        kUSB_ControllerEhci0

#define SERIAL_PORT_TYPE_BLE_WU            1
#define DEBUG_CONSOLE_TRANSMIT_BUFFER_LEN  8192
#define DEBUG_CONSOLE_TX_RELIABLE_ENABLE   0

#define BUTTON_SHORT_PRESS_THRESHOLD       500U
#define BUTTON_LONG_PRESS_THRESHOLD        800U
#define FLASH_ADAPTER_SIZE                 0x10000
#define CONFIG_BT_GATT_CLIENT              1
#define BUTTON_COUNT                       1
#define CONFIG_BT_CENTRAL                  1
#define CONFIG_BT_OBSERVER                 1
#define DEBUG_CONSOLE_RX_ENABLE            1
#define OSA_USED                           1
#define CONFIG_BT_MAX_CONN                 16
#define CONFIG_BT_L2CAP_TX_BUF_COUNT       8
#define CONFIG_BT_DIS_MODEL                "Wireless UART Demo"
#define CONFIG_BT_PERIPHERAL               1
#define CONFIG_BT_DEVICE_NAME              "NXP_WU"
#define CONFIG_BT_SMP                      1
#define CONFIG_BT_SETTINGS                 1
#define CONFIG_BT_HOST_CRYPTO              1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST    1
#define porting_errno                      1

#define CONFIG_WORK_QUEUE_TASK_STACK_SIZE 4096

#define CONFIG_BT_SNOOP 0

#define CONFIG_BT_MAX_PAIRED 16

#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
    #define CONFIG_BT_RX_STACK_SIZE 3000
#else
    #define CONFIG_BT_RX_STACK_SIZE 1024
#endif

#ifndef BOARD_USER_BUTTON_GPIO
#define BOARD_USER_BUTTON_GPIO BOARD_SW2_GPIO_PORT
#endif
#ifndef BOARD_USER_BUTTON_GPIO_PIN
#define BOARD_USER_BUTTON_GPIO_PIN (BOARD_SW2_GPIO_PIN)
#endif
#define BOARD_USER_BUTTON_IRQ  GPIO_INTA_IRQn
#define BOARD_USER_BUTTON_NAME "SW2"

#define WIFI_BOARD_RW610

#if defined(WIFI_BOARD_RW610)
#define RW610
#define WIFI_BT_USE_IMU_INTERFACE
#define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_WW_rw610.h"
#else
#error "Please define macro for RW610 board"
#endif

#include "wifi_config.h"
/* Due to the changes in wifi configuration,
   please do not use '#undef' to diasble wifi uart task. */
#define CONFIG_UART_INTERRUPT 0

#include "edgefast_bluetooth_config.h"
#include "edgefast_bluetooth_debug_config.h"
#include "monolithic_config.h"