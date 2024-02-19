/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_mailbox.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PRIMARY_CORE_MAILBOX_CPU_ID   kMAILBOX_CM33_Core0
#define SECONDARY_CORE_MAILBOX_CPU_ID kMAILBOX_CM33_Core1

/* Address of RAM, where the image for core1 should be copied */
#define CORE1_BOOT_ADDRESS 0x2004E000

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
extern int core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif
#define START_EVENT 1234
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif

void start_secondary_core(uint32_t sec_core_boot_addr);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Shared variable by both cores, before changing of this variable the cores must
   first take mailbox mutex, after changing the shared variable must return mutex */
volatile uint32_t g_shared = 0;
/* For the flow control */
volatile bool g_secondary_core_started = false;

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

void start_secondary_core(uint32_t sec_core_boot_addr)
{
    /* Boot source for Core 1 from flash */
    SYSCON->CPBOOT = (sec_core_boot_addr & SYSCON_CPBOOT_CPBOOT_MASK);

    int32_t temp = SYSCON->CPUCTRL;
    temp |= 0xc0c48000;
    SYSCON->CPUCTRL = temp | SYSCON_CPUCTRL_CPU1RSTEN_MASK | SYSCON_CPUCTRL_CPU1CLKEN_MASK;
    SYSCON->CPUCTRL = (temp | SYSCON_CPUCTRL_CPU1CLKEN_MASK) & (~SYSCON_CPUCTRL_CPU1RSTEN_MASK);
}
void MAILBOX_IRQHandler()
{
    if (START_EVENT == MAILBOX_GetValue(MAILBOX, PRIMARY_CORE_MAILBOX_CPU_ID))
    {
        g_secondary_core_started = true;
    }
    MAILBOX_ClearValueBits(MAILBOX, PRIMARY_CORE_MAILBOX_CPU_ID, 0xffffffff);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware.*/
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("Mailbox mutex example\r\n");

    /* Init Mailbox */
    MAILBOX_Init(MAILBOX);

    /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);

#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image */
    uint32_t core1_image_size;
    core1_image_size = get_core1_image_size();
    PRINTF("Copy CORE1 image to address: 0x%x, size: %d\r\n", CORE1_BOOT_ADDRESS, core1_image_size);

    /* Copy application from FLASH to RAM */
    memcpy((void *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#endif

    /* Start the secondary core */
    start_secondary_core(CORE1_BOOT_ADDRESS);

    /* Wait for start and initialization of secondary core */
    while (!g_secondary_core_started)
        ;

    /* Send address of shared variable to the secondary core */
    MAILBOX_SetValue(MAILBOX, SECONDARY_CORE_MAILBOX_CPU_ID, (uint32_t)&g_shared);

    while (1)
    {
        /* Get Mailbox mutex */
        while (MAILBOX_GetMutex(MAILBOX) == 0)
            ;

        /* The core0 has mutex, can change shared variable g_shared */
        g_shared++;

        PRINTF("Core0 has mailbox mutex, update shared variable to: %d\r\n", g_shared);

        /* Set mutex to allow access other core to shared variable */
        MAILBOX_SetMutex(MAILBOX);

        /* Add several nop instructions to allow the opposite core to get the mutex */
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
    }
}
