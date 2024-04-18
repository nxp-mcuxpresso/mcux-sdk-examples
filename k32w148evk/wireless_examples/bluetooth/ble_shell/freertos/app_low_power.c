/*! *********************************************************************************
* Copyright 2021-2023 NXP
*
* \file
*
* This is a source file for the common application low power and NVM code.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "EmbeddedTypes.h"
#include "fsl_os_abstraction.h"
#include "app_conn.h"

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d>0)
#include "PWR_Interface.h"
#endif

#include "fwk_platform_ble.h"
#include "fwk_freertos_utils.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
*\fn           void vApplicationIdleHook(void)
*\brief        Idle hook function which places the microcontroller into a power
*              saving mode.
*
*\param  [in]  none.
*
*\retval       void.
********************************************************************************** */
void vApplicationIdleHook(void)
{
    /* call some background tasks required by connectivity */
#if ((gAppUseNvm_d) || \
    (defined gAppOtaASyncFlashTransactions_c && (gAppOtaASyncFlashTransactions_c > 0)))

    /* Use a specific tick compensation mechanism implemented in the Connectivity Framework for FreeRTOS idle hook
     * This function, in pair with FWK_PostIdleHookTickCompensation, will measure the time taken during this idle
     * hook and estimate the ticks missed by the kernel by comparing the TickCount and the time elapsed using SOC
     * timers. This is useful when performing operations that block the system for more than 1 tick. */
    FWK_PreIdleHookTickCompensation();

    OSA_DisableIRQGlobal();

    if (PLATFORM_CheckNextBleConnectivityActivity() == true)
    {
        BluetoothLEHost_ProcessIdleTask();
    }

    OSA_EnableIRQGlobal();

    FWK_PostIdleHookTickCompensation();
#endif
}

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d>0) && defined(SDK_OS_FREE_RTOS)

#define TICK_TO_US(tick) (uint64_t)((uint64_t)tick * (uint64_t)portTICK_PERIOD_MS * (uint64_t)1000)
#define US_TO_TICK(us)   (TickType_t)((uint64_t)us / ((uint64_t)portTICK_PERIOD_MS * (uint64_t)1000U))


/*! *********************************************************************************
*\private
*\fn           void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
*\brief        This function will try to put the MCU into a deep sleep mode for at
*              most the maximum OS idle time specified. Else the MCU will enter a
*              sleep mode until the first IRQ.
*
*\param  [in]  xExpectedIdleTime    The idle time in OS ticks.
*
*\retval       none.
*
*\remarks      This feature is available only for FreeRTOS.
********************************************************************************** */
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    if (BluetoothLEHost_IsConnectivityTaskToProcess() == FALSE)
    {
        bool abortIdle = false;
        uint64_t actualIdleTimeUs = 0U, expectedIdleTimeUs = 0U;

        /* The OSA_InterruptDisable() API will prevent us to wakeup so we use
         * OSA_DisableIRQGlobal() */
        OSA_DisableIRQGlobal();

        /* Disable and prepare systicks for low power */
        abortIdle = PWR_SysticksPreProcess((uint32_t)xExpectedIdleTime, &expectedIdleTimeUs);

        if (abortIdle == false)
        {
                /* Enter low power with a maximal timeout */
                actualIdleTimeUs = PWR_EnterLowPower(expectedIdleTimeUs);

                /* Re enable systicks and compensate systick timebase */
                PWR_SysticksPostProcess(expectedIdleTimeUs, actualIdleTimeUs);
        }

        /* Exit from critical section */
        OSA_EnableIRQGlobal();
    }
}
#endif
