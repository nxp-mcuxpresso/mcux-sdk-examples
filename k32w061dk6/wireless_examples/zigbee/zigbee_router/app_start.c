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
#include "PDM.h"
#include "app_router_node.h"
#include "app_main.h"
#include "bdb_api.h"
#include "zigbee_config.h"
#include "app_leds.h"
#include "app_buttons.h"
#ifdef ENABLE_SUBG_IF
#include "MDI_ReadFirmVer.h"
#include "MDI_Programmer.h"
#endif
#ifdef NCP_HOST
#include "dbg.h"
#include "serial_link_ctrl.h"
#endif
#ifdef APP_ROUTER_NODE_CLI
#include "app_console.h"
#endif
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifndef TRACE_APP
    #define TRACE_APP   TRUE
#endif

#define HALT_ON_EXCEPTION   FALSE

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

static void APP_vInitialise(void);
#ifdef ENABLE_SUBG_IF
static void APP_vCheckMtgState(void);
#endif
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

extern void *_stack_low_water_mark;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern void OSA_TimeInit(void);
/****************************************************************************
 *
 * NAME: vAppMain
 *
 * DESCRIPTION:
 * Entry point for application from a cold start.
 *
 * RETURNS:
 * Never returns.
 *
 ****************************************************************************/
void vAppMain(void)
{
#ifndef NCP_HOST
    /* Initialise LEDs and buttons */
    APP_vLedInitialise();
#if !defined(LNT_MODE_APP) && !defined(USART1_FTDI)
    /* DK6 specific code */
    /* GPIO1 is used for USART1 RX */
    APP_bButtonInitialise();
#endif
#endif

	APP_vInitResources();
    APP_vInitZigbeeResources();
    APP_vInitialise();
#ifdef APP_ROUTER_NODE_CLI
    APP_vConsoleInitialise();
#endif
    BDB_vStart();
}

/****************************************************************************
 *
 * NAME: vAppRegisterPWRMCallbacks
 *
 * DESCRIPTION:
 * Power manager callback.
 * Called to allow the application to register
 * sleep and wake callbacks.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void vAppRegisterPWRMCallbacks(void)
{
    /* nothing to register as device does not sleep */
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: APP_vInitialise
 *
 * DESCRIPTION:
 * Initialises Zigbee stack, hardware and application.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void APP_vInitialise(void)
{
    /* Initialise the Persistent Data Manager */
    PDM_eInitialise(1200, 63, NULL);

#ifdef ENABLE_SUBG_IF
    APP_vCheckMtgState();
#endif

#ifdef NCP_HOST
    (void)eSL_SerialInit();
    DBG_vPrintf(TRUE, "serial Link initialised\n");

    extern uint8_t rxDmaTimerHandle;
    ZTIMER_eStart(rxDmaTimerHandle, 100);
#endif

    /* Initialise application */
    APP_vInitialiseRouter();
}

#ifdef ENABLE_SUBG_IF
/****************************************************************************
 *
 * NAME: APP_vCheckMtgState
 *
 * DESCRIPTION:
 * Initialises MTG comms
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_vCheckMtgState(void)
{
	vMDI_RecoverDioSettings();
    vMDI_PrintIOConfig();

    /* Reset the Radio, don't care */
   	vMDI_Reset868MtGRadio();

   	/* Send SYNC request, wait for SYNC resp */
   	vMDI_SyncWithMtG();
}
#endif
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
