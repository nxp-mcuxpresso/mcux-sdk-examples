/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpmsg_lite.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "mcmgr.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RPMSG_LITE_LINK_ID (RL_PLATFORM_K32L3A60_M4_M0_LINK_ID)

/* Address of memory, from which the secondary core will boot */
#define CORE1_BOOT_ADDRESS 0x01000000

#define REMOTE_EPT_ADDR               (30U)
#define LOCAL_EPT_ADDR                (40U)
#define APP_RPMSG_READY_EVENT_DATA    (1U)
#define APP_RPMSG_EP_READY_EVENT_DATA (2U)

typedef struct the_message
{
    uint32_t DATA;
} THE_MESSAGE, *THE_MESSAGE_PTR;

#define SH_MEM_TOTAL_SIZE (6144U)
#if defined(__ICCARM__) /* IAR Workbench */
#pragma location = "rpmsg_sh_mem_section"
static char rpmsg_lite_base[SH_MEM_TOTAL_SIZE];
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION) /* Keil MDK */
static char rpmsg_lite_base[SH_MEM_TOTAL_SIZE] __attribute__((section("rpmsg_sh_mem_section")));
#elif defined(__GNUC__)
static char rpmsg_lite_base[SH_MEM_TOTAL_SIZE] __attribute__((section(".noinit.$rpmsg_sh_mem")));
#else
#error "RPMsg: Please provide your definition of rpmsg_lite_base[]!"
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static THE_MESSAGE volatile msg = {0};

/* This is the read callback, note we are in a task context when this callback
is invoked, so kernel primitives can be used freely */
static int32_t my_ept_read_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    int32_t *has_received = priv;

    if (payload_len <= sizeof(THE_MESSAGE))
    {
        (void)memcpy((void *)&msg, payload, payload_len);
        *has_received = 1;
    }
    (void)PRINTF("Primary core received a msg\r\n");
    (void)PRINTF("Message: Size=%x, DATA = %i\r\n", payload_len, msg.DATA);
    return RL_RELEASE;
}

static void RPMsgRemoteReadyEventHandler(uint16_t eventData, void *context)
{
    uint16_t *data = (uint16_t *)context;

    *data = eventData;
}

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
    (void)MCMGR_EarlyInit();
}

/*!
 * @brief Main function
 */
int main(void)
{
    volatile int32_t has_received;
    volatile uint16_t RPMsgRemoteReadyEventData = 0;
    struct rpmsg_lite_ept_static_context my_ept_context;
    struct rpmsg_lite_endpoint *my_ept;
    struct rpmsg_lite_instance rpmsg_ctxt;
    struct rpmsg_lite_instance *my_rpmsg;

    /* Initialize standard SDK demo application pins */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image */
    uint32_t core1_image_size;
    core1_image_size = get_core1_image_size();
    (void)PRINTF("Copy CORE1 image to address: 0x%x, size: %d\r\n", (void *)(char *)CORE1_BOOT_ADDRESS,
                 core1_image_size);

    /* Copy application from FLASH to RAM */
    (void)memcpy((void *)(char *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#endif

    /* Initialize MCMGR before calling its API */
    (void)MCMGR_Init();

    /* Register the application event before starting the secondary core */
    (void)MCMGR_RegisterEvent(kMCMGR_RemoteApplicationEvent, RPMsgRemoteReadyEventHandler,
                              (void *)&RPMsgRemoteReadyEventData);

    /* Boot Secondary core application */
    (void)MCMGR_StartCore(kMCMGR_Core1, (void *)(char *)CORE1_BOOT_ADDRESS, (uint32_t)rpmsg_lite_base,
                          kMCMGR_Start_Synchronous);

    /* Print the initial banner */
    (void)PRINTF("\r\nRPMsg demo starts\r\n");

    /* Wait until the secondary core application signals the rpmsg remote has been initialized and is ready to
     * communicate. */
    while (APP_RPMSG_READY_EVENT_DATA != RPMsgRemoteReadyEventData)
    {
    };

    my_rpmsg = rpmsg_lite_master_init(rpmsg_lite_base, SH_MEM_TOTAL_SIZE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS, &rpmsg_ctxt);

    my_ept = rpmsg_lite_create_ept(my_rpmsg, LOCAL_EPT_ADDR, my_ept_read_cb, (void *)&has_received, &my_ept_context);

    has_received = 0;

    /* Wait until the secondary core application signals the rpmsg remote endpoint has been created. */
    while (APP_RPMSG_EP_READY_EVENT_DATA != RPMsgRemoteReadyEventData)
    {
    };

    /* Send the first message to the remoteproc */
    msg.DATA = 0U;
    (void)rpmsg_lite_send(my_rpmsg, my_ept, REMOTE_EPT_ADDR, (char *)&msg, sizeof(THE_MESSAGE), RL_DONT_BLOCK);

    while (msg.DATA <= 100U)
    {
        if (1 == has_received)
        {
            has_received = 0;
            msg.DATA++;
            (void)rpmsg_lite_send(my_rpmsg, my_ept, REMOTE_EPT_ADDR, (char *)&msg, sizeof(THE_MESSAGE), RL_DONT_BLOCK);
        }
    }

    (void)rpmsg_lite_destroy_ept(my_rpmsg, my_ept);
    my_ept = ((void *)0);
    (void)rpmsg_lite_deinit(my_rpmsg);

    /* Print the ending banner */
    (void)PRINTF("\r\nRPMsg demo ends\r\n");
    for (;;)
    {
    }
}
