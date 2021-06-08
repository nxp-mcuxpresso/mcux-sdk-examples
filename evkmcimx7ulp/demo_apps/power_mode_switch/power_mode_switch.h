/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
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
    kAPP_PowerModeVlpr,      /* VLPR mode. */
    kAPP_PowerModeVlpw,      /* VLPW mode. */
    kAPP_PowerModeVlps,      /* VLPS mode. */
    kAPP_PowerModeHsrun,     /* HighSpeed RUN mode */
    kAPP_PowerModeLls,       /* LLS mode */
    kAPP_PowerModeVlls,      /* VLLS mode */
} app_power_mode_t;

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
