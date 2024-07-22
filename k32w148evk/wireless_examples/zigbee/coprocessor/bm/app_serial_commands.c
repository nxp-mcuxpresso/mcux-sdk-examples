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
#if !defined(K32W1480_SERIES) && !defined(MCXW716A_SERIES) && !defined(MCXW716C_SERIES)
#include "fsl_reset.h"
#endif
#include "app_uart.h"
#if defined(FSL_RTOS_FREE_RTOS) &&  DEBUG_STACK_DEPTH
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "app_main.h"
#include "serial_link_wkr.h"
#include "serial_link_cmds_wkr.h"
#include "PDM_IDs.h"
#include "app_crypto.h"

#include "version.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef TRACE_SERIAL
    #define TRACE_SERIAL      FALSE
#endif

#ifndef TRACE_APP
#define TRACE_APP   TRUE
#endif
#ifndef TRACE_TX_BUFFERS
#define TRACE_TX_BUFFERS    FALSE
#endif

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
PRIVATE int16 s_i16RxByte;

void APP_taskAtSerial(void)
{
    uint8_t u8UARTByte;

    if ( UART_bReceiveChar ( &u8UARTByte ) )
    {
        if (bRxBufferLocked)
        {
            u32OverwrittenRXMessage++;
        }

        if(TRUE == bSL_ReadMessage(&u16PacketType,
                                   &u16PacketLength,
                                   MAX_PACKET_RX_SIZE,
                                   &u8ReceivedSeqNo,
                                   au8LinkRxBuffer,
                                   u8UARTByte))
        {
            ZQ_bQueueSend(&APP_msgSerialRx, &u8UARTByte);
            bRxBufferLocked = TRUE;
        }
    }
}

PUBLIC void APP_SerialCmdTask(void)
{
    uint8 u8RxByte;
    s_i16RxByte = -1;

    if (ZQ_bQueueReceive(&APP_msgSerialRx, &u8RxByte))
    {
        s_i16RxByte = u8RxByte;
        /* TODO: Add a call to vApp_WatchdogRestart(); */
        vProcessIncomingSerialCommands();
        bRxBufferLocked = FALSE;
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
