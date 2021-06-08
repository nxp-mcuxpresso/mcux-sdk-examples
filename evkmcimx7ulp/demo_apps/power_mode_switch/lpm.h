/*
 * Copyright 2017-2018, NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LPM_H_
#define _LPM_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SYSTICK_BASE       LPTMR0
#define SYSTICK_IRQn       LPTMR0_IRQn
#define SYSTICK_HANDLER    LPTMR0_IRQHandler
#define SYSTICK_CLOCK_NAME kCLOCK_Lptmr0

/* Power mode definition of low power management.
 * Waken up duration Off > Dsm > Idle > Wait > Run.
 */
typedef enum _lpm_power_mode
{
    LPM_PowerModeRun = 0, /* Normal RUN mode */
    LPM_PowerModeWait,    /* WAIT mode. */
    LPM_PowerModeStop,    /* STOP mode. */
    LPM_PowerModeVlpr,    /* VLPR mode. */
    LPM_PowerModeVlpw,    /* VLPW mode. */
    LPM_PowerModeVlps,    /* VLPS mode. */
    LPM_PowerModeHsrun,   /* HighSpeed RUN mode */
    LPM_PowerModeLls,     /* LLS mode */
    LPM_PowerModeVlls,    /* VLLS mode */
} lpm_power_mode_t;

typedef bool (*lpm_power_mode_callback_t)(lpm_power_mode_t curMode, lpm_power_mode_t newMode, void *data);

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/* Initialize the Low Power Management */
bool LPM_Init(void);

/* Deinitialize the Low Power Management */
void LPM_Deinit(void);

/* Check if target power mode is valid to transit.
 * Return true if the transition is allowed.
 */
bool LPM_IsTargetModeValid(lpm_power_mode_t targetPowerMode, const char **pErrorMsg);

/* Set power mode, all registered listeners will be notified.
 * Return true if all the registered listeners return true.
 */
bool LPM_SetPowerMode(lpm_power_mode_t mode);

/* Get low power mode previously set.
 */
lpm_power_mode_t LPM_GetPowerMode(void);

/* LPM_SetPowerMode() won't switch system power status immediately,
 * instead, such operation is done by LPM_WaitForInterrupt().
 * It can be callled in idle task of FreeRTOS, or main loop in bare
 * metal application. The sleep depth of this API depends
 * on current power mode set by LPM_SetPowerMode().
 * The timeoutMilliSec means if no interrupt occurs before timeout, the
 * system will be waken up by systick. If timeout exceeds hardware timer
 * limitation, timeout will be reduced to maximum time of hardware.
 * timeoutMilliSec only works in FreeRTOS, in bare metal application,
 * it's just ignored.
 * Return true if power mode switch succeeds.
 */
bool LPM_WaitForInterrupt(uint32_t timeoutMilliSec);

/* Register power mode switch listener. When LPM_SetPowerMode()
 * is called, all the registered listeners will be invoked. The
 * listener returns true if it allows the power mode switch,
 * otherwise returns FALSE.
 */
void LPM_RegisterPowerListener(lpm_power_mode_callback_t callback, void *data);

/* Unregister power mode switch listener */
void LPM_UnregisterPowerListener(lpm_power_mode_callback_t callback, void *data);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _LPM_H_ */
