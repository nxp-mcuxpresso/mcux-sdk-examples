/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "task.h"

#include "fsl_lpuart.h"
#include "fsl_irqsteer.h"
#include "app_srtm.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RPMSG_LITE_SHMEM_BASE         (VDEV1_VRING_BASE)
#define RPMSG_LITE_LINK_ID            (RL_PLATFORM_IMX8QX_CM4_USER_LINK_ID)
#define RPMSG_LITE_NS_ANNOUNCE_STRING "rpmsg-openamp-demo-channel"
#define RPMSG_LITE_MASTER_IS_LINUX
#define APP_TASK_STACK_SIZE (256)
#ifndef LOCAL_EPT_ADDR
#define LOCAL_EPT_ADDR (30)
#endif

/* Globals */
static char app_buf[512]; /* Each RPMSG buffer can carry less than 512 payload */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static TaskHandle_t app_task_handle = NULL;

static void app_task(void *param)
{
    volatile uint32_t remote_addr;
    struct rpmsg_lite_endpoint *volatile my_ept;
    volatile rpmsg_queue_handle my_queue;
    struct rpmsg_lite_instance *volatile my_rpmsg;
    void *rx_buf;
    uint32_t len;
    int32_t result;
    void *tx_buf;
    uint32_t size;

    /* Print the initial banner */
    PRINTF("\r\nRPMSG String Echo FreeRTOS RTOS API Demo...\r\n");

#ifdef MCMGR_USED
    uint32_t startupData;

    /* Get the startup data */
    (void)MCMGR_GetStartupData(kMCMGR_Core1, &startupData);

    my_rpmsg = rpmsg_lite_remote_init((void *)startupData, RPMSG_LITE_LINK_ID, RL_NO_FLAGS);

    /* Signal the other core we are ready */
    (void)MCMGR_SignalReady(kMCMGR_Core1);
#else
    my_rpmsg = rpmsg_lite_remote_init((void *)RPMSG_LITE_SHMEM_BASE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS);
#endif /* MCMGR_USED */

    while (0 == rpmsg_lite_is_link_up(my_rpmsg))
        ;

    my_queue = rpmsg_queue_create(my_rpmsg);
    my_ept   = rpmsg_lite_create_ept(my_rpmsg, LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, my_queue);
    (void)rpmsg_ns_announce(my_rpmsg, my_ept, RPMSG_LITE_NS_ANNOUNCE_STRING, RL_NS_CREATE);

    PRINTF("\r\nNameservice sent, ready for incoming messages...\r\n");

    for (;;)
    {
        /* Get RPMsg rx buffer with message */
        result =
            rpmsg_queue_recv_nocopy(my_rpmsg, my_queue, (uint32_t *)&remote_addr, (char **)&rx_buf, &len, RL_BLOCK);
        if (result != 0)
        {
            assert(false);
        }

        /* Copy string from RPMsg rx buffer */
        assert(len < sizeof(app_buf));
        memcpy(app_buf, rx_buf, len);
        app_buf[len] = 0; /* End string by '\0' */

        if ((len == 2) && (app_buf[0] == 0xd) && (app_buf[1] == 0xa))
            PRINTF("Get New Line From Master Side\r\n");
        else
            PRINTF("Get Message From Master Side : \"%s\" [len : %d]\r\n", app_buf, len);

        /* Get tx buffer from RPMsg */
        tx_buf = rpmsg_lite_alloc_tx_buffer(my_rpmsg, &size, RL_BLOCK);
        assert(tx_buf);
        /* Copy string to RPMsg tx buffer */
        memcpy(tx_buf, app_buf, len);
        /* Echo back received message with nocopy send */
        result = rpmsg_lite_send_nocopy(my_rpmsg, my_ept, remote_addr, tx_buf, len);
        if (result != 0)
        {
            assert(false);
        }
        /* Release held RPMsg rx buffer */
        result = rpmsg_queue_nocopy_free(my_rpmsg, rx_buf);
        if (result != 0)
        {
            assert(false);
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialize standard SDK demo application pins */
    uint32_t freq;
    sc_pm_clock_rate_t src_rate = SC_133MHZ;

    sc_ipc_t ipc = BOARD_InitRpc();
    BOARD_InitPins(ipc);
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitMemory();

    /* Power on LPI2C. */
    if (sc_pm_set_resource_power_mode(ipc, SC_R_M4_0_I2C, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on LPI2C\r\n");
    }
    /* Set LPI2C clock */
    freq = CLOCK_SetIpFreq(kCLOCK_M4_0_Lpi2c, SC_24MHZ);
    if (freq == 0)
    {
        PRINTF("Error: Failed to set LPI2C frequency\r\n");
    }
    if (!CLOCK_EnableClockExt(kCLOCK_M4_0_Lpi2c, 0))
    {
        PRINTF("Error: sc_pm_clock_enable failed\r\n");
    }

    /*
     * LPI2C1
     */

    if (sc_pm_set_resource_power_mode(ipc, SC_R_I2C_1, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on LPI2C\r\n");
    }
    if (sc_pm_clock_enable(ipc, SC_R_I2C_1, SC_PM_CLK_PER, true, false) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to enable SC_R_I2C_1 clock \r\n");
    }

    if (sc_pm_set_clock_rate(ipc, SC_R_I2C_1, SC_PM_CLK_PER, &src_rate) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to set SC_R_I2C_1 clock rate\r\n");
    }

    /*
     *
     */
    if (sc_pm_set_resource_power_mode(ipc, SC_R_MU_5B, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on MU_5B!\r\n");
    }

    if (sc_pm_set_resource_power_mode(ipc, SC_R_IRQSTR_M4_0, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on IRQSTEER!\r\n");
    }

    IRQSTEER_Init(IRQSTEER);
    IRQSTEER_EnableInterrupt(IRQSTEER, LSIO_MU8_INT_B_IRQn);

    if (sc_pm_set_resource_power_mode(ipc, SC_R_MU_8B, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on MU_8B!\r\n");
    }

    APP_SRTM_Init();
    APP_SRTM_StartCommunication();

#ifdef MCMGR_USED
    /* Initialize MCMGR before calling its API */
    (void)MCMGR_Init();
#endif /* MCMGR_USED */

    if (xTaskCreate(app_task, "APP_TASK", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &app_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create application task\r\n");
        for (;;)
            ;
    }

    vTaskStartScheduler();

    PRINTF("Failed to start FreeRTOS on core0.\n");
    for (;;)
        ;
}
