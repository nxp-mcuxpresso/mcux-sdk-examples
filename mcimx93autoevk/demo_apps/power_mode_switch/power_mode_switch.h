/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _POWER_MODE_SWITCH_H_
#define _POWER_MODE_SWITCH_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Power mode definition used in application. */
typedef enum _app_power_mode
{
    kAPP_PowerModeRun = 'A', /* Normal RUN mode */
    kAPP_PowerModeWait,      /* WAIT mode. */
    kAPP_PowerModeStop,      /* STOP mode. */
    kAPP_PowerModeSuspend,   /* SUSPEND mode. */

} app_power_mode_t;

#define OSCPLL_LPM_START 3U
#define OSCPLL_LPM_END   12U

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _POWER_MODE_SWITCH_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
