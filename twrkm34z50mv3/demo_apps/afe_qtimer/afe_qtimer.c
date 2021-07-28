/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_afe.h"
#include "fsl_cmp.h"
#include "fsl_vref.h"
#include "fsl_qtmr.h"
#include "fsl_xbar.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_AFE_FIRST_CHANNEL  2U
#define DEMO_AFE_SECOND_CHANNEL 3U
/* About 23 mV */
#define DEMO_AFE_VREF_TRIM               2U
#define DEMO_AFE_BASEADDR                AFE
#define DEMO_AFE_FIRST_IRQ_HANDLER_FUNC  AFE_CH2_IRQHandler
#define DEMO_AFE_SECOND_IRQ_HANDLER_FUNC AFE_CH3_IRQHandler
#define DEMO_AFE_FIRST_CHANNEL_IRQn      AFE_CH2_IRQn
#define DEMO_AFE_SECOND_CHANNEL_IRQn     AFE_CH3_IRQn

#define DEMO_CMP_BASEADDR      CMP1
#define DEMO_CMP_PLUS_CHANNEL  2U
#define DEMO_CMP_MINUS_CHANNEL 3U

#define TMRPRCLK                 (double)(CLOCK_GetFreq(kCLOCK_BusClk) / 64)
#define DEMO_QUAD_TIMER_BASEADDR TMR0

#define SIM_MISC_CTL_TMR0SCSEL SIM_MISC_CTL_TMR0SCSSEL

#define TMR2FREQ(x) (double)(TMRPRCLK / (double)x) /* Calculate frequency */
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief Initialize AFE module.
 *
 */
static void APP_AFE_Config(void);

/*!
 * @brief Initialize VREF module.
 *
 */
static void APP_VREF_Config(void);

/*!
 * @brief Initialize CMP module.
 *
 */
static void APP_CMP_Config(void);

/*!
 * @brief Initialize QTIMER module.
 *
 */
static void APP_QTIMER_Config(void);

/*!
 * @brief Initialize XBAR module.
 *
 */
static void APP_XBAR_Config(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_bAfeFirstChannelConvDone  = false; /* Conversion done flag */
volatile bool g_bAfeSecondChannelConvDone = false; /* Conversion done flag */
volatile int32_t g_result0                = 0;
volatile int32_t g_result1                = 0;

double g_freq_tmr; /* Frequency of quad timer */

/*******************************************************************************
 * Code
 ******************************************************************************/

void DEMO_AFE_FIRST_IRQ_HANDLER_FUNC(void)
{
    /* Check conversion complete */
    if ((kAFE_Channel2ConversionCompleteFlag & AFE_GetChannelStatusFlags(DEMO_AFE_BASEADDR)))
    {
        g_result0 = (int32_t)AFE_GetChannelConversionValue(DEMO_AFE_BASEADDR, DEMO_AFE_FIRST_CHANNEL);
        /* Set conversion done flag */
        g_bAfeFirstChannelConvDone = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

void DEMO_AFE_SECOND_IRQ_HANDLER_FUNC(void)
{
    /* Check conversion complete */
    if ((kAFE_Channel3ConversionCompleteFlag & AFE_GetChannelStatusFlags(DEMO_AFE_BASEADDR)))
    {
        g_result1 = (int32_t)AFE_GetChannelConversionValue(DEMO_AFE_BASEADDR, DEMO_AFE_SECOND_CHANNEL);
        /* Set conversion done flag */
        g_bAfeSecondChannelConvDone = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

static void APP_AFE_Config(void)
{
    afe_config_t afeExampleStruct;
    afe_channel_config_t afeChnExampleStruct;

    /* Get AFE config default */
    AFE_GetDefaultConfig(&afeExampleStruct);
    afeExampleStruct.startupCount = 80U; /* startupCnt = (Clk_freq/Clk_div)*20e-6 */

    /* Get channel config default */
    AFE_GetDefaultChannelConfig(&afeChnExampleStruct);
    /* Initialize AFE module */
    AFE_Init(DEMO_AFE_BASEADDR, &afeExampleStruct);
    /* Configure AFE channel */
    AFE_SetChannelConfig(DEMO_AFE_BASEADDR, DEMO_AFE_FIRST_CHANNEL, &afeChnExampleStruct);
    AFE_SetChannelConfig(DEMO_AFE_BASEADDR, DEMO_AFE_SECOND_CHANNEL, &afeChnExampleStruct);
    /* Enable interrupt */
    AFE_EnableChannelInterrupts(DEMO_AFE_BASEADDR, kAFE_Channel2InterruptEnable | kAFE_Channel3InterruptEnable);
    EnableIRQ(DEMO_AFE_FIRST_CHANNEL_IRQn);
    EnableIRQ(DEMO_AFE_SECOND_CHANNEL_IRQn);
    /* Disable DMA */
    AFE_EnableChannelDMA(DEMO_AFE_BASEADDR, kAFE_Channel2DMAEnable | kAFE_Channel3DMAEnable, false);
}

static void APP_CMP_Config(void)
{
    cmp_config_t mCmpConfigStruct;

    CMP_GetDefaultConfig(&mCmpConfigStruct);

    /* Initialize the CMP comparator. */
    CMP_Init(DEMO_CMP_BASEADDR, &mCmpConfigStruct);
    CMP_SetInputChannels(DEMO_CMP_BASEADDR, DEMO_CMP_PLUS_CHANNEL, DEMO_CMP_MINUS_CHANNEL);
}

static void APP_XBAR_Config(void)
{
    /* Initialize xbar module. */
    XBAR_Init(XBAR);
    /* Configure the XBAR signal connections. */
    XBAR_SetSignalsConnection(XBAR, kXBAR_InputCmp1Output, kXBAR_OutputTmrCh0SecInput);
}

static void APP_QTIMER_Config(void)
{
    qtmr_config_t qtmrConfig;

    QTMR_GetDefaultConfig(&qtmrConfig);

    /* Set clock prescaler */
    qtmrConfig.primarySource = kQTMR_ClockDivide_64;
    QTMR_Init(DEMO_QUAD_TIMER_BASEADDR, &qtmrConfig);
    /* Set qtimer work in input capture mode */
    QTMR_SetupInputCapture(DEMO_QUAD_TIMER_BASEADDR, kQTMR_Counter0InputPin, false, true, kQTMR_RisingEdge);

    /* Select secondary source count from XBAR */
    SIM->MISC_CTL |= SIM_MISC_CTL_TMR0SCSEL(1);
    /* Start timer */
    QTMR_StartTimer(DEMO_QUAD_TIMER_BASEADDR, kQTMR_PriSrcRiseEdge);
}

static void APP_VREF_Config(void)
{
    vref_config_t config;

    /* Get vref default configure */
    VREF_GetDefaultConfig(&config);
#if defined(FSL_FEATURE_VREF_HAS_LOW_REFERENCE) && FSL_FEATURE_VREF_HAS_LOW_REFERENCE
    /* Enable low reference volt */
    config.enableLowRef = true;
#endif /* FSL_FEATURE_VREF_HAS_LOW_REFERENCE */
    /* Initialize vref */
    VREF_Init(VREF, &config);
#if defined(FSL_FEATURE_VREF_HAS_LOW_REFERENCE) && FSL_FEATURE_VREF_HAS_LOW_REFERENCE
    /* Vref set trim reference */
    VREF_SetLowReferenceTrimVal(VREF, 3U);
#endif /* FSL_FEATURE_VREF_HAS_LOW_REFERENCE */
    /* Vref set trim, this value will not be the same with every boards */
    VREF_SetTrimVal(VREF, DEMO_AFE_VREF_TRIM);
}

int main(void)
{
    uint32_t regData        = 0;
    uint32_t measureCounter = 0;

    /* Initialize board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Select clkSrc for AFEclk */
    CLOCK_SetAfeClkSrc(1U);

    /* Initialize Vref. */
    APP_VREF_Config();
    /* Initialize Afe */
    APP_AFE_Config();
    /* Initialize Cmp */
    APP_CMP_Config();
    /* Initialize Qtimer */
    APP_QTIMER_Config();
    /* Initialize Xbar */
    APP_XBAR_Config();

    PRINTF("AFE QTIMER DEMO.\r\n\r\n");

    /* Software trigger conversion */
    AFE_DoSoftwareTriggerChannel(DEMO_AFE_BASEADDR, kAFE_Channel2Trigger | kAFE_Channel3Trigger);

    /* Read channel 1 result conversion */
    while (!g_bAfeFirstChannelConvDone)
    {
    }
    g_bAfeFirstChannelConvDone = false;
    PRINTF("First Channel value: %d \r\n\r\n", g_result0);

    /* Read channel 2 result conversion */
    while (!g_bAfeSecondChannelConvDone)
    {
    }
    g_bAfeSecondChannelConvDone = false;

    PRINTF("Second Channel value: %d\r\n\r\n", g_result1);

    while (1)
    {
        if (QTMR_GetStatus(DEMO_QUAD_TIMER_BASEADDR) & kQTMR_EdgeFlag)
        {
            /* Clear capture flag */
            QTMR_ClearStatusFlags(DEMO_QUAD_TIMER_BASEADDR, kQTMR_EdgeFlag);
            /* Read capture value from capture value register */
            regData = DEMO_QUAD_TIMER_BASEADDR->CAPT;
            /* Calculate frequency */
            g_freq_tmr = TMR2FREQ(regData);

            /* Skip first value */
            if (measureCounter != 0)
            {
                PRINTF("Frequency: %d\r\n", (uint32_t)g_freq_tmr);
            }
            measureCounter++;
        }
        if (measureCounter >= 10)
        {
            PRINTF("\r\nAFE QTIMER DEMO FINISH.\r\n\r\n");
            break;
        }
    }
    return 0;
}
