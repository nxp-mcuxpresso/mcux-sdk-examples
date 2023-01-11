/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
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
#include "fsl_inputmux.h"
#include "fsl_pint.h"
#include "fsl_usart.h"
#include "pmic_support.h"

#include "fsl_pca9420.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * The option is used for measurement of the deep sleep wake-up latency. When the application calls
 * POWER_EnterDeepSleep(), the power library will control the PLL power depending on the
 * exclude_from_pd parameter and the PLL running status. If the PLL is not in the exclude_from_pd list
 * and is running, it will be powered off before WFI and restored after WFI by the power library. Otherwise,
 * the PLL control will not be changed in POWER_EnterDeepSleep().
 * As the PLL initialization costs time to stabilize the clock, user will see the duration of
 * POWER_EnterDeepSleep() is longer than expectation.
 * To get rid of the PLL clock initialization time from deep sleep wake-up latency measurement, we can set
 * POWER_DOWN_PLL_BEFORE_DEEP_SLEEP to 1, then the demo itself will disable the PLL before
 * calling POWER_EnterDeepSleep(), and restore PLL after that. Thus we get the real duration of system
 * deep sleep wake-up time.
 */
#define POWER_DOWN_PLL_BEFORE_DEEP_SLEEP \
    0 /* By default, we keep the application as simple as possible to make good OOBE. */

#define APP_USART_RX_ERROR               kUSART_RxError
#define APP_USER_WAKEUP_KEY_NAME         "SW2"
#define APP_USER_WAKEUP_KEY_GPIO         GPIO
#define APP_USER_WAKEUP_KEY_PORT         0
#define APP_USER_WAKEUP_KEY_PIN          10
#define APP_USER_WAKEUP_KEY_INPUTMUX_SEL kINPUTMUX_GpioPort0Pin10ToPintsel
/*!< Power down all unnecessary blocks and enable RBB during deep sleep. */
#define APP_DEEPSLEEP_RUNCFG0 (SYSCTL0_PDRUNCFG0_RBBSRAM_PD_MASK | SYSCTL0_PDRUNCFG0_RBB_PD_MASK)
#define APP_DEEPSLEEP_RAM_APD 0xFFC00000U /* 0x280000 - 0x4FFFFF keep powered */
#define APP_DEEPSLEEP_RAM_PPD 0x0U
#define APP_EXCLUDE_FROM_DEEPSLEEP                                                                           \
    (((const uint32_t[]){APP_DEEPSLEEP_RUNCFG0,                                                              \
                         (SYSCTL0_PDSLEEPCFG1_FLEXSPI0_SRAM_APD_MASK | SYSCTL0_PDSLEEPCFG1_SRAM_SLEEP_MASK), \
                         APP_DEEPSLEEP_RAM_APD, APP_DEEPSLEEP_RAM_PPD}))

#define APP_EXCLUDE_FROM_DEEP_POWERDOWN      (((const uint32_t[]){0, 0, 0, 0}))
#define APP_EXCLUDE_FROM_FULL_DEEP_POWERDOWN (((const uint32_t[]){0, 0, 0, 0}))
#define DEMO_IS_XIP_FLEXSPI0()                                                                                  \
    ((((uint32_t)BOARD_InitDebugConsole >= 0x08000000U) && ((uint32_t)BOARD_InitDebugConsole < 0x10000000U)) || \
     (((uint32_t)BOARD_InitDebugConsole >= 0x18000000U) && ((uint32_t)BOARD_InitDebugConsole < 0x20000000U)))
const char *gWakeupInfoStr[] = {"Sleep [Press the user key to wakeup]", "Deep Sleep [Press the user key to wakeup]",
#if (defined(FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE) && FSL_FEATURE_SYSCON_HAS_POWERDOWN_MODE)
                                "Powerdown [Reset to wakeup]", "Deep Powerdown [Reset to wakeup]"};
#else
                                "Deep Powerdown [Reset to wakeup]", "Full Deep Powerdown [Reset to wakeup]"};
#endif
uint32_t gCurrentPowerMode;
volatile bool pintFlag = false;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_ConfigPMICModes(pca9420_modecfg_t *cfg, uint32_t num);
#if POWER_DOWN_PLL_BEFORE_DEEP_SLEEP
void BOARD_DisablePll();
void BOARD_RestorePll();
#endif
static uint32_t APP_GetUserSelection(void);
static void APP_InitWakeupPin(void);
static void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*PLL status*/
extern const clock_sys_pll_config_t g_sysPllConfig_BOARD_BootClockRUN;
extern const clock_audio_pll_config_t g_audioPllConfig_BOARD_BootClockRUN;
AT_QUICKACCESS_SECTION_CODE(void BOARD_SetFlexspiClock(FLEXSPI_Type *base, uint32_t src, uint32_t divider));


void BOARD_ConfigPMICModes(pca9420_modecfg_t *cfg, uint32_t num)
{
    assert(cfg);

    /* Configuration PMIC mode to align with power lib like below:
     *  0b00    run mode, no special.
     *  0b01    deep sleep mode, vddcore 0.6V.
     *  0b10    deep powerdown mode, vddcore off.
     *  0b11    full deep powerdown mode vdd1v8 and vddcore off. */

    /* Mode 1: VDDCORE 0.6V. */
    cfg[1].sw1OutVolt = kPCA9420_Sw1OutVolt0V600;
    /* Mode 2: VDDCORE off. */
    cfg[2].enableSw1Out = false;

    /* Mode 3: VDDCORE, VDD1V8 and VDDIO off. */
    cfg[3].enableSw1Out  = false;
    cfg[3].enableSw2Out  = false;
    cfg[3].enableLdo2Out = false;
}

#if POWER_DOWN_PLL_BEFORE_DEEP_SLEEP
void BOARD_DisablePll(void)
{
    if (DEMO_IS_XIP_FLEXSPI0())
    {
        /* Let FlexSPI run on FRO. */
        BOARD_SetFlexspiClock(FLEXSPI0, 3U, 2U);
    }
    /* Let CPU run on FRO div 2 (96MHz) before power down SYS PLL. */
    CLOCK_AttachClk(kFRO_DIV1_to_MAIN_CLK);
    /* Disable the PFD clock output first. */
    CLOCK_DeinitSysPfd(kCLOCK_Pfd0);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd2);
    CLOCK_DeinitAudioPfd(kCLOCK_Pfd0);
    /* Disable PLL. */
    CLOCK_DeinitSysPll();
    CLOCK_DeinitAudioPll();
    /* Power down SysOsc */
    POWER_EnablePD(kPDRUNCFG_PD_SYSXTAL);
}

void BOARD_RestorePll(void)
{
    /* Power on SysOsc */
    POWER_DisablePD(kPDRUNCFG_PD_SYSXTAL);
    CLOCK_EnableSysOscClk(true, true, BOARD_SYSOSC_SETTLING_US);
    /*Restore PLL*/
    CLOCK_InitSysPll(&g_sysPllConfig_BOARD_BootClockRUN);
    CLOCK_InitAudioPll(&g_audioPllConfig_BOARD_BootClockRUN);
    /*Restore PFD*/
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 24);   /* Enable main PLL clock 396MHz. */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 24);   /* Enable aux0 PLL clock 396MHz for SDIO */
    CLOCK_InitAudioPfd(kCLOCK_Pfd0, 26); /* Configure audio_pll_clk to 368.64MHz */
    /* Let CPU run on SYS PLL PFD0 with divider 2 (250Mhz). */
    CLOCK_AttachClk(kMAIN_PLL_to_MAIN_CLK);
    if (DEMO_IS_XIP_FLEXSPI0())
    {
        /* Move FlexSPI clock source from FRO clock to Main clock. */
        BOARD_SetFlexspiClock(FLEXSPI0, 0U, 2U);
    }
}
#endif /* POWER_DOWN_PLL_BEFORE_DEEP_SLEEP */

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    pca9420_modecfg_t pca9420ModeCfg[4];
    uint32_t i;

    /* BE CAUTIOUS TO SET CORRECT VOLTAGE RANGE ACCORDING TO YOUR BOARD/APPLICATION. PAD SUPPLY BEYOND THE RANGE DO
       HARM TO THE SILICON. */
    power_pad_vrange_t vrange = {.Vdde0Range = kPadVol_171_198,
                                 .Vdde1Range = kPadVol_171_198,
                                 .Vdde2Range = kPadVol_171_198,
                                 .Vdde3Range = kPadVol_300_360,
                                 .Vdde4Range = kPadVol_171_198};

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Disable the clock for unused modules. */
    CLOCK_DisableClock(kCLOCK_RomCtrlr);
    CLOCK_DisableClock(kCLOCK_OtpCtrl);
    CLOCK_DisableClock(kCLOCK_Rng);
    CLOCK_DisableClock(kCLOCK_Puf);
    CLOCK_DisableClock(kCLOCK_HashCrypt);
    CLOCK_DisableClock(kCLOCK_Flexcomm14);
    CLOCK_AttachClk(kNONE_to_FLEXCOMM14);
    CLOCK_DisableClock(kCLOCK_Flexcomm2);
    CLOCK_AttachClk(kNONE_to_FLEXCOMM2);
    CLOCK_DisableClock(kCLOCK_Crc);

    if (!DEMO_IS_XIP_FLEXSPI0()) /* If not run XIP from FlexSPI flash, close FlexSPI clock for power saving.*/
    {
        BOARD_DeinitFlash(FLEXSPI0);
        CLOCK_DisableClock(kCLOCK_Flexspi0);
        CLOCK_AttachClk(kNONE_to_FLEXSPI0_CLK);
    }

    /* PMIC PCA9420 */
    BOARD_InitPmic();
    for (i = 0; i < ARRAY_SIZE(pca9420ModeCfg); i++)
    {
        PCA9420_GetDefaultModeConfig(&pca9420ModeCfg[i]);
    }
    BOARD_ConfigPMICModes(pca9420ModeCfg, ARRAY_SIZE(pca9420ModeCfg));
    PCA9420_WriteModeConfigs(&pca9420Handle, kPCA9420_Mode0, &pca9420ModeCfg[0], ARRAY_SIZE(pca9420ModeCfg));

    /* Configure PMIC Vddcore value according to CM33 clock. DSP not used in this demo. */
    BOARD_SetPmicVoltageForFreq(CLOCK_GetFreq(kCLOCK_CoreSysClk), 0U);

    /* Indicate to power library that PMIC is used. */
    POWER_UpdatePmicRecoveryTime(1);

    POWER_SetPadVolRange(&vrange);

    /* Determine the power mode before bring up. */
    if ((POWER_GetEventFlags() & PMC_FLAGS_DEEPPDF_MASK) != 0)
    {
        PRINTF("Board wake up from deep or full deep power down mode.\r\n");
        POWER_ClearEventFlags(PMC_FLAGS_DEEPPDF_MASK);
    }

    APP_InitWakeupPin();

    PRINTF("Power Manager Demo.\r\n");
    PRINTF("The \"user key\" is: %s\r\n", APP_USER_WAKEUP_KEY_NAME);

    while (1)
    {
        gCurrentPowerMode = APP_GetUserSelection();
        PRINTF("Entering %s ...\r\n", gWakeupInfoStr[gCurrentPowerMode]);
        pintFlag = false;
        /* Enter the low power mode. */
        switch (gCurrentPowerMode)
        {
            case kPmu_Sleep: /* Enter sleep mode. */
                POWER_EnterSleep();
                break;
            case kPmu_Deep_Sleep: /* Enter deep sleep mode. */
                BOARD_SetPmicVoltageBeforeDeepSleep();
#if POWER_DOWN_PLL_BEFORE_DEEP_SLEEP
                /* Disable Pll before enter deep sleep mode */
                BOARD_DisablePll();
#endif
                BOARD_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP);
#if POWER_DOWN_PLL_BEFORE_DEEP_SLEEP
                /* Restore Pll before enter deep sleep mode */
                BOARD_RestorePll();
#endif
                BOARD_RestorePmicVoltageAfterDeepSleep();
                break;
            case kPmu_Deep_PowerDown: /* Enter deep power down mode. */
                PRINTF(
                    "Press any key to confirm to enter the deep power down mode and wakeup the device by "
                    "reset.\r\n\r\n");
                GETCHAR();
                BOARD_SetPmicVoltageBeforeDeepPowerDown();
                BOARD_EnterDeepPowerDown(APP_EXCLUDE_FROM_DEEP_POWERDOWN);
                /* After deep power down wakeup, the code will restart and cannot reach here. */
                break;
            case kPmu_Full_Deep_PowerDown: /* Enter full deep power down mode. */
                PRINTF(
                    "Press any key to confirm to enter the full deep power down mode and wakeup the device by "
                    "reset.\r\n\r\n");
                GETCHAR();
                BOARD_SetPmicVoltageBeforeDeepPowerDown();
                POWER_EnterFullDeepPowerDown(APP_EXCLUDE_FROM_FULL_DEEP_POWERDOWN);
                /* After full deep power down wakeup, the code will restart and cannot reach here. */
                break;
            default:
                break;
        }
        if (pintFlag)
        {
            PRINTF("Pin event occurs\r\n");
        }
        PRINTF("Wakeup.\r\n");
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
    GPIO_PinInit(APP_USER_WAKEUP_KEY_GPIO, APP_USER_WAKEUP_KEY_PORT, APP_USER_WAKEUP_KEY_PIN, &gpioPinConfigStruct);

    /* Configure the Input Mux block and connect the trigger source to PinInt channle. */
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, APP_USER_WAKEUP_KEY_INPUTMUX_SEL); /* Using channel 0. */
    INPUTMUX_Deinit(INPUTMUX); /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */

    /* Configure the interrupt for SW pin. */
    PINT_Init(PINT);
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableFallEdge, pint_intr_callback);
    PINT_EnableCallback(PINT); /* Enable callbacks for PINT */

    EnableDeepSleepIRQ(PIN_INT0_IRQn);
}

/*
 * Callback function when wakeup key is pressed.
 */
static void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    pintFlag = true;
}

/*
 * Get user selection from UART.
 */
static uint32_t APP_GetUserSelection(void)
{
    uint32_t ch;

    /* Clear rx overflow error once it happens during low power mode. */
    if (APP_USART_RX_ERROR == (APP_USART_RX_ERROR & USART_GetStatusFlags((USART_Type *)BOARD_DEBUG_UART_BASEADDR)))
    {
        USART_ClearStatusFlags((USART_Type *)BOARD_DEBUG_UART_BASEADDR, APP_USART_RX_ERROR);
    }

    PRINTF(
        "Select an option\r\n"
        "\t1. Sleep mode\r\n"
        "\t2. Deep Sleep mode\r\n"
        "\t3. Deep power down mode\r\n"
        "\t4. Full deep power down mode\r\n");
    while (1)
    {
        ch = GETCHAR();
        if ((ch < '1') || (ch > '4')) /* Only '1', '2', '3' , '4'. */
        {
            continue;
        }
        else
        {
            ch = ch - '1'; /* Only 0, 1, 2 , 3 . */
            break;
        }
    }
    switch (ch)
    {
        case 0:
            ch = kPmu_Sleep;
            break;
        case 1:
            ch = kPmu_Deep_Sleep;
            break;
        case 2:
            ch = kPmu_Deep_PowerDown;
            break;
        case 3:
            ch = kPmu_Full_Deep_PowerDown;
            break;
        default:
            break;
    }
    return ch;
}
