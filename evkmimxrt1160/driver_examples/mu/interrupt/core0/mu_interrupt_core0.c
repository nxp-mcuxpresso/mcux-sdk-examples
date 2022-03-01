/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_mu.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_MU            MUA
#define APP_MU_IRQn       MUA_IRQn
#define APP_MU_IRQHandler MUA_IRQHandler

#define BOOT_CORE1_BY_MU 0

#define CORE1_BOOT_ADDRESS 0x20200000

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
extern unsigned char core1_image_start[];
#define CORE1_IMAGE_START core1_image_start
#elif defined(__GNUC__)
extern const char core1_image_start[];
extern const char *core1_image_end;
extern int core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif
/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U

/* Channel transmit and receive register */
#define CHN_MU_REG_NUM kMU_MsgReg0

/* How many message is used to test message sending */
#define MSG_LENGTH 32U

/*
 * Use core 0 to boot core 1.
 */
#ifndef CORE0_BOOT_CORE1
#define CORE0_BOOT_CORE1 1
#endif

/* Use MU to boot core 1. */
#ifndef BOOT_CORE1_BY_MU
#define BOOT_CORE1_BY_MU 1
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_BootCore1(void);
#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t g_msgSend[MSG_LENGTH];
static uint32_t g_msgRecv[MSG_LENGTH];
volatile uint32_t g_curSend = 0;
volatile uint32_t g_curRecv = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * For the dual core project, generally primary core starts first, initializes
 * the system, then starts the secondary core to run.
 * In the case that debugging dual-core at the same time (for example, using IAR+DAPLink),
 * the secondary core is started by debugger. Then the secondary core might
 * run when the primary core initialization not finished. The SRC->GPR is used
 * here to indicate whether secondary core could go. When started, the secondary core
 * should check and wait the flag in SRC->GPR, the primary core sets the
 * flag in SRC->GPR when its initialization work finished.
 */
#define BOARD_SECONDARY_CORE_GO_FLAG 0xa5a5a5a5u
#define BOARD_SECONDARY_CORE_SRC_GPR kSRC_GeneralPurposeRegister20

void BOARD_SetSecondaryCoreGoFlag(void)
{
    SRC_SetGeneralPurposeRegister(SRC, BOARD_SECONDARY_CORE_SRC_GPR, BOARD_SECONDARY_CORE_GO_FLAG);
}

static void BOARD_InitLedPin(void)
{
    const gpio_pin_config_t config = {
        .direction     = kGPIO_DigitalOutput,
        .outputLogic   = 1,
        .interruptMode = kGPIO_NoIntmode,
    };

    GPIO_PinInit(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, &config);
}

void APP_BootCore1(void)
{
    IOMUXC_LPSR_GPR->GPR0 = IOMUXC_LPSR_GPR_GPR0_CM4_INIT_VTOR_LOW(CORE1_BOOT_ADDRESS >> 3);
    IOMUXC_LPSR_GPR->GPR1 = IOMUXC_LPSR_GPR_GPR1_CM4_INIT_VTOR_HIGH(CORE1_BOOT_ADDRESS >> 16);

    /* Read back to make sure write takes effect. */
    (void)IOMUXC_LPSR_GPR->GPR0;
    (void)IOMUXC_LPSR_GPR->GPR1;

    /* If CM4 is already running (released by debugger), then reset it.
       If CM4 is not running, release it. */
    if ((SRC->SCR & SRC_SCR_BT_RELEASE_M4_MASK) == 0)
    {
        SRC_ReleaseCoreReset(SRC, kSRC_CM4Core);
    }
    else
    {
        SRC_AssertSliceSoftwareReset(SRC, kSRC_M4CoreSlice);
    }

    BOARD_SetSecondaryCoreGoFlag();
}

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size()
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


/*!
 * @brief Function to fill the g_msgSend array.
 * This function set the g_msgSend values 0, 1, 2, 3...
 */
static void FillMsgSend(void)
{
    uint32_t i;
    for (i = 0U; i < MSG_LENGTH; i++)
    {
        g_msgSend[i] = i;
    }
}

/*!
 * @brief Function to clear the g_msgRecv array.
 * This function set g_msgRecv to be 0.
 */
static void ClearMsgRecv(void)
{
    uint32_t i;
    for (i = 0U; i < MSG_LENGTH; i++)
    {
        g_msgRecv[i] = 0U;
    }
}

/*!
 * @brief Function to validate the received messages.
 * This function compares the g_msgSend and g_msgRecv, if they are the same, this
 * function returns true, otherwise returns false.
 */
static bool ValidateMsgRecv(void)
{
    uint32_t i;
    for (i = 0U; i < MSG_LENGTH; i++)
    {
        PRINTF("Send: %d. Receive %d\r\n", g_msgSend[i], g_msgRecv[i]);

        if (g_msgRecv[i] != g_msgSend[i])
        {
            return false;
        }
    }
    return true;
}

void APP_MU_IRQHandler(void)
{
    uint32_t flag = 0;

    flag = MU_GetStatusFlags(APP_MU);
    if ((flag & kMU_Tx0EmptyFlag) == kMU_Tx0EmptyFlag)
    {
        if (g_curSend < MSG_LENGTH)
        {
            MU_SendMsgNonBlocking(APP_MU, CHN_MU_REG_NUM, g_msgSend[g_curSend++]);
        }
        else
        {
            MU_DisableInterrupts(APP_MU, kMU_Tx0EmptyInterruptEnable);
        }
    }
    if ((flag & kMU_Rx0FullFlag) == kMU_Rx0FullFlag)
    {
        if (g_curRecv < MSG_LENGTH)
        {
            g_msgRecv[g_curRecv++] = MU_ReceiveMsgNonBlocking(APP_MU, CHN_MU_REG_NUM);
        }
        else
        {
            MU_DisableInterrupts(APP_MU, kMU_Rx0FullInterruptEnable);
        }
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Function to copy core1 image to execution address.
 */
static void APP_CopyCore1Image(void)
{
#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image  - not required on MCUXpresso IDE. MCUXpresso copies the secondary core
       image to the target memory during startup automatically */
    uint32_t core1_image_size = get_core1_image_size();

    PRINTF("Copy Secondary core image to address: 0x%x, size: %d\r\n", CORE1_BOOT_ADDRESS, core1_image_size);

    /* Copy Secondary core application from FLASH to the target memory. */
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache_by_Addr((void *)CORE1_BOOT_ADDRESS, core1_image_size);
#endif
    memcpy((void *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache_by_Addr((void *)CORE1_BOOT_ADDRESS, core1_image_size);
#endif
#endif
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware.*/
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitLedPin();
    NVIC_EnableIRQ(APP_MU_IRQn);

    APP_CopyCore1Image();

    /* MUA init */
    MU_Init(APP_MU);

#if CORE0_BOOT_CORE1
    /* Boot core 1. */
#if BOOT_CORE1_BY_MU
    MU_BootCoreB(APP_MU, APP_CORE1_BOOT_MODE);
#else
    APP_BootCore1();
#endif
#endif

    /* Print the initial banner */
    PRINTF("\r\nMU example interrupt!\r\n");

    /* Wait Core 1 is Boot Up */
    while (BOOT_FLAG != MU_GetFlags(APP_MU))
    {
    }

    /* Fill the g_msgSend array before send */
    FillMsgSend();
    /* Clear the g_msgRecv array before receive */
    ClearMsgRecv();
    /* Enable transmit and receive interrupt */
    MU_EnableInterrupts(APP_MU, (kMU_Tx0EmptyInterruptEnable | kMU_Rx0FullInterruptEnable));
    /* Wait the data send and receive finish */
    while ((g_curSend < MSG_LENGTH) || (g_curRecv < MSG_LENGTH))
    {
    }

    /* Compare the data send and receive */
    if (true == ValidateMsgRecv())
    {
        PRINTF("MU example run succeed!");
    }
    else
    {
        PRINTF("MU example run Error!");
    }

    while (1)
    {
    }
}
