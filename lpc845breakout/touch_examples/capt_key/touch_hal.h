/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __TOUCH_HAL_H__
#define __TOUCH_HAL_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/* Initialize the CAPT. */
void TOUCH_HAL_Init(void);

/* Wait and save CAPT data to the array rawData. */
void TOUCH_HAL_WaitDataReady(int16_t rawData[]);

void TOUCH_HAL_CAPT_IRQHandler(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* __TOUCH_HAL_H__ */
