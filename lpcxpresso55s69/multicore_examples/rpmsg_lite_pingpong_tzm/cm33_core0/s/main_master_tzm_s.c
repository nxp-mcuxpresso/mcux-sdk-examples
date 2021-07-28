/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if (__ARM_FEATURE_CMSE & 1) == 0
#error "Need ARMv8-M security extensions"
#elif (__ARM_FEATURE_CMSE & 2) == 0
#error "Compile with --cmse"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpmsg_lite.h"
#include "veneer_table.h"
#include "tzm_config.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "mcmgr.h"

#include "fsl_common.h"
#include "fsl_power.h"
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
#define REMOTE_EPT_ADDR               (30U)
#define LOCAL_EPT_ADDR                (40U)
#define APP_RPMSG_READY_EVENT_DATA    (1U)
#define APP_RPMSG_EP_READY_EVENT_DATA (2U)

#define NON_SECURE_START 0x00010000

typedef struct the_message
{
    uint32_t DATA;
} THE_MESSAGE, *THE_MESSAGE_PTR;

#define SH_MEM_TOTAL_SIZE (6144U)
#if defined(__ICCARM__) /* IAR Workbench */
#pragma location = "rpmsg_sh_mem_section"
char rpmsg_lite_base[SH_MEM_TOTAL_SIZE];
#elif defined(__GNUC__)
char rpmsg_lite_base[SH_MEM_TOTAL_SIZE] __attribute__((section(".noinit.$rpmsg_sh_mem")));
#elif defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)) /* Keil MDK */
char rpmsg_lite_base[SH_MEM_TOTAL_SIZE] __attribute__((section("rpmsg_sh_mem_section")));
#else
#error "RPMsg: Please provide your definition of rpmsg_lite_base[]!"
#endif

/* typedef for non-secure callback functions */
typedef void (*funcptr_ns)(void) __attribute__((cmse_nonsecure_call));

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
THE_MESSAGE volatile msg = {0};
extern struct rpmsg_lite_instance *rpmsg_lite_instance_s;
struct rpmsg_lite_instance rpmsg_lite_ctxt_s;

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
    PRINTF("Primary core received a msg\r\n");
    PRINTF("Message: Size=%x, DATA = %i\r\n", (unsigned int)payload_len, (int)msg.DATA);
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
    /* The TrustZone should be configured as early as possible after RESET.
       Therefore it is called from SystemInit() during startup. The SystemInitHook()
       weak function overloading is used for this purpose. */
    BOARD_InitTrustZone();

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
    struct rpmsg_lite_ept_static_context my_ept_context_s;
    struct rpmsg_lite_endpoint *my_ept_s;
    funcptr_ns ResetHandler_ns;

    /* Initialize standard SDK demo application pins */
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Check the silicone version */
    if (0x0 == Chip_GetVersion())
    {
        /* The rpmsg_lite_pingpong_tzm example does not work correctly in A0 silicone version */
        assert(false);
    }

#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* This section ensures the secondary core image is copied from flash location to the target RAM memory.
       It consists of several steps: image size calculation and image copying.
       These steps are not required on MCUXpresso IDE which copies the secondary core image to the target memory during
       startup automatically. */
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
    (void)PRINTF("\r\nData exchange in secure domain\r\n");

    /* Wait until the secondary core application signals the rpmsg remote has been initialized and is ready to
     * communicate. */
    while (APP_RPMSG_READY_EVENT_DATA != RPMsgRemoteReadyEventData)
    {
    };

    rpmsg_lite_instance_s =
        rpmsg_lite_master_init(rpmsg_lite_base, SH_MEM_TOTAL_SIZE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS, &rpmsg_lite_ctxt_s);

    my_ept_s = rpmsg_lite_create_ept(rpmsg_lite_instance_s, LOCAL_EPT_ADDR, my_ept_read_cb, (void *)&has_received,
                                     &my_ept_context_s);

    has_received = 0;

    /* Wait until the secondary core application signals the rpmsg remote endpoint has been created. */
    while (APP_RPMSG_EP_READY_EVENT_DATA != RPMsgRemoteReadyEventData)
    {
    };

    /* Send the first message to the remoteproc */
    msg.DATA = 0U;
    (void)rpmsg_lite_send(rpmsg_lite_instance_s, my_ept_s, REMOTE_EPT_ADDR, (char *)&msg, sizeof(THE_MESSAGE),
                          RL_DONT_BLOCK);

    while (msg.DATA <= 50U)
    {
        if (1 == has_received)
        {
            has_received = 0;
            msg.DATA++;
            (void)rpmsg_lite_send(rpmsg_lite_instance_s, my_ept_s, REMOTE_EPT_ADDR, (char *)&msg, sizeof(THE_MESSAGE),
                                  RL_DONT_BLOCK);
        }
    }

    /* jump to the non-secure domain */
    /* Set non-secure main stack (MSP_NS) */
    __TZ_set_MSP_NS(*((uint32_t *)(NON_SECURE_START)));

    /* Set non-secure vector table */
    SCB_NS->VTOR = NON_SECURE_START;

    /* Get non-secure reset handler */
    ResetHandler_ns = (funcptr_ns)(*((uint32_t *)((NON_SECURE_START) + 4U)));

    /* Call non-secure application */
    (void)PRINTF("\r\nEntering normal world now.\r\n");
    /* Jump to normal world */
    ResetHandler_ns();
    for (;;)
    {
        /* This point should never be reached */
    }
}
