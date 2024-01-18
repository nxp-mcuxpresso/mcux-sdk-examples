/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SAI_LOW_POWER_AUDIO_H_
#define _SAI_LOW_POWER_AUDIO_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_PowerUpSlot (5U)
#define APP_PowerDnSlot (6U)

/*
 * LPM state of M core
 */
typedef enum lpm_power_status_m33
{
    LPM_M33_STATE_RUN,
    LPM_M33_STATE_WAIT,
    LPM_M33_STATE_STOP,
} LPM_POWER_STATUS_M33;
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _SAI_LOW_POWER_AUDIO_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
