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
#include "app_leds.h"
#ifdef ENABLE_SUBG_IF
#include "MDI_ReadFirmVer.h"
#include "MDI_Programmer.h"
#endif
#if defined(gWCI2_UseCoexistence_d) && (gWCI2_UseCoexistence_d == 1)
#include "MWS.h"
#include "GPIO_Adapter.h"
#include "pin_mux.h"
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
PUBLIC uint8 u8JNReadyForCmds;
#if defined(gWCI2_UseCoexistence_d) && (gWCI2_UseCoexistence_d == 1)
/* Avoid including MMAC specific stuff, just declare as extern */
extern void vDynEnableCoex(void *, void *, void *, void *, void *);
extern void sched_enable();
#endif
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
#if defined(gWCI2_UseCoexistence_d) && (gWCI2_UseCoexistence_d == 1)
    mwsStatus_t status;
    gpioOutputPinConfig_t *rf_req;
    gpioInputPinConfig_t *rf_tx_deny;
    gpioInputPinConfig_t *rf_rx_deny;
#endif

    /* Initialise the Persistent Data Manager */
    PDM_eInitialise(1200, 63, NULL);

#ifdef ENABLE_SUBG_IF
    APP_vCheckMtgState();
#endif
    UART_vInit(NULL);
#if defined(gWCI2_UseCoexistence_d) && (gWCI2_UseCoexistence_d == 1)
    BOARD_GetWCI2CoexPins((void **)&rf_req,
                          (void **)&rf_tx_deny,
                          (void **)&rf_rx_deny);

    status = WCI2_CoexistenceInit(rf_req, rf_tx_deny, rf_rx_deny);
    if (status == gMWS_Success_c)
    {
        vDynEnableCoex((void *)WCI2_CoexistenceRegister, (void *)WCI2_CoexistenceRequestAccess,
                       (void *)WCI2_CoexistenceSetPriority, (void *)WCI2_CoexistenceReleaseAccess,
                       (void *)WCI2_CoexistenceChangeAccess);
        WCI2_CoexistenceEnable();
    }

#endif /* defined(gWCI2_UseCoexistence_d) && (gWCI2_UseCoexistence_d == 1) */
    /* Initialise application */
    APP_vInitialiseCoordinator();
#if defined(gWCI2_UseCoexistence_d) && (gWCI2_UseCoexistence_d == 1)
    sched_enable();
#endif
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
