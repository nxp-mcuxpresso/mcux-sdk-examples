/*
* Copyright 2019, 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <EmbeddedTypes.h>
#include "dbg.h"
#include "ZQueue.h"
#include "ZTimer.h"
#include "app_common.h"
#include "app_buttons.h"
#include "app_main.h"

#include "zb_platform.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef TRACE_APP_BUTTON
    #define TRACE_APP_BUTTON            TRUE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void APP_vButtonCb(uint8_t button);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern tszQueue APP_msgAppEvents;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: APP_bButtonInitialise
 *
 * DESCRIPTION:
 * Button Initialization
 *
 * PARAMETER: void
 *
 * RETURNS: bool_t
 *
 ****************************************************************************/
bool_t APP_bButtonInitialise(void)
{
    return zbPlatButtonInit(APP_BUTTONS_NUM, APP_vButtonCb);
}

/****************************************************************************
 *
 * NAME: button_cb
 *
 * DESCRIPTION:
 * Callback that's called from button component on button press
 *
 * PARAMETER:
 *
 * RETURNS:
 *
 ****************************************************************************/
static void APP_vButtonCb(uint8_t button)
{
    /* Button will hold the button ID */
    APP_tsEvent sButtonEvent;

    sButtonEvent.eType = APP_E_EVENT_BUTTON_DOWN;
    sButtonEvent.uEvent.sButton.u8Button = button;

    if(ZQ_bQueueSend(&APP_msgAppEvents, &sButtonEvent) == FALSE)
    {
        DBG_vPrintf(TRACE_APP_BUTTON, "Button: Failed to post Event %d \r\n", sButtonEvent.eType);
    }
}

/****************************************************************************
 *
 * NAME: APP_u32GetButtonsState
 *
 * DESCRIPTION:
 * Used to get the button states
 *
 * PARAMETER:
 *
 * RETURNS:
 * button mask (1 pressed, 0 not pressed)
 ****************************************************************************/
uint32_t APP_u32GetButtonsState(void)
{
    return zbPlatButtonGetState();
}
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
