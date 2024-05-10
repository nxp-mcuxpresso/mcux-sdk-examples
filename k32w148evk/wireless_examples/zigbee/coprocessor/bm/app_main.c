/*
* Copyright 2019, 2023 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
/* FreeRTOS kernel includes. */

#ifdef FSL_RTOS_FREE_RTOS
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "portmacro.h"

/* For uxTaskGetStackHighWaterMark */
#if DEBUG_STACK_DEPTH
#include "task.h"
#endif
#endif

#include "EmbeddedTypes.h"
#include "ZQueue.h"
#include "ZTimer.h"
#include "zigbee_config.h"
#include "fsl_gpio.h"
#include "app_crypto.h"
#include "SecLib.h"

#ifdef K32W1480_SERIES
#include "fwk_platform.h"
#include "fwk_platform_ics.h"
#include "fsl_component_mem_manager.h"
#include "fsl_component_timer_manager.h"
#else
#include "MemManager.h"
#endif
#include "app_coordinator.h"
#ifndef DUAL_MODE_APP
#include "app_serial_commands.h"
#endif
#include "app_buttons.h"
#include "app_main.h"
#include "app_leds.h"
#include "zps_apl_zdo.h"
#include "dbg.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define APP_NUM_STD_TMRS             (1)
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
/* timers */
uint8_t u8APP_DiscoveryTimer;

/* queue handles */
tszQueue APP_msgAppEvents;
extern tszQueue APP_msgSerialRx;
uint32_t u32Togglems;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static ZTIMER_tsTimer asTimers[APP_ZTIMER_STORAGE + ZIGBEE_TIMER_STORAGE];
extern const uint8_t gUseRtos_c;
void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus);



/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

extern void vAppMain(void);

#ifndef DUAL_MODE_APP
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
void main_task (uint32_t parameter)
{
#if DEBUG_STACK_DEPTH
    UBaseType_t uxHighWaterMark;
#endif

    /* e.g. osaEventFlags_t ev; */
    static uint8_t initialized = FALSE;

    if(!initialized)
    {
        /* place initialization code here... */
        initialized = TRUE;
#ifndef K32W1480_SERIES
        TMR_Init();
#else
        PLATFORM_SwitchToOsc32k();
        PLATFORM_InitTimerManager();
#endif
        CRYPTO_u8RandomInit();
        SecLib_Init();
        MEM_Init();
#ifdef K32W1480_SERIES
#if defined(USE_NBU) && (USE_NBU == 1)
        PLATFORM_InitNbu();
        PLATFORM_InitMulticore();
        PLATFORM_FwkSrvInit();
        PLATFORM_SendChipRevision();
        PLATFORM_LoadHwParams();

#endif
#endif
        vAppMain();

        ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);

#if defined(FSL_RTOS_FREE_RTOS) && DEBUG_STACK_DEPTH
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        DBG_vPrintf(TRUE, "Stack High Watermark = %d B\r\n",
        uxHighWaterMark * sizeof(unsigned int));
#endif
    }

    /* Signal to LPC that I'm ready to receive serial commands */
    extern uint8 u8JNReadyForCmds;
    u8JNReadyForCmds = 1;

    while(1)
    {
         /* place event handler code here... */
        APP_vRunZigbee();
        ZTIMER_vTask();
        APP_ZpsEventTask();
        APP_taskAtSerial();
        APP_SerialCmdTask();

        if(!gUseRtos_c)
        {
            break;
        }
    }
}
#endif /* DUAL_MODE_APP */

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
    //ZTIMER_eOpen(&u8APP_DiscoveryTimer,      APPDiscoveryTimeout,     NULL,  ZTIMER_FLAG_PREVENT_SLEEP);

    ZQ_vQueueCreate(&APP_msgAppEvents,        APP_QUEUE_SIZE,          sizeof(APP_tsEvent),         NULL);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
