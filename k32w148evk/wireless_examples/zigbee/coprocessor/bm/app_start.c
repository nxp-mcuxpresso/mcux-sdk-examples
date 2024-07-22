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
#include "serial_link_cmds_wkr.h"
#include "app_coordinator.h"
#include "app_serial_commands.h"
#include "bdb_api.h"
#include "app_leds.h"
#ifdef ENABLE_SUBG_IF
#include "MDI_ReadFirmVer.h"
#include "MDI_Programmer.h"
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
PUBLIC tsNcpDeviceDesc           sNcpDeviceDesc = {FACTORY_NEW, E_STARTUP, ZPS_ZDO_DEVICE_COORD, 0x0000000000000002UL, FALSE};
PUBLIC uint8 u8JNReadyForCmds;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PUBLIC uint8 u8Error;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

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
    APP_vInitResources();
    APP_vInitZigbeeResources();
    APP_vInitialise();
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
    UART_vInit(NULL);
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

PUBLIC void APP_vResetDeviceState(void)
{
    sNcpDeviceDesc.eState = FACTORY_NEW;
    vSaveDevicePdmRecord();

}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
