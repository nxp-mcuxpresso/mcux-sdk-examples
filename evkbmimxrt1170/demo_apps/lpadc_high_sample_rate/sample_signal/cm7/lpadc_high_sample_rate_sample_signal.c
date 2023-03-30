/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "fsl_common.h"

#include "fsl_lpadc.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPADC_BASE        LPADC1
#define DEMO_LPADC_CHANNEL_NUM 0U
#define DEMO_LPADC_IRQn        ADC1_IRQn
#define DEMO_LPADC_IRQ_HANDLER ADC1_IRQHandler

#define DEMO_DMAMUX_BASE DMAMUX0

#define DEMO_DMA_BASE    DMA0
#define DEMO_DMA_CHANNEL 0U
#define DEMO_DMA_SOURCE  66U
#define BUFF_LENGTH 1000

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitADCClock(void);
static void DEMO_ADCInit(void);
static void DEMO_EDMAInit(void);
static void DEMO_OutputResult(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
edma_handle_t g_EDMA_Handle;
edma_transfer_config_t g_transferConfig;
AT_NONCACHEABLE_SECTION_INIT(uint16_t destAddr[BUFF_LENGTH]);

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_InitADCClock(void)
{
    clock_root_config_t adc1ClkRoot;
    /* Set ADC1 CLK as 88MHz */
    adc1ClkRoot.mux      = 6U; /* Set clock source as SYS PLL2 CLK. */
    adc1ClkRoot.div      = 6U;
    adc1ClkRoot.clockOff = false;
    CLOCK_SetRootClock(kCLOCK_Root_Adc1, &adc1ClkRoot);
}

void EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        LPADC_Deinit(DEMO_LPADC_BASE);
        DEMO_OutputResult();
    }
}

int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitADCClock();

    PRINTF("ADC High Sample Rate Demo: Sample Input Signal\r\n");
    DEMO_ADCInit();
    DEMO_EDMAInit();

    PRINTF("\r\nPlease input the analog signal to the ADC channel.\r\n");
    PRINTF("In order to recover the input signal as completely as possible,\r\n");
    PRINTF("please ensure that the frequency of the input signal is less than 1MHz\r\n");
    PRINTF("\r\nPlease input any key to trigger ADC conversion.\r\n");
    GETCHAR();

    LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1UL);
    while (1)
    {
    }
}

/*!
 * @brief Initialize ADC module, set ADC as continuous sample mode.
 */
static void DEMO_ADCInit(void)
{
    lpadc_config_t adcConfig;
    lpadc_conv_trigger_config_t softwareTriggerConfig;
    lpadc_conv_command_config_t commandConfig;

    LPADC_GetDefaultConfig(&adcConfig);
    adcConfig.enableAnalogPreliminary = true;
    adcConfig.powerLevelMode          = kLPADC_PowerLevelAlt4; /* Highest power setting. */
    adcConfig.FIFOWatermark           = 0x7U;
    adcConfig.referenceVoltageSource  = kLPADC_ReferenceVoltageAlt1;
    LPADC_Init(DEMO_LPADC_BASE, &adcConfig);

#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CALOFS) && FSL_FEATURE_LPADC_HAS_CTRL_CALOFS
#if defined(FSL_FEATURE_LPADC_HAS_OFSTRIM) && FSL_FEATURE_LPADC_HAS_OFSTRIM
    /* Request offset calibration. */
#if defined(DEMO_LPADC_DO_OFFSET_CALIBRATION) && DEMO_LPADC_DO_OFFSET_CALIBRATION
    LPADC_DoOffsetCalibration(DEMO_LPADC_BASE);
#else
    LPADC_SetOffsetValue(DEMO_LPADC_BASE, DEMO_LPADC_OFFSET_VALUE_A, DEMO_LPADC_OFFSET_VALUE_B);
#endif /* DEMO_LPADC_DO_OFFSET_CALIBRATION */
#endif /* FSL_FEATURE_LPADC_HAS_OFSTRIM */
    /* Request gain calibration. */
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CALOFS */

#if (defined(FSL_FEATURE_LPADC_HAS_CFG_CALOFS) && FSL_FEATURE_LPADC_HAS_CFG_CALOFS)
    /* Do auto calibration. */
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CFG_CALOFS */

    /*
     *   config->sampleScaleMode            = kLPADC_SampleFullScale;
     *   config->channelSampleMode          = kLPADC_SampleChannelSingleEndSideA;
     *   config->channelNumber              = 0U;
     *   config->chainedNextCmdNumber       = 0U;
     *   config->enableAutoChannelIncrement = false;
     *   config->loopCount                  = 0U;
     *   config->hardwareAverageMode        = kLPADC_HardwareAverageCount1;
     *   config->sampleTimeMode             = kLPADC_SampleTimeADCK3;
     *   config->hardwareCompareMode        = kLPADC_HardwareCompareDisabled;
     *   config->hardwareCompareValueHigh   = 0U;
     *   config->hardwareCompareValueLow    = 0U;
     *   config->conversionResolutionMode  = kLPADC_ConversionResolutionStandard;
     *   config->enableWaitTrigger          = false;
     */
    LPADC_GetDefaultConvCommandConfig(&commandConfig);

    commandConfig.sampleTimeMode           = kLPADC_SampleTimeADCK5;
    commandConfig.channelNumber            = DEMO_LPADC_CHANNEL_NUM;
    commandConfig.chainedNextCommandNumber = 1U;
    commandConfig.sampleScaleMode          = kLPADC_SampleFullScale;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, 1U, &commandConfig);

    LPADC_GetDefaultConvTriggerConfig(&softwareTriggerConfig);
    softwareTriggerConfig.targetCommandId = 1U;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &softwareTriggerConfig);

    LPADC_DoResetFIFO(DEMO_LPADC_BASE);

    LPADC_EnableFIFOWatermarkDMA(DEMO_LPADC_BASE, true);
}

static void DEMO_OutputResult(void)
{
    uint32_t i;

    for (i = 0U; i < BUFF_LENGTH; i++)
    {
        PRINTF("%d\r\n", destAddr[i] / 8UL);
    }
}

static void DEMO_EDMAInit(void)
{
    edma_config_t userConfig;

    DMAMUX_Init(DEMO_DMAMUX_BASE);
    DMAMUX_SetSource(DEMO_DMAMUX_BASE, DEMO_DMA_CHANNEL, DEMO_DMA_SOURCE);
    DMAMUX_EnableChannel(DEMO_DMAMUX_BASE, DEMO_DMA_CHANNEL);

    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(DEMO_DMA_BASE, &userConfig);
    EDMA_CreateHandle(&g_EDMA_Handle, DEMO_DMA_BASE, DEMO_DMA_CHANNEL);
    EDMA_SetCallback(&g_EDMA_Handle, EDMA_Callback, NULL);
    EDMA_PrepareTransfer(&g_transferConfig, (void *)&(DEMO_LPADC_BASE->RESFIFO), 2U, destAddr, sizeof(destAddr[0]),
                         2U * 8U, BUFF_LENGTH * 2U, kEDMA_PeripheralToMemory);
    EDMA_SubmitTransfer(&g_EDMA_Handle, &g_transferConfig);
    EDMA_StartTransfer(&g_EDMA_Handle);
}
