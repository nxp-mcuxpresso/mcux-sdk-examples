/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "lpm.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_mu.h"
#include "mcmgr.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CPU_NAME "iMXRT1189"

#define APP_WAKEUP_BUTTON_GPIO        BOARD_USER_BUTTON_GPIO
#define APP_WAKEUP_BUTTON_GPIO_PIN    BOARD_USER_BUTTON_GPIO_PIN
#define APP_WAKEUP_BUTTON_IRQ         BOARD_USER_BUTTON_IRQ
#define APP_WAKEUP_BUTTON_IRQ_HANDLER BOARD_USER_BUTTON_IRQ_HANDLER
#define APP_WAKEUP_BUTTON_NAME        BOARD_USER_BUTTON_NAME

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void GPIO_ClearStopRequest(void);
void GPIO_SetStopRequest(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile uint32_t g_msgRecv;
static volatile bool isMsgReceived = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void GPIO_ClearStopRequest(void)
{
    CCM->GPR_SHARED8 &= ~CCM_GPR_SHARED8_m33_gpio1_ipg_stop_MASK;
    CCM->GPR_SHARED12 &= ~CCM_GPR_SHARED12_m7_gpio1_ipg_stop_MASK;
}

void GPIO_SetStopRequest(void)
{
    CCM->GPR_SHARED8 |= CCM_GPR_SHARED8_m33_gpio1_ipg_stop_MASK;
    CCM->GPR_SHARED12 |= CCM_GPR_SHARED12_m7_gpio1_ipg_stop_MASK;
}

static void RemoteApplicationEventHandler(uint16_t eventData, void *context)
{
    g_msgRecv     = eventData;
    isMsgReceived = true;
}

#ifndef CORE1_GET_INPUT_FROM_CORE0
void APP_WAKEUP_BUTTON_IRQ_HANDLER(void)
{
    /* Clear GPIO stop request. */
    GPIO_ClearStopRequest();
	
    if ((1U << APP_WAKEUP_BUTTON_GPIO_PIN) &
        RGPIO_GetPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, kRGPIO_InterruptOutput0))
    {
        /* Disable interrupt. */
        RGPIO_SetPinInterruptConfig(APP_WAKEUP_BUTTON_GPIO, APP_WAKEUP_BUTTON_GPIO_PIN, kRGPIO_InterruptOutput0,
                                    kRGPIO_InterruptOrDMADisabled);
        RGPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, kRGPIO_InterruptOutput0,
                                      1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        GPC_DisableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
    }
    SDK_ISR_EXIT_BARRIER;
}
#endif

static void APP_SetWakeupConfig(void)
{
#ifndef CORE1_GET_INPUT_FROM_CORE0
    PRINTF("Press button %s to wake up system.\r\n", APP_WAKEUP_BUTTON_NAME);
    RGPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, kRGPIO_InterruptOutput0, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
    /* Enable GPIO pin interrupt */
    RGPIO_SetPinInterruptConfig(APP_WAKEUP_BUTTON_GPIO, APP_WAKEUP_BUTTON_GPIO_PIN, kRGPIO_InterruptOutput0,
                                kRGPIO_InterruptFallingEdge);
    NVIC_ClearPendingIRQ(APP_WAKEUP_BUTTON_IRQ);
    /* Enable the Interrupt */
    EnableIRQ(APP_WAKEUP_BUTTON_IRQ);
    /* Mask all interrupt first */
    GPC_DisableAllWakeupSource(CPU_SLICE);
    /* Enable GPC interrupt */
    GPC_EnableWakeupSource(APP_WAKEUP_BUTTON_IRQ);

    /* Request GPIO to stop, it is configured to exit low power mode via GPIO asynchronous wake up signal. */
    GPIO_SetStopRequest();
#else
    PRINTF("Wake up system via MU interrupt.\r\n");
    /* Enable receive interrupt */
    MU_EnableInterrupts(MU_BASE, kMU_Rx3FullInterruptEnable);
    /* Enable the Interrupt */
    EnableIRQ(MU_IRQ);
    /* Mask all interrupt first */
    GPC_DisableAllWakeupSource(CPU_SLICE);
    /* Enable GPC interrupt */
    GPC_EnableWakeupSource(MU_IRQ);
#endif
}

void RunModeSwitch(void)
{
    uint8_t ch, target;

    while (1)
    {
        PRINTF("\r\nRUN mode switch:\r\n");
        PRINTF("Press %c to enter OverDrive RUN\r\n", (uint8_t)'A');
        PRINTF("Press %c to enter Normal RUN\r\n", (uint8_t)'B');
        PRINTF("Press %c to enter UnderDrive RUN\r\n", (uint8_t)'C');
        PRINTF("Press 'Q' to exit\r\n");
        PRINTF("\r\nWaiting for select...\r\n");

#ifndef CORE1_GET_INPUT_FROM_CORE0
        /* Wait for user response */
        ch = GETCHAR();
#else
        while (!isMsgReceived)
            ;
        isMsgReceived = false;
        ch            = g_msgRecv;
#endif

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        if (ch == 'Q')
        {
            break;
        }
        target = (ch - 'A');

        if (target < 3)
        {
            uint32_t coreClk = CLOCK_GetRootClockFreq(kCLOCK_Root_M7);
            SDK_DelayAtLeastUs(100000U, coreClk);
            // run mode switch controlled by core0, here only print system status.
            PrintSystemStatus();
        }
    }
}

void CpuModeSwitch(void)
{
    bool sysSleepEn = false;
    uint8_t ch, target;
    gpc_cpu_mode_t cpuMode;

    while (1)
    {
        PRINTF("\r\nCPU mode switch:\r\n");
        PRINTF("Press %c to enter CPU mode: RUN\r\n", (uint8_t)'A' + kGPC_RunMode);
        PRINTF("Press %c to enter CPU mode: WAIT\r\n", (uint8_t)'A' + kGPC_WaitMode);
        PRINTF("Press %c to enter CPU mode: STOP\r\n", (uint8_t)'A' + kGPC_StopMode);
        PRINTF("Press %c to enter CPU mode: SUSPEND\r\n", (uint8_t)'A' + kGPC_SuspendMode);
        PRINTF("Press %c to enter CPU mode: STOP, system sleep\r\n", (uint8_t)'A' + 2 + kGPC_StopMode);
        PRINTF("Press %c to enter CPU mode: SUSPEND, system sleep\r\n", (uint8_t)'A' + 2 + kGPC_SuspendMode);
        PRINTF("Press 'Q' to exit\r\n");
        PRINTF("\r\nWaiting for select...\r\n");

#ifndef CORE1_GET_INPUT_FROM_CORE0
        /* Wait for user response */
        ch = GETCHAR();
#else
        while (!isMsgReceived)
            ;
        isMsgReceived = false;
        ch            = g_msgRecv;
#endif

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        if (ch == 'Q')
        {
            break;
        }
        target = (ch - 'A');

        if (target < 6)
        {
            if (target > 3)
            {
                sysSleepEn = true;
                target     = target - 2;
            }
            else
            {
                sysSleepEn = false;
            }
            cpuMode = (gpc_cpu_mode_t)target;
            if (cpuMode != kGPC_RunMode)
            {
                APP_SetWakeupConfig();
                PRINTF("Target CPU mode is %s\r\n", GET_CPU_MODE_NAME(cpuMode));
                if (sysSleepEn)
                {
                    PRINTF("System sleep\r\n");
                }
                PRINTF("Go...\r\n");
                CpuModeTransition(cpuMode, sysSleepEn);
                PrintSystemStatus();
            }
            else
            {
                PRINTF("CPU already in RUN mode!\r\n");
            }
        }
    }
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
 * @brief main demo function.
 */
int main(void)
{
    uint32_t startupData, i;
    mcmgr_status_t status;
    uint8_t ch;

    /* Init board hardware.*/
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_InitDebugConsole();
    // EnableIRQ(MU_IRQ);

    /* Initialize MCMGR, install generic event handlers */
    (void)MCMGR_Init();

    MU_EnableInterrupts(MU_BASE, kMU_GenInt0InterruptEnable);

    /* Get the startup data */
    do
    {
        status = MCMGR_GetStartupData(&startupData);
    } while (status != kStatus_MCMGR_Success);

    /* Make a noticable delay after the reset */
    /* Use startup parameter from the master core... */
    for (i = 0; i < startupData; i++)
    {
        SDK_DelayAtLeastUs(100000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    }

    /* Register the application event */
    (void)MCMGR_RegisterEvent(kMCMGR_RemoteApplicationEvent, RemoteApplicationEventHandler, ((void *)0));

    PRINTF("\r\nThis is core1.\r\n");

    PRINTF("\r\nCPU wakeup source 0x%x...\r\n", SRC_GENERAL_REG->SRSR);
    PRINTF("\r\n***********************************************************\r\n");
    PRINTF("\tPower Mode Switch Demo for %s\r\n", CPU_NAME);
    PRINTF("***********************************************************\r\n");

#ifdef CORE1_GET_INPUT_FROM_CORE0
    PRINTF("\r\nCore1 receive message from core0.\r\n");
#endif

    PrintSystemStatus();

    while (1)
    {
        PRINTF("\r\nPlease select the desired operation:\r\n");
        PRINTF("Press  %c to demonstrate run mode switch.\r\n", (uint8_t)'A');
        PRINTF("Press  %c to demonstrate cpu mode switch.\r\n", (uint8_t)'B');
        PRINTF("\r\nWaiting for select...\r\n");

#ifndef CORE1_GET_INPUT_FROM_CORE0
        /* Wait for user response */
        ch = GETCHAR();
#else
        while (!isMsgReceived)
            ;
        isMsgReceived = false;
        ch            = g_msgRecv;
#endif

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        switch (ch)
        {
            case 'A':
                RunModeSwitch();
                break;
            case 'B':
                CpuModeSwitch();
                break;
            default:
                break;
        }
    }
}
