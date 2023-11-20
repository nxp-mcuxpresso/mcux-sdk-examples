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
#include "app_main.h"
#include "zigbee_config.h"
#include "PDM.h"
#include "app_uart.h"
#include "app_coordinator.h"
#include "bdb_api.h"
#include "app_leds.h"
#ifdef ENABLE_SUBG_IF
#include "MDI_ReadFirmVer.h"
#include "MDI_Programmer.h"
#endif
#ifdef NCP_HOST
#include <pthread.h>
#include "dbg.h"
#include "serial_link_ctrl.h"
#endif
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

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


/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
extern uint8* ZPS_pu8AplZdoGetVsOUI(void);
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
	APP_vLedInitialise();
	APP_vInitResources();
    APP_vInitZigbeeResources();
    APP_vInitialise();
    BDB_vStart();
}

#ifdef NCP_HOST
extern void HOST_TO_JN_UART_ISR(void);
pthread_mutex_t host_to_jn_uart_isr_lock = PTHREAD_MUTEX_INITIALIZER;

void *SL_thread_routine(void *param)
{
    while (1 != 0)
    {
        HOST_TO_JN_UART_ISR();
    }
    return NULL;
}
#endif
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
    UART_vInit();
#ifdef NCP_HOST
    (void)eSL_SerialInit();
    DBG_vPrintf(TRUE, "serial Link initialised\n");

    pthread_t cThread;
    if (pthread_create(&cThread, NULL, SL_thread_routine, NULL))
    {
        perror("ERROR creating SL thread.");
    }
    extern uint8_t rxDmaTimerHandle;
    ZTIMER_eStart(rxDmaTimerHandle, 100);
#endif

    /* Initialise application */
    APP_vInitialiseCoordinator();
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
