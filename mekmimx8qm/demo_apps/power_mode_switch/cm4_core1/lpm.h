/*
 * Copyright 2018-2019, NXP
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
#ifdef FSL_RTOS_FREE_RTOS
#if defined(MIMX8QM_CM4_CORE0)
#define SYSTICK_BASE    LSIO__GPT4
#define SYSTICK_IRQn    LSIO_GPT4_INT_IRQn
#define SYSTICK_HANDLER LSIO_GPT4_INT_IRQHandler
#define SYSTICK_CLOCK   kCLOCK_LSIO_Gpt4
#define SYSTICK_RSRC    SC_R_GPT_4
#elif defined(MIMX8QM_CM4_CORE1)
#define SYSTICK_BASE    LSIO__GPT3
#define SYSTICK_IRQn    LSIO_GPT3_INT_IRQn
#define SYSTICK_HANDLER LSIO_GPT3_INT_IRQHandler
#define SYSTICK_CLOCK   kCLOCK_LSIO_Gpt3
#define SYSTICK_RSRC    SC_R_GPT_3
#else
#error "No valid CPU core defined."
#endif
#endif /* FSL_RTOS_FREE_RTOS */

/*!
 * @brief Power mode definition of low power management.
 * Waken up duration Off > Dsm > Idle > Wait > Run.
 */
typedef enum _lpm_power_mode
{
    LPM_PowerModeRun = 0, /*!< Normal RUN mode */
    LPM_PowerModeWait,    /*!< WAIT mode. */
    LPM_PowerModeStop,    /*!< STOP mode. */
    LPM_PowerModeVlpr,    /*!< VLPR mode. */
    LPM_PowerModeVlpw,    /*!< VLPW mode. */
    LPM_PowerModeVlps,    /*!< VLPS mode. */
    LPM_PowerModeLls,     /*!< LLS mode */
    LPM_PowerModeVlls,    /*!< VLLS mode */
} lpm_power_mode_t;

#ifdef FSL_RTOS_FREE_RTOS
typedef bool (*lpm_power_mode_callback_t)(lpm_power_mode_t curMode, lpm_power_mode_t newMode, void *data);
#endif
/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*!
 * @brief Initialize the Low Power Management.
 */
bool LPM_Init(void);

/*!
 * @brief Deinitialize the Low Power Management.
 */
void LPM_Deinit(void);

/*!
 * @brief Check if target power mode is valid to transit.
 *
 * @param targetPowerMode the target power mode.
 * @param pErrorMsg the pointer of error message.
 * @return is target mode valid.
 */
bool LPM_IsTargetModeValid(lpm_power_mode_t targetPowerMode, const char **pErrorMsg);

/*!
 * @brief Set power mode, all registered listeners will be notified.
 * @param mode the mode need to change to.
 * @return Return true if all the registered listeners return true.
 */
bool LPM_SetPowerMode(lpm_power_mode_t mode);

/*!
 * @brief Get low power mode previously set.
 * @return return current low power mode.
 */
lpm_power_mode_t LPM_GetPowerMode(void);

/*!
 * @brief LPM_SetPowerMode() won't switch system power status immediately,
 * instead, such operation is done by LPM_WaitForInterrupt().
 * It can be callled in idle task of FreeRTOS, or main loop in bare
 * metal application. The sleep depth of this API depends
 * on current power mode set by LPM_SetPowerMode().
 * @param timeoutMilliSec The timeoutMilliSec means if no interrupt occurs before timeout, the
 * system will be waken up by systick. If timeout exceeds hardware timer
 * limitation, timeout will be reduced to maximum time of hardware.
 * timeoutMilliSec only works in FreeRTOS, in bare metal application,
 * it's just ignored.
 * @return Return true if power mode switch succeeds.
 */
bool LPM_WaitForInterrupt(uint32_t timeoutMilliSec);

#ifdef FSL_RTOS_FREE_RTOS
/*!
 * @brief Register power mode switch listener. When LPM_SetPowerMode()
 * is called, all the registered listeners will be invoked.
 *
 * @param callback callback function
 * @param data data passed to callback function
 * @return The listener returns true if it allows the power mode switch, otherwise returns FALSE.
 */
void LPM_RegisterPowerListener(lpm_power_mode_callback_t callback, void *data);

/*!
 *@brief Unregister power mode switch listener
 */
void LPM_UnregisterPowerListener(lpm_power_mode_callback_t callback, void *data);
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus*/
#endif /* _LPM_H_ */
