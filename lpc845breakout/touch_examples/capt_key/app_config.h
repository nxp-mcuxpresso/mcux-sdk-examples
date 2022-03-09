/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

/*******************************************************************************
 * Application configuration
 ******************************************************************************/
#define DEMO_ACOMP_BASE         ACOMP
#define DEMO_ACOMP_CAPT_CHANNEL 5U

/* If the channel sample data variance is less than this value, then the channel
 * sample data is stable, used in calibration stage.
 */
#define APP_CHANNEL_STABLE_VARIANCE 2

/* Glitch filter level used in pressed key detection. */
#define APP_GLITCH_FILTER_LEVEL 3

/*******************************************************************************
 * Touch Configuration
 ******************************************************************************/
#define TOUCH_X_CHANNEL_COUNT 1

/*
 * How many samples are saved and used to determine touch result.
 */
#define TOUCH_WINDOW_LENGTH 4

/*
 * This macro configures the touch event detection level.
 * In the case of multiple X pins (multiple channels), if the difference
 * between the maximum channel and minimum channel is larger than this threashold,
 * then there is a potential touch event.
 * In the case of single X pin, if the difference between sampled value and
 * baseline value is larger than this threashold, the there is a potential
 * touch event.
 */
#define TOUCH_YES_TOUCH_THRESHOLD_LOW 80

/*
 * If the touched channel sample data decreases, set this to 1.
 * If the touched channel sample data increases, set this to 0.
 */
#define TOUCH_PRESSED_KEY_COUNT_DECREASE 1

/*******************************************************************************
 * Touch HAL Configuration
 ******************************************************************************/
#define TOUCH_HAL_CAPT             CAPT
#define TOUCH_HAL_CAPT_ENABLE_PINS (kCAPT_X0Pin)
#define TOUCH_HAL_CAPT_IRQn        CMP_CAPT_IRQn
#define TOUCH_HAL_CAPT_IRQHandler  CMP_CAPT_DriverIRQHandler

/* Calculate the clock divider to make sure CAPT work in 2Mhz FCLK. */
#define TOUCH_HAL_CAPT_CLK_DIVIDER (CLOCK_GetFroFreq() / 2000000U - 1U)

/* Delay between poll round, the delay time between two poll round
 * is TOUCH_HAL_CAPT_DELAY_BETWEEN_POLL * 4096 * FCLK period
 */
#define TOUCH_HAL_CAPT_DELAY_BETWEEN_POLL 20U

#endif /* __APP_CONFIG_H__ */
