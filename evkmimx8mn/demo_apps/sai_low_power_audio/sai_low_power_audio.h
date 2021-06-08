/*
 * Copyright 2018 NXP
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
 * LPM state of M4 core
 */
typedef enum lpm_power_status_m7
{
    LPM_M7_STATE_RUN,
    LPM_M7_STATE_WAIT,
    LPM_M7_STATE_STOP,
} LPM_POWER_STATUS_M7;
/*
 * Clock Speed of M4 core
 */
typedef enum lpm_m7_clock_speed
{
    LPM_M7_HIGH_FREQ,
    LPM_M7_LOW_FREQ
} LPM_M7_CLOCK_SPEED;
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
