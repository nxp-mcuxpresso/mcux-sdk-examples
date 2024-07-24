/** @file host_sleep.h
 *
 *  @brief Host sleep file
 *
 *  Copyright 2021 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _HOST_SLEEP_H_
#define _HOST_SLEEP_H_

#include "board.h"

#include "fsl_common.h"
#include "lpm.h"

#include "fsl_adapter_gpio.h"
#include "wifi_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define CPU_NAME "iMXRT1062"

#define APP_WAKEUP_BUTTON_GPIO           GPIO5
#define APP_WAKEUP_BUTTON_GPIO_PORT      5
#define APP_WAKEUP_BUTTON_GPIO_PIN       0U
#define APP_WAKEUP_BUTTON_IRQ            BOARD_USER_BUTTON_IRQ
#define APP_WAKEUP_BUTTON_NAME           BOARD_USER_BUTTON_NAME
#define APP_WAKEUP_BUTTON_INTTERUPT_TYPE kHAL_GpioInterruptFallingEdge

#define APP_WAKEUP_SNVS_IRQ         SNVS_HP_WRAPPER_IRQn
#define APP_WAKEUP_SNVS_IRQ_HANDLER SNVS_HP_WRAPPER_IRQHandler

typedef enum _app_wakeup_source
{
    kAPP_WakeupSourceTimer, /*!< Wakeup by Timer.        */
    kAPP_WakeupSourcePin,   /*!< Wakeup by external pin. */
} app_wakeup_source_t;

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

void APP_PowerPreSwitchHook(lpm_power_mode_t targetMode);
void APP_PowerPostSwitchHook(lpm_power_mode_t targetMode);
lpm_power_mode_t APP_GetLPMPowerMode(void);
lpm_power_mode_t APP_GetRunMode(void);

#if CONFIG_HOST_SLEEP
int hostsleep_init(void (*wlan_hs_pre_cfg)(void), void (*wlan_hs_post_cfg)(void));
void mcu_suspend();
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /*_HOST_SLEEP_H_*/
