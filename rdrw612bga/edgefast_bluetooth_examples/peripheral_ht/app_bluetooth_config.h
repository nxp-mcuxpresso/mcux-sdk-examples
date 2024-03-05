/*
 *  Copyright 2020-2021 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#define USB_HOST_CONFIG_EHCI (1U)
#define CONTROLLER_ID        kUSB_ControllerEhci0

#define CONFIG_BT_PERIPHERAL            1
#define CONFIG_BT_DEVICE_NAME           "peripheral_ht"
#define CONFIG_BT_PHY_UPDATE            1
#define CONFIG_BT_AUTO_PHY_UPDATE       1
#define CONFIG_BT_DATA_LEN_UPDATE       1
#define CONFIG_BT_AUTO_DATA_LEN_UPDATE  1
#define CONFIG_BT_SMP                   1
#define CONFIG_BT_SETTINGS              1
#define CONFIG_BT_HOST_CRYPTO           1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 1
#include "edgefast_bluetooth_config.h"

#include "edgefast_bluetooth_debug_config.h"

/* Enable/Disable sensors module for temperature measurements */
#define APP_USE_SENSORS      1

/* Enable/Disable low power entry on tickless idle */
#define APP_LOWPOWER_ENABLED 1

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
/* If low power is enabled, force tickless idle enable in FreeRTOS */
#define configUSE_TICKLESS_IDLE 1
#endif
