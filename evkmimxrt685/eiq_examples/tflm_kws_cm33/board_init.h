/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_INIT_H_
#define _BOARD_INIT_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* DMIC source from audio pll, divider 8, 24.576 MHz / 8 = 3.072 MHz */
#define BOARD_DMIC_CLOCK_DIV 8

void BOARD_Init();

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_INIT_H_ */
