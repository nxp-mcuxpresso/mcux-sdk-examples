/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LCD_SUPPORT_H
#define LCD_SUPPORT_H

#include "display_support.h"

/*******************************************************************************
 * API
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void BOARD_InitTouchPanel(void);

void start_touch_thread(void);

#if defined(__cplusplus)
}
#endif

#endif /* LCD_SUPPORT_H */
