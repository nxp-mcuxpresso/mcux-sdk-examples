/* -------------------------------------------------------------------------- */
/*                           Copyright 2020-2024 NXP                          */
/*                            All rights reserved.                            */
/*                    SPDX-License-Identifier: BSD-3-Clause                   */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "board_platform.h"
#include "board.h"
#include "board_lp.h"
#include "fsl_spc.h"
#include "fsl_port.h"
#include "fwk_platform.h"
#include "fwk_platform_lowpower.h"
#include "fsl_component_timer_manager.h"
#include "fsl_pm_core.h"
#include "fwk_debug.h"
#include "SecLib.h"
#include "RNG_Interface.h"

#if ((defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)) ||               \
     (defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)) || \
     (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0)))
/* board_comp.h only required for above services   */
#include "board_comp.h"
#include "app.h"
#endif

#if defined(gAppUseSensors_d) && (gAppUseSensors_d > 0)
#include "fwk_platform_sensors.h"
#endif

#if (defined(gDebugConsoleEnable_d) && (gDebugConsoleEnable_d == 1))
#include "fsl_debug_console.h"
#endif

/* -------------------------------------------------------------------------- */
/*                             Private prototypes                             */
/* -------------------------------------------------------------------------- */

/*!
 * \brief Callback registered to SDK Power Manager to get notified of entry/exit of low power modes
 *
 * \param[in] eventType event specifying if we entered or exited from low power mode
 * \param[in] powerState low power mode used during low power period
 * \param[in] data Optional data passed when the callback got registered (not used currently)
 * \return status_t
 */
static status_t BOARD_LowpowerCb(pm_event_type_t eventType, uint8_t powerState, void *data);

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

static pm_notify_element_t boardLpNotifyGroup = {
    .notifyCallback = &BOARD_LowpowerCb,
    .data           = NULL,
};

#if defined(gBoard_ManageSwdPinsInLowPower_d) && (gBoard_ManageSwdPinsInLowPower_d > 0)
/* Variables to store the PCR register value for SWD_DIO and SWD_CLK pins */
static uint32_t mSWDIO_PCR_Save;
static uint32_t mSWDCLK_PCR_Save;
#endif

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

#if defined(gBoard_ManageSwdPinsInLowPower_d) && (gBoard_ManageSwdPinsInLowPower_d > 0)
static void BOARD_SetSWDPinsLowPower(bool_t isLowPower)
{
    bool_t   clock_portA        = FALSE;
    uint32_t clock_porta_config = (*(volatile uint32_t *)kCLOCK_PortA);

    /* Activate PORTA clock if disabled */
    if ((clock_porta_config & MRCC_CC_MASK) == 0U)
    {
        clock_portA = TRUE;
        CLOCK_EnableClock(kCLOCK_PortA);
    }

    if (isLowPower)
    {
        /* Store SWDIO PCR value */
        mSWDIO_PCR_Save = PORTA->PCR[0];
        /* Store SWDCLK PCR value */
        mSWDCLK_PCR_Save = PORTA->PCR[1];
        /* Disable SWDIO pin */
        PORT_SetPinMux(PORTA, 0, kPORT_PinDisabledOrAnalog);
        /* Disable SWDCLK pin */
        PORT_SetPinMux(PORTA, 1, kPORT_PinDisabledOrAnalog);
    }
    else
    {
        /* Enable SWDIO pin */
        PORTA->PCR[0] = mSWDIO_PCR_Save;
        /* Disable SWDIO pin */
        PORTA->PCR[1] = mSWDCLK_PCR_Save;
    }

    /* Restore PORTA clock settings */
    if (clock_portA)
    {
        *(volatile uint32_t *)kCLOCK_PortA = clock_porta_config;
    }
}
#endif

/*!
 * @brief Basic lowpower entry call to flush serial transaction and disable peripherals pin to avoid leakage in lowpower
 *    typically called from wakeup from deep sleep
 */
static void BOARD_EnterLowPowerCb(void)
{
    uint8_t bank_mask;

#if defined(gAppUseSensors_d) && (gAppUseSensors_d > 0)
    PLATFORM_SaveAdcContext();
    PLATFORM_DeinitAdc();
#endif

#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
    while (BOARD_IsAppConsoleBusy() == true)
    {
        ;
    }
#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 1)
    while (BOARD_IsApp2ConsoleBusy() == true)
    {
        ;
    }
#endif /* defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 1) */
#endif /* defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0) */

#if defined(gDebugConsoleEnable_d) && (gDebugConsoleEnable_d == 1)
    while (BOARD_IsDebugConsoleBusy() == true)
    {
        ;
    }
#endif

#if defined(BOARD_DBG_SWO_PIN_ENABLE) && (BOARD_DBG_SWO_PIN_ENABLE != 0)
    /* Prevents leakage in lowpower caused by SWO pins in pull down mode if previously enabled in BOARD_InitPins()
     */
    BOARD_DeInitSWO();
#endif

    /* Notify TimerManager the system is going to low power
     * It will make sure to sync its timebase and program the hardware timers accordingly */
    TM_EnterLowpower();

    /* In order to optimize the consumption in lowpower we do not retain all RAM banks.
     * We obtain the banks that need to be retained by the application thanks to
     * PLATFORM_GetDefaultRamBanksRetained but this function is linker script dependant.
     * If you change the linker script you can implement you GetRamBanksRetained or give
     * directly a mask to PLATFORM_SetRamBanksRetained.
     * Do this at the end of the callback to avoid doing another allocation after it */
    bank_mask = PLATFORM_GetDefaultRamBanksRetained();
    PLATFORM_SetRamBanksRetained(bank_mask);
}

static void BOARD_EnterPowerDownCb(void)
{
    return;
}

/*!
 * @brief Basic lowpower exit callback to reinitialize clock and pin mux configuration,
 *    typically called from wakeup from deep sleep and other lowest power mode
 */
static void BOARD_ExitLowPowerCb(void)
{
    /* Notify TimerManager the system is exiting low power
     * This will resync its timebase and schedule the TimerManager task */
    TM_ExitLowpower();

#if defined(BOARD_DBG_SWO_PIN_ENABLE) && (BOARD_DBG_SWO_PIN_ENABLE != 0)
    /* Enable back SWO that has been disabled to prevent leakage in lowpower */
    BOARD_InitSWO();
#endif

#if defined(gAppUseSensors_d) && (gAppUseSensors_d > 0)
    PLATFORM_RestoreAdcContext();
#endif

#if defined(DBG_PWR) && (DBG_PWR == 1)
    /* Debug purpose only */
    static int nb = 0;
    PWR_DBG_LOG("%d", nb++);
#endif
    return;
}

/*!
 * \brief This function called after exiting Power Down mode. It should be used to
 *        restore peripherals used by the application that need specific restore
 *        procedure such as LPUART1, etc..
 */
static void BOARD_ExitPowerDownCb(void)
{
    /* Allow the the cryptographic HW acceleration to reinitialize next time we will need it
     * The reinit of the S200 will work only if the S200 really went to power down */
    SecLib_ReInit();
    /* Allow the the RNG HW accelerator reinitialization.Required for S200 RNG. */
    (void)RNG_ReInit();

#if defined(gAppHighSystemClockFrequency_d) && (gAppHighSystemClockFrequency_d > 0)
    /* Set Core frequency to 96Mhz , core voltage to 1.1v */
    BOARD_BootClockHSRUN();
#else
    /* Set Core frequency to 48Mhz , core voltage to 1.0v */
    BOARD_BootClockRUN();
#endif

#if defined(gDebugConsoleEnable_d) && (gDebugConsoleEnable_d == 1)
    BOARD_ReinitDebugConsole();
#endif

#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
#if (DEFAULT_APP_UART == 1)
    /* Re enable properly UART1, this peripheral exits low power */
    BOARD_ReinitSerialManager((serial_handle_t)gSerMgrIf);
#else

    /* UART 0 is located in wakeup domain and remains initialized in low power mode to allow wakeup */
#if !defined(gAppLpuart0WakeUpSourceEnable_d) || (gAppLpuart0WakeUpSourceEnable_d == 0)
    BOARD_ReinitSerialManager((serial_handle_t)gSerMgrIf);
#endif /* defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0) */
#endif /* (DEFAULT_APP_UART==1) */
#endif /* !defined(gAppLpuart0WakeUpSourceEnable_d) || (gAppLpuart0WakeUpSourceEnable_d==0) */

#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 1)
#if !defined(gAppLpuart0WakeUpSourceEnable_d) || (gAppLpuart0WakeUpSourceEnable_d == 0) || (DEFAULT_APP_UART == 0)
    BOARD_ReinitSerialManager2((serial_handle_t)gSerMgrIf2);
#endif /* defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 1) */
#endif /* !defined(gAppLpuart0WakeUpSourceEnable_d) || (gAppLpuart0WakeUpSourceEnable_d==0) || (DEFAULT_APP_UART==0)*/

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    /* The button shall be reinitialized after the Serial Manager because button callbacks of the application may call
     * the Serial Manager */
    BOARD_Button0ExitPowerDown((button_handle_t)g_buttonHandle[0]);
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
    BOARD_Button1ExitPowerDown((button_handle_t)g_buttonHandle[1]);
#endif
#endif
}

/* -------------------------------------------------------------------------- */
/*                              Lowpower callbacks                             */
/* -------------------------------------------------------------------------- */

static status_t BOARD_LowpowerCb(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    status_t ret = kStatus_Success;
    if (powerState < PLATFORM_DEEP_SLEEP_STATE)
    {
        /* Nothing to do when entering WFI or Sleep low power state
            NVIC fully functionnal to trigger upcoming interrupts */
    }
    else
    {
        if (eventType == kPM_EventEnteringSleep)
        {
            BOARD_EnterLowPowerCb();

            if (powerState >= PLATFORM_POWER_DOWN_STATE)
            {
                /* Power gated low power modes often require extra specific
                 * entry/exit low power procedures, those should be implemented
                 * in the following BOARD API */
                BOARD_EnterPowerDownCb();
            }
        }
        else
        {
            /* Check if Main power domain domain really went to Power down,
             *   powerState variable is just an indication, Lowpower mode could have been skipped by an immediate wakeup
             */
            PLATFORM_PowerDomainState_t main_pd_state = PLATFORM_NO_LOWPOWER;
            PLATFORM_status_t           status;

            status = PLATFORM_GetLowpowerMode(PLATFORM_MainDomain, &main_pd_state);
            assert(status == PLATFORM_Successful);
            (void)status;

            if (main_pd_state == PLATFORM_POWER_DOWN_MODE)
            {
                /* Process wake up from power down mode on Main domain
                 *  Note that Wake up domain has not been in power down mode */
                BOARD_ExitPowerDownCb();
            }

            BOARD_ExitLowPowerCb();
        }
    }
    return ret;
}

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void BOARD_LowPowerInit(void)
{
    status_t status;

    status = PM_RegisterNotify(kPM_NotifyGroup2, &boardLpNotifyGroup);
    assert(status == kStatus_Success);
    (void)status;
}
