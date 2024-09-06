/*
 * Copyright 2020, NXP
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
/*!
 * @brief .
 */
typedef void (*wakeup_handler)(IRQn_Type irq);

typedef struct _lpm_config
{
    /* If wait time is less than the threshold(in milliseconds), PM2 wait will run into PM1 instead.
       And the PM1 wait will not trigger tickless idle. */
    uint32_t threshold;
} lpm_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/* Initialize the Low Power Management.
 */
void LPM_Init(const lpm_config_t *config);

/* Deinitialize the Low Power Management */
void LPM_Deinit(void);

/* Enable wakeup source */
void LPM_EnableWakeupSource(IRQn_Type source);

/* Disable wakeup source */
void LPM_DisableWakeupSource(IRQn_Type source);

/* Set power mode.
 */
void LPM_SetPowerMode(uint32_t mode, const power_sleep_config_t *config);

/* Get low power mode previously set.
 */
uint32_t LPM_GetPowerMode(void);

/* LPM_SetPowerMode() won't switch system power status immediately,
 * instead, such operation is done by LPM_WaitForInterrupt().
 * It can be callled in idle task of FreeRTOS, or main loop in bare
 * metal application. The sleep depth of this API depends
 * on current power mode set by LPM_SetPowerMode().
 */
uint32_t LPM_WaitForInterrupt(uint32_t timeoutMilliSec);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _LPM_H_ */
