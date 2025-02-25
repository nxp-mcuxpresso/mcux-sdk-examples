/*
* Copyright 2019, 2023-2024 NXP
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/


/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#ifdef NCP_HOST
#include <signal.h>
#endif
#include "zb_platform.h"
#include "EmbeddedTypes.h"

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

#include "ZQueue.h"
#include "ZTimer.h"
#include "zigbee_config.h"
#include "app_crypto.h"
#ifndef NCP_HOST
#include "fsl_gpio.h"
#endif
#if IS_MCXW_SERIES_OR_RW_SERIES
#include "fwk_platform.h"
#include "fsl_component_mem_manager.h"
#if IS_MCXW_SERIES
#include "fwk_platform_ics.h"
#endif
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
#include "app_uart.h"
#include "app_buttons.h"
#include "app_main.h"
#include "app_leds.h"
#include "zps_apl_zdo.h"
#include "dbg.h"
#ifdef NCP_HOST
#include "serial_link_ctrl.h"
#include "app_common_ncp.h"
#include "app_console.h"
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
#ifndef NCP_HOST
uint8_t  u8LedTimer;
#endif
/* queue handles */
tszQueue APP_msgAppEvents;
uint32_t u32Togglems;
#ifdef NCP_HOST
extern tszQueue appQueueHandle;
extern tszQueue zclQueueHandle;
#endif
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static ZTIMER_tsTimer asTimers[APP_ZTIMER_STORAGE + ZIGBEE_TIMER_STORAGE];
extern const uint8_t gUseRtos_c;
#ifdef NCP_HOST
static uint8_t bAppIsRunning = TRUE;
static uint8_t bNcpHostTaskIsRunning = TRUE;
pid_t ncpHostPid = 0;
#endif


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
#if IS_NOT_MCXW_SERIES_OR_RW_SERIES
        TMR_Init();
#else
#if IS_MCXW_SERIES
        PLATFORM_SwitchToOsc32k();
#endif
        PLATFORM_InitTimerManager();
#endif
        CRYPTO_Init();
        CRYPTO_u8RandomInit();
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

void sigint_handler(int sig)
{
    DBG_vPrintf(TRUE,"****************** \r\n");
    if (sig == SIGINT)
    {
        DBG_vPrintf(TRUE, "Ctrl+C was pressed, application will terminate\n");
    }
    else if (sig == SIGTSTP)
    {
        DBG_vPrintf(TRUE, "Ctrl+Z was pressed, application will terminate\n");
    }

    kill(ncpHostPid, SIGHUP);
    bAppIsRunning = false;
}

void sighup_handler(int signum)
{
    bNcpHostTaskIsRunning = false;
}

#include <sys/wait.h>

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        DBG_vPrintf(TRUE,"Usage %s /dev/ttyX\r\n", argv[0]);
        exit(-1);
    }

    while (bAppIsRunning)
    {
        ncpHostPid = fork();
        if (ncpHostPid < 0)
        {
            DBG_vPrintf(TRUE, "Failed to create NCP Host Application process\n");
            return 0;
        }
        if (ncpHostPid == 0)
        {
            DBG_vPrintf(TRACE_APP, "Created NCP Host Task with pid %d\n", getpid());
            struct sigaction sa;

            /* SIGHUP signal to allow parent to terminate child */
            memset(&sa, 0, sizeof(struct sigaction));
            sa.sa_flags     = 0;
            sa.sa_handler   = sighup_handler;
            sigemptyset(&sa.sa_mask);

            if (sigaction(SIGHUP, &sa, NULL) == -1)
            {
                DBG_vPrintf(TRUE, "Fail to set up signal handler %d\n", SIGHUP);
                exit(EXIT_FAILURE);
            }

            /* ignore SIGINT, SIGTSTP signals inherited from parent */
            memset(&sa, 0, sizeof(struct sigaction));
            sa.sa_flags     = 0;
            sa.sa_handler   = SIG_IGN;
            sigemptyset(&sa.sa_mask);

            if (sigaction(SIGINT, &sa, NULL) == -1)
            {
                DBG_vPrintf(TRUE, "Fail to set up signal handler %d\n", SIGINT);
                exit(EXIT_FAILURE);
            }
            if (sigaction(SIGTSTP, &sa, NULL) == -1)
            {
                DBG_vPrintf(TRUE, "Fail to set up signal handler %d\n", SIGINT);
                exit(EXIT_FAILURE);
            }

#if ENABLE_SERIAL_LINK_FILE_LOGGING
            if (argc == 3)
            {
                if (!bSL_LoggerInit(argv[2]))
                {
                    DBG_vPrintf(TRUE, "Failed to open log file %s", argv[2]);
                    return 0;
                }
            }
            else
            {
                DBG_vPrintf(TRUE, "Provide filename for Serial Link logger\n");
                return 0;
            }
#endif
            /* Initialize UART to be used for host <-> coprocessor communication */
            UART_vInit(argv[1]);
            UART_vSetBaudRate(SERIAL_BAUD_RATE);

            CRYPTO_u8RandomInit();

            DBG_vPrintf(TRUE, "MAIN\n");

            bNcpHostTaskIsRunning = TRUE;
            
            vAppMain();

            while (bNcpHostTaskIsRunning)
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

            UART_vFree();
#if ENABLE_SERIAL_LINK_FILE_LOGGING
            vSL_LoggerFree();
#endif
            DBG_vPrintf(TRACE_APP, "Terminated NCP Host Task with pid %d\n", getpid());
            exit(EXIT_SUCCESS);
        }
        else
        {
            struct sigaction sa;
            int status;

             /* Catch SIGINT & SIGTSTP to allow user to terminate application */
            memset(&sa, 0, sizeof(struct sigaction));
            sa.sa_flags     = 0;
            sa.sa_handler   = sigint_handler;
            sigemptyset(&sa.sa_mask);

            if (sigaction(SIGINT, &sa, NULL) == -1)
            {
                DBG_vPrintf(TRUE, "Fail to set up signal handler %d\n", SIGINT);
                exit(EXIT_FAILURE);
            }
            if (sigaction(SIGTSTP, &sa, NULL) == -1)
            {
                DBG_vPrintf(TRUE, "Fail to set up signal handler %d\n", SIGTSTP);
                exit(EXIT_FAILURE);
            }

            /* Parent of child process for NCP Host application */
            waitpid(ncpHostPid, &status, 0);

            APP_vConsoleDeinitialise();

            if (WIFEXITED(status))
            {
                 DBG_vPrintf(TRACE_APP, "NCP Host Task with pid %d exited with status %d\n",
                         ncpHostPid, WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                 DBG_vPrintf(TRACE_APP, "NCP Host Task with pid %d exited with signal %d\n",
                         ncpHostPid, WTERMSIG(status));

                 if (WTERMSIG(status) == SIGABRT)
                 {
                     /* TODO : to be handled */
                 }

                 if (WTERMSIG(status) == SIGSEGV)
                 {
                    DBG_vPrintf(TRACE_APP, "NCP Host Task with pid %d encounterd a segmentation fault \n", ncpHostPid);
                 }
                 
                 bAppIsRunning = false;
            }
         }
    }
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
    ZQ_vQueueCreate(&appQueueHandle,        APP_QUEUE_SIZE,          sizeof(uintptr_t),         NULL);
    ZQ_vQueueCreate(&zclQueueHandle,        ZCL_QUEUE_SIZE,          sizeof(uintptr_t),         NULL);
#endif
}

#ifndef NCP_HOST
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
#endif
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
