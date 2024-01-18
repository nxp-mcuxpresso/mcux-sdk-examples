/** @file host_sleep.c
 *
 *  @brief Host sleep file
 *
 *  Copyright 2021-2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef CONFIG_HOST_SLEEP

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_common.h"
#include "fsl_mu.h"
#include "fsl_soc_src.h"
#include "fsl_gpc.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "host_sleep.h"
#include "cli.h"
#include "wlan.h"

GPIO_HANDLE_DEFINE(s_WakeupGpioHandle);

static void (*wlan_host_sleep_pre_cfg)(void);
static void (*wlan_host_sleep_post_cfg)(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint32_t g_savedPrimask;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void GPC_EnableWakeupSource(uint32_t irq)
{
    GPC_CM_EnableIrqWakeup(GPC_CPU_MODE_CTRL_0, irq, true);
}

void GPC_DisableWakeupSource(uint32_t irq)
{
    GPC_CM_EnableIrqWakeup(GPC_CPU_MODE_CTRL_0, irq, false);
}

void APP_WAKEUP_Callback(void *param)
{
    if ((1U << APP_WAKEUP_GPIO_PIN) & GPIO_GetPinsInterruptFlags(APP_WAKEUP_GPIO))
    {
        /* Disable interrupt. */
        GPIO_DisableInterrupts(APP_WAKEUP_GPIO, 1U << APP_WAKEUP_GPIO_PIN);
        GPIO_ClearPinsInterruptFlags(APP_WAKEUP_GPIO, 1U << APP_WAKEUP_GPIO_PIN);
        GPC_DisableWakeupSource(APP_WAKEUP_IRQ);
    }
    SDK_ISR_EXIT_BARRIER;
}

void GPC_DisableAllWakeupSource(GPC_CPU_MODE_CTRL_Type *base)
{
    uint8_t i;

    for (i = 0; i < GPC_CPU_MODE_CTRL_CM_IRQ_WAKEUP_MASK_COUNT; i++)
    {
        base->CM_IRQ_WAKEUP_MASK[i] |= 0xFFFFFFFF;
    }
}

void APP_SetWakeupConfig(void)
{
    hal_gpio_pin_config_t sw_config = {
        kHAL_GpioDirectionIn,
        0,
        APP_WAKEUP_GPIO_PORT,
        APP_WAKEUP_GPIO_PIN,
    };

    HAL_GpioInit(s_WakeupGpioHandle, &sw_config);
    HAL_GpioSetTriggerMode(s_WakeupGpioHandle, APP_WAKEUP_INTTERUPT_TYPE);
    HAL_GpioInstallCallback(s_WakeupGpioHandle, APP_WAKEUP_Callback, NULL);

    GPIO_ClearPinsInterruptFlags(APP_WAKEUP_GPIO, 1U << APP_WAKEUP_GPIO_PIN);
    /* Enable GPIO pin interrupt */
    GPIO_EnableInterrupts(APP_WAKEUP_GPIO, 1U << APP_WAKEUP_GPIO_PIN);
    /* Enable the Interrupt */
    EnableIRQ(APP_WAKEUP_IRQ);
    /* Mask all interrupt first */
    GPC_DisableAllWakeupSource(GPC_CPU_MODE_CTRL_0);
    /* Enable GPC interrupt */
    GPC_EnableWakeupSource(APP_WAKEUP_IRQ);
}

AT_QUICKACCESS_SECTION_CODE(void SystemEnterSleepMode(gpc_cpu_mode_t cpuMode));
void SystemEnterSleepMode(gpc_cpu_mode_t cpuMode)
{
    assert(cpuMode != kGPC_RunMode);

    g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();

    if (cpuMode == kGPC_WaitMode)
    {
        /* Clear the SLEEPDEEP bit to go into sleep mode (WAIT) */
        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    }
    else /* STOP and SUSPEND mode */
    {
        /* Set the SLEEPDEEP bit to enable deep sleep mode (STOP) */
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    }
    /* WFI instruction will start entry into WAIT/STOP mode */
    __WFI();

    EnableGlobalIRQ(g_savedPrimask);
    __DSB();
    __ISB();
}

void CpuModeTransition(void)
{
    GPC_CM_SetNextCpuMode(GPC_CPU_MODE_CTRL_0, kGPC_WaitMode);
    GPC_CM_EnableCpuSleepHold(GPC_CPU_MODE_CTRL_0, true);

    GPC_CPU_MODE_CTRL_0->CM_NON_IRQ_WAKEUP_MASK |=
        GPC_CPU_MODE_CTRL_CM_NON_IRQ_WAKEUP_MASK_EVENT_WAKEUP_MASK_MASK |
        GPC_CPU_MODE_CTRL_CM_NON_IRQ_WAKEUP_MASK_DEBUG_WAKEUP_MASK_MASK; /* Mask debugger wakeup */

    GPC_CM_RequestStandbyMode(GPC_CPU_MODE_CTRL_0, kGPC_WaitMode);
    SystemEnterSleepMode(kGPC_WaitMode);
}
void mcu_suspend()
{
    if (wlan_host_sleep_pre_cfg)
    {
        wlan_host_sleep_pre_cfg();
    }
    APP_SetWakeupConfig();
    CpuModeTransition();
    if (wlan_host_sleep_post_cfg)
    {
        wlan_host_sleep_post_cfg();
    }
}

int hostsleep_init(void (*wlan_hs_pre_cfg)(void), void (*wlan_hs_post_cfg)(void))
{
    wlan_host_sleep_pre_cfg = wlan_hs_pre_cfg;
    wlan_host_sleep_post_cfg = wlan_hs_post_cfg;

    return 0;
}

#endif
