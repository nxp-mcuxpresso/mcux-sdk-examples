/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
 
void TIMER_Init(void);

int TIMER_GetTimeInUS(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _TIMER_H_ */
