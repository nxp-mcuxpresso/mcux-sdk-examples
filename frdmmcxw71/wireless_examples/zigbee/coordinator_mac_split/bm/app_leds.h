/*
* Copyright 2019, 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


#ifndef APP_LEDS_H
#define APP_LEDS_H

#include <EmbeddedTypes.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


typedef enum {
    APP_E_LEDS_LED_1 = 0,
    APP_E_LEDS_LED_2,
} APP_teLeds;

typedef enum {
	APP_E_LED_OFF = 0,
	APP_E_LED_ON
} APP_teLedStates;

#define APP_LEDS_NUM                     (2UL)

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
void APP_vLedInitialise(void);
void APP_vSetLed(uint8_t u8Led, bool_t bState);
uint8_t APP_u8GetLedStates(void);


/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /*APP_LEDS_H*/
