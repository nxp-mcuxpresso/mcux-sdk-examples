/*
* Copyright 2019, 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "EmbeddedTypes.h"
#include "app.h"
#include "dbg.h"
#include "zps_gen.h"
#include "app_coordinator.h"
#include "app_zcl_task.h"
#include "app_buttons.h"
#include "app_common.h"
#include "app_serial_commands.h"
#include "app_main.h"
#include "ZQueue.h"
#include "ZTimer.h"
#if !(defined(K32W1480_SERIES) || defined(MCXW716A_SERIES) || defined(MCXW716C_SERIES) || defined(NCP_HOST))
#include "fsl_reset.h"
#endif
#ifndef NCP_HOST
#include "MicroSpecific.h"
#endif
#include "app_console.h"
#if defined(FSL_RTOS_FREE_RTOS) &&  DEBUG_STACK_DEPTH
#include "FreeRTOS.h"
#include "task.h"
#endif
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef TRACE_SERIAL
    #define TRACE_SERIAL      FALSE
#endif

#ifndef TRACE_REMOTE_ECHO
    #define TRACE_REMOTE_ECHO FALSE
#endif
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
#define COMMAND_BUF_SIZE   80

typedef struct
{
    uint8_t  au8Buffer[COMMAND_BUF_SIZE];
    uint8_t  u8Pos;
}tsCommand;

#define stricmp strcasecmp

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static void vProcessRxChar(uint8_t u8Char);

static void vProcessCommand(char *tmp);
static void vPrintUnkownCommand(char *token);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static tsCommand sCommand;
char * strings[] = {
        "START UP",
        "NFN START",
        "RUNNING"
        };
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/******************************************************************************
 * NAME: APP_taskAtSerial
 *
 * DESCRIPTION:
 *
 *
 * PARAMETERS:      Name            Usage
 *
 * RETURNS:
 * None
 ****************************************************************************/
void APP_taskAtSerial( void)
{
    uint8_t u8RxByte;
    if ( APP_bConsoleReceiveChar ( &u8RxByte ) ) {
        vProcessRxChar ( u8RxByte);
    }
}

/******************************************************************************
 * NAME: vProcessRxChar
 *
 * DESCRIPTION:
 * Processes the received character
 *
 * PARAMETERS:      Name            Usage
 * uint8_t          u8Char          Character
 *
 * RETURNS:
 * None
 ****************************************************************************/
static void vProcessRxChar(uint8_t u8Char)
{
    const uint8_t ASCII_CR = 0x0D;
    const uint8_t ASCII_LF = 0x0A;
    static uint8_t u8PreviousIsCR = 0;

    if ((u8Char >= 'a' && u8Char <= 'z'))
    {
        u8Char -= ('a' - 'A');
    }
    if ((sCommand.u8Pos < COMMAND_BUF_SIZE)  && (u8Char != ASCII_CR) && (u8Char != ASCII_LF))
    {
        sCommand.au8Buffer[sCommand.u8Pos++] = u8Char;
    }
    else if (sCommand.u8Pos >= COMMAND_BUF_SIZE)
    {
        DBG_vPrintf(TRACE_SERIAL, "OverFlow\r\n");
        memset(sCommand.au8Buffer, 0, COMMAND_BUF_SIZE);
        sCommand.u8Pos = 0;
    }

    /* Handles CR, LF or CRLF line breaks */
    if (u8Char == ASCII_CR)
    {
        if (sCommand.u8Pos)
            vProcessCommand(NULL);
        else
            vPrintUnkownCommand("");

        u8PreviousIsCR = 1;
    }
    else if (u8Char == ASCII_LF)
    {
        if (u8PreviousIsCR)
        {
            /* CRLF, so skip the LF line break */
            u8PreviousIsCR = 0;
        }
        else
        {
            if (sCommand.u8Pos)
                vProcessCommand(NULL);
            else
                vPrintUnkownCommand("");
        }
    }
    else
    {
        u8PreviousIsCR = 0;
    }
}

/******************************************************************************
 * NAME: vProcessCommand
 *
 * DESCRIPTION:
 * Processed the received command
 *
 * PARAMETERS:      Name            Usage
 *
 * RETURNS:
 * None
 ****************************************************************************/
static void vProcessCommand(char *tmp)
{
    uint8_t *token = NULL;

    if (tmp)
    {
        strncpy((char *)sCommand.au8Buffer, tmp, COMMAND_BUF_SIZE);
        sCommand.au8Buffer[COMMAND_BUF_SIZE - 1] = 0;
    }

    token = sCommand.au8Buffer;

    APP_tsEvent sButtonEvent;
    sButtonEvent.eType = APP_E_EVENT_NONE;

    if (0 == stricmp((char*)token, "toggle"))
    {
        DBG_vPrintf(TRACE_SERIAL, "Toggle\r\n");
        sButtonEvent.eType = APP_E_EVENT_SERIAL_TOGGLE;
    }
    else if (0 == stricmp((char*)token, "steer"))
    {
        DBG_vPrintf(TRACE_SERIAL, "Steer\r\n");
        sButtonEvent.eType = APP_E_EVENT_SERIAL_NWK_STEER;
    }
    else if (0 == stricmp((char*)token, "form"))
    {
        DBG_vPrintf(TRACE_SERIAL, "Form\r\n");
        sButtonEvent.eType = APP_E_EVENT_SERIAL_FORM_NETWORK;
    }
    else if (0 == stricmp((char*)token, "find"))
    {
        DBG_vPrintf(TRACE_SERIAL, "Find\r\n");
        sButtonEvent.eType = APP_E_EVENT_SERIAL_FIND_BIND_START;
    }
    else if (0 == stricmp((char*)token, "factory reset"))
    {
        DBG_vPrintf(TRACE_SERIAL, "Factory reset\r\n");
        APP_vFactoryResetRecords();
#ifndef NCP_HOST
        MICRO_DISABLE_INTERRUPTS();
        RESET_SystemReset();
#endif
    }
    else if (0 == stricmp((char*)token, "soft reset"))
    {
#ifndef NCP_HOST
        MICRO_DISABLE_INTERRUPTS();
        RESET_SystemReset();
#endif
    }

#if defined(FSL_RTOS_FREE_RTOS) && DEBUG_STACK_DEPTH
    else if (0 == stricmp((char*)token, "stack"))
    {
        UBaseType_t uxHighWaterMark;

        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        DBG_vPrintf(TRUE, "Stack High Watermark = %d B\r\n", uxHighWaterMark * sizeof(unsigned int));
    }
#endif
    else if (!tmp)
    {
        vPrintUnkownCommand((char*)token);
    }

    if (sButtonEvent.eType != APP_E_EVENT_NONE)
    {
        if( ZQ_bQueueSend(&APP_msgAppEvents, &sButtonEvent)  == FALSE)
        {
            DBG_vPrintf(1, "Queue Overflow has happened \n");
        }
    }


    memset(sCommand.au8Buffer, 0, COMMAND_BUF_SIZE);
    sCommand.u8Pos = 0;

}

/******************************************************************************
 * NAME: vPrintUnkownCommand
 *
 * DESCRIPTION:
 * Received an unkown command, print list of available commands
 *
 * PARAMETERS:      Name            Usage
 *
 * RETURNS:
 * None
 ****************************************************************************/
static void vPrintUnkownCommand(char *token)
{
    DBG_vPrintf(1, "Unkown serial command %s\r\n", token);
    DBG_vPrintf(1,"Commands Supported \r\n");
    DBG_vPrintf(1,"****************** \r\n");
    DBG_vPrintf(1,"toggle - sends on\\off toggle command\r\n");
    DBG_vPrintf(1,"steer  - opens permit join window\r\n");
    DBG_vPrintf(1,"form   - forms the network\r\n");
    DBG_vPrintf(1,"find   - makes a binding on a target which is identifying\r\n");
    DBG_vPrintf(1,"factory reset  - clears all data and starts afresh \r\n");
    DBG_vPrintf(1,"soft reset  - retains network configuration but resets the node \r\n");
#if defined(FSL_RTOS_FREE_RTOS) && DEBUG_STACK_DEPTH
    DBG_vPrintf(1,"stack  - display stack high watermark \r\n");
#endif

}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
