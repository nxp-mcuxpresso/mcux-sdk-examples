/*
* Copyright 2019, 2023-2024 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef APP_BUTTONS_H
#define APP_BUTTONS_H

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <EmbeddedTypes.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* 
 * Application generic button, whenever the APP is
 * using a button, it should reference it by this enum.
 * app_buttons.c will decide which buttons are assigned
 * to each instance of this enum.
 */
typedef enum {
    APP_E_BUTTONS_BUTTON_1 = 0,
    APP_E_BUTTONS_BUTTON_2,
    APP_E_BUTTONS_BUTTON_3,
    APP_E_BUTTONS_BUTTON_4,
    APP_E_BUTTONS_BUTTON_5,
} APP_teButtons;

#ifndef APP_BUTTONS_NUM
#define APP_BUTTONS_NUM                     (1UL)
#endif

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
bool_t APP_bButtonInitialise(void);
uint32_t APP_u32GetButtonsState(void);

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /*APP_BUTTONS_H*/
