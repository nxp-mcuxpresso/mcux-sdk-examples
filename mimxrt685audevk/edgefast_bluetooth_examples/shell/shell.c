/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno/errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <porting.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <bluetooth/services/bas.h>

#include "fsl_shell.h"

#include "shell.h"

#include "db_gen.h"

#include "fsl_debug_console.h"
#include "host_msd_fatfs.h"

#include "shell_bt.h"

extern serial_handle_t g_serialHandle;

SDK_ALIGN(static uint8_t shell_handle_buffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t shell_handle;

void edgefast_bt_pal_shell_task(void *pvParameters)
{
    uint8_t level = 0;
    PRINTF("Edgefast Bluetooth PAL shell demo start...\n");

    shell_handle = (shell_handle_t)&shell_handle_buffer[0];
    if (kStatus_SHELL_Success != SHELL_Init(shell_handle, g_serialHandle, ""))
    {
        PRINTF("Shell initialization failed!\r\n");
        return;
    }

    bt_CommandInit(shell_handle);

    while (1)
    {
        vTaskDelay(1000);
        bt_bas_set_battery_level(level++);
        if (level > 100)
        {
            level = 0;
        }
    }
}
