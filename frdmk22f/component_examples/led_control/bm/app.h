/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define LED_TYPE_MONOCHROME_COUNT 1
#define BOARD_SOURCE_CLOCK        48000000
#define RED_INSTACE               0
#define RED_CHANNEL               6

#define BUTTON_COUNT       1
#define BUTTON_NAME        BOARD_SW3_NAME
#define TIMER_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
