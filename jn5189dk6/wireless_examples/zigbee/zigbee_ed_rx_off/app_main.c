/*
* Copyright 2019 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "EmbeddedTypes.h"
#include "ZQueue.h"
#include "ZTimer.h"
#include "zigbee_config.h"
#include "fsl_gpio.h"
#include "app_crypto.h"
#if IS_MCXW_SERIES
#include "fwk_platform.h"
#include "fwk_platform_ics.h"
#include "fsl_component_mem_manager.h"
#include "fsl_component_timer_manager.h"
#else
#include "MemManager.h"
#include "TimersManager.h"
#endif
#include "app_buttons.h"
#include "app_main.h"
#include "app_end_device_node.h"
#include "app_leds.h"
#include "PWR_Interface.h"
#include "pwrm.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef OT_ZB_SUPPORT
#define APP_NUM_STD_TMRS             (1)
#else
#define APP_NUM_STD_TMRS             (2)
#endif
#define APP_QUEUE_SIZE               (1)
#define APP_ZTIMER_STORAGE           ( APP_NUM_STD_TMRS )


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
uint8_t  u8TimerPoll;
#ifdef OT_ZB_SUPPORT
uint8_t  u8TimerScan;
uint8_t  u8TimerFb;
#endif
/* queue handles */
tszQueue APP_msgAppEvents;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static ZTIMER_tsTimer asTimers[APP_ZTIMER_STORAGE + ZIGBEE_TIMER_STORAGE];
extern const uint8_t gUseRtos_c;




/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

extern void vAppMain(bool_t bColdStart);

/****************************************************************************
 *
 * NAME: main_task
 *
 * DESCRIPTION:
 * Main  execution loop
 *
 * RETURNS:
 * Never
 *
 ****************************************************************************/
#ifndef OT_ZB_SUPPORT
void main_task (uint32_t parameter)
{

    /* e.g. osaEventFlags_t ev; */
    static uint8_t initialized = FALSE;

    if(!initialized)
    {
        /* place initialization code here... */
        initialized = TRUE;

#if IS_MCXW_SERIES
        PLATFORM_SwitchToOsc32k();
        PLATFORM_InitTimerManager();
#endif
        PWR_vColdStart();
#if IS_NOT_MCXW_SERIES
        TMR_Init();
#endif

        CRYPTO_u8RandomInit();
        CRYPTO_Init();
        MEM_Init();

#if IS_MCXW_SERIES
#if defined(USE_NBU) && (USE_NBU == 1)
        PLATFORM_InitNbu();
        PLATFORM_InitMulticore();
        PLATFORM_FwkSrvInit();
        PLATFORM_SendChipRevision();
        PLATFORM_LoadHwParams();
#endif
#endif
        vAppMain(TRUE);
    }

    while(1)
    {

         /* place event handler code here... */
        APP_vRunZigbee();
        ZTIMER_vTask();

        APP_taskEndDevicNode();

#if IS_NOT_MCXW_SERIES
        PWR_EnterLowPower();
#else
        PWR_EnterLowPower(0);
#endif

        if(!gUseRtos_c)
        {
            break;
        }
    }
}
#endif
/****************************************************************************
 *
 * NAME: APP_vInitResources
 *
 * DESCRIPTION:
 * Initialise resources (timers, queue's etc)
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
void APP_vInitResources(void)
{
    /* Initialise the Z timer module */
    ZTIMER_eInit(asTimers, sizeof(asTimers) / sizeof(ZTIMER_tsTimer));

    /* Create Z timers */
    ZTIMER_eOpen(&u8TimerPoll,          APP_cbTimerPoll,        NULL, ZTIMER_FLAG_PREVENT_SLEEP);
#ifdef OT_ZB_SUPPORT
    ZTIMER_eOpen(&u8TimerScan,          APP_cbTimerScan,        NULL, ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerFb,            APP_cbTimerFb,          NULL, ZTIMER_FLAG_PREVENT_SLEEP);
#endif
    ZQ_vQueueCreate(&APP_msgAppEvents,  APP_QUEUE_SIZE,       sizeof(APP_tsEvent),         NULL);

}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
