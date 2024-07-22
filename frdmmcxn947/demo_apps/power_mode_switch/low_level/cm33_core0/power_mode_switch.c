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
#include "fsl_lptmr.h"
#include "fsl_wuu.h"
#include "fsl_vbat.h"
#include "fsl_gpio.h"
#include "fsl_cache_lpcac.h"
#include "fsl_port.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_POWER_MODE_NAME                                          \
    {                                                                \
        "Active", "Sleep", "DeepSleep", "PowerDown", "DeepPowerDown" \
    }

#define APP_POWER_MODE_DESC                                                                                         \
    {                                                                                                               \
        "Acitve: Core clock is 48MHz, power consumption is about 7.8 mA.  ",                                        \
            "Sleep: CPU0 clock is off, System and Bus clock remain ON, power consumption is about 5.85 mA. ",       \
            "Deep Sleep: Core/System/Bus clock are gated off. ",                                                    \
            "Power Down: Core/System/Bus clock are gated off, both CORE_MAIN and CORE_WAKE power domains are put "  \
            "into state retention mode.",                                                                           \
            "Deep Power Down: The whole VDD_CORE voltage domain is power gated. \r\nPlease note that exiting from " \
            "DPD mode will cause wakeup reset."                                                                     \
    }

#define APP_CMC           CMC0
#define APP_RAM_ARRAYS_DS (0x3F0077FE)
#define APP_RAM_ARRAYS_PD (0x3F0077FE)

#define APP_LPTMR             LPTMR0
#define APP_LPTMR_IRQN        LPTMR0_IRQn
#define APP_LPTMR_IRQ_HANDLER LPTMR0_IRQHandler
#define APP_LPTMR_CLK_SOURCE  (16000UL)

#define APP_SPC                           SPC0
#define APP_SPC_LPTMR_LPISO_VALUE         (0x1EU) /* VDD_USB, VDD_P2, VDD_P3, VDD_P4. */
#define APP_SPC_LPTMR_ISO_DOMAINS         "VDD_USB, VDD_P2, VDD_P3, VDD_P4"
#define APP_SPC_WAKEUP_BUTTON_LPISO_VALUE (0x3EU) /* VDD_USB, VDD_P2, VDD_P3, VDD_P4, VBAT. */
#define APP_SPC_WAKEUP_BUTTON_ISO_DOMAINS "VDD_USB, VDD_P2, VDD_P3, VDD_P4, VBAT."
#define APP_SPC_MAIN_POWER_DOMAIN         (kSPC_PowerDomain0)
#define APP_SPC_WAKE_POWER_DOMAIN         (kSPC_PowerDomain1)

#define APP_VBAT             VBAT0
#define APP_VBAT_IRQN        VBAT0_IRQn
#define APP_VBAT_IRQ_HANDLER VBAT0_IRQHandler

#define APP_WUU                    WUU0
#define APP_WUU_WAKEUP_LPTMR_IDX   6U
#define APP_WUU_WAKEUP_BUTTON_IDX  5U /* P0_23, SW2 on FRDM board. */
#define APP_WUU_IRQN               WUU_IRQn
#define APP_WUU_IRQ_HANDLER        WUU_IRQHandler
#define APP_WUU_WAKEUP_BUTTON_NAME "SW2"

#define APP_DEBUG_CONSOLE_RX_PORT   PORT1
#define APP_DEBUG_CONSOLE_RX_GPIO   GPIO1
#define APP_DEBUG_CONSOLE_RX_PIN    8U
#define APP_DEBUG_CONSOLE_RX_PINMUX kPORT_MuxAlt2

#define APP_DEBUG_CONSOLE_TX_PORT   PORT1
#define APP_DEBUG_CONSOLE_TX_GPIO   GPIO1
#define APP_DEBUG_CONSOLE_TX_PIN    9U
#define APP_DEBUG_CONSOLE_TX_PINMUX kPORT_MuxAlt2

#define APP_INVALIDATE_CACHE        L1CACHE_InvalidateCodeCache()


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_InitDebugConsole(void);
void APP_DeinitDebugConsole(void);
static void APP_SetSPCConfiguration(void);
static void APP_SetVBATConfiguration(void);
static void APP_SetCMCConfiguration(void);
static void APP_InitLptimer(void);

static uint8_t APP_GetWakeupTimeout(void);
static void APP_GetWakeupConfig(app_power_mode_t targetMode);
static app_wakeup_source_t APP_SelectWakeupSource(void);

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
    BOARD_BootClockFROHF48M();
    BOARD_InitDebugConsole();
}

void APP_DeinitDebugConsole(void)
{
    DbgConsole_Deinit();
    PORT_SetPinMux(APP_DEBUG_CONSOLE_RX_PORT, APP_DEBUG_CONSOLE_RX_PIN, kPORT_MuxAsGpio);
    PORT_SetPinMux(APP_DEBUG_CONSOLE_TX_PORT, APP_DEBUG_CONSOLE_TX_PIN, kPORT_MuxAsGpio);
}

void APP_LPTMR_IRQ_HANDLER(void)
{
    if (kLPTMR_TimerInterruptEnable & LPTMR_GetEnabledInterrupts(APP_LPTMR))
    {
        LPTMR_DisableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable);
        LPTMR_ClearStatusFlags(APP_LPTMR, kLPTMR_TimerCompareFlag);
        LPTMR_StopTimer(APP_LPTMR);
    }
}

void APP_VBAT_IRQ_HANDLER(void)
{
    if (VBAT_GetStatusFlags(APP_VBAT) & kVBAT_StatusFlagWakeupPin)
    {
        VBAT_ClearStatusFlags(APP_VBAT, kVBAT_StatusFlagWakeupPin);
    }
}

void APP_WUU_IRQ_HANDLER(void)
{
    if (WUU_GetExternalWakeUpPinsFlag(APP_WUU) == (1UL << (uint32_t)APP_WUU_WAKEUP_BUTTON_IDX))
    {
        /* Enter into WUU IRQ handler, 3 timess toggle. */
        WUU_ClearExternalWakeUpPinsFlag(APP_WUU, (1UL << (uint32_t)APP_WUU_WAKEUP_BUTTON_IDX));
    }
}


int main(void)
{
    uint32_t freq;
    app_power_mode_t targetPowerMode;
    bool needSetWakeup = false;

    BOARD_InitPins();
    BOARD_BootClockFROHF48M();
    BOARD_InitDebugConsole();

    /* Release the I/O pads and certain peripherals to normal run mode state, for in Power Down mode
     * they will be in a latched state. */
    if ((CMC_GetSystemResetStatus(APP_CMC) & kCMC_WakeUpReset) != 0UL)
    {
        SPC_ClearPeriphIOIsolationFlag(APP_SPC);
    }

    if ((CMC_GetSystemResetStatus(APP_CMC) & kCMC_WakeUpReset) == 0UL)
    {
        /* If wakeup from low power, following steps should be ignored. */
        APP_SetVBATConfiguration();
        APP_SetSPCConfiguration();
    }
        
    APP_InitLptimer();
    /* Enable LPTMR interrupt to handle wakeup event. */
    EnableIRQ(APP_LPTMR_IRQN);
#if !(defined(APP_NOT_SUPPORT_WAKEUP_BUTTON) && APP_NOT_SUPPORT_WAKEUP_BUTTON)
    /* Enable WUU interrupt because external pin trigger WUU ISR. */
    EnableIRQ(APP_WUU_IRQN);
#endif

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
        /* Clear CORE_WAKE power domain's low power request flag. */
        SPC_ClearPowerDomainLowPowerRequestFlag(APP_SPC, APP_SPC_WAKE_POWER_DOMAIN);
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

static void APP_InitLptimer(void)
{
    lptmr_config_t lptmr_config;
    LPTMR_GetDefaultConfig(&lptmr_config);
    /* Select clock source as FRO16K which is operatable in deep-sleep or deeper power modes. */
    lptmr_config.prescalerClockSource = kLPTMR_PrescalerClock_1;
    LPTMR_Init(LPTMR0, &lptmr_config);
}

static void APP_SetSPCConfiguration(void)
{
    status_t status;

    spc_active_mode_regulators_config_t activeModeRegulatorOption;

#if defined(APP_INVALIDATE_CACHE)
    APP_INVALIDATE_CACHE;
#endif /* defined(APP_INVALIDATE_CACHE) */

    /* Disable all modules that controlled by SPC in active mode. */
    SPC_DisableActiveModeAnalogModules(APP_SPC, kSPC_controlAllModules);

    /* Disable LVDs and HVDs */
    SPC_EnableActiveModeCoreHighVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeCoreLowVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeSystemHighVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeSystemLowVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeIOHighVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeIOLowVoltageDetect(APP_SPC, false);

    while(SPC_GetBusyStatusFlag(APP_SPC))
        ;

    activeModeRegulatorOption.bandgapMode = kSPC_BandgapEnabledBufferDisabled;
    activeModeRegulatorOption.lpBuff      = false;
    /* DCDC output voltage is 1.1V in active mode. */
    activeModeRegulatorOption.DCDCOption.DCDCVoltage           = kSPC_DCDC_NormalVoltage;
    activeModeRegulatorOption.DCDCOption.DCDCDriveStrength     = kSPC_DCDC_NormalDriveStrength;
    activeModeRegulatorOption.SysLDOOption.SysLDOVoltage       = kSPC_SysLDO_NormalVoltage;
    activeModeRegulatorOption.SysLDOOption.SysLDODriveStrength = kSPC_SysLDO_LowDriveStrength;
    activeModeRegulatorOption.CoreLDOOption.CoreLDOVoltage     = kSPC_CoreLDO_NormalVoltage;
#if defined(FSL_FEATURE_SPC_HAS_CORELDO_VDD_DS) && FSL_FEATURE_SPC_HAS_CORELDO_VDD_DS
    activeModeRegulatorOption.CoreLDOOption.CoreLDODriveStrength = kSPC_CoreLDO_NormalDriveStrength;
#endif /* FSL_FEATURE_SPC_HAS_CORELDO_VDD_DS */

    status = SPC_SetActiveModeRegulatorsConfig(APP_SPC, &activeModeRegulatorOption);
    /* Disable Vdd Core Glitch detector in active mode. */
    SPC_DisableActiveModeVddCoreGlitchDetect(APP_SPC, true);
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

    lowPowerRegulatorOption.lpIREF      = false;
    lowPowerRegulatorOption.bandgapMode = kSPC_BandgapDisabled;
    lowPowerRegulatorOption.lpBuff      = false;
    /* Enable Core IVS, which is only useful in power down mode. */
    lowPowerRegulatorOption.CoreIVS = true;
    /* DCDC output voltage is 1.0V in some low power mode(Deep sleep, Power Down). DCDC is disabled in Deep Power Down.
     */
    lowPowerRegulatorOption.DCDCOption.DCDCVoltage             = kSPC_DCDC_MidVoltage;
    lowPowerRegulatorOption.DCDCOption.DCDCDriveStrength       = kSPC_DCDC_LowDriveStrength;
    lowPowerRegulatorOption.SysLDOOption.SysLDODriveStrength   = kSPC_SysLDO_LowDriveStrength;
    lowPowerRegulatorOption.CoreLDOOption.CoreLDOVoltage       = kSPC_CoreLDO_MidDriveVoltage;
    lowPowerRegulatorOption.CoreLDOOption.CoreLDODriveStrength = kSPC_CoreLDO_LowDriveStrength;

    status = SPC_SetLowPowerModeRegulatorsConfig(APP_SPC, &lowPowerRegulatorOption);
    /* Disable Vdd Core Glitch detector in low power mode. */
    SPC_DisableLowPowerModeVddCoreGlitchDetect(APP_SPC, true);
    if (status != kStatus_Success)
    {
        PRINTF("Fail to set regulators in Low Power Mode.");
        return;
    }
    while (SPC_GetBusyStatusFlag(APP_SPC))
        ;

    /* Disable LDO_CORE since it is bypassed. */
    SPC_EnableCoreLDORegulator(APP_SPC, false);

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
    if (VBAT_CheckFRO16kEnabled(APP_VBAT) == false)
    {
        /* In case of FRO16K is not enabled, enable it firstly. */
        VBAT_EnableFRO16k(APP_VBAT, true);
    }
    VBAT_UngateFRO16k(APP_VBAT, kVBAT_EnableClockToVddSys);

    /* Disable Bandgap to save current consumption. */
    if (VBAT_CheckBandgapEnabled(APP_VBAT))
    {
        VBAT_EnableBandgapRefreshMode(APP_VBAT, false);
        VBAT_EnableBandgap(APP_VBAT, false);
    }
}

static void APP_SetCMCConfiguration(void)
{
    /* Disable low power debug. */
    CMC_EnableDebugOperation(APP_CMC, false);
    /* Allow all power mode */
    CMC_SetPowerModeProtection(APP_CMC, kCMC_AllowAllLowPowerModes);

#if (defined(FSL_FEATURE_MCX_CMC_HAS_NO_FLASHCR_WAKE) && FSL_FEATURE_MCX_CMC_HAS_NO_FLASHCR_WAKE)
    /* From workaround of errata 051993, enable doze feature. */
    CMC_ConfigFlashMode(APP_CMC, true, false);
#else
    /* Disable flash memory accesses and place flash memory in low-power state whenever the core clock
       is gated. And an attempt to access the flash memory will cause the flash memory to exit low-power
       state for the duration of the flash memory access. */
    CMC_ConfigFlashMode(APP_CMC, true, true, false);
#endif 
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
    app_wakeup_source_t wakeupSource;
    uint8_t lptmrTimeOutValue;
    char *isoDomains = NULL;

    wakeupSource = APP_SelectWakeupSource();

    switch (wakeupSource)
    {
        case kAPP_WakeupSourceLptmr:
        {
            PRINTF("LPTMR Selected As Wakeup Source!\r\n");
            /* Wakeup source is LPTMR, user should input wakeup timeout value. */
            lptmrTimeOutValue = APP_GetWakeupTimeout();
            PRINTF("Will wakeup in %d seconds.\r\n", lptmrTimeOutValue);
            /* In case of target mode is powerdown/deep power down mode. */
            WUU_SetInternalWakeUpModulesConfig(APP_WUU, APP_WUU_WAKEUP_LPTMR_IDX, kWUU_InternalModuleInterrupt);
            LPTMR_SetTimerPeriod(APP_LPTMR, (APP_LPTMR_CLK_SOURCE * lptmrTimeOutValue) - 1U);
            LPTMR_EnableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable);
            LPTMR_StartTimer(APP_LPTMR);

            if (targetMode > kAPP_PowerModeSleep)
            {
                /* Isolate some power domains that are not used in low power modes.*/
                SPC_SetExternalVoltageDomainsConfig(APP_SPC, APP_SPC_LPTMR_LPISO_VALUE, 0x0U);
                isoDomains = APP_SPC_LPTMR_ISO_DOMAINS;
                PRINTF("Isolate power domains: %s\r\n", isoDomains);
            }
            break;
        }
#if !(defined(APP_NOT_SUPPORT_WAKEUP_BUTTON) && APP_NOT_SUPPORT_WAKEUP_BUTTON)
        case kAPP_WakeupSourceButton:
        {
            PRINTF("Wakeup Button Selected As Wakeup Source.\r\n");
            /* Set WUU to detect on rising edge for all power modes. */
            wuu_external_wakeup_pin_config_t wakeupButtonConfig;

            wakeupButtonConfig.edge  = kWUU_ExternalPinFallingEdge;
            wakeupButtonConfig.event = kWUU_ExternalPinInterrupt;
            wakeupButtonConfig.mode  = kWUU_ExternalPinActiveAlways;
            WUU_SetExternalWakeUpPinsConfig(APP_WUU, APP_WUU_WAKEUP_BUTTON_IDX, &wakeupButtonConfig);
            PRINTF("Please press %s to wakeup.\r\n", APP_WUU_WAKEUP_BUTTON_NAME);
            if (targetMode > kAPP_PowerModeSleep)
            {
                /* Isolate some power domains that are not used in low power modes.*/
                SPC_SetExternalVoltageDomainsConfig(APP_SPC, APP_SPC_WAKEUP_BUTTON_LPISO_VALUE, 0x0U);
                isoDomains = APP_SPC_WAKEUP_BUTTON_ISO_DOMAINS;
                PRINTF("Isolate power domains: %s\r\n", isoDomains);
            }
            break;
        }
#endif
        default:
            break;
    }
}

static app_wakeup_source_t APP_SelectWakeupSource(void)
{
    char ch;
    PRINTF("Please select wakeup source:\r\n");

    PRINTF("\tPress %c to select TIMER as wakeup source;\r\n", kAPP_WakeupSourceLptmr);
#if !(defined(APP_NOT_SUPPORT_WAKEUP_BUTTON) && APP_NOT_SUPPORT_WAKEUP_BUTTON)
    PRINTF("\tPress %c to select WAKE-UP-BUTTON as wakeup source;\r\n", kAPP_WakeupSourceButton);
#endif

    PRINTF("Waiting for wakeup source select...\r\n");
    ch = GETCHAR();

    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch -= 'a' - 'A';
    }

    return (app_wakeup_source_t)ch;
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
    config.wake_domain = kCMC_ActiveOrSleepMode;

    /* Switch as ROSC before entering into sleep mode, and disable other clock sources to decrease current consumption.
     */
    CMC_EnterLowPowerMode(APP_CMC, &config);
}

static void APP_EnterDeepSleepMode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepSleepMode;
    config.wake_domain = kCMC_DeepSleepMode;

    /* Power off some RAM blocks.  */
    CMC_PowerOffSRAMLowPowerOnly(APP_CMC, APP_RAM_ARRAYS_DS);

    CMC_EnterLowPowerMode(APP_CMC, &config);
}

static void APP_EnterPowerDownMode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_PowerDownMode;
    config.wake_domain = kCMC_PowerDownMode;

    /* Power off some RAM blocks. */
    CMC_PowerOffSRAMLowPowerOnly(APP_CMC, APP_RAM_ARRAYS_PD);
    L1CACHE_InvalidateCodeCache();
    /* Enable CORE VDD Internal Voltage scaling to decrease current consumption in power down mode. */
    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(APP_SPC, true);
    CMC_EnterLowPowerMode(APP_CMC, &config);
}

static void APP_EnterDeepPowerDownMode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepPowerDown;
    config.wake_domain = kCMC_DeepPowerDown;
    /* Power off some RAM blocks which are powered by VBAT? */

    CMC_EnterLowPowerMode(APP_CMC, &config);
}
