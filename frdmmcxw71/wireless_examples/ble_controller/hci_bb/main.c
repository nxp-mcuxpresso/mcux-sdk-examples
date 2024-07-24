/*! *********************************************************************************
 * \addtogroup Main
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2022 NXP
* All rights reserved.
*
* \file
*
* This is the source file for the main entry point for a bare-metal application.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
 *************************************************************************************
 * Include
 *************************************************************************************
 ************************************************************************************/
#include "app.h"
#include "fsl_os_abstraction.h"
#include "fwk_platform_ble.h"
#if defined(CPU_KW45B41Z83AFTA)
#include "fwk_platform_lcl.h"
#endif

#include "fwk_platform.h"
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0)
#include "PWR_Interface.h"
#endif

#define USE_OSA_API_IN_MAIN     1

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

#if USE_OSA_API_IN_MAIN
static OSA_TASK_HANDLE_DEFINE(s_startTaskHandle);
#endif

/************************************************************************************
*************************************************************************************
* Public functions prototypes
*************************************************************************************
************************************************************************************/
extern void main_task(uint32_t param);

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
#if USE_OSA_API_IN_MAIN
void start_task(void *argument)
{
    main_task((uint32_t)argument);
}

static OSA_TASK_DEFINE(start_task, gMainThreadPriority_c, 1, gMainThreadStackSize_c, 0);
#endif

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
int main(void)
{
    /* Init OSA: should be called before any other OSA API */
    OSA_Init();

    BOARD_InitHardware();

    PLATFORM_InitBle();
#if defined(CPU_KW45B41Z83AFTA) && (!defined(FPGA_SUPPORT) || (FPGA_SUPPORT == 0))
/* Temporaryly disable RF_GPO & DTEST settings if FEM or COEX is configured */
#if (!defined(gAppConfigureFEM) || (gAppConfigureFEM == 0)) && \
    (!defined(gAppConfigureCOEX) || (gAppConfigureCOEX == 0))
    PLATFORM_InitLcl();
#endif
#endif

    APP_InitServices();

#if USE_OSA_API_IN_MAIN  /* keep using the OSA interface  */
    (void)OSA_TaskCreate((osa_task_handle_t)s_startTaskHandle, OSA_TASK(start_task), NULL);

    /*start scheduler*/
    OSA_Start();

#else  /* Example of baremetal loop if user doesn't want to use OSA API */

#if (FSL_OSA_BM_TIMER_CONFIG != FSL_OSA_BM_TIMER_NONE)
    /* OSA_Start() is not called in this implementation but nevertheless we need to start
      timers for the bare metal scheduler for delays, wait on timeout , etc.. */
    OSA_TimeInit();
#endif

    while(TRUE)
    {
        OSA_ProcessTasks();

        main_task(NULL);

        /* Before executing WFI, need to execute some connectivity background tasks
            (usually done in Idle thread) such as NVM save in Idle, etc.. */
        //BluetoothLEHost_ProcessIdleTask();

        OSA_DisableIRQGlobal();

        /* Check if some connectivity tasks have turned to ready state from interrupts or
              if messages are to be processed in Application process */
        if ( OSA_TaskShouldYield() == FALSE )
        {
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0)
            PWR_EnterLowPower();
#else
            __WFI();
#endif
        }

        OSA_EnableIRQGlobal();
    }
#endif

    /* Won't run here */
    assert(0);
    return 0;
}

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
