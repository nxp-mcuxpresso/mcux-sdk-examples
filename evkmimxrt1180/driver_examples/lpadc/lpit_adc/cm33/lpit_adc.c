/*
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpadc.h"
#include "fsl_lpit.h"

#include "fsl_xbar.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LPIT_CLK_FREQ        CLOCK_GetRootClockFreq(kCLOCK_Root_Bus_Aon)
#define DEMO_LPIT_BASE       LPIT1
#define LPIT_CHANNEL         kLPIT_Chnl_0
#define DEMO_LPIT_IRQn       LPIT1_IRQn
#define DEMO_LPIT_IRQHandler LPIT1_IRQHandler
#define LPIT_PERIOD          1000000U

#define DEMO_LPADC_BASE             ADC1
#define DEMO_LPADC_IRQn             ADC1_IRQn
#define DEMO_LPADC_IRQ_HANDLER_FUNC ADC1_IRQHandler
#define DEMO_LPADC_USER_CHANNEL     7U
#define DEMO_LPADC_USER_CMDID       1U /* CMD1 */
/* ERRATA051385: ADC INL/DNL degrade under high ADC clock frequency when VREFH selected as reference. */
#define DEMO_LPADC_VREF_SOURCE kLPADC_ReferenceVoltageAlt2
void DEMO_InitLPIT(void);
void DEMO_InitLPADC(void);
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_LpadcConversionCompletedFlag = false;
volatile uint32_t g_LpadcInterruptCounter    = 0U;
lpadc_conv_result_t g_LpadcResultConfigStruct;
const uint32_t g_LpadcFullRange = 4096U;

/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_LPADC_IRQ_HANDLER_FUNC(void)
{
    g_LpadcInterruptCounter++;
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    if (LPADC_GetConvResult(DEMO_LPADC_BASE, &g_LpadcResultConfigStruct, 0U))
#else
    if (LPADC_GetConvResult(DEMO_LPADC_BASE, &g_LpadcResultConfigStruct))
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U)) */
    {
        g_LpadcConversionCompletedFlag = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init xbara module. */
    XBAR_Init(kXBAR_DSC1);

    /* Configure the XBARA signal connections. */
    XBAR_SetSignalsConnection(kXBAR1_InputPit1Trigger0, kXBAR1_OutputAdc12HwTrig0);

    if (NVIC_GetEnableIRQ(GPIO1_0_IRQn))
    {
        NVIC_DisableIRQ(GPIO1_0_IRQn);
    }
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("LPADC Interrupt Example\r\n");

    DEMO_InitLPIT();
    DEMO_InitLPADC();

    while (1)
    {
        /* Check whether occur interupt and toggle LED */
        while (!g_LpadcConversionCompletedFlag)
        {
        }
        PRINTF("ADC interrupt count: %d\r\n", g_LpadcInterruptCounter);
        PRINTF("ADC value: %d\r\n", (g_LpadcResultConfigStruct.convValue) >> 3U);
        g_LpadcConversionCompletedFlag = false;
    }
}

/* Initialize the LPIT. */
void DEMO_InitLPIT(void)
{
    /* Structure of initialize LPIT */
    lpit_config_t lpitConfig;
    lpit_chnl_params_t lpitChannelConfig;

    /*
     * lpitConfig.enableRunInDebug = false;
     * lpitConfig.enableRunInDoze = false;
     */
    LPIT_GetDefaultConfig(&lpitConfig);

    /* Init LPIT module. */
    LPIT_Init(DEMO_LPIT_BASE, &lpitConfig);

    lpitChannelConfig.chainChannel          = false;
    lpitChannelConfig.enableReloadOnTrigger = false;
    lpitChannelConfig.enableStartOnTrigger  = false;
    lpitChannelConfig.enableStopOnTimeout   = false;
    lpitChannelConfig.timerMode             = kLPIT_PeriodicCounter;
    /* Set default values for the trigger source */
    lpitChannelConfig.triggerSelect = kLPIT_Trigger_TimerChn0;
    lpitChannelConfig.triggerSource = kLPIT_TriggerSource_External;

    /* Init lpit channel */
    LPIT_SetupChannel(DEMO_LPIT_BASE, LPIT_CHANNEL, &lpitChannelConfig);

    /* Set timer period for channels using. */
    LPIT_SetTimerPeriod(DEMO_LPIT_BASE, LPIT_CHANNEL, USEC_TO_COUNT(LPIT_PERIOD, LPIT_CLK_FREQ));

    LPIT_StartTimer(DEMO_LPIT_BASE, LPIT_CHANNEL);
}

/* Initialize the LPADC. */
void DEMO_InitLPADC(void)
{
    /* Structure of initialize LPADC */
    lpadc_config_t mLpadcConfigStruct;
    lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
    lpadc_conv_command_config_t mLpadcCommandConfigStruct;

    /* Init LPADC module. */
    LPADC_GetDefaultConfig(&mLpadcConfigStruct);
    mLpadcConfigStruct.enableAnalogPreliminary = true;
    mLpadcConfigStruct.referenceVoltageSource  = DEMO_LPADC_VREF_SOURCE;
    mLpadcConfigStruct.conversionAverageMode   = kLPADC_ConversionAverage128;
    LPADC_Init(DEMO_LPADC_BASE, &mLpadcConfigStruct);

    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    mLpadcCommandConfigStruct.channelNumber = DEMO_LPADC_USER_CHANNEL;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &mLpadcCommandConfigStruct);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
    mLpadcTriggerConfigStruct.targetCommandId       = DEMO_LPADC_USER_CMDID;     /* CMD15 is executed. */
    mLpadcTriggerConfigStruct.enableHardwareTrigger = true;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */

    /* Enable the watermark interrupt. */
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFO0WatermarkInterruptEnable);
#else
    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFOWatermarkInterruptEnable);
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */

    EnableIRQ(DEMO_LPADC_IRQn);

    PRINTF("ADC Full Range: %d\r\n", g_LpadcFullRange);

    if (kLPADC_SampleFullScale == mLpadcCommandConfigStruct.sampleScaleMode)
    {
        PRINTF("Full channel scale (Factor of 1).\r\n");
    }
    else if (kLPADC_SamplePartScale == mLpadcCommandConfigStruct.sampleScaleMode)
    {
        PRINTF("Divided input voltage signal. (Factor of 30/64).\r\n");
    }
}
