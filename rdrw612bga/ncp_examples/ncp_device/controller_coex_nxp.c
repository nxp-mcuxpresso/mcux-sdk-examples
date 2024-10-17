/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#if defined(CONFIG_NCP_OT) && (CONFIG_NCP_OT == 1)
#include "ot_platform_common.h"
#include "fwk_platform_coex.h"
#elif defined(CONFIG_NCP_BLE) && (CONFIG_NCP_BLE == 1)
#include "fwk_platform_ble.h"
#endif

/* Initialize the platform */
void coex_controller_init(void)
{

#if defined(CONFIG_NCP_OT) && (CONFIG_NCP_OT == 1)
    /* Initialize 15.4+BLE combo controller first
     * Any other attemps to init a 15.4 or BLE single mode will be ignored
     * For BLE, ethermind calls controller_init which calls PLATFORM_InitBle which calls PLATFORM_InitControllers(connBle_c)
     * As we already initialized the controller in combo mode here, the PLATFORM_InitControllers(connBle_c) call does nothing
     * Same applies for PLATFORM_InitOt which calls PLATFORM_InitControllers(conn802_15_4_c)
     * For RW61x, 15.4/BLE combo image must be flashed at 15.4 firmware address Z154_IMAGE_A_OFFSET (0x85e0000) */
    PLATFORM_InitControllers(conn802_15_4_c | connBle_c);
#elif defined(CONFIG_NCP_BLE) && (CONFIG_NCP_BLE == 1)
    /* BTonly firmware download */
    PLATFORM_InitBle();
#endif
}
