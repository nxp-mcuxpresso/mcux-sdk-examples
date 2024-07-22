/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_cmc.h"
#include "fsl_lptmr.h"
#include "fsl_lpuart.h"
#include "fsl_port.h"
#include "fsl_spc.h"
#include "fsl_vbat.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_POWER_NAME                                                     \
    {                                                                      \
        "Sleep", "Deep Sleep", "Power Down", "Deep Power Down" \
    }
#define APP_TARGET_POWER_NUM (4U)

#define APP_SLEEP_CONSTRAINTS                                                                                   \
    19U, PM_RESC_RAMA0_8K_ACTIVE, PM_RESC_RAMA1_8K_ACTIVE, PM_RESC_RAMA2_8K_ACTIVE, PM_RESC_RAMA3_8K_ACTIVE,    \
        PM_RESC_RAMX0_32K_ACTIVE, PM_RESC_RAMX1_32K_ACTIVE, PM_RESC_RAMX2_32K_ACTIVE, PM_RESC_RAMB0_32K_ACTIVE, \
        PM_RESC_RAMC0_32K_ACTIVE, PM_RESC_RAMC1_32K_ACTIVE, PM_RESC_RAMD0_32K_ACTIVE, PM_RESC_RAMD1_32K_ACTIVE, \
        PM_RESC_RAME0_32K_ACTIVE, PM_RESC_RAME1_32K_ACTIVE, PM_RESC_RAMF0_32K_ACTIVE, PM_RESC_RAMF1_32K_ACTIVE, \
        PM_RESC_RAMG01_32K_ACTIVE, PM_RESC_RAMG23_32K_ACTIVE, PM_RESC_RAMH01_32K_ACTIVE

#define APP_DEEP_SLEEP_CONSTRAINTS                                                            \
    19U, PM_RESC_RAMA0_8K_RETENTION, PM_RESC_RAMA1_8K_RETENTION, PM_RESC_RAMA2_8K_RETENTION,  \
        PM_RESC_RAMA3_8K_RETENTION, PM_RESC_RAMX0_32K_RETAINED, PM_RESC_RAMX1_32K_RETAINED,   \
        PM_RESC_RAMX2_32K_RETAINED, PM_RESC_RAMB0_32K_RETAINED, PM_RESC_RAMC0_32K_RETAINED,   \
        PM_RESC_RAMC1_32K_RETAINED, PM_RESC_RAMD0_32K_RETAINED, PM_RESC_RAMD1_32K_RETAINED,   \
        PM_RESC_RAME0_32K_RETAINED, PM_RESC_RAME1_32K_RETAINED, PM_RESC_RAMF0_32K_RETAINED,   \
        PM_RESC_RAMF1_32K_RETAINED, PM_RESC_RAMG01_32K_RETAINED, PM_RESC_RAMG23_32K_RETAINED, \
        PM_RESC_RAMH01_32K_RETAINED

#define APP_POWER_DOWN_CONSTRAINTS                                                            \
    19U, PM_RESC_RAMA0_8K_RETENTION, PM_RESC_RAMA1_8K_RETENTION, PM_RESC_RAMA2_8K_RETENTION,  \
        PM_RESC_RAMA3_8K_RETENTION, PM_RESC_RAMX0_32K_RETAINED, PM_RESC_RAMX1_32K_RETAINED,   \
        PM_RESC_RAMX2_32K_RETAINED, PM_RESC_RAMB0_32K_RETAINED, PM_RESC_RAMC0_32K_RETAINED,   \
        PM_RESC_RAMC1_32K_RETAINED, PM_RESC_RAMD0_32K_RETAINED, PM_RESC_RAMD1_32K_RETAINED,   \
        PM_RESC_RAME0_32K_RETAINED, PM_RESC_RAME1_32K_RETAINED, PM_RESC_RAMF0_32K_RETAINED,   \
        PM_RESC_RAMF1_32K_RETAINED, PM_RESC_RAMG01_32K_RETAINED, PM_RESC_RAMG23_32K_RETAINED, \
        PM_RESC_RAMH01_32K_RETAINED

#define APP_DEEP_POWER_DOWN_CONSTRAINTS \
    4U, PM_RESC_RAMA0_8K_RETENTION, PM_RESC_RAMA1_8K_RETENTION, PM_RESC_RAMA2_8K_RETENTION, PM_RESC_RAMA3_8K_RETENTION


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_InitWakeupSource(void);
void APP_Lptmr0WakeupService(void);
void APP_StartLptmr(uint64_t timeOutTickes);
void APP_StopLptmr(void);
uint32_t APP_GetWakeupTimeout(void);
void APP_RegisterNotify(void);
void APP_SetConstraints(uint8_t powerMode);
void APP_ReleaseConstraints(uint8_t powerMode);

static uint8_t APP_GetTargetPowerMode(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern pm_handle_t g_pmHandle;
AT_ALWAYS_ON_DATA(pm_handle_t g_pmHandle);
AT_ALWAYS_ON_DATA(uint8_t g_targetPowerMode);
AT_ALWAYS_ON_DATA(uint32_t g_irqMask);
static const char *const g_targetPowerNameArray[APP_TARGET_POWER_NUM] = APP_POWER_NAME;

/*******************************************************************************
 * Code
 ******************************************************************************/
AT_ALWAYS_ON_DATA(pm_wakeup_source_t g_lptmr0WakeupSource);


static status_t APP_UartControlCallback(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (eventType == kPM_EventEnteringSleep)
    {
        /* De-init uart */
        PRINTF("\r\n De-init UART.");
        /* Wait for debug console output finished. */
        while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
        {
        }
        DbgConsole_Deinit();
        CLOCK_DisableClock(kCLOCK_Port1);
    }
    else
    {
        /* Re-init uart. */
        BOARD_InitPins();
        BOARD_BootClockFROHF48M();
        BOARD_InitDebugConsole();
        PRINTF("\r\n Re-init UART.");
    }

    return kStatus_Success;
}

static status_t APP_VBATControlCallback(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (eventType == kPM_EventEnteringSleep)
    {
        /* Enable sram regulator to supply the RAMA in low-power mode, and enable FRO 16k to clock LPTMR. */
        PRINTF("\r\n Enable VBAT.");
        VBAT_EnableFRO16k(VBAT0, true);
        VBAT_UngateFRO16k(VBAT0, kVBAT_EnableClockToVddSys);
        if (kStatus_Success != VBAT_EnableBandgap(VBAT0, true))
        {
            assert(false);
            PRINTF("\r\n VBAT bandgap enable failed.");
        }
        VBAT_EnableBandgapRefreshMode(VBAT0, true);
        if (kStatus_Success != VBAT_EnableBackupSRAMRegulator(VBAT0, true))
        {
            assert(false);
            PRINTF("\r\n VBAT SRAM LDO enable failed.");
        }
    }
    else
    {
        /* Disable VBAT internal components to save power. */
        if (kStatus_Success != VBAT_EnableBackupSRAMRegulator(VBAT0, false))
        {
            assert(false);
            PRINTF("\r\n VBAT SRAM LDO disable failed.");
        }
        VBAT_EnableBandgapRefreshMode(VBAT0, false);
        if (kStatus_Success != VBAT_EnableBandgap(VBAT0, false))
        {
            assert(false);
            PRINTF("\r\n VBAT bandgap disable failed.");
        }
        VBAT_GateFRO16k(VBAT0, kVBAT_EnableClockToVddSys);
        VBAT_EnableFRO16k(VBAT0, false);
        PRINTF("\r\n Disable VBAT.");
    }

    return kStatus_Success;
}

static status_t APP_EntryPowerModeInfoPrint(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (eventType == kPM_EventEnteringSleep)
    {
        if (powerState == 4U)
        {
            PRINTF("\r\n Please note that exiting from deep power down will cause wakeup reset.");
        }
    }

    return kStatus_Success;
}

AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t g_notify0) = {
    .notifyCallback = APP_EntryPowerModeInfoPrint,
    .data           = NULL,
};

AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t g_notify1) = {
    .notifyCallback = APP_VBATControlCallback,
    .data           = NULL,
};

AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t g_notify2) = {
    .notifyCallback = APP_UartControlCallback,
    .data           = NULL,
};

/*! @brief Wakeup source service function, should be executed if the corresponding wakeup event occurred. */
void APP_Lptmr0WakeupService(void)
{
    if (kLPTMR_TimerInterruptEnable & LPTMR_GetEnabledInterrupts(LPTMR0))
    {
        LPTMR_DisableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);
        LPTMR_ClearStatusFlags(LPTMR0, kLPTMR_TimerCompareFlag);
        LPTMR_StopTimer(LPTMR0);
    }
}

void LPTMR0_IRQHandler(void)
{
    PM_TriggerWakeSourceService(&g_lptmr0WakeupSource);
}

/*! @brief Start lptmr before entering low power mode. */
void APP_StartLptmr(uint64_t timeOutTickes)
{
    const lptmr_config_t DEMO_LPTMR_config = {.timerMode            = kLPTMR_TimerModeTimeCounter,
                                              .pinSelect            = kLPTMR_PinSelectInput_0,
                                              .pinPolarity          = kLPTMR_PinPolarityActiveHigh,
                                              .enableFreeRunning    = false,
                                              .bypassPrescaler      = true,
                                              .prescalerClockSource = kLPTMR_PrescalerClock_1,
                                              .value                = kLPTMR_Prescale_Glitch_0};

    CLOCK_SetupClk16KClocking(kCLOCK_Clk16KToVsys);
    LPTMR_Init(LPTMR0, &DEMO_LPTMR_config);
    LPTMR_SetTimerPeriod(LPTMR0, USEC_TO_COUNT(timeOutTickes, 16384UL));
    LPTMR_EnableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);
    LPTMR_StartTimer(LPTMR0);
}

/*! @brief stop low power timer after waking up from low power mode. */
void APP_StopLptmr(void)
{
    LPTMR_StopTimer(LPTMR0);
}

/*! @brief Init wakeuup source, insert it to the wakeup source list, if the wakeup source is the lptmr,
 *  also need to register the timer controller to the power manager handle.
 */
void APP_InitWakeupSource(void)
{
    PM_InitWakeupSource(&g_lptmr0WakeupSource, PM_WSID_LPTMR0, APP_Lptmr0WakeupService, true);
    PM_RegisterTimerController(&g_pmHandle, APP_StartLptmr, APP_StopLptmr, NULL, NULL);
}

uint32_t APP_GetWakeupTimeout(void)
{
    uint8_t timeout;
    uint32_t timeoutTicks;

    while (1)
    {
        PRINTF("\r\n Select the wake up timeout in seconds.");
        PRINTF("\r\n The allowed range is 1s - 9s.");
        PRINTF("\r\n Eg. enter 5 to wake up in 5 seconds.");
        PRINTF("\r\n Waiting for input timeout value...");

        timeout = GETCHAR();
        if ((timeout > '0') && (timeout <= '9'))
        {
            timeout -= '0';
            PRINTF("\r\n Will wakeup in %d seconds.", timeout);
            timeoutTicks = (timeout * 1000000UL);
            return timeoutTicks;
        }
        PRINTF("\r\n Wrong value!");
    }
}

void APP_RegisterNotify(void)
{
    if (kStatus_PMSuccess != PM_RegisterNotify(kPM_NotifyGroup0, &g_notify0))
    {
        assert(false);
        PRINTF("\r\n Register notify0 failed");
    }
    if (kStatus_PMSuccess != PM_RegisterNotify(kPM_NotifyGroup1, &g_notify1))
    {
        assert(false);
        PRINTF("\r\n Register notify1 failed");
    }
    if (kStatus_PMSuccess != PM_RegisterNotify(kPM_NotifyGroup2, &g_notify2))
    {
        assert(false);
        PRINTF("\r\n Register notify2 failed");
    }
}

void APP_SetConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case 0: /* sleep mode */
        {
            PM_SetConstraints(PM_LP_STATE_SLEEP, APP_SLEEP_CONSTRAINTS);
            break;
        }

        case 1: /* deep sleep mode */
        {
            PM_SetConstraints(PM_LP_STATE_DEEP_SLEEP, APP_DEEP_SLEEP_CONSTRAINTS);
            break;
        }

        case 2: /* power down mode */
        {
            PM_SetConstraints(PM_LP_STATE_POWER_DOWN, APP_POWER_DOWN_CONSTRAINTS);
            break;
        }

        case 3: /* deep power down mode */
        {
            PM_SetConstraints(PM_LP_STATE_DEEP_POWER_DOWN, APP_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }

        default:
        {
            /* This branch will never be hit. */
            assert(false);
            break;
        }
    }
}

void APP_ReleaseConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case 0: /* sleep mode */
        {
            PM_ReleaseConstraints(PM_LP_STATE_SLEEP, APP_SLEEP_CONSTRAINTS);
            break;
        }
        case 1: /* deep sleep mode */
        {
            PM_ReleaseConstraints(PM_LP_STATE_DEEP_SLEEP, APP_DEEP_SLEEP_CONSTRAINTS);
            break;
        }
        case 2: /* power down mode */
        {
            PM_ReleaseConstraints(PM_LP_STATE_POWER_DOWN, APP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        case 3: /* deep power down mode */
        {
            PM_ReleaseConstraints(PM_LP_STATE_DEEP_POWER_DOWN, APP_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        default:
        {
            /* This branch should never be hit. */
            assert(false);
            break;
        }
    }
}


int main(void)
{
    uint32_t timeoutUs = 0UL;

    BOARD_InitPins();
    BOARD_BootClockFROHF48M();
    BOARD_InitDebugConsole();

    /* Disable the flash cache LPCAC */
    SYSCON->LPCAC_CTRL |= SYSCON_LPCAC_CTRL_DIS_LPCAC_MASK;

    /* Disable Debugger in low-power modes. */
    CMC_EnableDebugOperation(CMC0, false);
    /*Disable Flash access while in low-power modes. */
    CMC_ConfigFlashMode(CMC0, true, false);
    /* Disable LDO_CORE regulator. */
    SPC_EnableCoreLDORegulator(SPC0, false);

    if ((CMC_GetSystemResetStatus(CMC0) & kCMC_WakeUpReset) != 0UL)
    {
        SPC_ClearPeriphIOIsolationFlag(SPC0);
    }
    PRINTF("\r\nPower Manager Test.\r\n");
    PRINTF("\r\nNormal Boot.\r\n");
    PM_CreateHandle(&g_pmHandle);

    APP_RegisterNotify();
    APP_InitWakeupSource();
    while (1)
    {
        g_targetPowerMode = APP_GetTargetPowerMode();
        if (g_targetPowerMode >= APP_TARGET_POWER_NUM)
        {
            PRINTF("\r\nWrong Input! Please reselect.\r\n");
            continue;
        }
        PRINTF("Selected to enter %s.\r\n", g_targetPowerNameArray[(uint8_t)g_targetPowerMode]);
        timeoutUs = APP_GetWakeupTimeout();
        APP_SetConstraints(g_targetPowerMode);
        g_irqMask = DisableGlobalIRQ();
        PM_EnablePowerManager(true);
        PM_EnterLowPower(timeoutUs);
        PM_EnablePowerManager(false);
        EnableGlobalIRQ(g_irqMask);
        APP_ReleaseConstraints(g_targetPowerMode);
        PRINTF("\r\nNext Loop\r\n");
    }
}

static uint8_t APP_GetTargetPowerMode(void)
{
    uint32_t i;
    uint8_t ch;
    uint8_t g_targetPowerModeIndex;

    PRINTF("\r\nPlease select the desired power mode:\r\n");
    for (i = 0UL; i < APP_TARGET_POWER_NUM; i++)
    {
        PRINTF("\tPress %c to enter: %s\r\n", ('A' + i), g_targetPowerNameArray[i]);
    }

    PRINTF("\r\nWaiting for power mode select...\r\n\r\n");

    ch = GETCHAR();

    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch -= 'a' - 'A';
    }

    g_targetPowerModeIndex = ch - 'A';

    return g_targetPowerModeIndex;
}
