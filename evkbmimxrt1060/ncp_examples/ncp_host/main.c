/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

// SDK Included Files
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_os_abstraction.h"
#include "ncp_host_app.h"
#include "ncp_adapter.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern int ncp_system_app_init();
#if CONFIG_NCP_WIFI
extern int ncp_wifi_app_init();
#endif
#if CONFIG_NCP_BLE
extern int ncp_ble_app_init();
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

#define TASK_MAIN_STACK_SIZE   1500

#define TASK_MAIN_PRIO   1
static OSA_TASK_HANDLE_DEFINE(main_task_handle);
void task_main(void *param);
static OSA_TASK_DEFINE(task_main, TASK_MAIN_PRIO, 1, TASK_MAIN_STACK_SIZE, 0);

void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName)
{
    PRINTF("ERROR: stack overflow on task %s.\r\n", pcTaskName);

    /* Disable interrupts to avoid context switch */
    taskDISABLE_INTERRUPTS();

    /* Unused Parameters */
    (void)xTask;
    (void)pcTaskName;

    /* Loop forever */
    for (;;)
    {
    }
}

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

void task_main(void *param)
{
    int32_t result = 0;
    (void)result;

    PRINTF("Initialize NCP Host APP\r\n");
    printSeparator();

    result = ncp_host_app_init();
    assert(NCP_SUCCESS == result);
    result = ncp_adapter_init();
    assert(NCP_SUCCESS == result);

    result = ncp_system_app_init();
    assert(NCP_SUCCESS == result);
#if CONFIG_NCP_WIFI
    result = ncp_wifi_app_init();
    assert(NCP_SUCCESS == result);
#endif
#if CONFIG_NCP_BLE
    result = ncp_ble_app_init();
    assert(NCP_SUCCESS == result);
#endif

    printSeparator();

    while (1)
    {
        /* wait for interface up */
        OSA_TimeDelay(5000);
    }
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
int main(void)
{
    BaseType_t result = 0;
    (void)result;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    //    POWER_PowerOffBle();

    printSeparator();
    PRINTF("NCP Host APP\r\n");
    printSeparator();

    (void)OSA_TaskCreate((osa_task_handle_t)main_task_handle, OSA_TASK(task_main), NULL);

    OSA_Start();
    for (;;)
        ;
}
