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
#include "ZQueue.h"
#include "ZTimer.h"
#include "zigbee_config.h"
#include "fsl_gpio.h"
#include "app_crypto.h"
#if IS_MCXW_SERIES_OR_RW_SERIES
#include "fwk_platform.h"
#include "fsl_component_mem_manager.h"
#if IS_MCXW_SERIES
#include "fwk_platform_ics.h"
#endif
#else
#include "MemManager.h"
#include "TimersManager.h"
#endif
#include "app_buttons.h"
#include "app_main.h"
#include "app_end_device_node.h"
#include "app_leds.h"

#ifdef APP_END_DEVICE_NODE_CLI
#include "app_serial_commands.h"
#endif

#if IS_MCXW_SERIES
int PLATFORM_SwitchToOsc32k();
#endif
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define APP_NUM_STD_TMRS             (2)
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
uint8_t  u8LedTimer;
uint8_t  u8TimerPoll;
/* queue handles */
tszQueue APP_msgAppEvents;
uint32_t u32Togglems;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static ZTIMER_tsTimer asTimers[APP_ZTIMER_STORAGE + ZIGBEE_TIMER_STORAGE];
extern const uint8_t gUseRtos_c;




/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

extern void vAppMain(void);

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

    /* e.g. osaEventFlags_t ev; */
    static uint8_t initialized = FALSE;

    if(!initialized)
    {
        /* place initialization code here... */
        initialized = TRUE;
#if IS_NOT_MCXW_SERIES_OR_RW_SERIES
        TMR_Init();
#else
#if IS_MCXW_SERIES
        PLATFORM_SwitchToOsc32k();
#endif
        PLATFORM_InitTimerManager();
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
        vAppMain();
    }

    while(1)
    {

         /* place event handler code here... */
        APP_vRunZigbee();
        ZTIMER_vTask();

        APP_taskEndDevicNode();
#ifdef APP_END_DEVICE_NODE_CLI
        APP_taskAtSerial();
#endif


        if(!gUseRtos_c)
        {
            break;
        }
    }
}

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
    ZTIMER_eOpen(&u8LedTimer,           APP_cbTimerLed,         NULL, ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerPoll,          APP_cbTimerPoll,        NULL, ZTIMER_FLAG_PREVENT_SLEEP);
    ZQ_vQueueCreate(&APP_msgAppEvents,  APP_QUEUE_SIZE,       sizeof(APP_tsEvent),         NULL);

}


/****************************************************************************
*
* NAME: APP_cbTimerLed
*
* DESCRIPTION:
* Timer callback to to toggle LEDs
*
* PARAMETER:
*
* RETURNS:
*
****************************************************************************/
void APP_cbTimerLed(void *pvParam)
{
	static bool_t bCurrentState = TRUE;
	APP_vSetLed(APP_E_LEDS_LED_2, bCurrentState);
	ZTIMER_eStop(u8LedTimer);
    ZTIMER_eStart(u8LedTimer, ZTIMER_TIME_MSEC(u32Togglems));
	bCurrentState = !bCurrentState;
}
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
