/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpmsg_lite.h"
#include "veneer_table.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RPMSG_LITE_LINK_ID (RL_PLATFORM_LPC55S69_M33_M33_LINK_ID)

/* Address of RAM, where the image for core1 should be copied */
#define CORE1_BOOT_ADDRESS 0x30033000

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
extern unsigned char core1_image_start[];
#define CORE1_IMAGE_START core1_image_start
#elif (defined(__GNUC__)) && (!defined(__MCUXPRESSO))
extern const char core1_image_start[];
extern const char *core1_image_end;
extern uint32_t core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif
#define PRINTF_NSE DbgConsole_Printf_NSE

#define REMOTE_EPT_ADDR   (30U)
#define LOCAL_NS_EPT_ADDR (41U)

typedef struct the_message
{
    uint32_t DATA;
} THE_MESSAGE, *THE_MESSAGE_PTR;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void)
{
    uint32_t image_size;
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    image_size = (uint32_t)&Image$$CORE1_REGION$$Length;
#elif defined(__ICCARM__)
#pragma section = "__core1_image"
    image_size = (uint32_t)__section_end("__core1_image") - (uint32_t)&core1_image_start;
#elif defined(__GNUC__)
    image_size = (uint32_t)core1_image_size;
#endif
    return image_size;
}
#endif

void SystemInit(void)
{
}
THE_MESSAGE volatile msg = {0};

/* This is the read callback for the non-secure endpoint */
static int32_t my_ept_read_cb_ns(void *payload, int32_t payload_len, uint32_t src, void *priv)
{
    int32_t *has_received = priv;

    if (payload_len <= sizeof(THE_MESSAGE))
    {
        *has_received = 1;
    }
    return RL_RELEASE;
}

/*!
 * @brief Main function
 */
int main(void)
{
    volatile int32_t has_received;
    struct rpmsg_lite_endpoint *my_ept_ns;
    struct rpmsg_lite_endpoint_callback_data_descr_ns callback_data_ns = {(void *)&has_received, (void *)&msg};
    struct rpmsg_lite_endpoint_callback_descr_ns callback_descr_ns     = {(rl_ept_rx_cb_ns_t)my_ept_read_cb_ns,
                                                                          &callback_data_ns};
    char str[50];

    /* Initialize standard SDK demo application pins */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    // CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    // BOARD_InitBootPins();
    // BOARD_InitBootClocks();
    // BOARD_InitDebugConsole();

    /* Print the initial banner */
    PRINTF_NSE("\r\nNon-secure portion of the application started!\r\n");
    PRINTF_NSE("\r\nData exchange in non-secure domain\r\n");

    /* Create non-secure endpoint */
    my_ept_ns = rpmsg_lite_create_ept_nse(LOCAL_NS_EPT_ADDR, &callback_descr_ns);

    has_received = 0;

    /* Send the first message from the non-secure domain to the remoteproc */
    msg.DATA                                    = 1000U;
    struct rpmsg_lite_send_params_ns msg_params = {REMOTE_EPT_ADDR, (char *)&msg, sizeof(THE_MESSAGE)};
    (void)rpmsg_lite_send_nse(my_ept_ns, &msg_params, RL_DONT_BLOCK);

    while (msg.DATA <= 1050U)
    {
        if (1 == has_received)
        {
            PRINTF_NSE("Primary core received a msg\r\n");
            sprintf(str, "Message: DATA = %i\r\n", (int)msg.DATA);
            PRINTF_NSE(str);
            has_received = 0;
            msg.DATA++;
            (void)rpmsg_lite_send_nse(my_ept_ns, &msg_params, RL_DONT_BLOCK);
        }
    }

    rpmsg_lite_destroy_ept_nse(my_ept_ns);

    /* Print the ending banner */
    PRINTF_NSE("\r\nRPMsg demo ends\r\n");

    for (;;)
    {
    }
}
