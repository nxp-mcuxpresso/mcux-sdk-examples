/*
 * Copyright 2020 NXP
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

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CPU_NAME "iMXRT1176"

#define APP_WAKEUP_BUTTON_GPIO        BOARD_USER_BUTTON_GPIO
#define APP_WAKEUP_BUTTON_GPIO_PIN    BOARD_USER_BUTTON_GPIO_PIN
#define APP_WAKEUP_BUTTON_IRQ         BOARD_USER_BUTTON_IRQ
#define APP_WAKEUP_BUTTON_IRQ_HANDLER BOARD_USER_BUTTON_IRQ_HANDLER
#define APP_WAKEUP_BUTTON_NAME        BOARD_USER_BUTTON_NAME
/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U

/* Channel transmit and receive register */
#define CHN_MU_REG_NUM 0U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile uint32_t g_msgRecv;
static volatile bool isMsgReceived = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void MU_IRQ_HANDLER(void)
{
    if (kMU_Rx0FullFlag & MU_GetStatusFlags(MU_BASE))
    {
        g_msgRecv     = MU_ReceiveMsgNonBlocking(MU_BASE, CHN_MU_REG_NUM);
        isMsgReceived = true;
        /* We do not disable MU interrupt here since we always get input from core0. */
    }
    if (kMU_GenInt0Flag & MU_GetStatusFlags(MU_BASE))
    {
        MU_ClearStatusFlags(MU_BASE, kMU_GenInt0Flag);
    }
    SDK_ISR_EXIT_BARRIER;
}

#ifndef CORE1_GET_INPUT_FROM_CORE0
void APP_WAKEUP_BUTTON_IRQ_HANDLER(void)
{
    if ((1U << APP_WAKEUP_BUTTON_GPIO_PIN) & GPIO_GetPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO))
    {
        /* Disable interrupt. */
        GPIO_DisableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
        GPC_DisableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
    }
    SDK_ISR_EXIT_BARRIER;
}
#endif

static void APP_SetWakeupConfig(void)
{
#ifndef CORE1_GET_INPUT_FROM_CORE0
    PRINTF("Press button %s to wake up system.\r\n", APP_WAKEUP_BUTTON_NAME);
    GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
    /* Enable GPIO pin interrupt */
    GPIO_EnableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
    /* Enable the Interrupt */
    EnableIRQ(APP_WAKEUP_BUTTON_IRQ);
    /* Mask all interrupt first */
    GPC_DisableAllWakeupSource(GPC_CPU_MODE_CTRL);
    /* Enable GPC interrupt */
    GPC_EnableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
#else
    PRINTF("Wake up system via MU interrupt.\r\n");
    /* Enable receive interrupt */
    MU_EnableInterrupts(MU_BASE, kMU_Rx0FullInterruptEnable);
    /* Enable the Interrupt */
    EnableIRQ(MU_IRQ);
    /* Mask all interrupt first */
    GPC_DisableAllWakeupSource(GPC_CPU_MODE_CTRL);
    /* Enable GPC interrupt */
    GPC_EnableWakeupSource(MU_IRQ);
#endif
}

void CpuModeSwitchInSetPoint(uint8_t setpoint)
{
    bool stbyEn = false;
    uint8_t ch, target;
    gpc_cpu_mode_t cpuMode;

    GPC_CM_RequestRunModeSetPointTransition(GPC_CPU_MODE_CTRL, setpoint);

    while (1)
    {
        PRINTF("\r\nCPU mode switch:\r\n");
        PRINTF("Press %c to enter CPU mode: RUN\r\n", (uint8_t)'A' + kGPC_RunMode);
        PRINTF("Press %c to enter CPU mode: WAIT\r\n", (uint8_t)'A' + kGPC_WaitMode);
        PRINTF("Press %c to enter CPU mode: STOP\r\n", (uint8_t)'A' + kGPC_StopMode);
        PRINTF("Press %c to enter CPU mode: SUSPEND\r\n", (uint8_t)'A' + kGPC_SuspendMode);
        PRINTF("Press %c to enter CPU mode: WAIT, system standby\r\n", (uint8_t)'A' + 3 + kGPC_WaitMode);
        PRINTF("Press %c to enter CPU mode: STOP, system standby\r\n", (uint8_t)'A' + 3 + kGPC_StopMode);
        PRINTF("Press %c to enter CPU mode: SUSPEND, system standby\r\n", (uint8_t)'A' + 3 + kGPC_SuspendMode);
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

        if (target < 7)
        {
            if (target > 3)
            {
                stbyEn = true;
                target = target - 3;
            }
            else
            {
                stbyEn = false;
            }
            cpuMode = (gpc_cpu_mode_t)target;
            if (cpuMode != kGPC_RunMode)
            {
                APP_SetWakeupConfig();
                PRINTF("Target CPU mode is %s\r\n", GET_CPU_MODE_NAME(cpuMode));
                PRINTF("Go...\r\n");
                if (cpuMode == kGPC_SuspendMode)
                {
                    PRINTF("System will wake up from reset!\r\n");
                }
                CpuModeTransition(cpuMode, stbyEn);
                PrintSystemStatus();
            }
            else
            {
                PRINTF("CPU already in RUN mode!\r\n");
            }
        }
    }
}

void TypicalSetPointTransition(void)
{
    bool stbyEn = false;
    gpc_cm_wakeup_sp_sel_t wakeupSel;
    gpc_cpu_mode_t cpuMode;
    uint8_t ch, target, targetSp, i, activeSpCnt, standbySpCnt;
#ifndef SINGLE_CORE_M7
    uint8_t activeSp[]  = {1, 0, 5, 7, 9, 11, 12};
    uint8_t standbySp[] = {1, 0, 5, 7, 10, 11, 15};
#else
    uint8_t activeSp[]  = {1, 0, 5, 7};
    uint8_t standbySp[] = {1, 0, 5, 10, 15};
#endif

    activeSpCnt  = sizeof(activeSp) / sizeof(uint8_t);
    standbySpCnt = sizeof(standbySp) / sizeof(uint8_t);

    while (1)
    {
        PRINTF("\r\nSet Point Transition:\r\n");
        for (i = 0; i < activeSpCnt; i++)
        {
            PRINTF("Press %c to enter Set Point: %d\r\n", (uint8_t)'A' + i, activeSp[i]);
            cpuMode = getCpuMode(getCore0Status(activeSp[i]));
            PRINTF("    M7 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#ifndef SINGLE_CORE_M7
            cpuMode = getCpuMode(getCore1Status(activeSp[i]));
            PRINTF("    M4 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#endif
        }
        for (i = 0; i < standbySpCnt; i++)
        {
            cpuMode = kGPC_SuspendMode;
            PRINTF("Press %c to enter Set Point: %d\r\n", (uint8_t)'A' + activeSpCnt + i, standbySp[i]);
            PRINTF("    M7 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#ifndef SINGLE_CORE_M7
            PRINTF("    M4 CPU mode: %s\r\n", GET_CPU_MODE_NAME(cpuMode));
#endif
            PRINTF("    System standby\r\n");
        }
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

        if (target < (activeSpCnt + standbySpCnt))
        {
            if (target < activeSpCnt)
            {
                targetSp = activeSp[target];
                if (getCore1Status(targetSp) == kCORE_NormalRun)
                {
                    cpuMode = kGPC_RunMode;
                }
                else if (getCore1Status(targetSp) == kCORE_ClockGated)
                {
                    cpuMode = kGPC_StopMode;
                }
                else if (getCore1Status(targetSp) == kCORE_PowerGated)
                {
                    cpuMode = kGPC_SuspendMode;
                }
                wakeupSel = kGPC_CM_WakeupSetpoint;
                stbyEn    = false;
            }
            else
            {
                target    = target - activeSpCnt;
                targetSp  = standbySp[target];
                cpuMode   = kGPC_SuspendMode;
                wakeupSel = kGPC_CM_RequestPreviousSetpoint;
                stbyEn    = true;
            }

            if (cpuMode != kGPC_RunMode)
            {
                APP_SetWakeupConfig();
                PRINTF("Target CPU mode is %s\r\n", GET_CPU_MODE_NAME(cpuMode));
                PRINTF("Target setpoint in sleep mode is %d\r\n", targetSp);
                if (wakeupSel == kGPC_CM_WakeupSetpoint)
                {
                    PRINTF("Target setpoint after waking up is %d\r\n", targetSp);
                }
                else
                {
                    PRINTF("Target setpoint after waking up is the setpoint before entering low power mode.\r\n");
                }
                PRINTF("Go...\r\n");
                if (cpuMode == kGPC_SuspendMode)
                {
                    PRINTF("System will wake up from reset!\r\n");
                }
                PowerModeTransition(cpuMode, targetSp, targetSp, wakeupSel, stbyEn);
            }
            else
            {
                PRINTF("Target setpoint is %d\r\n", targetSp);
                GPC_CM_RequestRunModeSetPointTransition(GPC_CPU_MODE_CTRL, targetSp);
            }
            PrintSystemStatus();
        }
    }
}

/*!
 * @brief main demo function.
 */
int main(void)
{
    gpc_cpu_mode_t preCpuMode;
    uint8_t ch;

    /* Init board hardware.*/
    clock_root_config_t rootCfg = {0};

    BOARD_ConfigMPU();
    BOARD_InitPins();

    rootCfg.mux = kCLOCK_LPUART12_ClockRoot_MuxOscRc16M;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_Lpuart12, &rootCfg);

    BOARD_InitDebugConsole();
    EnableIRQ(MU_IRQ);

    preCpuMode = GPC_CM_GetPreviousCpuMode(GPC_CPU_MODE_CTRL);
    if (preCpuMode != kGPC_RunMode)
    {
        PRINTF("\r\nSystem wake up from reset!\r\n");
    }
    else
    {
        PRINTF("\r\nThis is core1.\r\n");
        /* MU init */
        MU_Init(MU_BASE);

        /* Send flag to Core 0 to indicate Core 1 has startup */
        MU_SetFlags(MU_BASE, BOOT_FLAG);

        /* Enable receive interrupt */
        MU_EnableInterrupts(MU_BASE, kMU_GenInt0InterruptEnable | kMU_Rx0FullInterruptEnable);

        PRINTF("\r\nCPU wakeup source 0x%x...\r\n", SRC->SRSR);
        PRINTF("\r\n***********************************************************\r\n");
        PRINTF("\tPower Mode Switch Demo for %s\r\n", CPU_NAME);
        PRINTF("***********************************************************\r\n");

#ifdef CORE1_GET_INPUT_FROM_CORE0
        PRINTF("\r\nCore1 receive message from core0.\r\n");
#endif
    }

    PrintSystemStatus();

    while (1)
    {
        PRINTF("\r\nPlease select the desired operation:\r\n");
        PRINTF("Press  %c to demonstrate typical set point transition.\r\n", (uint8_t)'A');
        PRINTF("Press  %c to demonstrate cpu mode switch in setpoint 0.\r\n", (uint8_t)'B');
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
                TypicalSetPointTransition();
                break;
            case 'B':
                CpuModeSwitchInSetPoint(0);
                break;
            default:
                break;
        }
    }
}
