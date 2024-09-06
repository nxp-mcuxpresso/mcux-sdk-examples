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

#include "fsl_power.h"
#include "fsl_mu.h"
#include "pmic_support.h"
#include "fsl_iopctl.h"
#include "fsl_irtc.h"
#include "power_demo_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_NORMAL_RUN_VOLT    900000U /* The VDD1 voltage during normal run. */
#define DEMO_LOW_POWER_RUN_VOLT 700000U /* The VDD1 voltage during low power run. Used for CPU1 DS while CPU0 active.*/
#define APP_POWER_NAME                                                                           \
    {                                                                                            \
        "Sleep", "Deep Sleep", "Deep Sleep Retention", "Deep Power Down", "Full Deep Power Down" \
    }
#define APP_TARGET_POWER_NUM (5U)

#define APP_SLEEP_CONSTRAINTS                                                                                     \
    8U, PM_RESC_SENSEP_MAINCLK_ON, PM_RESC_SENSES_MAINCLK_ON, PM_RESC_FRO2_ON, PM_RESC_FRO2_EN, PM_RESC_LPOSC_ON, \
        PM_RESC_SRAM24_128KB_ACTIVE, PM_RESC_SRAM25_128KB_ACTIVE, PM_RESC_SRAM26_512KB_ACTIVE

#define APP_DEEP_SLEEP_CONSTRAINTS \
    3U, PM_RESC_SRAM24_128KB_RETENTION, PM_RESC_SRAM25_128KB_RETENTION, PM_RESC_SRAM26_512KB_RETENTION

#define APP_DSR_CONSTRAINTS APP_DEEP_SLEEP_CONSTRAINTS

#define APP_DEEP_POWER_DOWN_CONSTRAINTS 0U

#define APP_FULL_DEEP_POWER_DOWN_CONSTRAINTS 0U

#define APP_USER_WAKEUP_KEY_GPIO BOARD_SW6_GPIO
#define APP_USER_WAKEUP_KEY_PIN  BOARD_SW6_GPIO_PIN
#define APP_SW_IRQ_HANDLER       GPIO80_IRQHandler
#define APP_SW_IRQN              GPIO80_IRQn
#define APP_SW_RESET_RSTn        kGPIO8_RST_SHIFT_RSTn
#define APP_SW_CLOCK_EN          kCLOCK_Gpio8
#define APP_SW_LP_REQ            kPower_GPIO8_LPREQ

#define APP_RTC             RTC1
#define APP_RTC_IRQN        RTC1_IRQn
#define APP_RTC_IRQ_HANDLER RTC1_IRQHandler


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitPowerConfig(void);
status_t APP_ControlCallback_notify(pm_event_type_t eventType, uint8_t powerState, void *data);
void APP_InitWakeupSource(void);
void APP_RegisterNotify(void);
uint32_t APP_GetWakeupTimeout(void);
void APP_SetConstraints(uint8_t powerMode);
void APP_ReleaseConstraints(uint8_t powerMode);
void BOARD_RestorePeripheralsAfterDSR(void);

static uint8_t APP_GetTargetPowerMode(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern pm_handle_t g_pmHandle;
extern uint8_t g_targetPowerMode;

AT_ALWAYS_ON_DATA(pm_wakeup_source_t g_UserkeyWakeupSource);
AT_ALWAYS_ON_DATA(pm_wakeup_source_t g_timerWakeupSource);

AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t g_notify1) = {
    .notifyCallback = APP_ControlCallback_notify,
    .data           = NULL,
};
AT_ALWAYS_ON_DATA(pm_handle_t g_pmHandle);
AT_ALWAYS_ON_DATA(uint8_t g_targetPowerMode);
AT_ALWAYS_ON_DATA(uint32_t g_irqMask);
static const char *const g_targetPowerNameArray[APP_TARGET_POWER_NUM] = APP_POWER_NAME;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*! @brief Decrease the CPU frequency and supply voltage for lower power consumption.
 */
static inline void BOARD_PrepareForDS(void)
{
    /* Change to a lower frequency to safely decrease the VDD1 voltage when CPU1 enter low power mode but CPU0 is active
     * and still requires sense shared main clock. */
    CLOCK_AttachClk(kLPOSC_to_SENSE_BASE);
    CLOCK_AttachClk(kSENSE_BASE_to_SENSE_MAIN);
    CLOCK_EnableFroClkOutput(FRO2, kCLOCK_FroDiv6OutEn); /* Need Keep DIV6. */
#if defined(DEMO_POWER_SUPPLY_OPTION) && (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
    BOARD_SetPmicVdd1Voltage(DEMO_LOW_POWER_RUN_VOLT);
#endif
#if (defined(BOARD_PMIC_CONFIG_USE_SEMA4) && (BOARD_PMIC_CONFIG_USE_SEMA4 != 0U))
    CLOCK_DisableClock(kCLOCK_Sema420);
#endif
    CLOCK_DisableClock(kCLOCK_LPI2c15);

    /* Low power handshake to for async interrupt. */
    if (POWER_ModuleEnterLPRequest(APP_SW_LP_REQ) != kStatus_Success)
    {
         assert(false);
    }
}

/*! @brief Decrease the CPU frequency and supply voltage for lower power consumption.
 */
static inline void BOARD_RestoreAfterDS(void)
{
#if (defined(BOARD_PMIC_CONFIG_USE_SEMA4) && (BOARD_PMIC_CONFIG_USE_SEMA4 != 0U))
    CLOCK_EnableClock(kCLOCK_Sema420);
#endif
    CLOCK_EnableClock(kCLOCK_LPI2c15);
#if defined(DEMO_POWER_SUPPLY_OPTION) && (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
    BOARD_SetPmicVdd1Voltage(DEMO_NORMAL_RUN_VOLT);
#endif
    CLOCK_EnableFroClkOutput(FRO2, kCLOCK_FroDiv1OutEn | kCLOCK_FroDiv3OutEn | kCLOCK_FroDiv6OutEn);
    CLOCK_AttachClk(kFRO2_DIV1_to_SENSE_MAIN);
    CLOCK_AttachClk(kFRO2_DIV3_to_SENSE_BASE);
}

void BOARD_RestorePeripheralsAfterDSR(void)
{
    BOARD_InitDebugConsole();
}

void APP_PrepareForSleep(uint8_t powerState)
{
  switch(powerState)
  {
        case PM_LP_STATE_SLEEP:
        {
               break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            BOARD_PrepareForDS();
            break;
        }
        case PM_LP_STATE_DSR:
        {
            BOARD_PrepareForDS();
            break;
        }
        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
              /* Need keep sense shared main clock in case CPU0 enters power down mode after CPU1. */
            CLOCK_AttachClk(kLPOSC_to_SENSE_BASE);
            CLOCK_AttachClk(kSENSE_BASE_to_SENSE_MAIN);
#if (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
            BOARD_SetPmicVoltageBeforeDeepPowerDown();
#endif
            break;
        }
        case PM_LP_STATE_FULL_DEEP_POWER_DOWN:
        {
              /* Need keep sense shared main clock in case CPU0 enters power down mode after CPU1. */
            CLOCK_AttachClk(kLPOSC_to_SENSE_BASE);
            CLOCK_AttachClk(kSENSE_BASE_to_SENSE_MAIN);
#if (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
            BOARD_SetPmicVoltageBeforeDeepPowerDown();
#endif
            break;
        }
       default:
        /*No action need to be done. */
        break;
    }
}

void APP_RestoreAfterSleep(uint8_t powerState)
{
  switch(powerState)
  {
        case PM_LP_STATE_SLEEP:
        {
               break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            BOARD_RestoreAfterDS();
            break;
        }
        case PM_LP_STATE_DSR:
        {
            BOARD_RestoreAfterDS();
            BOARD_RestorePeripheralsAfterDSR();
            break;
        }

       default:
        /*No action need to be done. */
        break;
    }
}

status_t APP_ControlCallback_notify(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (eventType == kPM_EventEnteringSleep)
    {
        PRINTF("APP_ControlCallback ENTERING LP mode %d -- Notify\r\n", powerState);
        APP_PrepareForSleep(powerState);
    }
    else
    {
        APP_RestoreAfterSleep(powerState);
        PRINTF("APP_ControlCallback EXITING LP mode %d -- Notify\r\n", powerState);

    }

    return kStatus_Success;
}

void APP_SW_IRQ_HANDLER(void)
{
    POWER_ModuleExitLPRequest(APP_SW_LP_REQ); /* Clear the low power request before accessing the GPIO. */
    GPIO_GpioClearInterruptFlags(APP_USER_WAKEUP_KEY_GPIO, 1U << APP_USER_WAKEUP_KEY_PIN);
}

void APP_RTC_IRQ_HANDLER(void)
{
    uint32_t flags = IRTC_GetStatusFlags(APP_RTC);
    if (0U != flags)
    {
         /* Unlock to allow register write operation */
        IRTC_SetWriteProtection(APP_RTC, false);
        /*Clear all irtc flag */
        IRTC_ClearStatusFlags(APP_RTC, flags);
    }
}

void APP_StartTimer(uint64_t timeOutTickes)
{
    if(timeOutTickes != 0U)
    {
        IRTC_SetWriteProtection(APP_RTC, false);
        IRTC_ClearStatusFlags(APP_RTC, kIRTC_WakeTimerFlag);
        IRTC_EnableInterrupts(APP_RTC, kIRTC_WakeTimerInterruptEnable);
        IRTC_SetWakeupCount(APP_RTC, true, (timeOutTickes + 1023UL) / 1024UL);
    }
}

void APP_StopTimer(void)
{
    IRTC_SetWriteProtection(APP_RTC, false);
    IRTC_DisableInterrupts(APP_RTC, kIRTC_WakeTimerInterruptEnable);
}

void APP_InitWakeupSource(void)
{
    gpio_pin_config_t gpioPinConfigStruct;
    irtc_config_t irtcConfig;

    PM_InitWakeupSource(&g_UserkeyWakeupSource, (uint32_t)APP_SW_IRQN, NULL, true);
    PM_InitWakeupSource(&g_timerWakeupSource, (uint32_t)APP_RTC_IRQN, NULL, true);
    PM_RegisterTimerController(&g_pmHandle, APP_StartTimer, APP_StopTimer, NULL, NULL);

    /* Set SW pin as GPIO input. */
    gpioPinConfigStruct.pinDirection = kGPIO_DigitalInput;
    gpioPinConfigStruct.outputLogic  = 0U;

    RESET_ClearPeripheralReset(APP_SW_RESET_RSTn);
    CLOCK_EnableClock(APP_SW_CLOCK_EN);

    GPIO_SetPinInterruptConfig(APP_USER_WAKEUP_KEY_GPIO, APP_USER_WAKEUP_KEY_PIN, kGPIO_InterruptFallingEdge);
    GPIO_PinInit(APP_USER_WAKEUP_KEY_GPIO, APP_USER_WAKEUP_KEY_PIN, &gpioPinConfigStruct);

    IRTC_GetDefaultConfig(&irtcConfig);
    if (IRTC_Init(APP_RTC, &irtcConfig) != kStatus_Success)
    {
        PRINTF("RTC Init Failed.\r\n");
    }
}

uint32_t APP_GetWakeupTimeout(void)
{
    uint8_t timeout;
    uint32_t timeoutTicks = 0U;

    if (g_targetPowerMode < PM_LP_STATE_DSR)
    {
        PRINTF("\r\n Press User button(SW6) to wakeup from Sleep or DeepSleep mode.\r\n");
        timeoutTicks = 0U;
    }
    else if (g_targetPowerMode == PM_LP_STATE_DSR)
    {
        while (1)
        {
            PRINTF("\r\n Select the wake up timeout in seconds.");
            PRINTF("\r\n The allowed range is 0s - 9s.");
            PRINTF("\r\n Eg. enter 0 means no timeout and need reset to wakeup.");
            PRINTF("\r\n Eg. enter 5 to wake up in 5 seconds.");
            PRINTF("\r\n Waiting for input timeout value...");

            timeout = GETCHAR();
            if ((timeout >= '0') && (timeout <= '9'))
            {
                timeout -= '0';
                if (timeout != 0U)
                {
                    PRINTF("\r\n Will wakeup in %d seconds.", timeout);
                    timeoutTicks = (timeout * 1000000UL);
                }
                else
                {
                    timeoutTicks = 0U;
                }
                return timeoutTicks;
            }
            PRINTF("\r\n Wrong value!");
        }
    }
    else
    {
        PRINTF("\r\n The chip enters Deep Power Down or Full Deep Power Down mode only when both cores requested the mode.\r\n");
        PRINTF(" Use reset button to wakeup from Deep Power Down or Full Deep Power Down.\r\n\r\n");
    }

    return timeoutTicks;
}

void APP_RegisterNotify(void)
{
    PM_RegisterNotify(kPM_NotifyGroup2, &g_notify1);
}

void APP_SetConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case PM_LP_STATE_SLEEP:
        {
            PM_SetConstraints(PM_LP_STATE_SLEEP, APP_SLEEP_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            PM_SetConstraints(PM_LP_STATE_DEEP_SLEEP, APP_DEEP_SLEEP_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DSR:
        {
            PM_SetConstraints(PM_LP_STATE_DSR, APP_DSR_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
            PM_SetConstraints(PM_LP_STATE_DEEP_POWER_DOWN, APP_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_FULL_DEEP_POWER_DOWN:
        {
            PM_SetConstraints(PM_LP_STATE_FULL_DEEP_POWER_DOWN, APP_FULL_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        default:
        {
            /* This branch will never be hit. */
            break;
        }
    }
}

void APP_ReleaseConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case PM_LP_STATE_SLEEP:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SLEEP, APP_SLEEP_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            PM_ReleaseConstraints(PM_LP_STATE_DEEP_SLEEP, APP_DEEP_SLEEP_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DSR:
        {
            PM_ReleaseConstraints(PM_LP_STATE_DSR, APP_DSR_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
            PM_ReleaseConstraints(PM_LP_STATE_DEEP_POWER_DOWN, APP_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_FULL_DEEP_POWER_DOWN:
        {
            PM_ReleaseConstraints(PM_LP_STATE_FULL_DEEP_POWER_DOWN, APP_FULL_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        default:
        {
            /* This branch should never be hit. */
            break;
        }
    }
}

void BOARD_NotifyBoot(void)
{
    RESET_ClearPeripheralReset(kMU1_RST_SHIFT_RSTn);
    MU_Init(MU1_MUB);
    MU_SetFlags(MU1_MUB, BOOT_FLAG);
    MU_Deinit(MU1_MUB);
}

void BOARD_InitPowerConfig(void)
{
    /* Enable the used modules in sense side. */
    POWER_DisablePD(kPDRUNCFG_SHUT_SENSEP_MAINCLK);
    POWER_DisablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK);
    POWER_DisablePD(kPDRUNCFG_GATE_FRO2);
    POWER_DisablePD(kPDRUNCFG_PD_FRO2);
    POWER_DisablePD(kPDRUNCFG_PD_LPOSC); /* Used by RTC. */

    SYSCON3->SENSE_AUTOGATE_EN = 0x3U;
    CLOCK_EnableClock(kCLOCK_Cpu1); /*Let CPU1 control it's clock. */

    /* Disable unused clock. */
    CLOCK_DisableClock(kCLOCK_Glikey1);
    CLOCK_DisableClock(kCLOCK_Glikey2);
    CLOCK_DisableClock(kCLOCK_Glikey4);
    CLOCK_DisableClock(kCLOCK_Glikey5);
    CLOCK_DisableClock(kCLOCK_SenseAccessRamArbiter0);
    CLOCK_DisableClock(kCLOCK_MediaAccessRamArbiter1);
    CLOCK_AttachClk(kNONE_to_SYSTICK);
    CLOCK_AttachClk(kNONE_to_MICFIL0);

    /* Disable unused modules. */
    POWER_EnablePD(kPDRUNCFG_SHUT_RAM0_CLK);
    POWER_EnablePD(kPDRUNCFG_SHUT_COMNN_MAINCLK);
    POWER_EnablePD(kPDRUNCFG_SHUT_MEDIA_MAINCLK);
    POWER_EnablePD(kPDRUNCFG_PD_SYSXTAL);
    POWER_EnablePD(kPDRUNCFG_PD_PLLANA);
    POWER_EnablePD(kPDRUNCFG_PD_PLLLDO);
    POWER_EnablePD(kPDRUNCFG_PD_AUDPLLANA);
    POWER_EnablePD(kPDRUNCFG_PD_AUDPLLLDO);
    POWER_EnablePD(kPDRUNCFG_PD_ADC0);
    POWER_EnablePD(kPDRUNCFG_SHUT_RAM1_CLK); /* Compute access RAM arbiter1 clock. */
    POWER_EnablePD(kPDRUNCFG_LP_DCDC);
    PMC1->PDRUNCFG1 = 0x7FFFFFFFU;
    PMC1->PDRUNCFG2 &= ~(0x3FFC0000U); /* Power up all the SRAM partitions in Sense domain. */
    PMC1->PDRUNCFG3 &= ~(0x3FFC0000U);
    POWER_EnablePD(kPDRUNCFG_PPD_OCOTP);
    POWER_ApplyPD();

    /* Request the domains out of sense into RBB mode. */
    /* POWER_EnableRunAFBB(kPower_BodyBiasVdd1); */
    /* POWER_EnableRunNBB(kPower_BodyBiasVdd1Sram); */
    POWER_EnableRunRBB(kPower_BodyBiasVdd2 | kPower_BodyBiasVddn | kPower_BodyBiasVdd2Sram);
    POWER_EnableSleepRBB(kPower_BodyBiasVdd2 | kPower_BodyBiasVddn | kPower_BodyBiasVdd2Sram | kPower_BodyBiasVdd1 |
                         kPower_BodyBiasVdd1Sram);
    POWER_ApplyPD();

    power_regulator_voltage_t ldo = {
        .LDO.vsel0 = 700000,  /* 0x14: 700mv, 0.45 V + 12.5 mV * x */
        .LDO.vsel1 = 800000,  /* 0x1C: 800mv*/
        .LDO.vsel2 = 900000,  /* 0x24: 900mv */
        .LDO.vsel3 = 1000000, /* 0x2C: 1000mv */
    };

    power_lvd_voltage_t lvd = {
        .VDD12.lvl0 = 500000, /* 500mv */
        .VDD12.lvl1 = 600000, /* 600mv */
        .VDD12.lvl2 = 700000, /* 700mv */
        .VDD12.lvl3 = 800000, /* 800mv */
    };

    POWER_ConfigRegulatorSetpoints(kRegulator_Vdd1LDO, &ldo, &lvd);

#if defined(DEMO_POWER_SUPPLY_OPTION) && (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_MIXED)
    POWER_SelectRunSetpoint(kRegulator_Vdd1LDO, 2U);
    POWER_SelectSleepSetpoint(kRegulator_Vdd1LDO, 0U);
    POWER_SelectRunSetpoint(kRegulator_Vdd2LDO, 0U);
    POWER_SelectSleepSetpoint(kRegulator_Vdd2LDO, 0U);
    POWER_ApplyPD();
#elif defined(DEMO_POWER_SUPPLY_OPTION) && (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
    POWER_DisableLPRequestMask(kPower_MaskLpi2c15);
    BOARD_InitPmic();
    /* Select the lowest LVD setpoint. */
    POWER_SelectRunSetpoint(kRegulator_Vdd2LDO, 0U);
    POWER_SelectSleepSetpoint(kRegulator_Vdd2LDO, 0U);
    POWER_SelectRunSetpoint(kRegulator_Vdd1LDO, 0U);
    POWER_SelectSleepSetpoint(kRegulator_Vdd1LDO, 0U);
    POWER_ApplyPD();

    BOARD_SetPmicVdd1Voltage(900000U);
#endif

#if (DEMO_POWER_ENABLE_DEBUG == 0U)
    CLOCK_DisableClock(kCLOCK_Dbg);
#endif
}

/* Set IO pads to default. */
void BOARD_DisableIoPads(void)
{
    uint8_t port, pin;

    RESET_ClearPeripheralReset(kIOPCTL1_RST_SHIFT_RSTn);
    CLOCK_EnableClock(kCLOCK_Iopctl1);

    port = 8;
    pin  = 5U; /* Keep JTAG pin unchanged. */

    for (; pin <= 31U; pin++)
    {
        IOPCTL_PinMuxSet(port, pin, 0U);
    }

    port = 9U;
    for (pin = 0U; pin <= 2U; pin++)
    {
        IOPCTL_PinMuxSet(port, pin, 0U);
    }

    port = 10U;
    for (pin = 0U; pin <= 17U; pin++)
    {
        IOPCTL_PinMuxSet(port, pin, 0U);
    }
}


int main(void)
{
    uint32_t timeoutUs = 0UL;

    BOARD_DisableIoPads();
    POWER_DisablePD(kPDRUNCFG_PD_FRO2); /* Sense uses FRO2. */
    BOARD_InitPins();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();

    BOARD_InitPowerConfig();

    BOARD_NotifyBoot(); /* Set boot flag. */

    if ((POWER_GetEventFlags() & PMC_FLAGS_DEEPPDF_MASK) != 0)
    {
        PRINTF("Board wake up from deep or full deep power down mode.\r\n");
        POWER_ClearEventFlags(PMC_FLAGS_DEEPPDF_MASK);
    }
    POWER_ClearEventFlags(0xFFFFFFFF);
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
