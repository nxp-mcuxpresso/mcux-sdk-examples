/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_power.h"
#include "pmic_support.h"
#include "power_demo_config.h"
#include "fsl_lpuart.h"
#include "fsl_gpio.h"
#include "fsl_irtc.h"
#include "fsl_iopctl.h"

#include "fsl_mu.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_NORMAL_RUN_VOLT    900000U /* The VDD1 voltage during normal run. */
#define DEMO_LOW_POWER_RUN_VOLT 700000U /* The VDD1 voltage during low power run. Used for CPU1 DS while CPU0 active.*/
#ifndef DEMO_POWER_CPU1_PRINT_ENABLE
#define DEMO_POWER_CPU1_PRINT_ENABLE 1U
#endif
#if DEMO_POWER_CPU1_PRINT_ENABLE
#define DEMO_LOG PRINTF
#else
#define DEMO_LOG(...)
#endif

#define APP_CORE_NAME "CPU1"

#define APP_USER_WAKEUP_KEY_GPIO BOARD_SW6_GPIO
#define APP_USER_WAKEUP_KEY_PIN  BOARD_SW6_GPIO_PIN
#define APP_USER_WAKEUP_KEY_NAME "SW6"
#define APP_SW_IRQ_HANDLER       GPIO80_IRQHandler
#define APP_SW_IRQN              GPIO80_IRQn
#define APP_SW_RESET_RSTn        kGPIO8_RST_SHIFT_RSTn
#define APP_SW_CLOCK_EN          kCLOCK_Gpio8
#define APP_SW_LP_REQ            kPower_GPIO8_LPREQ

#define APP_RTC             RTC1
#define APP_RTC_IRQN        RTC1_ALARM_IRQn
#define APP_RTC_IRQ_HANDLER RTC1_ALARM_IRQHandler

#define APP_USART_RX_ERROR (kLPUART_RxOverrunFlag | kLPUART_RxFifoUnderflowFlag)

/*!< Power down all unnecessary blocks and enable RBB during deep sleep. */
#define APP_DEEPSLEEP_SLEEPCFG    (0U)        /* SLEEPCON->SLEEPCFG */
#define APP_DEEPSLEEP_PDSLEEPCFG0 (0U)        /* PMC->PDSLEEPCFG0 */
#define APP_DSR_PDSLEEPCFG0       (PMC_PDSLEEPCFG0_PMICMODE(2U))        /* PMC->PDSLEEPCFG0, override default PMIC mode. */
#define APP_DEEPSLEEP_RAM_APD     0x3FFC0000U /* PMC->PDSLEEPCFG2, 0x580000 - 0x67FFFF([PT18-PT29]) keep powered */
#define APP_DEEPSLEEP_RAM_PPD     0x0U        /* PMC->PDSLEEPCFG3 */
#define APP_EXCLUDE_FROM_DEEPSLEEP                                                                     \
    (((const uint32_t[]){APP_DEEPSLEEP_SLEEPCFG, APP_DEEPSLEEP_PDSLEEPCFG0, 0U, APP_DEEPSLEEP_RAM_APD, \
                         APP_DEEPSLEEP_RAM_PPD, 0U, 0U}))
#define APP_EXCLUDE_FROM_DSR                                                                     \
    (((const uint32_t[]){APP_DEEPSLEEP_SLEEPCFG, APP_DSR_PDSLEEPCFG0, 0U, APP_DEEPSLEEP_RAM_APD, \
                         APP_DEEPSLEEP_RAM_PPD, 0U, 0U}))
#define APP_EXCLUDE_FROM_DEEP_POWERDOWN      (((const uint32_t[]){0, PMC_PDSLEEPCFG0_PMICMODE(3U), 0, 0, 0, 0, 0})) /* Override default PMIC mode.*/
#define APP_EXCLUDE_FROM_FULL_DEEP_POWERDOWN (((const uint32_t[]){0, 0, 0, 0, 0, 0, 0}))
typedef enum _app_wakeup_source
{
    kAPP_WakeupSourcePin,   /*!< Wakeup by external pin. */
    kAPP_WakeupSourceRtc,   /*!< Wakeup by RTC.        */
    kAPP_WakeupSourceReset, /*!< Wakeup by Reset. */
} app_wakeup_source_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
const char *gWakeupInfoStr[] = {"Sleep", "Deep Sleep", "Deep Sleep Retention", "Deep Powerdown [Reset to wakeup]",
                                "Full Deep Powerdown [Reset to wakeup]"};
uint32_t gCurrentPowerMode;

static uint32_t s_wakeupTimeout;           /* Wakeup timeout. (Unit: seconds) */
static app_wakeup_source_t s_wakeupSource; /* Wakeup source. */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitPowerConfig(void);
void BOARD_RunActiveTest(void);
void BOARD_RestorePeripheralsAfterDSR(void);
void BOARD_EnterSleep(void);
void BOARD_EnterDeepSleep(const uint32_t exclude_from_pd[7]);
void BOARD_RequestDSR(const uint32_t exclude_from_pd[7]);
void BOARD_RequestDPD(const uint32_t exclude_from_pd[7]);
void BOARD_RequestFDPD(const uint32_t exclude_from_pd[7]);

static uint32_t APP_GetUserSelection(void);
static void APP_InitWakeupPin(void);
/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_RestorePeripheralsAfterDSR(void)
{
    BOARD_InitDebugConsole();
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
    POWER_EnableRunAFBB(kPower_BodyBiasVdd1);
    POWER_EnableRunNBB(kPower_BodyBiasVdd1Sram);
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


void BOARD_RunActiveTest(void)
{
    DEMO_LOG("\r\nThis test mode will keep CPU in run mode but close all other unused modules for a while.\n");
    DEMO_LOG("\r\nPlease don't input any charator until the mode finished.\n");

    DbgConsole_Deinit();

    /* Disable clocks - CLKCTL1(Sense private) */
    CLOCK_DisableClock(kCLOCK_Syscon1); /* CLKCTL1->PSCCTL0 */
    CLKCTL1->PSCCTL1 = 0U; /* Disable clock for INPUTMUX, WWDT2-3, MU3, SEMA42_3, UTICK1, MRT1, CTIMER5-7, PINT, GPIO,
                              FLEXCOMM, eDMA, HiFI1, SenseAccessRAM0. */

    /* Disable clocks - CLKCTL3(Sense shared) */
    CLOCK_DisableClock(kCLOCK_Mu1);
    CLOCK_DisableClock(kCLOCK_Iopctl1);
    CLOCK_DisableClock(kCLOCK_Syscon3);
    CLOCK_DisableClock(kCLOCK_Sema420);
    CLOCK_DisableClock(kCLOCK_LPI2c15);
    CLOCK_DisableClock(kCLOCK_Rtc);

    /* Disable clock slice. */
    CLOCK_AttachClk(kNONE_to_LPI2C15);
    CLOCK_AttachClk(kNONE_to_FLEXCOMM19);

    CLOCK_EnableFroClkOutput(FRO2, kCLOCK_FroDiv1OutEn);

    POWER_EnablePD(kPDRUNCFG_APD_OCOTP);
    /* Note, the debug will not work anymore when the sense shared mainclk is disabled. */
    POWER_EnablePD(kPDRUNCFG_PD_LPOSC);
    POWER_EnablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK);
    POWER_ApplyPD();
    CLOCK_DisableClock(kCLOCK_Sleepcon1);

    /* Simulate a task. */
    uint32_t i;
    for (i = 0U; i < 500U; i++)
    {
        SDK_DelayAtLeastUs(10000U, CLOCK_GetCoreSysClkFreq());
    }

    CLOCK_EnableClock(kCLOCK_Sleepcon1);
    CLOCK_EnableClock(kCLOCK_Syscon1); /* CLKCTL1->PSCCTL0 */
    CLOCK_EnableClock(kCLOCK_Iopctl1);
    POWER_DisablePD(kPDRUNCFG_PD_LPOSC);
    POWER_DisablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK);
    CLOCK_EnableFroClkOutput(FRO2, kCLOCK_FroDiv1OutEn | kCLOCK_FroDiv3OutEn | kCLOCK_FroDiv6OutEn);
    POWER_DisablePD(kPDRUNCFG_APD_OCOTP);
    POWER_ApplyPD();

    CLOCK_AttachClk(kSENSE_BASE_to_LPI2C15);
    CLOCK_AttachClk(kSENSE_BASE_to_FLEXCOMM19);

    /* Enable clocks - CLKCTL3(Sense shared) */
    CLOCK_EnableClock(kCLOCK_Mu1);
    CLOCK_EnableClock(kCLOCK_Syscon3);
    CLOCK_EnableClock(kCLOCK_Sema420);
    CLOCK_EnableClock(kCLOCK_LPI2c15);

    BOARD_InitDebugConsole();
}

void BOARD_EnterSleep(void)
{
    uint32_t irqMask;
    /* Disable clock for unused modules. */
    DbgConsole_Deinit();
    CLOCK_DisableClock(kCLOCK_Syscon1);
    CLOCK_DisableClock(kCLOCK_Iopctl1);
    CLOCK_DisableClock(kCLOCK_Mu1);
    CLOCK_DisableClock(kCLOCK_Syscon3);
    CLOCK_DisableClock(kCLOCK_Sema420);
    CLOCK_DisableClock(kCLOCK_LPI2c15);
    CLOCK_AttachClk(kLPOSC_to_SENSE_BASE); /* To disable FRO2 DIV3, switch sense base to LPOSC. */
    CLOCK_EnableFroClkOutput(FRO2, kCLOCK_FroDiv1OutEn);

    irqMask = DisableGlobalIRQ();
    POWER_EnablePD(kPDRUNCFG_PD_LPOSC);
    POWER_EnablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK);

    POWER_EnterSleep();

    POWER_DisablePD(kPDRUNCFG_PD_LPOSC);
    POWER_DisablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK);

    EnableGlobalIRQ(irqMask);
    __ISB();

    /* Re-enable clock for modules. */
    CLOCK_EnableClock(kCLOCK_Syscon1);
    CLOCK_EnableClock(kCLOCK_Iopctl1);
    CLOCK_EnableFroClkOutput(FRO2, kCLOCK_FroDiv1OutEn | kCLOCK_FroDiv3OutEn | kCLOCK_FroDiv6OutEn);
    CLOCK_AttachClk(kFRO2_DIV3_to_SENSE_BASE);
    CLOCK_EnableClock(kCLOCK_Mu1);
    CLOCK_EnableClock(kCLOCK_Syscon3);
    CLOCK_EnableClock(kCLOCK_Sema420);
    CLOCK_EnableClock(kCLOCK_LPI2c15);
    BOARD_InitDebugConsole();
}

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

void BOARD_EnterDeepSleep(const uint32_t exclude_from_pd[7])
{
    BOARD_PrepareForDS();
    POWER_EnterDeepSleep(exclude_from_pd);
    BOARD_RestoreAfterDS();
}

void BOARD_RequestDSR(const uint32_t exclude_from_pd[7])
{
    BOARD_PrepareForDS();
    POWER_RequestDSR(exclude_from_pd);
    BOARD_RestoreAfterDS();
}

void BOARD_RequestDPD(const uint32_t exclude_from_pd[7])
{
    /* Need keep sense shared main clock in case CPU0 enters power down mode after CPU1. */
    CLOCK_AttachClk(kLPOSC_to_SENSE_BASE);
    CLOCK_AttachClk(kSENSE_BASE_to_SENSE_MAIN);
    POWER_RequestDeepPowerDown(exclude_from_pd);
}

void BOARD_RequestFDPD(const uint32_t exclude_from_pd[7])
{
    CLOCK_AttachClk(kLPOSC_to_SENSE_BASE);
    CLOCK_AttachClk(kSENSE_BASE_to_SENSE_MAIN);
    POWER_RequestFullDeepPowerDown(exclude_from_pd);
}
void APP_SW_IRQ_HANDLER(void)
{
    POWER_ModuleExitLPRequest(APP_SW_LP_REQ); /* Clear the low power request before accessing the GPIO. */
    GPIO_GpioClearInterruptFlags(APP_USER_WAKEUP_KEY_GPIO, 1U << APP_USER_WAKEUP_KEY_PIN);
    DisableDeepSleepIRQ(APP_SW_IRQN);

    CLOCK_DisableClock(APP_SW_CLOCK_EN);
}

void APP_RTC_IRQ_HANDLER(void)
{
    uint32_t flags = IRTC_GetStatusFlags(APP_RTC);
    if ((flags & kIRTC_AlarmFlag) != 0U)
    {
        /* Unlock to allow register write operation */
        IRTC_SetWriteProtection(APP_RTC, false);
        /*Clear all irtc flag */
        IRTC_ClearStatusFlags(APP_RTC, flags);
    }

    DisableDeepSleepIRQ(APP_RTC_IRQN);
}

/*
 * Get user selection from UART.
 */
static uint32_t APP_GetUserSelection(void)
{
    uint32_t ch;

    /* Clear rx overflow error once it happens during low power mode. */
    if (APP_USART_RX_ERROR == (APP_USART_RX_ERROR & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
    {
        LPUART_ClearStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR, APP_USART_RX_ERROR);
    }

    DEMO_LOG(
        "Select an option\r\n"
        "\t1. Sleep mode\r\n"
        "\t2. Deep Sleep mode\r\n"
        "\t3. Deep Sleep Retention mode\r\n"
        "\t4. Deep power down mode\r\n"
        "\t5. Full deep power down mode\r\n"
        "\t6. Active test mode\r\n");
    while (1)
    {
        ch = GETCHAR();
        if ((ch < '1') || (ch > '6'))
        {
            continue;
        }
        else
        {
            ch = ch - '1';
            break;
        }
    }
    switch (ch)
    {
        case 0:
            ch = kPower_Sleep;
            break;
        case 1:
            ch = kPower_DeepSleep;
            break;
        case 2:
            ch = kPower_DeepSleepRetention;
            break;
        case 3:
            ch = kPower_DeepPowerDown;
            break;
        case 4:
            ch = kPower_FullDeepPowerDown;
            break;
        default:
            break;
    }
    return ch;
}

/* Get wakeup source by user input. */
static app_wakeup_source_t APP_GetWakeupSource(uint32_t mode)
{
    uint8_t ch;

    while (true)
    {
        DEMO_LOG("Select the wake up source:\r\n");
        if ((mode != kPower_DeepPowerDown) && (mode != kPower_FullDeepPowerDown))
        {
            DEMO_LOG("Press T for RTC.\r\n");
            if (mode != kPower_DeepSleepRetention)
            {
                DEMO_LOG("Press S for wakeup pin(%s).\r\n", APP_USER_WAKEUP_KEY_NAME);
            }
            DEMO_LOG("Press N for Reset.\r\n");
            DEMO_LOG("\r\nWaiting for key press..\r\n\r\n");
            ch = GETCHAR();
            DEMO_LOG("%c\r\n", ch);

            if ((ch >= 'a') && (ch <= 'z'))
            {
                ch -= 'a' - 'A';
            }

            if (ch == 'T')
            {
                return kAPP_WakeupSourceRtc;
            }
            else if ((ch == 'S') && (mode != kPower_DeepSleepRetention))
            {
                return kAPP_WakeupSourcePin;
            }
            else if (ch == 'N')
            {
                return kAPP_WakeupSourceReset;
            }
            else
            {
                DEMO_LOG("Wrong value!\r\n");
            }
        }
        else
        {
            DEMO_LOG("Reset to wakeup.\r\n");
            return kAPP_WakeupSourceReset;
        }
    }
}

/*!
 * @brief Get input from user about wakeup timeout
 */
static uint8_t APP_GetWakeupTimeout(void)
{
    uint8_t timeout;

    while (1)
    {
        DEMO_LOG("Select the wake up timeout in seconds.\r\n");
        DEMO_LOG("The allowed range is 1s ~ 9s.\r\n");
        DEMO_LOG("Eg. enter 5 to wake up in 5 seconds.\r\n");
        DEMO_LOG("\r\nWaiting for input timeout value...\r\n\r\n");

        timeout = GETCHAR();
        DEMO_LOG("%c\r\n", timeout);
        if ((timeout > '0') && (timeout <= '9'))
        {
            return timeout - '0';
        }
        DEMO_LOG("Wrong value!\r\n");
    }
}

void APP_GetWakeupConfig(uint32_t targetMode)
{
    /* Get wakeup source by user input. */
    s_wakeupSource = APP_GetWakeupSource(targetMode);

    if (kAPP_WakeupSourceRtc == s_wakeupSource)
    {
        /* Wakeup source is Timer, user should input wakeup timeout value. */
        s_wakeupTimeout = APP_GetWakeupTimeout();
    }
    else
    {
        s_wakeupTimeout = 0U;
    }
}

/*
 * Setup a GPIO input pin as wakeup source.
 */
static void APP_InitWakeupPin(void)
{
    gpio_pin_config_t gpioPinConfigStruct;

    /* Set SW pin as GPIO input. */
    gpioPinConfigStruct.pinDirection = kGPIO_DigitalInput;
    gpioPinConfigStruct.outputLogic  = 0U;

    RESET_ClearPeripheralReset(APP_SW_RESET_RSTn);
    CLOCK_EnableClock(APP_SW_CLOCK_EN);

    GPIO_SetPinInterruptConfig(APP_USER_WAKEUP_KEY_GPIO, APP_USER_WAKEUP_KEY_PIN, kGPIO_InterruptFallingEdge);
    GPIO_PinInit(APP_USER_WAKEUP_KEY_GPIO, APP_USER_WAKEUP_KEY_PIN, &gpioPinConfigStruct);
}

static void APP_InitWakeupTimer(void)
{
    irtc_config_t irtcConfig;

    IRTC_GetDefaultConfig(&irtcConfig);
    if (IRTC_Init(APP_RTC, &irtcConfig) != kStatus_Success)
    {
        DEMO_LOG("RTC Init Failed.\r\n");
    }
}

static void APP_SetWakeupConfig(void)
{
    irtc_datetime_t datetime;

    if (s_wakeupSource == kAPP_WakeupSourceRtc)
    {
        /* Read the RTC seconds register to get current time in seconds */
        IRTC_GetDatetime(APP_RTC, &datetime);
        /* Add alarm seconds to current time */
        datetime.second += s_wakeupTimeout;
        if (datetime.second > 59U)
        {
            datetime.minute++;
            datetime.second -= 60U;
        }
        /* Unlock to allow register write operation */
        IRTC_SetWriteProtection(APP_RTC, false);
        /* Set alarm time in seconds */
        IRTC_SetAlarm(APP_RTC, &datetime);
        /* Enable RTC alarm interrupt */
        IRTC_EnableInterrupts(APP_RTC, kIRTC_AlarmInterruptEnable);
        EnableDeepSleepIRQ(APP_RTC_IRQN);
        DEMO_LOG("RTC wake up after %d seconds.\r\n", s_wakeupTimeout);
    }
    else if (s_wakeupSource == kAPP_WakeupSourcePin)
    {
        APP_InitWakeupPin();
        EnableDeepSleepIRQ(APP_SW_IRQN);
        /* Low power handshake to for async interrupt. */
        if (POWER_ModuleEnterLPRequest(APP_SW_LP_REQ) != kStatus_Success)
        {
            assert(false);
        }
        DEMO_LOG("Push wakeup PIN to wake up.\r\n");
    }
    else
    {
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t freq = 0U;

    /* Init board hardware. */
    BOARD_DisableIoPads();
    POWER_DisablePD(kPDRUNCFG_PD_FRO2); /* Sense uses FRO2. */
    BOARD_InitPins();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();

    BOARD_InitPowerConfig();

    BOARD_NotifyBoot(); /* Set boot flag. */

    /* Determine the power mode before bring up. */
    if ((POWER_GetEventFlags() & PMC_FLAGS_DEEPPDF_MASK) != 0)
    {
        DEMO_LOG("Board wake up from deep or full deep power down mode.\r\n");
        POWER_ClearEventFlags(PMC_FLAGS_DEEPPDF_MASK);
    }
    POWER_ClearEventFlags(0xFFFFFFFF);

    APP_InitWakeupTimer();

    while (1)
    {
        freq = CLOCK_GetCoreSysClkFreq();

        DEMO_LOG("\r\n####################  Power Mode Switch - %s ####################\n\r\n", APP_CORE_NAME);
        DEMO_LOG("    Build Time: %s--%s \r\n", __DATE__, __TIME__);
        DEMO_LOG("    Core Clock: %dHz \r\n", freq);

        gCurrentPowerMode = APP_GetUserSelection();

        if (gCurrentPowerMode == 5U)
        {
            BOARD_RunActiveTest();
            continue;
        }

        APP_GetWakeupConfig(gCurrentPowerMode);
        APP_SetWakeupConfig();

        DEMO_LOG("Entering %s ...\r\n", gWakeupInfoStr[gCurrentPowerMode]);
        /* Enter the low power mode. */
        switch (gCurrentPowerMode)
        {
            case kPower_Sleep: /* Enter sleep mode. */
                BOARD_EnterSleep();
                break;
            case kPower_DeepSleep: /* Enter deep sleep mode. */
                BOARD_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP);
                break;
            case kPower_DeepSleepRetention:
                BOARD_RequestDSR(APP_EXCLUDE_FROM_DSR);
                BOARD_RestorePeripheralsAfterDSR(); /* Restore the peripherals whose states are not retained in DSR. */
                break;
            case kPower_DeepPowerDown: /* Request deep power down(DPD) mode. The SOC enters DPD only when both domains
                                          requested DPD mode. */
                DEMO_LOG(
                    "The chip enters DPD only when both domains request entering DPD mode. Wakeup the device by "
                    "reset\r\n\r\n");
#if (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
                BOARD_SetPmicVoltageBeforeDeepPowerDown();
#endif
                BOARD_RequestDPD(APP_EXCLUDE_FROM_DEEP_POWERDOWN);
                /* After deep power down wakeup, the code will restart and cannot reach here. */
                break;
            case kPower_FullDeepPowerDown: /* Request full deep power down(FDPD) mode. The SOC enters FDPD only when
                                              both domains requested FDPD mode. */
                DEMO_LOG(
                    "The chip enters FDPD only when both domains request entering FDPD mode. Wakeup the device by "
                    "reset\r\n\r\n");
#if (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
                BOARD_SetPmicVoltageBeforeDeepPowerDown();
#endif
                BOARD_RequestFDPD(APP_EXCLUDE_FROM_FULL_DEEP_POWERDOWN);
                /* After full deep power down wakeup, the code will restart and cannot reach here. */
                break;
            default:
                break;
        }

        DEMO_LOG("Wakeup.\r\n");
    }
}
