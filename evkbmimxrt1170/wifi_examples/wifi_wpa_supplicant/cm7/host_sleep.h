/** @file host_sleep.h
 *
 *  @brief Host sleep file
 *
 *  Copyright 2021-2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _HOST_SLEEP_H_
#define _HOST_SLEEP_H_

#include "fsl_adapter_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_WAKEUP_GPIO           GPIO5
#define APP_WAKEUP_GPIO_PORT      5
#define APP_WAKEUP_GPIO_PIN       12
#define APP_WAKEUP_IRQ            GPIO5_Combined_0_15_IRQn
#define APP_WAKEUP_INTTERUPT_TYPE kHAL_GpioInterruptFallingEdge

/*******************************************************************************
 * Variables
 ******************************************************************************/

#ifdef CONFIG_HOST_SLEEP
int hostsleep_init(void (*wlan_hs_pre_cfg)(void), void (*wlan_hs_post_cfg)(void));
void mcu_suspend();
#endif

#endif /*_HOST_SLEEP_H_*/
