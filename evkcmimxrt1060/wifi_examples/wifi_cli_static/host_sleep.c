/** @file host_sleep.c
 *
 *  @brief Host sleep file
 *
 *  Copyright 2021 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#include "host_sleep.h"

#if CONFIG_HOST_SLEEP

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "fsl_os_abstraction.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "fsl_debug_console.h"
#include "lpm.h"
#include "fsl_lpuart.h"
#include "specific.h"

#include "board.h"

#include "fsl_debug_console.h"


GPIO_HANDLE_DEFINE(s_WakeupGpioHandle);
GPIO_HANDLE_DEFINE(h_WakeupGpioHandle);

static void (*wlan_host_sleep_pre_cfg)(void);
static void (*wlan_host_sleep_post_cfg)(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static lpm_power_mode_t s_targetPowerMode;
static lpm_power_mode_t s_curRunMode = LPM_PowerModeOverRun;
OSA_SEMAPHORE_HANDLE_DEFINE(s_wakeupSig);

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_WAKEUP_BUTTON_Callback(void *param)
{
    LPM_DisableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
    OSA_SemaphorePost((osa_semaphore_handle_t)s_wakeupSig);
}

static void APP_SetWakeupConfig(lpm_power_mode_t targetMode)
{
    GPIO_ClearPinsInterruptFlags(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
    /* Enable GPIO pin interrupt */
    GPIO_EnableInterrupts(APP_WAKEUP_BUTTON_GPIO, 1U << APP_WAKEUP_BUTTON_GPIO_PIN);
    NVIC_ClearPendingIRQ(APP_WAKEUP_BUTTON_IRQ);
    NVIC_SetPriority(APP_WAKEUP_BUTTON_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY + 2);
    /* Enable the Interrupt */
    EnableIRQ(APP_WAKEUP_BUTTON_IRQ);
    /* Enable GPC interrupt */
    LPM_EnableWakeupSource(APP_WAKEUP_BUTTON_IRQ);
}

void APP_HOST_WAKEUP_Callback(void *param)
{
    LPM_DisableWakeupSource(APP_HOST_WAKEUP_IRQ);
    OSA_SemaphorePost((osa_semaphore_handle_t)s_wakeupSig);
}

static void HOST_SetWakeupConfig(lpm_power_mode_t targetMode)
{
    GPIO_ClearPinsInterruptFlags(APP_HOST_WAKEUP_GPIO, 1U << APP_HOST_WAKEUP_GPIO_PIN);
    /* Enable GPIO pin interrupt */
    GPIO_EnableInterrupts(APP_HOST_WAKEUP_GPIO, 1U << APP_HOST_WAKEUP_GPIO_PIN);
    NVIC_ClearPendingIRQ(APP_HOST_WAKEUP_IRQ);
    NVIC_SetPriority(APP_HOST_WAKEUP_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY + 2);
    /* Enable the Interrupt */
    EnableIRQ(APP_HOST_WAKEUP_IRQ);
    /* Enable GPC interrupt */
    LPM_EnableWakeupSource(APP_HOST_WAKEUP_IRQ);
}

lpm_power_mode_t APP_GetRunMode(void)
{
    return s_curRunMode;
}

void APP_SetRunMode(lpm_power_mode_t powerMode)
{
    s_curRunMode = powerMode;
}

/*
 * Check whether could switch to target power mode from current mode.
 * Return true if could switch, return false if could not switch.
 */
bool APP_CheckPowerMode(lpm_power_mode_t originPowerMode, lpm_power_mode_t targetPowerMode)
{
    bool modeValid = true;

    /* Don't need to change power mode if current mode is already the target mode. */
    if (originPowerMode == targetPowerMode)
    {
        PRINTF("Already in the target power mode.\r\n");
        modeValid = false;
    }

    return modeValid;
}

void APP_PowerPreSwitchHook(lpm_power_mode_t targetMode)
{
    if (targetMode == LPM_PowerModeSNVS)
    {
        PRINTF("Now shutting down the system...\r\n");
    }

    if (targetMode > LPM_PowerModeRunEnd)
    {
        /* Wait for debug console output finished. */
        while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
        {
        }
        DbgConsole_Deinit();

        /*
         * Set pin for current leakage.
         * Debug console RX pin: Set pinmux to GPIO input.
         * Debug console TX pin: Don't need to change.
         */
        ConfigUartRxPinToGpio();
    }
}

void APP_PowerPostSwitchHook(lpm_power_mode_t targetMode)
{
    if (targetMode > LPM_PowerModeRunEnd)
    {
        /*
         * Debug console RX pin is set to GPIO input, need to re-configure pinmux.
         * Debug console TX pin: Don't need to change.
         */
        ReConfigUartRxPin();
        BOARD_InitDebugConsole();
    }
    else
    {
        /* update current run mode */
        APP_SetRunMode(targetMode);
    }
}

lpm_power_mode_t APP_GetLPMPowerMode(void)
{
    return s_targetPowerMode;
}

static void PowerModeSwitch(lpm_power_mode_t mode)
{
    s_targetPowerMode = mode;

    if (s_targetPowerMode <= LPM_PowerModeEnd)
    {
        /* If could not set the target power mode, loop continue. */
        if (!APP_CheckPowerMode(s_curRunMode, s_targetPowerMode))
        {
            return;
        }

        if (!LPM_SetPowerMode(s_targetPowerMode))
        {
            return;
        }
        else
        {
            if (s_targetPowerMode <= LPM_PowerModeRunEnd)
            {
                switch (s_targetPowerMode)
                {
                    case LPM_PowerModeOverRun:
                        LPM_OverDriveRun(s_curRunMode);
                        break;
                    case LPM_PowerModeFullRun:
                        LPM_FullSpeedRun(s_curRunMode);
                        break;
                    case LPM_PowerModeLowSpeedRun:
                        LPM_LowSpeedRun(s_curRunMode);
                        break;
                    case LPM_PowerModeLowPowerRun:
                        LPM_LowPowerRun(s_curRunMode);
                        break;
                    default:
                        break;
                }
                APP_SetRunMode(s_targetPowerMode);
                return;
            }
            else if (LPM_PowerModeSNVS == s_targetPowerMode)
            {
                APP_SetWakeupConfig(s_targetPowerMode);
                HOST_SetWakeupConfig(s_targetPowerMode);
                APP_PowerPreSwitchHook(s_targetPowerMode);
                LPM_EnterSNVS();
            }
            else
            {
                APP_SetWakeupConfig(s_targetPowerMode);
                HOST_SetWakeupConfig(s_targetPowerMode);
                vPortPRE_SLEEP_PROCESSING(0);
                vPortPOST_SLEEP_PROCESSING(0);
                if (OSA_SemaphoreWait((osa_semaphore_handle_t)s_wakeupSig, portMAX_DELAY) == pdFALSE)
                {
                    assert(0);
                }
                /* Invalidate I-Cache in case instruction fetch incorrectly after wake up. */
                if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
                {
                    SCB_InvalidateICache();
                }
            }
            LPM_SetPowerMode(s_curRunMode);
        }
    }
}

void mcu_suspend()
{
    if (wlan_host_sleep_pre_cfg)
    {
        wlan_host_sleep_pre_cfg();
    }
    PowerModeSwitch(LPM_PowerModeSysIdle);
    if (wlan_host_sleep_post_cfg)
    {
        wlan_host_sleep_post_cfg();
    }
}

int hostsleep_init(void (*wlan_hs_pre_cfg)(void), void (*wlan_hs_post_cfg)(void))
{
    osa_status_t status;
    if (true != LPM_Init(s_curRunMode))
    {
        PRINTF("LPM Init Failed!\r\n");
        return -1;
    }

    wlan_host_sleep_pre_cfg = wlan_hs_pre_cfg;
    wlan_host_sleep_post_cfg = wlan_hs_post_cfg;

    status = OSA_SemaphoreCreateBinary((osa_semaphore_handle_t)s_wakeupSig);
    if (status != KOSA_StatusSuccess)
    {
        PRINTF("Create s_wakeupSig sem failed");
        return -1;
    }
    /* Make current resource count 0 for signal purpose */
    if (OSA_SemaphoreWait((osa_semaphore_handle_t)s_wakeupSig, 0) == pdTRUE)
    {
        assert(0);
    }

    hal_gpio_pin_config_t sw_config = {
        kHAL_GpioDirectionIn,
        0,
        APP_WAKEUP_BUTTON_GPIO_PORT,
        APP_WAKEUP_BUTTON_GPIO_PIN,
    };

    HAL_GpioInit(s_WakeupGpioHandle, &sw_config);
    HAL_GpioSetTriggerMode(s_WakeupGpioHandle, APP_WAKEUP_BUTTON_INTTERUPT_TYPE);
    HAL_GpioInstallCallback(s_WakeupGpioHandle, APP_WAKEUP_BUTTON_Callback, NULL);

    hal_gpio_pin_config_t hw_config = {
        kHAL_GpioDirectionIn,
        0,
        APP_HOST_WAKEUP_GPIO_PORT,
        APP_HOST_WAKEUP_GPIO_PIN,
    };

#ifdef WIFI_IW612_BOARD_MURATA_2EL_M2
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_00_GPIO1_IO00, 0U);
#elif defined(WIFI_88W8987_BOARD_MURATA_1ZM_M2) || defined(WIFI_IW416_BOARD_MURATA_1XK_M2)
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_10_GPIO1_IO26, 0U);
#endif
    HAL_GpioInit(h_WakeupGpioHandle, &hw_config);
    HAL_GpioSetTriggerMode(h_WakeupGpioHandle, APP_HOST_WAKEUP_INTTERUPT_TYPE);
    HAL_GpioInstallCallback(h_WakeupGpioHandle, APP_HOST_WAKEUP_Callback, NULL);

    return 0;
}

#endif
