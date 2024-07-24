/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_cmc.h"
#include "fsl_spc.h"
#include "fsl_clock.h"
#include "fsl_debug_console.h"
#include "power_mode_switch.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpuart.h"
#include "fsl_waketimer.h"
#include "fsl_wuu.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_POWER_MODE_NAME                                          \
    {                                                                \
        "Active", "Sleep", "DeepSleep", "PowerDown", "DeepPowerDown" \
    }

#define APP_POWER_MODE_DESC                                                                                           \
    {                                                                                                                 \
        "Acitve: Core clock is 96MHz.", "Sleep: CPU clock is off, and the system clock and bus clock remain ON. ",    \
            "Deep Sleep: Core/System/Bus clock are gated off. ",                                                      \
            "Power Down: Core/System/Bus clock are gated off, CORE_MAIN is in static state, Flash memory is powered " \
            "off.",                                                                                                   \
            "Deep Power Down: The whole VDD_CORE voltage domain is power gated."                                      \
    }

#define APP_CMC           CMC
#define APP_RAM_ARRAYS_DS (0x3F0077FE)
#define APP_RAM_ARRAYS_PD (0x3F0077FE)

#define APP_SPC                   SPC0
#define APP_SPC_ISO_VALUE         (0x6U) /* VDD_USB, VDD_P2. */
#define APP_SPC_ISO_DOMAINS       "VDD_USB, VDD_P2"
#define APP_SPC_MAIN_POWER_DOMAIN (kSPC_PowerDomain0)

#define APP_WUU             WUU0
#define APP_WUU_WAKEUP_IDX  2U
#define APP_WUU_IRQN        WUU0_IRQn
#define APP_WUU_IRQ_HANDLER WUU0_IRQHandler

/* LPUART RX */
#define APP_DEBUG_CONSOLE_RX_PORT   PORT0
#define APP_DEBUG_CONSOLE_RX_GPIO   GPIO0
#define APP_DEBUG_CONSOLE_RX_PIN    2U
#define APP_DEBUG_CONSOLE_RX_PINMUX kPORT_MuxAlt2
/* LPUART TX */
#define APP_DEBUG_CONSOLE_TX_PORT   PORT0
#define APP_DEBUG_CONSOLE_TX_GPIO   GPIO0
#define APP_DEBUG_CONSOLE_TX_PIN    3U
#define APP_DEBUG_CONSOLE_TX_PINMUX kPORT_MuxAlt2


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_InitDebugConsole(void);
void APP_DeinitDebugConsole(void);
static void APP_SetSPCConfiguration(void);
static void APP_SetVBATConfiguration(void);
static void APP_SetCMCConfiguration(void);
static void APP_InitWaketimer(void);

static uint8_t APP_GetWakeupTimeout(void);
static void APP_GetWakeupConfig(app_power_mode_t targetMode);

static void APP_PowerPreSwitchHook(void);
static void APP_PowerPostSwitchHook(void);

static void APP_EnterSleepMode(void);
static void APP_EnterDeepSleepMode(void);
static void APP_EnterPowerDownMode(void);
static void APP_EnterDeepPowerDownMode(void);
static void APP_PowerModeSwitch(app_power_mode_t targetPowerMode);
static app_power_mode_t APP_GetTargetPowerMode(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

char *const g_modeNameArray[] = APP_POWER_MODE_NAME;
char *const g_modeDescArray[] = APP_POWER_MODE_DESC;

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_InitDebugConsole(void)
{
    /*
     * Debug console RX pin is set to disable for current leakage, need to re-configure pinmux.
     * Debug console TX pin: Don't need to change.
     */
    BOARD_InitPins();
    BOARD_BootClockFRO48M();
    BOARD_InitDebugConsole();
}

void APP_DeinitDebugConsole(void)
{
    DbgConsole_Deinit();
    PORT_SetPinMux(APP_DEBUG_CONSOLE_RX_PORT, APP_DEBUG_CONSOLE_RX_PIN, kPORT_MuxAsGpio);
    PORT_SetPinMux(APP_DEBUG_CONSOLE_TX_PORT, APP_DEBUG_CONSOLE_TX_PIN, kPORT_MuxAsGpio);
}

void APP_WUU_IRQ_HANDLER(void)
{
    PRINTF("\r\nWUU ISR.\r\n");
}



int main(void)
{
    uint32_t freq;
    app_power_mode_t targetPowerMode;
    bool needSetWakeup = false;

    CLOCK_SetupFRO16KClocking(kCLKE_16K_SYSTEM | kCLKE_16K_COREMAIN);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Release the I/O pads and certain peripherals to normal run mode state, for in Power Down mode
     * they will be in a latched state. */
    if ((CMC_GetSystemResetStatus(APP_CMC) & kCMC_WakeUpReset) != 0UL)
    {
        SPC_ClearPeriphIOIsolationFlag(APP_SPC);
    }

    APP_SetVBATConfiguration();
    APP_SetSPCConfiguration();
    APP_InitWaketimer();

    EnableIRQ(APP_WUU_IRQN);
    EnableIRQ(WAKETIMER0_IRQn);

    PRINTF("\r\nNormal Boot.\r\n");

    while (1)
    {
        if ((CMC_GetSystemResetStatus(APP_CMC) & kCMC_WakeUpReset) != 0UL)
        {
            /* Close ISO flags. */
            SPC_ClearPeriphIOIsolationFlag(APP_SPC);
        }

        /* Clear CORE_MAIN power domain's low power request flag. */
        SPC_ClearPowerDomainLowPowerRequestFlag(APP_SPC, APP_SPC_MAIN_POWER_DOMAIN);
        SPC_ClearLowPowerRequest(APP_SPC);

        /* Normal start. */
        APP_SetCMCConfiguration();

        freq = CLOCK_GetFreq(kCLOCK_CoreSysClk);
        PRINTF("\r\n###########################    Power Mode Switch Demo    ###########################\r\n");
        PRINTF("    Core Clock = %dHz \r\n", freq);
        PRINTF("    Power mode: Active\r\n");
        targetPowerMode = APP_GetTargetPowerMode();

        if ((targetPowerMode > kAPP_PowerModeMin) && (targetPowerMode < kAPP_PowerModeMax))
        {
            /* If target mode is Active mode, don't need to set wakeup source. */
            if (targetPowerMode == kAPP_PowerModeActive)
            {
                needSetWakeup = false;
            }
            else
            {
                needSetWakeup = true;
            }
        }

        /* Print description of selected power mode. */
        PRINTF("\r\n");
        if (needSetWakeup)
        {
            APP_GetWakeupConfig(targetPowerMode);
            APP_PowerPreSwitchHook();
            APP_PowerModeSwitch(targetPowerMode);
            APP_PowerPostSwitchHook();
        }

        PRINTF("\r\nNext loop.\r\n");
    }
}

static void APP_InitWaketimer(void)
{
    waketimer_config_t waketimer_config;
    WAKETIMER_GetDefaultConfig(&waketimer_config);
    WAKETIMER_Init(WAKETIMER0, &waketimer_config);
}

static void APP_SetSPCConfiguration(void)
{
    status_t status;

    spc_active_mode_regulators_config_t activeModeRegulatorOption;

    SPC_EnableSRAMLdo(APP_SPC, true);

    /* Disable all modules that controlled by SPC in active mode.. */
    SPC_DisableActiveModeAnalogModules(APP_SPC, kSPC_controlAllModules);

    /* Disable LVDs and HVDs */
    SPC_EnableActiveModeCoreLowVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeSystemHighVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeSystemLowVoltageDetect(APP_SPC, false);

    while (SPC_GetBusyStatusFlag(APP_SPC))
        ;

    activeModeRegulatorOption.bandgapMode = kSPC_BandgapEnabledBufferDisabled;
    /* DCDC output voltage is 1.1V in active mode. */
    activeModeRegulatorOption.CoreLDOOption.CoreLDOVoltage       = kSPC_CoreLDO_MidDriveVoltage;
    activeModeRegulatorOption.CoreLDOOption.CoreLDODriveStrength = kSPC_CoreLDO_NormalDriveStrength;

    status = SPC_SetActiveModeRegulatorsConfig(APP_SPC, &activeModeRegulatorOption);
#if !(defined(FSL_FEATURE_MCX_SPC_HAS_NO_GLITCH_DETECT) && FSL_FEATURE_MCX_SPC_HAS_NO_GLITCH_DETECT)
    /* Disable Vdd Core Glitch detector in active mode. */
    SPC_DisableActiveModeVddCoreGlitchDetect(APP_SPC, true);
#endif
    if (status != kStatus_Success)
    {
        PRINTF("Fail to set regulators in Active mode.");
        return;
    }
    while (SPC_GetBusyStatusFlag(APP_SPC))
        ;

    SPC_DisableLowPowerModeAnalogModules(APP_SPC, kSPC_controlAllModules);
    SPC_SetLowPowerWakeUpDelay(APP_SPC, 0xFF);
    spc_lowpower_mode_regulators_config_t lowPowerRegulatorOption;

    lowPowerRegulatorOption.lpIREF                             = false;
    lowPowerRegulatorOption.bandgapMode                        = kSPC_BandgapDisabled;
    lowPowerRegulatorOption.CoreLDOOption.CoreLDOVoltage       = kSPC_CoreLDO_MidDriveVoltage;
    lowPowerRegulatorOption.CoreLDOOption.CoreLDODriveStrength = kSPC_CoreLDO_LowDriveStrength;

    status = SPC_SetLowPowerModeRegulatorsConfig(APP_SPC, &lowPowerRegulatorOption);
#if !(defined(FSL_FEATURE_MCX_SPC_HAS_NO_GLITCH_DETECT) && FSL_FEATURE_MCX_SPC_HAS_NO_GLITCH_DETECT)
    /* Disable Vdd Core Glitch detector in low power mode. */
    SPC_DisableLowPowerModeVddCoreGlitchDetect(APP_SPC, true);
#endif
    if (status != kStatus_Success)
    {
        PRINTF("Fail to set regulators in Low Power Mode.");
        return;
    }
    while (SPC_GetBusyStatusFlag(APP_SPC))
        ;

    /* Enable Low power request output to observe the entry/exit of
     * low power modes(including: deep sleep mode, power down mode, and deep power down mode).
     */
    spc_lowpower_request_config_t lpReqConfig = {
        .enable   = true,
        .override = kSPC_LowPowerRequestNotForced,
        .polarity = kSPC_LowTruePolarity,
    };

    SPC_SetLowPowerRequestConfig(APP_SPC, &lpReqConfig);
}

static void APP_SetVBATConfiguration(void)
{
    CLOCK_SetupFRO16KClocking(kCLKE_16K_SYSTEM | kCLKE_16K_COREMAIN);
}

static void APP_SetCMCConfiguration(void)
{
    /* Disable low power debug. */
    CMC_EnableDebugOperation(APP_CMC, false);
    /* Allow all power mode */
    CMC_SetPowerModeProtection(APP_CMC, kCMC_AllowAllLowPowerModes);

    /* Disable flash memory accesses and place flash memory in low-power state whenever the core clock
       is gated. And an attempt to access the flash memory will cause the flash memory to exit low-power
       state for the duration of the flash memory access. */
    CMC_ConfigFlashMode(APP_CMC, true, true, false);
}

static app_power_mode_t APP_GetTargetPowerMode(void)
{
    uint8_t ch;

    app_power_mode_t inputPowerMode;

    do
    {
        PRINTF("\r\nSelect the desired operation \n\r\n");
        for (app_power_mode_t modeIndex = kAPP_PowerModeActive; modeIndex <= kAPP_PowerModeDeepPowerDown; modeIndex++)
        {
            PRINTF("\tPress %c to enter: %s mode\r\n", modeIndex,
                   g_modeNameArray[(uint8_t)(modeIndex - kAPP_PowerModeActive)]);
        }

        PRINTF("\r\nWaiting for power mode select...\r\n\r\n");

        ch = GETCHAR();

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        inputPowerMode = (app_power_mode_t)ch;

        if ((inputPowerMode > kAPP_PowerModeDeepPowerDown) || (inputPowerMode < kAPP_PowerModeActive))
        {
            PRINTF("Wrong Input!");
        }
    } while (inputPowerMode > kAPP_PowerModeDeepPowerDown);

    PRINTF("\t%s\r\n", g_modeDescArray[(uint8_t)(inputPowerMode - kAPP_PowerModeActive)]);

    return inputPowerMode;
}

/* Get wakeup timeout and wakeup source. */
static void APP_GetWakeupConfig(app_power_mode_t targetMode)
{
    uint8_t timeOutValue;
    char *isoDomains = NULL;

    timeOutValue = APP_GetWakeupTimeout();
    PRINTF("Will wakeup in %d seconds.\r\n", timeOutValue);
    /* In case of target mode is powerdown/deep power down mode. */
    WUU_SetInternalWakeUpModulesConfig(APP_WUU, APP_WUU_WAKEUP_IDX, kWUU_InternalModuleInterrupt);
    WAKETIMER_StartTimer(WAKETIMER0, timeOutValue * 1000);

    if (targetMode > kAPP_PowerModeSleep)
    {
        /* Isolate some power domains that are not used in low power modes.*/
        SPC_SetExternalVoltageDomainsConfig(APP_SPC, APP_SPC_ISO_VALUE, 0x0U);
        isoDomains = APP_SPC_ISO_DOMAINS;
        PRINTF("Isolate power domains: %s\r\n", isoDomains);
    }
}

/*!
 * Get input from user about wakeup timeout
 */
static uint8_t APP_GetWakeupTimeout(void)
{
    uint8_t timeout;

    while (1)
    {
        PRINTF("Select the wake up timeout in seconds.\r\n");
        PRINTF("The allowed range is 1s ~ 9s.\r\n");
        PRINTF("Eg. enter 5 to wake up in 5 seconds.\r\n");
        PRINTF("\r\nWaiting for input timeout value...\r\n\r\n");

        timeout = GETCHAR();
        PRINTF("%c\r\n", timeout);
        if ((timeout > '0') && (timeout <= '9'))
        {
            return timeout - '0';
        }
        PRINTF("Wrong value!\r\n");
    }
}

static void APP_PowerPreSwitchHook(void)
{
    /* Wait for debug console output finished. */
    while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
    {
    }
    APP_DeinitDebugConsole();
}

static void APP_PowerPostSwitchHook(void)
{
    BOARD_BootClockFRO48M();
    APP_InitDebugConsole();
}

static void APP_PowerModeSwitch(app_power_mode_t targetPowerMode)
{
    if (targetPowerMode != kAPP_PowerModeActive)
    {
        switch (targetPowerMode)
        {
            case kAPP_PowerModeSleep:
                APP_EnterSleepMode();
                break;
            case kAPP_PowerModeDeepSleep:
                APP_EnterDeepSleepMode();
                break;
            case kAPP_PowerModePowerDown:
                APP_EnterPowerDownMode();
                break;
            case kAPP_PowerModeDeepPowerDown:
                APP_EnterDeepPowerDownMode();
                break;
            default:
                assert(false);
                break;
        }
    }
}

static void APP_EnterSleepMode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateCoreClock;
    config.main_domain = kCMC_ActiveOrSleepMode;

    CMC_EnterLowPowerMode(APP_CMC, &config);
}

static void APP_EnterDeepSleepMode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepSleepMode;

    CMC_EnterLowPowerMode(APP_CMC, &config);
}

static void APP_EnterPowerDownMode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_PowerDownMode;

    CMC_EnterLowPowerMode(APP_CMC, &config);
}

static void APP_EnterDeepPowerDownMode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepPowerDown;

    CMC_EnterLowPowerMode(APP_CMC, &config);
}
