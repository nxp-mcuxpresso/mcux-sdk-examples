/*
 * Copyright 2018-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_ostimer.h"

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_OSTIMER_FREQ         CLOCK_GetLpOscFreq()
#define EXAMPLE_OSTIMER              OSTIMER_CPU0
#define EXAMPLE_OSTIMER_IRQn         OS_EVENT_IRQn
#define EXAMPLE_EnableDeepSleepIRQ() EnableDeepSleepIRQ(OS_EVENT_IRQn)

#define APP_DEEPSLEEP_SLEEPCFG    (SLEEPCON0_SLEEPCFG_LPOSC_PD_MASK)
#define APP_DEEPSLEEP_PDSLEEPCFG0 (0U)
#define APP_DEEPSLEEP_RAM_APD     0x1F00U /* 0x80000 - 0x2FFFFF([PT8-PT12]) keep powered */
#define APP_DEEPSLEEP_RAM_PPD     (0U)
#define APP_DEEPSLEEP_PDSLEEPCFG4 \
    (PMC_PDSLEEPCFG4_CPU0_CCACHE_MASK | PMC_PDSLEEPCFG4_CPU0_SCACHE_MASK | PMC_PDSLEEPCFG4_OCOTP_MASK)
#define APP_DEEPSLEEP_PDSLEEPCFG5 (0U)
#define APP_EXCLUDE_FROM_DEEPSLEEP                                                                     \
    (((const uint32_t[]){APP_DEEPSLEEP_SLEEPCFG, APP_DEEPSLEEP_PDSLEEPCFG0, 0U, APP_DEEPSLEEP_RAM_APD, \
                         APP_DEEPSLEEP_RAM_PPD, APP_DEEPSLEEP_PDSLEEPCFG4, APP_DEEPSLEEP_PDSLEEPCFG5}))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Enter deep sleep mode. */
void EXAMPLE_EnterDeepSleep(void);
/* Restore clock configuration. */
void BOARD_RestoreClockConfig(void);
/* Change to a safe clock source for power mode change. */
void BOARD_ClockSafeConfig(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern const clock_main_pll_config_t g_mainPllConfig_BOARD_BootClockRUN;
extern const clock_audio_pll_config_t g_audioPllConfig_BOARD_BootClockRUN;
volatile bool matchFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_DisablePll(void)
{
    /* Special sequence is needed for the PLL power up/initialization. The application should manually handle the states
   changes for the PLL if the PLL power states configuration are different in Active mode and Deep Sleep mode. */

    /* Disable the PFD clock output first. */
    CLOCK_DeinitMainPfd(kCLOCK_Pfd0);
    CLOCK_DeinitMainPfd(kCLOCK_Pfd1);
    CLOCK_DeinitMainPfd(kCLOCK_Pfd2);
    CLOCK_DeinitMainPfd(kCLOCK_Pfd3);

    CLOCK_DeinitAudioPfd(kCLOCK_Pfd1);
    CLOCK_DeinitAudioPfd(kCLOCK_Pfd3);
    /* Disable PLL. */
    CLOCK_DeinitMainPll();
    CLOCK_DeinitAudioPll();
}

void BOARD_RestorePll(void)
{
    /*Restore PLL*/
    CLOCK_InitMainPll(&g_mainPllConfig_BOARD_BootClockRUN);
    CLOCK_InitAudioPll(&g_audioPllConfig_BOARD_BootClockRUN);
    /*Restore PFD*/
    CLOCK_InitMainPfd(kCLOCK_Pfd0, 20U);
    CLOCK_InitMainPfd(kCLOCK_Pfd1, 24U);
    CLOCK_InitMainPfd(kCLOCK_Pfd2, 18U);
    CLOCK_InitMainPfd(kCLOCK_Pfd3, 19U);

    CLOCK_InitAudioPfd(kCLOCK_Pfd1, 24U);
    CLOCK_InitAudioPfd(kCLOCK_Pfd3, 26U);
}

/* Change main clock to a safe source. */
void BOARD_ClockSafeConfig(void)
{
    /* Switch to FRO1 for safe configure. */
    CLOCK_AttachClk(kFRO1_DIV1_to_COMPUTE_BASE);
    CLOCK_AttachClk(kCOMPUTE_BASE_to_COMPUTE_MAIN);
    CLOCK_SetClkDiv(kCLOCK_DivCmptMainClk, 1U);
    CLOCK_SetClkDiv(kCLOCK_DivComputeRamClk, 1U);
    CLOCK_AttachClk(kFRO1_DIV1_to_RAM);
    CLOCK_AttachClk(kFRO1_DIV1_to_COMMON_BASE);
    CLOCK_AttachClk(kCOMMON_BASE_to_COMMON_VDDN);
    CLOCK_SetClkDiv(kCLOCK_DivCommonVddnClk, 1U);

    CLOCK_AttachClk(kFRO1_DIV3_to_MEDIA_VDD2_BASE);
    CLOCK_AttachClk(kFRO1_DIV1_to_MEDIA_VDDN_BASE);
    CLOCK_AttachClk(kMEDIA_VDD2_BASE_to_MEDIA_MAIN);
    CLOCK_AttachClk(kMEDIA_VDDN_BASE_to_MEDIA_VDDN);

    BOARD_XspiClockSafeConfig(); /*Change to common_base. */

    BOARD_DisablePll();
}

/* Restore main clock. */
void BOARD_RestoreClockConfig(void)
{
    BOARD_RestorePll();

    CLOCK_SetClkDiv(kCLOCK_DivCmptMainClk, 2U);
    CLOCK_AttachClk(kMAIN_PLL_PFD0_to_COMPUTE_MAIN); /* Switch to PLL 230MHZ */
    CLOCK_SetClkDiv(kCLOCK_DivMediaMainClk, 2U);
    CLOCK_AttachClk(kMAIN_PLL_PFD0_to_MEDIA_MAIN);   /* Switch to PLL 230MHZ */
    CLOCK_SetClkDiv(kCLOCK_DivMediaVddnClk, 2U);
    CLOCK_AttachClk(kMAIN_PLL_PFD0_to_MEDIA_VDDN);   /* Switch to PLL 230MHZ */
    CLOCK_SetClkDiv(kCLOCK_DivComputeRamClk, 2U);
    CLOCK_AttachClk(kMAIN_PLL_PFD0_to_RAM);          /* Switch to PLL 230MHZ */
    CLOCK_SetClkDiv(kCLOCK_DivCommonVddnClk, 2U);
    CLOCK_AttachClk(kMAIN_PLL_PFD3_to_COMMON_VDDN);  /* Switch to 250MHZ */

    BOARD_SetXspiClock(XSPI0, 3U, 1U);               /* Main PLL PDF1 DIV2. */
}

void EXAMPLE_EnterDeepSleep(void)
{
    BOARD_ClockSafeConfig();

    /* Enter deep sleep mode by using power API. */
    POWER_SelectSleepSetpoint(kRegulator_Vdd2LDO, 0U); /* Select lowest voltage when DS. */
    POWER_EnableSleepRBB(kPower_BodyBiasVdd1 | kPower_BodyBiasVdd2 | kPower_BodyBiasVddn | kPower_BodyBiasVdd1Sram |
                         kPower_BodyBiasVdd2Sram);
    POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP);

    BOARD_RestoreClockConfig();
}

/* User Callback. */
void EXAMPLE_OstimerCallback(void)
{
    matchFlag = true;
    /* User code. */
}

/* Set the match value with unit of millisecond. OStimer will trigger a match interrupt after the a certain time. */
static status_t EXAMPLE_SetMatchInterruptTime(OSTIMER_Type *base, uint32_t ms, uint32_t freq, ostimer_callback_t cb)
{
    uint64_t timerTicks = OSTIMER_GetCurrentTimerValue(base);

    /* Translate the millisecond to ostimer count value. */
    timerTicks += MSEC_TO_COUNT(ms, freq);

    /* Set the match value with unit of ticks. */
    return OSTIMER_SetMatchValue(base, timerTicks, cb);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_AttachClk(kLPOSC_to_OSTIMER);
    CLOCK_SetClkDiv(kCLOCK_DivOstimerClk, 1U);

    PRINTF("Press any key to start example.\r\n\r\n");
    GETCHAR();
    PRINTF("Board will enter power deep sleep mode, and then wakeup by OS timer after about 5 seconds.\r\n");
    PRINTF("After Board wakeup, the OS timer will trigger the match interrupt about every 2 seconds.\r\n");

    /* Intialize the OS timer, setting clock configuration. */
    OSTIMER_Init(EXAMPLE_OSTIMER);

    matchFlag = false;

    /* Set the OS timer match value. */
    if (kStatus_Success ==
        EXAMPLE_SetMatchInterruptTime(EXAMPLE_OSTIMER, 5000U, EXAMPLE_OSTIMER_FREQ, EXAMPLE_OstimerCallback))
    {
        /* Enable OSTIMER IRQ under deep sleep mode. */
        EXAMPLE_EnableDeepSleepIRQ();
        /* Enter deep sleep mode. */
        EXAMPLE_EnterDeepSleep();

        /* Wait until OS timer interrupt occurrs. */
        while (!matchFlag)
        {
        }

        /* Wakeup from deep sleep mode. */
        PRINTF("Board wakeup from deep sleep mode.\r\n\r\n");
    }
    else
    {
        PRINTF("SetMatchInterruptTime failed: set time has already expired! \r\n");
    }

    while (1)
    {
        matchFlag = false;

        /* Set the match value to trigger the match interrupt. */
        if (kStatus_Success ==
            EXAMPLE_SetMatchInterruptTime(EXAMPLE_OSTIMER, 2000U, EXAMPLE_OSTIMER_FREQ, EXAMPLE_OstimerCallback))
        {
            /* Wait for the match value to be reached. */
            while (!matchFlag)
            {
            }

            PRINTF("OS timer match value reached\r\n");
        }
        else
        {
            PRINTF("SetMatchInterruptTime failed: set time has already expired! \r\n");
        }
    }
}
