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
#ifndef NCP_HOST
#include "fsl_gpio.h"
#include "RNG_Interface.h"
#include "SecLib.h"
#endif
#ifdef K32W1480_SERIES
#include "fwk_platform.h"
#include "fwk_platform_ics.h"
#include "fsl_component_mem_manager.h"
#else
#ifndef NCP_HOST
#include "MemManager.h"
#include "TimersManager.h"
#endif
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
#ifdef NCP_HOST
#include "serial_link_ctrl.h"
#endif
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define APP_NUM_STD_TMRS             (2)
#ifdef NCP_HOST
#define APP_QUEUE_SIZE               (30)

#ifndef ZCL_QUEUE_SIZE
#define ZCL_QUEUE_SIZE              35U
#endif

#else
#define APP_QUEUE_SIZE               (1)
#endif

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
uint8_t u8TimerId;
uint8_t  u8LedTimer;
/* queue handles */
tszQueue APP_msgAppEvents;
uint32_t u32Togglems;
#ifdef NCP_HOST
tszQueue appQueueHandle;
tszQueue zclQueueHandle;
#endif
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static ZTIMER_tsTimer asTimers[APP_ZTIMER_STORAGE + ZIGBEE_TIMER_STORAGE];
extern const uint8_t gUseRtos_c;




/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

extern void vAppMain(void);
#ifdef NCP_HOST
void APP_vNcpMainTask(void);
#endif
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
#ifndef NCP_HOST
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
        RNG_Init();
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
#if defined(FSL_RTOS_FREE_RTOS) && DEBUG_STACK_DEPTH
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        DBG_vPrintf(TRUE, "Stack High Watermark = %d B\r\n",
		uxHighWaterMark * sizeof(unsigned int));
#endif
    }

    while(1)
    {

         /* place event handler code here... */
    	APP_vRunZigbee();
    	ZTIMER_vTask();
        APP_taskCoordinator();
        APP_taskAtSerial();
        if(!gUseRtos_c)
        {
            break;
        }
    }
}
#else
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
int ncp_fd;

#ifdef ENABLE_SERIAL_LINK_FILE_LOGGING
int log_fd;
bool log_started;
void vSerialLogToFile(uint8_t *buf, uint32_t len)
{
    if (log_started)
    {
        write(log_fd, buf, len);
    }
}

int open_log_file(char *filename)
{
    int ret = 0;

    log_fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (!log_fd)
    {
        DBG_vPrintf(TRUE,"Error opening %s\r\n", filename);
        ret = -1;
    }
    log_started = TRUE;
    DBG_vPrintf(TRUE,"Starting logging serial comms to %s", filename);
    return ret;
}
#endif
int open_rcp(char *dev_name)
{
	struct termios tios;
	int ret = 0;

	ncp_fd = open(dev_name, O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);
	if (!ncp_fd)
	{
		DBG_vPrintf(TRUE,"Error opening %s\r\n", dev_name);
		ret = -1;
		goto exit_error;
	}

	tcgetattr(ncp_fd, &tios);

	cfmakeraw(&tios);

    tios.c_cflag = CS8 | HUPCL | CREAD | CLOCAL;

	ret = cfsetspeed(&tios, SERIAL_BAUD_RATE);
	if (ret != 0)
	{
		DBG_vPrintf(TRUE, "Failed to set speed");
		ret = -2;
		goto exit_error;
	}


	//tios.c_cflag |= CRTSCTS;

//    tios.c_lflag = 0;
//    tios.c_cc[VMIN] = 1;
//    tios.c_cc[VTIME] = 0;

	ret = tcsetattr(ncp_fd, TCSANOW, &tios);
	if (ret != 0)
	{
	    DBG_vPrintf(TRUE, "Failed tcsetattr");
	    ret = -3;
	    goto exit_error;
	}

	ret = tcflush(ncp_fd, TCIOFLUSH);
	if (ret != 0)
	{
        DBG_vPrintf(TRUE, "Failed tcflush");
	}

exit_error:
	return ret;
}

int set_stdin_non_blocking()
{
	struct termios term, oldterm;
	int ret = 0;

	if (tcgetattr(STDIN_FILENO, &term))
	{
		ret = -1;
		goto exit_error;
	}

	oldterm = term;
	term.c_lflag &= ~(ECHO | ICANON);
	if (tcsetattr(STDIN_FILENO, TCSANOW, &term))
	{
		ret = -2;
		goto exit_error;
	}

exit_error:
	return ret;
}

#include <sys/time.h>
#include <signal.h>

void increment_tick_count(int s)
{
    extern volatile uint32_t u32TickCount;

    u32TickCount++;
}

int set_timer_alarm()
{
    struct itimerval tick_count_timer;

    tick_count_timer.it_value.tv_sec = 0;
    tick_count_timer.it_value.tv_usec= 1000;

    tick_count_timer.it_interval.tv_sec  = 0;
    tick_count_timer.it_interval.tv_usec = 1000;

    signal(SIGALRM, increment_tick_count);

    setitimer(ITIMER_REAL, &tick_count_timer, NULL);
}


int main(int argc, char * argv[])
{
	int ret = 0;

	if (argc < 2)
	{
		DBG_vPrintf(TRUE,"Usage %s /dev/ttyX\r\n", argv[0]);
		exit(-1);
	}

#if ENABLE_SERIAL_LINK_FILE_LOGGING
	if (argc == 3)
	{
	    ret = open_log_file(argv[2]);
	    if (ret != 0)
	    {
	        DBG_vPrintf(TRUE, "Failed to open log file %s", argv[2]);
	        exit(ret);
	    }
	}
#endif
	ret = open_rcp(argv[1]);
	if (ret != 0)
	{
		exit(ret);
	}

	ret = set_stdin_non_blocking();
	if (ret != 0)
	{
		DBG_vPrintf(TRUE, "Failed to set stdin non blocking\r\n");
		exit(ret);
	}

	set_timer_alarm();

	DBG_vPrintf(TRUE, "MAIN\n");

	vAppMain();

    while(1)
    {
        APP_vSeHostCheckRxBuffer();

         /* place event handler code here... */
        APP_vNcpMainTask();

    	APP_vRunZigbee();
    	ZTIMER_vTask();

        APP_taskCoordinator();
        APP_taskAtSerial();

        if(!gUseRtos_c)
        {
            break;
        }
    }
	close(ncp_fd);
}
#endif /* NCP_HOST */
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

    /* Create Z timers */
    ZTIMER_eOpen(&u8TimerId,            APP_cbTimerId,          NULL, ZTIMER_FLAG_PREVENT_SLEEP);
#ifndef NCP_HOST
    ZTIMER_eOpen(&u8LedTimer,           APP_cbTimerLed,  NULL, ZTIMER_FLAG_PREVENT_SLEEP);
#endif
    ZQ_vQueueCreate(&APP_msgAppEvents,        APP_QUEUE_SIZE,          sizeof(APP_tsEvent),         NULL);

#ifdef NCP_HOST
    ZQ_vQueueCreate(&appQueueHandle,        APP_QUEUE_SIZE,          4,         NULL);
    ZQ_vQueueCreate(&zclQueueHandle,        ZCL_QUEUE_SIZE,          4,         NULL);
#endif
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
	if( ZPS_bGetPermitJoiningStatus()|| u32Togglems != 500)
	{
	    ZTIMER_eStart(u8LedTimer, ZTIMER_TIME_MSEC(u32Togglems));
	    bCurrentState = !bCurrentState;
	}
	else
	{
		APP_vSetLed(APP_E_LEDS_LED_2, APP_E_LED_ON);
	}

}
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
