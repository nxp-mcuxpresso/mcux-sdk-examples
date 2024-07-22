/*
 * Copyright 2019, 2021-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <setjmp.h>

#include "fsl_cmc.h"
#include "fsl_spc.h"
#include "fsl_clock.h"
#include "fsl_debug_console.h"
#include "power_mode_switch.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "peripherals.h"
#include "board.h"
#include "fsl_lpuart.h"
#include "fsl_crc.h"
#include "fsl_vbat.h"
#include "fsl_ccm32k.h"
#include "fsl_port.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_MODE_NAME_ARRAY                                                                                     \
    {                                                                                                           \
        "Active", "Sleep1", "DeepSleep1", "DeepSleep2", "DeepSleep3", "DeepSleep4", "PowerDown1", "PowerDown2", \
            "PowerDown3", "PowerDown4", "DeepPowerDown1", "DeepPowerDown2", "PowerSwitchOff"                    \
    }

#define APP_MODE_DESC_ARRAY                                                                                            \
    {                                                                                                                  \
        "Active: DCDC in normal mode(voltage 1.5V); core voltage is 1.1V; core clock is 96MHz; power consumption is "  \
        "about 7.5 mA",                                                                                                \
            "Sleep1: Core, platform and peripheral clocks are gated; CORE_MAIN, CORE_WAKE and Radio domain enter "     \
            "into sleep mode; DCDC voltage is 1.25V; core voltage is 1.0V; all CM33 RAM retained; power consumption "  \
            "is about 53.5uA.",                                                                                        \
            "Deep Sleep1: Core, platform and peripheral clocks are gated; CORE_MAIN, CORE_WAKE and Radio domain "      \
            "enter into deep sleep mode; DCDC voltage is 1.25V; core voltages is 1.0V; all RAMs retained; OSC32K "     \
            "enabled; power consumption is about 2.8uA.",                                                              \
            "Deep Sleep2: Core, platform and peripheral clocks are gated; CORE_MAIN, CORE_WAKE and Radio domain "      \
            "enter into deep sleep mode; DCDC voltage is 1.25V; core voltages is 1.0V; 48KB of CM33 RAM retained; "    \
            "all radio RAMs retained; OSC32K enabled; power consumption is about 2.4uA. \r\nPlease note that core "    \
            "will fail to back to current context due to some used CM33 RAMs are powered off, will assert the "        \
            "software reset after waking up!",                                                                         \
            "Deep Sleep3: Core, platform and peripheral clocks are gated; CORE_MAIN, CORE_WAKE and Radio domain "      \
            "enter into deep sleep mode; DCDC voltage is 1.25V; core voltages is 1.0V; 16KB of CM33 RAM retained; "    \
            "all radio RAMs retained; OSC32K enabled; power consumption is about 1.8uA. \r\nPlease note that core "    \
            "will fail to back to current context due to some used CM33 RAMs are powered off, will assert the "        \
            "software reset after waking up!",                                                                         \
            "Deep Sleep4: Core, platform and peripheral clocks are gated; CORE_MAIN, CORE_WAKE and Radio domain "      \
            "enter into deep sleep mode; DCDC voltage is 1.25V; core voltages is 1.0V; 8KB of CM33 RAM retained; all " \
            "radio RAMs retained; OSC32K enabled; power consumption is about 1.8uA.\r\nPlease note that core will "    \
            "fail to back to current context due to all CM33 RAMs are powered off, will assert the software reset "    \
            "after waking up!",                                                                                        \
            "Power Down1: Core, platform and peripheral clocks are gated; CORE_MAIN, CORE_WAKE and Radio domain "      \
            "enter into power down mode; DCDC voltage is 1.25V; core voltages is 1.0V; 32KB of CM33 RAM retained; "    \
            "all radio RAMs retained; OSC32K enabled; power consumption is about 1.8uA.",                              \
            "Power Down2: Core, platform and peripheral clocks are gated; CORE_MAIN, CORE_WAKE and Radio domain "      \
            "enter into power down mode; DCDC voltage is 1.25V; core voltages is 1.0V; 16KB of CM33 RAM retained; "    \
            "all radio RAMs retained; OSC32K enabled; power consumption is about 1.8uA.",                              \
            "Power Down3: Core, platform and peripheral clocks are gated; CORE_MAIN, CORE_WAKE and Radio domain "      \
            "enter into power down mode; DCDC voltage is 1.25V; core voltages is 1.0V; 16KB of CM33 RAM retained; no " \
            "radio RAM retained; OSC32K enabled; power consumption is about 1.8uA.",                                   \
            "Power Down4: Core, platform and peripheral clocks are gated; CORE_MAIN, CORE_WAKE and Radio domain "      \
            "enter into power down mode; DCDC voltage is 1.25V; core voltages is 1.0V; 8KB of CM33 RAM retained; no "  \
            "radio RAM retained; OSC32K enabled; power consumption is about 1.8uA.",                                   \
            "Deep Power Down1: Core, platform and peripheral clocks are gated; CORE_MAIN, CORE_WAKE and Radio domain " \
            "enter into deep power down mode; DCDC voltage is 1.25V; core voltages is 1.0V; 8KB of CM33 RAM "          \
            "retained; no radio RAM retained; OSC32K enabled; power consumption is about 1.1uA.",                      \
            "Deep Power Down2: Core, platform and peripheral clocks are gated; CORE_MAIN, CORE_WAKE and Radio domain " \
            "enter into deep power down mode; DCDC voltage is 1.25V; core voltages is 1.0V; 8KB of CM33 RAM "          \
            "retained; no radio RAM retained; OSC32K enabled; power consumption is about 0.9uA. \r\nPlease note that " \
            "core will fail to back to current context due to all CM33 RAMs are powered off!",                         \
            "Switch off: All power domain except VBAT power domain are powered off; power consumption is about 0.2 "   \
            "uA."                                                                                                      \
    }
#define APP_CMC   CMC0
#define APP_LPTMR DEMO_LPTMR_PERIPHERAL
#define APP_WUU   DEMO_WUU_PERIPHERAL
#define APP_SPC   SPC0
#define APP_VBAT  VBAT0

#define WUU_WAKEUP_LPTMR_IDX  0U
#define WUU_WAKEUP_VBAT_IDX   4U
#define WUU_WAKEUP_BUTTON_IDX 11U

#define WUU_WAKEUP_BUTTON_NAME "SW3"

#define APP_LPTMR_IRQ_HANDLER DEMO_LPTMR_IRQHANDLER
#define APP_VBAT_IRQ_HANDLER  VBAT_IRQHandler

#define APP_LPTMR_LOW_POWER_ISO_DOMAINS         (0x7U)
#define APP_WAKEUP_BUTTON_LOW_POWER_ISO_DOMAINS (0x6U)
#define APP_VABT_LOW_POWER_ISO_DOMAINS          (0x7U)

#define APP_SRAM_ARRAYS_TO_POWER_OFF                                                                       \
    (kCMC_SRAMBank0 | kCMC_SRAMBank1 | kCMC_SRAMBank2 | kCMC_SRAMBank3 | kCMC_SRAMBank4 | kCMC_SRAMBank5 | \
     kCMC_SRAMBank6 | kCMC_SRAMBank7)

#define APP_DEEPSLEEP2_SRAM_POWER_MODE (0x1FU) // 48KB of RAM retained: STCM5(8K), STCM4(8K), STCM3(32K).
#define APP_DEEPSLEEP3_SRAM_POWER_MODE (0x3FU) // 16KB of RAM retained: STCM5(8K), STCM4(8K).
#define APP_DEEPSLEEP4_SRAM_POWER_MODE (0xCFU) // 8KB of RAM retained:  STCM5(8KB).
#define APP_POWERDOWN1_SRAM_POWER_MODE (0x3CU) // 32KB of RAM retained: STCM5(8K), STCM4(8K),CTCM1(8K),CTCM0(8K).
#define APP_POWERDOWN2_SRAM_POWER_MODE (0x3FU) // 16KB of RAM retained: STCM5(8K),STCM4(8K).
#define APP_POWERDOWN3_SRAM_POWER_MODE (0x3FU) // 16KB of RAM retained: STCM5(8K),STCM4(8K).
#define APP_POWERDOWN4_SRAM_POWER_MODE (0xCFU) // 8KB(STCM5) RAM retained.

/* Debug console RX pin: PORTC2 MUX: 3 */
#define DEBUG_CONSOLE_RX_PORT   PORTC
#define DEBUG_CONSOLE_RX_GPIO   GPIOC
#define DEBUG_CONSOLE_RX_PIN    2U
#define DEBUG_CONSOLE_RX_PINMUX kPORT_MuxAlt3
/* Debug console TX pin: PORTC3 MUX: 3 */
#define DEBUG_CONSOLE_TX_PORT   PORTC
#define DEBUG_CONSOLE_TX_GPIO   GPIOC
#define DEBUG_CONSOLE_TX_PIN    3U
#define DEBUG_CONSOLE_TX_PINMUX kPORT_MuxAlt3


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_InitDebugConsole(void);
void APP_DeinitDebugConsole(void);
void APP_DeinitWakeupButton(void);
static void APP_SetSPCConfiguration(void);
static void APP_SetCMCConfiguration(void);
static void APP_DeInitVbat(void);

static uint8_t APP_GetWakeupTimeout(void);
static app_wakeup_source_t APP_SelectWakeupSource(app_power_mode_t targetMode);
static void APP_GetWakeupConfig(app_power_mode_t targetMode);

static void APP_PowerPreSwitchHook(void);
static void APP_PowerPostSwitchHook(void);

static void APP_ShowPowerMode(cmc_low_power_mode_t powermode);
static void APP_EnterSleep1Mode(void);
static void APP_EnterDeepSleep1Mode(void);
static void APP_EnterDeepSleep2Mode(void);
static void APP_EnterDeepSleep3Mode(void);
static void APP_EnterDeepSleep4Mode(void);
static void APP_EnterPowerDown1Mode(void);
static void APP_EnterPowerDown2Mode(void);
static void APP_EnterPowerDown3Mode(void);
static void APP_EnterPowerDown4Mode(void);
static void APP_EnterDeepPowerDown1Mode(void);
static void APP_EnterDeepPowerDown2Mode(void);
static void APP_PowerModeSwitch(app_power_mode_t targetPowerMode);
static app_power_mode_t APP_GetTargetPowerMode(void);

static void APP_SaveRuntimeContext(void);
static void APP_RestoreRuntimeContext(void);

static void APP_WakeupFunction(void);
static void APP_SetPowerDownModeWakeupConfig(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint8_t s_wakeupTimeout; /* Wakeup timeout. (Unit: Second) */
volatile app_wakeup_source_t g_selectedWakeupSource;

AT_ALWAYS_ON_DATA(uint32_t g_scbVtor);
AT_ALWAYS_ON_DATA(uint32_t g_scbIcsr);
AT_ALWAYS_ON_DATA(uint32_t g_scbAircr);
AT_ALWAYS_ON_DATA(uint32_t g_scbCCR);
AT_ALWAYS_ON_DATA(uint8_t g_scbShp[12]);
AT_ALWAYS_ON_DATA(uint32_t g_scbShcsr);
AT_ALWAYS_ON_DATA(uint32_t g_nvicIser[16U]);
AT_ALWAYS_ON_DATA(uint8_t g_nvicIp[496U]);
AT_ALWAYS_ON_DATA(uint32_t g_cpuControl);
AT_ALWAYS_ON_DATA(uint32_t g_wakeupEntry[12]);
AT_ALWAYS_ON_DATA(jmp_buf g_coreContext);
AT_ALWAYS_ON_DATA(app_power_mode_t g_targetPowerMode);

AT_ALWAYS_ON_DATA_INIT(char *g_modeNameArray[]) = APP_MODE_NAME_ARRAY;
AT_ALWAYS_ON_DATA_INIT(char *g_modeDescArray[]) = APP_MODE_DESC_ARRAY;

extern uint32_t m_warmboot_stack_end;

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
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
}

void APP_DeinitDebugConsole(void)
{
    DbgConsole_Deinit();
    /*
     * Set pin for current leakage.
     * Debug console RX pin: Set to pinmux to disable.
     * Debug console TX pin: Don't need to change.
     */
    PORT_SetPinMux(DEBUG_CONSOLE_RX_PORT, DEBUG_CONSOLE_RX_PIN, kPORT_PinDisabledOrAnalog);
    PORT_SetPinMux(DEBUG_CONSOLE_TX_PORT, DEBUG_CONSOLE_TX_PIN, kPORT_PinDisabledOrAnalog);
}

void APP_DeinitWakeupButton(void)
{
    port_pin_config_t wakeupButtonDefaultConfig = {kPORT_PullDisable, (uint16_t)kPORT_LowPullResistor,
                                                   /* Slow slew rate is configured */
                                                   (uint16_t)kPORT_SlowSlewRate,
                                                   /* Passive input filter is disabled */
                                                   (uint16_t)kPORT_PassiveFilterDisable,
                                                   /* Open drain output is enabled */
                                                   (uint16_t)kPORT_OpenDrainEnable,
                                                   /* Low drive strength is configured */
                                                   (uint16_t)kPORT_LowDriveStrength,
                                                   /* Normal drive strength is configured */
                                                   (uint16_t)kPORT_NormalDriveStrength,
                                                   /* Pin is configured as default value */
                                                   (uint16_t)kPORT_PinDisabledOrAnalog,
                                                   /* Pin Control Register fields [15:0] are not locked */
                                                   (uint16_t)kPORT_UnlockRegister};
    PORT_SetPinConfig(PORTC, 6U, &wakeupButtonDefaultConfig);
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

void WUU0_IRQHandler(void)
{
    if ((WUU_GetExternalWakeUpPinsFlag(APP_WUU) & 0x800UL) != 0UL)
    {
        WUU_ClearExternalWakeUpPinsFlag(APP_WUU, 0x800UL);
    }
}


int main(void)
{
    uint32_t freq;
    cmc_low_power_mode_t curmode;
    bool needSetWakeup = false;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitBootPeripherals();

    CLOCK_DeinitSysOsc();
    CLOCK_DeinitSirc();

    APP_SetSPCConfiguration();

    PRINTF("\r\n----------------------- Normal Boot -----------------------\r\n");

    while (1)
    {
        PRINTF("\r\nSticky Reset Reason %x.\r\n", CMC_GetStickySystemResetStatus(APP_CMC));
        CMC_ClearStickySystemResetStatus(CMC0, CMC_GetStickySystemResetStatus(APP_CMC));
        APP_SetCMCConfiguration();
        APP_DeInitVbat();

        freq    = CLOCK_GetFreq(kCLOCK_CoreSysClk);
        curmode = CMC_GetMAINPowerMode(APP_CMC);
        PRINTF("\r\n###########################    Power Mode Switch Demo    ###########################\r\n");
        PRINTF("    Core Clock = %dHz \r\n", freq);

        APP_ShowPowerMode(curmode);
        g_targetPowerMode = APP_GetTargetPowerMode();

        /* If target mode is Active mode, don't need to set wakeup source. */
        if (g_targetPowerMode == kAPP_PowerModeActive)
        {
            needSetWakeup = false;
        }
        else
        {
            needSetWakeup = true;
        }
        PRINTF("%s\r\n", g_modeDescArray[(uint8_t)(g_targetPowerMode - kAPP_PowerModeActive)]);
        if (needSetWakeup)
        {
            APP_GetWakeupConfig(g_targetPowerMode);
            APP_PowerPreSwitchHook();
            APP_PowerModeSwitch(g_targetPowerMode);
            APP_PowerPostSwitchHook();
            PRINTF("\r\n************* Wake Up From %s Mode *************\r\n",
                   g_modeNameArray[(uint8_t)(g_targetPowerMode - kAPP_PowerModeActive)]);
        }

        PRINTF("\r\n-----------------------Next loop -----------------------\r\n");
    }
}

/*!
 * @brief In active mode, all HVDs/LVDs are disabled, DCDC regulated to 1.5V, Core LDO regulated to 1.1V;
 * In low power modes, all HVDs/LVDs are disabled, bandgap is disabled, DCDC regulated to 1.25V, Core LDO regulated to
 * 1.0V.
 */
static void APP_SetSPCConfiguration(void)
{
    status_t status;

    spc_active_mode_regulators_config_t activeModeRegulatorOption;

    /* Disable LVDs and HVDs in Active mode. */
    SPC_EnableActiveModeCoreHighVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeCoreLowVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeSystemHighVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeSystemLowVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeIOHighVoltageDetect(APP_SPC, false);
    SPC_EnableActiveModeIOLowVoltageDetect(APP_SPC, false);

    activeModeRegulatorOption.bandgapMode                      = kSPC_BandgapEnabledBufferDisabled;
    activeModeRegulatorOption.lpBuff                           = false;
    activeModeRegulatorOption.DCDCOption.DCDCVoltage           = kSPC_DCDC_NormalVoltage; /* DCDC regulate to 1.5V. */
    activeModeRegulatorOption.DCDCOption.DCDCDriveStrength     = kSPC_DCDC_NormalDriveStrength;
    activeModeRegulatorOption.SysLDOOption.SysLDOVoltage       = kSPC_SysLDO_NormalVoltage;
    activeModeRegulatorOption.SysLDOOption.SysLDODriveStrength = kSPC_SysLDO_NormalDriveStrength;
    activeModeRegulatorOption.CoreLDOOption.CoreLDOVoltage =
        kSPC_CoreLDO_NormalVoltage; /* Core LDO regulate to 1.1V. */
#if defined(FSL_FEATURE_SPC_HAS_CORELDO_VDD_DS) && FSL_FEATURE_SPC_HAS_CORELDO_VDD_DS
    activeModeRegulatorOption.CoreLDOOption.CoreLDODriveStrength = kSPC_CoreLDO_NormalDriveStrength;
#endif /* FSL_FEATURE_SPC_HAS_CORELDO_VDD_DS */

    status = SPC_SetActiveModeRegulatorsConfig(APP_SPC, &activeModeRegulatorOption);
    if (status != kStatus_Success)
    {
        PRINTF("Fail to set regulators in Active mode.");
        return;
    }

    spc_lowpower_mode_regulators_config_t lowPowerRegulatorOption;

    lowPowerRegulatorOption.lpIREF                             = false;
    lowPowerRegulatorOption.bandgapMode                        = kSPC_BandgapDisabled;
    lowPowerRegulatorOption.lpBuff                             = false;
    lowPowerRegulatorOption.CoreIVS                            = false;
    lowPowerRegulatorOption.DCDCOption.DCDCVoltage             = kSPC_DCDC_LowUnderVoltage;
    lowPowerRegulatorOption.DCDCOption.DCDCDriveStrength       = kSPC_DCDC_LowDriveStrength;
    lowPowerRegulatorOption.SysLDOOption.SysLDODriveStrength   = kSPC_SysLDO_LowDriveStrength;
    lowPowerRegulatorOption.CoreLDOOption.CoreLDOVoltage       = kSPC_CoreLDO_MidDriveVoltage;
    lowPowerRegulatorOption.CoreLDOOption.CoreLDODriveStrength = kSPC_CoreLDO_LowDriveStrength;

    status = SPC_SetLowPowerModeRegulatorsConfig(APP_SPC, &lowPowerRegulatorOption);
    if (status != kStatus_Success)
    {
        PRINTF("Fail to set regulators in Low Power Mode.");
        return;
    }

    SPC_SetLowPowerWakeUpDelay(APP_SPC, 0xFFFFU);
}

/*!
 * @brief 1. Set power mode protection; 2. Disable low power mode debug; 3. Enable Flash Doze mode.
 */
static void APP_SetCMCConfiguration(void)
{
    CMC_SetPowerModeProtection(APP_CMC, kCMC_AllowAllLowPowerModes);
    CMC_LockPowerModeProtectionSetting(APP_CMC);
    CMC_EnableDebugOperation(APP_CMC, false);
    CMC_ConfigFlashMode(APP_CMC, false, true, false);
}

/*!
 * @brief Disable Backup SRAM regulator, FRO16K and Bandgap which locates in VBAT power domain for most of power modes.
 *
 */
static void APP_DeInitVbat(void)
{
    VBAT_EnableBackupSRAMRegulator(APP_VBAT, false);
    VBAT_EnableFRO16k(APP_VBAT, false);
    while (VBAT_CheckFRO16kEnabled(APP_VBAT))
        ;
    VBAT_EnableBandgap(APP_VBAT, false);
    while (VBAT_CheckBandgapEnabled(APP_VBAT))
        ;
}

static app_wakeup_source_t APP_SelectWakeupSource(app_power_mode_t targetMode)
{
    app_wakeup_source_t wakeupSource;
    uint8_t ch;

    PRINTF("\r\nSelect the desired wakeup source:\r\n");
    if (targetMode != kAPP_PowerSwitchOff)
    {
        PRINTF("Press %c to select LPTMR0\r\n", kAPP_WakeupSourceLptmr);
        PRINTF("Press %c to select button\r\n", kAPP_WakeupSourceWakeupButton);
    }
    PRINTF("Press %c to Select VBAT\r\n", kAPP_WakeupSourceVbat);

    PRINTF("\r\nWaiting for wakeup source select...\r\n");

    ch = GETCHAR();
    PUTCHAR(ch);
    PRINTF("\n");
    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch -= 'a' - 'A';
    }

    wakeupSource = (app_wakeup_source_t)ch;

    return wakeupSource;
}

/* Get wakeup timeout and wakeup source. */
static void APP_GetWakeupConfig(app_power_mode_t targetMode)
{
    g_selectedWakeupSource = APP_SelectWakeupSource(targetMode);

    switch (g_selectedWakeupSource)
    {
        case kAPP_WakeupSourceLptmr:
        {
            /* Wakeup source is LPTMR, user should input wakeup timeout value. */
            s_wakeupTimeout = APP_GetWakeupTimeout();
            PRINTF("Will wakeup in %d seconds.\r\n", s_wakeupTimeout);
            /* Isolate some external power domain to save power. */
            SPC_SetExternalVoltageDomainsConfig(APP_SPC, APP_LPTMR_LOW_POWER_ISO_DOMAINS, 0x0U);
            WUU_SetInternalWakeUpModulesConfig(APP_WUU, WUU_WAKEUP_LPTMR_IDX, kWUU_InternalModuleInterrupt);
            LPTMR_SetTimerPeriod(APP_LPTMR, (32000U * s_wakeupTimeout) - 1U);
            LPTMR_EnableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable);
            LPTMR_StartTimer(APP_LPTMR);
            EnableIRQ(LPTMR0_IRQn);
            break;
        }

        case kAPP_WakeupSourceWakeupButton:
        {
            PRINTF(
                "Use wakeup button will increase current consumption due to VDD_IO_ABC power domain can not be "
                "isolated.\r\n");
            PRINTF("Please press %s to wakeup device.\r\n", WUU_WAKEUP_BUTTON_NAME);
            /* Isolate some external power domain to save power. */
            SPC_SetExternalVoltageDomainsConfig(APP_SPC, APP_WAKEUP_BUTTON_LOW_POWER_ISO_DOMAINS, 0x0U);
            wuu_external_wakeup_pin_config_t wakeupButtonConfig;
            wakeupButtonConfig.edge  = kWUU_ExternalPinRisingEdge;
            wakeupButtonConfig.event = kWUU_ExternalPinInterrupt;
            wakeupButtonConfig.mode  = kWUU_ExternalPinActiveAlways;
            WUU_SetExternalWakeUpPinsConfig(APP_WUU, WUU_WAKEUP_BUTTON_IDX, &wakeupButtonConfig);
            break;
        }

        case kAPP_WakeupSourceVbat:
        {
            PRINTF("Waiting for WAKEUP pin's falling edge input.\r\n");
            /* Isolate some external power domain to save power. */
            SPC_SetExternalVoltageDomainsConfig(APP_SPC, APP_VABT_LOW_POWER_ISO_DOMAINS, 0x0U);
            /* Clear all status flags. */
            VBAT_ClearStatusFlags(APP_VBAT, kVBAT_StatusFlagPORDetect | kVBAT_StatusFlagWakeupPin |
                                                kVBAT_StatusFlagBandgapTimer0 | kVBAT_StatusFlagBandgapTimer1);
            VBAT_DisableInterrupts(APP_VBAT, kVBAT_AllInterruptsEnable);
            VBAT_DisableWakeup(APP_VBAT, kVBAT_AllWakeupsEnable);
            WUU_SetInternalWakeUpModulesConfig(APP_WUU, WUU_WAKEUP_VBAT_IDX, kWUU_InternalModuleInterrupt);
            EnableIRQ(VBAT_IRQn);
            VBAT_EnableInterrupts(APP_VBAT, kVBAT_InterruptEnableWakeupPin);
            /* To wakeup from deep sleep or more higher mode, it is needed  to enable wakeup feature. */
            VBAT_EnableWakeup(APP_VBAT, kVBAT_WakeupEnableWakeupPin);
            break;
        }
        default:
        {
            assert(false);
            break;
        }
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

    if (g_selectedWakeupSource != kAPP_WakeupSourceWakeupButton)
    {
        APP_DeinitWakeupButton();
    }
}

static void APP_PowerPostSwitchHook(void)
{
    if (SPC_CheckPowerDomainLowPowerRequest(APP_SPC, kSPC_PowerDomain0))
    {
        SPC_ClearPowerDomainLowPowerRequestFlag(APP_SPC, kSPC_PowerDomain0);
    }
    if (SPC_CheckPowerDomainLowPowerRequest(APP_SPC, kSPC_PowerDomain1))
    {
        SPC_ClearPowerDomainLowPowerRequestFlag(APP_SPC, kSPC_PowerDomain1);
    }
    if (SPC_CheckPowerDomainLowPowerRequest(APP_SPC, kSPC_PowerDomain2))
    {
        RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK));
        RFMC->RF2P4GHZ_CTRL &= ~RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;
        SPC_ClearPowerDomainLowPowerRequestFlag(APP_SPC, kSPC_PowerDomain2);
    }
    SPC_ClearLowPowerRequest(APP_SPC);

    CLOCK_EnableClock(kCLOCK_Lpuart0);
    APP_InitDebugConsole();
}

static void APP_ShowPowerMode(cmc_low_power_mode_t powermode)
{
    if (powermode == kCMC_ActiveMode)
    {
        PRINTF("    Power mode: Active\r\n");
    }
}

static app_power_mode_t APP_GetTargetPowerMode(void)
{
    uint8_t ch;
    app_power_mode_t inputPowerMode;

    while (1)
    {
        PRINTF("\r\nPlease Select the desired power mode: \n\r\n");

        for (app_power_mode_t powerModeIndex = kAPP_PowerModeActive; powerModeIndex < kAPP_PowerModeMax;
             powerModeIndex++)
        {
            PRINTF("\tPress %c to enter %s mode!\r\n", powerModeIndex,
                   g_modeNameArray[(uint8_t)(powerModeIndex - kAPP_PowerModeActive)]);
        }
        PRINTF("\r\n\tWaiting for power mode select...\r\n\r\n");

        ch = GETCHAR();

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        PUTCHAR(ch);
        PUTCHAR('\n');

        inputPowerMode = (app_power_mode_t)ch;
        if ((inputPowerMode > kAPP_PowerModeMin) && (inputPowerMode < kAPP_PowerModeMax))
        {
            return inputPowerMode;
        }
        PRINTF("Wrong input, please re-try!\r\n");
    }
}

static void APP_PowerModeSwitch(app_power_mode_t targetPowerMode)
{
    if (targetPowerMode != kAPP_PowerModeActive)
    {
        switch (targetPowerMode)
        {
            case kAPP_PowerModeSleep1:
                APP_EnterSleep1Mode();
                break;
            case kAPP_PowerModeDeepSleep1:
                APP_EnterDeepSleep1Mode();
                break;
            case kAPP_PowerModeDeepSleep2:
                APP_EnterDeepSleep2Mode();
                break;
            case kAPP_PowerModeDeepSleep3:
                APP_EnterDeepSleep3Mode();
                break;
            case kAPP_PowerModeDeepSleep4:
                APP_EnterDeepSleep4Mode();
                break;
            case kAPP_PowerModePowerDown1:
                APP_EnterPowerDown1Mode();
                break;
            case kAPP_PowerModePowerDown2:
                APP_EnterPowerDown2Mode();
                break;
            case kAPP_PowerModePowerDown3:
                APP_EnterPowerDown3Mode();
                break;
            case kAPP_PowerModePowerDown4:
                APP_EnterPowerDown4Mode();
                break;
            case kAPP_PowerModeDeepPowerDown1:
                APP_EnterDeepPowerDown1Mode();
                break;
            case kAPP_PowerModeDeepPowerDown2:
                APP_EnterDeepPowerDown2Mode();
                break;
            case kAPP_PowerSwitchOff:
                SPC_PowerModeControlPowerSwitch(APP_SPC);
                APP_EnterDeepPowerDown1Mode();
                break;
            default:
                assert(false);
                break;
        }
    }
}

static void APP_EnterSleep1Mode(void)
{
    cmc_power_domain_config_t config;

    /* Set NBU into Sleep Mode */
    RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK)) | RFMC_RF2P4GHZ_CTRL_LP_MODE(0x1);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;

    /* Set MAIN_CORE and MAIN_WAKE power domain into sleep mode. */
    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_SleepMode;
    config.wake_domain = kCMC_SleepMode;
    CMC_EnterLowPowerMode(APP_CMC, &config);
}

static void APP_EnterDeepSleep1Mode(void)
{
    cmc_power_domain_config_t config;

    /* Enable CORE VDD Voltage scaling. */
    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(APP_SPC, true);

    /* Set NBU into Deep Sleep Mode */
    RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK)) | RFMC_RF2P4GHZ_CTRL_LP_MODE(0x3);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;

    /* Set MAIN_CORE and MAIN_WAKE power domain into Deep Sleep Mode. */
    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepSleepMode;
    config.wake_domain = kCMC_DeepSleepMode;

    CMC_EnterLowPowerMode(APP_CMC, &config);
}

static void APP_EnterDeepSleep2Mode(void)
{
    cmc_power_domain_config_t config;

    /* Enable CORE VDD Voltage scaling. */
    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(APP_SPC, true);

    /* Power off some CM33 RAMs in deep sleep mode.  */
    CMC_PowerOffSRAMLowPowerOnly(APP_CMC, APP_DEEPSLEEP2_SRAM_POWER_MODE);

    /* Set NBU into Deep Sleep Mode. */
    RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK)) | RFMC_RF2P4GHZ_CTRL_LP_MODE(0x3);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;

    /* Set MAIN_CORE and MAIN_WAKE power domain into deep sleep mode. */
    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepSleepMode;
    config.wake_domain = kCMC_DeepSleepMode;
    CMC_EnterLowPowerMode(APP_CMC, &config);
    /* Due to some CM33 RAMs are powered off in this mode, after waking up a hardfault will be asserted, do software
     * reset to re-start this demo. */
    NVIC_SystemReset();
}

static void APP_EnterDeepSleep3Mode(void)
{
    cmc_power_domain_config_t config;

    /* Set MAIN_CORE and MAIN_WAKE power domain into deep sleep mode. */
    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepSleepMode;
    config.wake_domain = kCMC_DeepSleepMode;

    /* Set NBU into Deep Sleep Mode. */
    RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK)) | RFMC_RF2P4GHZ_CTRL_LP_MODE(0x3);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;

    /* Enable CORE VDD Voltage scaling. */
    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(APP_SPC, true);
    CMC_PowerOffSRAMLowPowerOnly(APP_CMC, APP_DEEPSLEEP3_SRAM_POWER_MODE);

    CMC_EnterLowPowerMode(APP_CMC, &config);
    /* Due to some CM33 RAMs are powered off in this mode, after waking up a hardfault will be asserted, do software
     * reset to re-start this demo. */
    NVIC_SystemReset();
}

static void APP_EnterDeepSleep4Mode(void)
{
    cmc_power_domain_config_t config;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepSleepMode;
    config.wake_domain = kCMC_DeepSleepMode;

    /* Set NBU into Deep Sleep Mode. */
    RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK)) | RFMC_RF2P4GHZ_CTRL_LP_MODE(0x3);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;

    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(APP_SPC, true);
    CMC_PowerOffSRAMLowPowerOnly(APP_CMC, APP_DEEPSLEEP4_SRAM_POWER_MODE);

    CMC_EnterLowPowerMode(APP_CMC, &config);
    /* Due to some CM33 RAMs are powered off in this mode, after waking up a busfault will be asserted, do software
     * reset to re-start this demo. */
    NVIC_SystemReset();
}

static void APP_EnterPowerDown1Mode(void)
{
    cmc_power_domain_config_t config;

    /* Set NBU into Power Down mode. */
    RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK)) | RFMC_RF2P4GHZ_CTRL_LP_MODE(0x7);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_PowerDownMode;
    config.wake_domain = kCMC_DeepSleepMode;

    CMC_PowerOffSRAMLowPowerOnly(APP_CMC, APP_POWERDOWN1_SRAM_POWER_MODE);
    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(APP_SPC, true);
    APP_SetPowerDownModeWakeupConfig();

    APP_SaveRuntimeContext();

    CLOCK_DisableClock(kCLOCK_Secsubsys);
    memset(&g_coreContext, 0, sizeof(g_coreContext));
    if (setjmp(g_coreContext) == 0)
    {
        CMC_EnterLowPowerMode(APP_CMC, &config);
    }
    /* Wake up from power down1 mode. */
    APP_RestoreRuntimeContext();
    __enable_irq();
}

static void APP_EnterPowerDown2Mode(void)
{
    cmc_power_domain_config_t config;

    /* Set NBU into Power Down Mode. */
    RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK)) | RFMC_RF2P4GHZ_CTRL_LP_MODE(0x7);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;

    /* Set CORE_MAIN, CORE_WAKE power domain into power down mode. */
    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_PowerDownMode;
    config.wake_domain = kCMC_PowerDownMode;

    CMC_PowerOffSRAMLowPowerOnly(APP_CMC, APP_POWERDOWN2_SRAM_POWER_MODE);
    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(APP_SPC, true);
    /* Set wakeup function. */
    APP_SetPowerDownModeWakeupConfig();

    APP_SaveRuntimeContext();
    memset(&g_coreContext, 0, sizeof(g_coreContext));
    if (setjmp(g_coreContext) == 0)
    {
        CMC_EnterLowPowerMode(APP_CMC, &config);
    }
    /* Wake up from power down2 mode. */
    APP_RestoreRuntimeContext();
    __enable_irq();
}

static void APP_EnterPowerDown3Mode(void)
{
    cmc_power_domain_config_t config;

    /* Power off all radio RAMs. */
    RF_CMC1->RAM_PWR = 0x000004FF;
    /* Set NBU into Power Down Mode. */
    RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK)) | RFMC_RF2P4GHZ_CTRL_LP_MODE(0x7);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_PowerDownMode;
    config.wake_domain = kCMC_PowerDownMode;

    CMC_PowerOffSRAMLowPowerOnly(APP_CMC, APP_POWERDOWN3_SRAM_POWER_MODE);
    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(APP_SPC, true);
    APP_SetPowerDownModeWakeupConfig();

    APP_SaveRuntimeContext();
    memset(&g_coreContext, 0, sizeof(g_coreContext));
    if (setjmp(g_coreContext) == 0)
    {
        CMC_EnterLowPowerMode(APP_CMC, &config);
    }
    /* Wake up from power down2 mode. */
    APP_RestoreRuntimeContext();
    __enable_irq();
}

static void APP_EnterPowerDown4Mode(void)
{
    cmc_power_domain_config_t config;

    /* Power off all radion RAMs. */
    RF_CMC1->RAM_PWR = 0x000004FF;
    /* Set NBU into Power Down Mode. */
    RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK)) | RFMC_RF2P4GHZ_CTRL_LP_MODE(0x7);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;

    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_PowerDownMode;
    config.wake_domain = kCMC_PowerDownMode;

    CMC_PowerOffSRAMLowPowerOnly(APP_CMC, APP_POWERDOWN4_SRAM_POWER_MODE);
    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(APP_SPC, true);
    APP_SetPowerDownModeWakeupConfig();

    APP_SaveRuntimeContext();
    memset(&g_coreContext, 0, sizeof(g_coreContext));
    if (setjmp(g_coreContext) == 0)
    {
        CMC_EnterLowPowerMode(APP_CMC, &config);
    }
    /* Wake up from power down2 mode. */
    APP_RestoreRuntimeContext();
    __enable_irq();
}

static void APP_EnterDeepPowerDown1Mode(void)
{
    cmc_power_domain_config_t config;

    /* Power off all radio RAMs. */
    RF_CMC1->RAM_PWR = 0x000004FF;
    /* Set NBU into Deep Power Down Mode. */
    RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK)) | RFMC_RF2P4GHZ_CTRL_LP_MODE(0xF);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;

    /* Set CORE_MAIN, CORE_WAKE into deep power down mode. */
    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepPowerDown;
    config.wake_domain = kCMC_DeepPowerDown;

    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(APP_SPC, true);
    APP_SetPowerDownModeWakeupConfig();

    /* Enable VBAT backup SRAM regulator to supply STCM5, because CORE_LDO will be disabled in deep power down mode. */
    vbat_fro16k_config_t vbatFro16kConfig = {
        .enableFRO16k       = true,
        .enableFRO16kOutput = true,
    };
    VBAT_ConfigFRO16k(VBAT0, &vbatFro16kConfig);
    VBAT_EnableBandgap(VBAT0, true);
    VBAT_EnableBandgapRefreshMode(VBAT0, true);
    VBAT_EnableBackupSRAMRegulator(VBAT0, true);

    APP_SaveRuntimeContext();
    memset(&g_coreContext, 0, sizeof(g_coreContext));
    if (setjmp(g_coreContext) == 0)
    {
        CMC_EnterLowPowerMode(APP_CMC, &config);
    }
    /* Wake up from Deep power down mode. */
    APP_RestoreRuntimeContext();
    __enable_irq();
}

static inline void APP_EnterDeepPowerDown2Mode(void)
{
    cmc_power_domain_config_t config;

    /* Power off all radio RAMs. */
    RF_CMC1->RAM_PWR = 0x000004FF;
    /* Set NBU into Deep Power Down Mode. */
    RFMC->RF2P4GHZ_CTRL = (RFMC->RF2P4GHZ_CTRL & (~RFMC_RF2P4GHZ_CTRL_LP_MODE_MASK)) | RFMC_RF2P4GHZ_CTRL_LP_MODE(0xF);
    RFMC->RF2P4GHZ_CTRL |= RFMC_RF2P4GHZ_CTRL_LP_ENTER_MASK;

    /* Set CORE_MAIN, CORE_WAKE into deep power down mode. */
    config.clock_mode  = kCMC_GateAllSystemClocksEnterLowPowerMode;
    config.main_domain = kCMC_DeepPowerDown;
    config.wake_domain = kCMC_DeepPowerDown;

    SPC_EnableLowPowerModeCoreVDDInternalVoltageScaling(APP_SPC, true);
    APP_SetPowerDownModeWakeupConfig();
    APP_SaveRuntimeContext();
    memset(&g_coreContext, 0, sizeof(g_coreContext));
    if (setjmp(g_coreContext) == 0)
    {
        CMC_EnterLowPowerMode(APP_CMC, &config);
    }
}

static void APP_SaveRuntimeContext(void)
{
    uint32_t i = 0UL;

    g_scbVtor  = SCB->VTOR;
    g_scbIcsr  = SCB->ICSR;
    g_scbAircr = ((SCB->AIRCR) & ~SCB_AIRCR_VECTKEY_Msk) | (0x5FA << SCB_AIRCR_VECTKEY_Pos);
    g_scbCCR   = SCB->CCR;
    for (i = 0UL; i < 12UL; i++)
    {
        g_scbShp[i] = SCB->SHPR[i];
    }
    g_scbShcsr = SCB->SHCSR;
    for (i = 0UL; i < 16U; i++)
    {
        g_nvicIser[i] = NVIC->ISER[i];
    }
    for (i = 0UL; i < 496UL; i++)
    {
        g_nvicIp[i] = NVIC->IPR[i];
    }
    g_cpuControl = __get_CONTROL();
}

static void APP_RestoreRuntimeContext(void)
{
    uint32_t i;

    SCB->VTOR  = g_scbVtor;
    SCB->ICSR  = g_scbIcsr;
    SCB->AIRCR = g_scbAircr;
    SCB->CCR   = g_scbCCR;
    for (i = 0UL; i < 12UL; i++)
    {
        SCB->SHPR[i] = g_scbShp[i];
    }
    SCB->SHCSR = g_scbShcsr;
    for (i = 0UL; i < 16UL; i++)
    {
        NVIC->ISER[i] = g_nvicIser[i];
    }
    for (i = 0UL; i < 496UL; i++)
    {
        NVIC->IPR[i] = g_nvicIp[i];
    }
}

static void APP_SetPowerDownModeWakeupConfig(void)
{
    /* The first word from wakeup address must be SP. */
    g_wakeupEntry[0] = ((uint32_t)&m_warmboot_stack_end);
    /* The second word from wakeup address must be PC. */
    g_wakeupEntry[1] = (uint32_t)APP_WakeupFunction;

    for (uint32_t i = 2UL; i < 12UL; i++)
    {
        g_wakeupEntry[i] = i;
    }
    /* Set wake up address. */
    SPC_SetWakeUpValue(APP_SPC, (uint32_t)g_wakeupEntry);

    uint32_t crcResult;
    crc_config_t config;
    config.polynomial         = 0x04C11DB7U;
    config.seed               = 0xFFFFFFFFU;
    config.reflectIn          = false;
    config.reflectOut         = false;
    config.complementChecksum = false;
    config.crcBits            = kCrcBits32;
    config.crcResult          = kCrcFinalChecksum;

    CLOCK_DisableClock(kCLOCK_Crc0);
    CRC_Init(CRC0, &config);
    CRC_WriteData(CRC0, (const uint8_t *)((uint32_t)g_wakeupEntry), 48);
    crcResult        = CRC_Get32bitResult(CRC0);
    REGFILE1->REG[0] = crcResult;
}

static void APP_WakeupFunction(void)
{
    uint32_t ramAddress = 0x4000000UL;
    /* Please note that RAM ECC is enabled in KW45B41Z A1.
     * After waking up from low power modes that RAM blocks are powered off,
     * corresponding RAM blocks should be re-initialized.
     */
    while (ramAddress < 0x4004000UL)
    {
        *(uint32_t *)ramAddress = 0UL;
        ramAddress = ramAddress + 4UL;
    }

    ramAddress = 0x20004000UL;
    while (ramAddress < 0x20010000UL)
    {
        *(uint32_t *)ramAddress = 0UL;
        ramAddress = ramAddress + 4UL;
    }

    WDOG0->CS    = (uint32_t)((WDOG0->CS) & ~WDOG_CS_EN_MASK);
    WDOG0->TOVAL = 0xFFFF;
    WDOG0->CS    = (uint32_t)(((WDOG0->CS) & ~WDOG_CS_EN_MASK) | WDOG_CS_UPDATE_MASK);
    WDOG0->TOVAL = 0xFFFF;
    WDOG0->CNT   = 0xD928C520U;

#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2));    /* set CP10, CP11 Full Access in Secure mode */
#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    SCB_NS->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10, CP11 Full Access in Non-secure mode */
#endif                                                    /* (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U) */
#endif                                                    /* ((__FPU_PRESENT == 1) && (__FPU_USED == 1)) */

    /* Restore the CONTROL register. */
    __ASM volatile ("MSR control, %0" : : "r" (g_cpuControl) : "memory");
    __ISB();
    longjmp(g_coreContext, true);
}
