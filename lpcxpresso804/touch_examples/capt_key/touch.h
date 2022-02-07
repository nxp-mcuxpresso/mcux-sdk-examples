/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __TOUCH_H__
#define __TOUCH_H__

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

void TOUCH_WINDOWS_Init(void);
void TOUCH_WINDOWS_Puth(int16_t *input);
void TOUCH_WINDOWS_CalcSum(int32_t *output);
void TOUCH_WINDOWS_CalcAverage(int32_t *output);
void TOUCH_WINDOWS_CalcVariance(int32_t *average, int32_t *variance);
void TOUCH_WINDOWS_SetBaseline(int32_t *input);
int32_t TOUCH_GetPressedKeyIndex(int32_t *input);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* __TOUCH_H__ */
