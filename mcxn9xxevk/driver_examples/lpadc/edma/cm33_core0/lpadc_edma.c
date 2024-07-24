/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "fsl_lpadc.h"
#include "fsl_edma.h"
#include "fsl_lptmr.h"

#include "fsl_inputmux.h"
#include "fsl_spc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* ADC Defines */
#define DEMO_LPADC_BASE                  ADC0
#define DEMO_LPADC_IRQn                  ADC0_IRQn
#define DEMO_LPADC_IRQ_HANDLER_FUNC      ADC0_IRQHandler
#define DEMO_LPADC_USER_CHANNEL          2U
#define DEMO_LPADC_USER_CMDID            1U
#define DEMO_LPADC_DO_OFFSET_CALIBRATION true
#define DEMO_LPADC_OFFSET_VALUE_A        0x10U
#define DEMO_LPADC_OFFSET_VALUE_B        0x10U
#define DEMO_SPC_BASE                    SPC0
/* Use VREF_OUT driven from the VREF block as the reference volatage */
#define DEMO_LPADC_VREF_SOURCE kLPADC_ReferenceVoltageAlt3
#define DEMO_VREF_BASE         VREF0

/* EDMA Defines */
#define DEMO_DMA_BASEADDR    DMA0
#define DEMO_DMA_CHANNEL_0   0U
#define DEMO_DMA_IRQ         EDMA_0_CH0_IRQn
#define DEMO_DMA_IRQ_HANDLER EDMA_0_CH0_IRQHandler
#define BUFFER_LENGTH        50U
#define DEMO_DMA_REQUEST     kDma0RequestMuxAdc0FifoARequest

/* Low power timer for ADC Trigger */
#define LPTMR_TRIG_BASE         LPTMR0
#define LPTMR_TRIG_USEC_COUNT   1000U
#define LPTMR_TRIG_IRQn         LPTMR0_IRQn
#define LPTMR_TRIG_HANDLER      LPTMR0_IRQHandler
#define ADC_LPTMR_TRIG_CLOCK    kLPTMR_PrescalerClock_0
#define LPTMR_TRIG_SOURCE_CLOCK (12000000U)


#if (defined(DEMO_LPADC_USE_HIGH_RESOLUTION) && DEMO_LPADC_USE_HIGH_RESOLUTION)
#define LPADC_FULLRANGE   65536U
#define LPADC_RESULTSHIFT 0U
#else
#define LPADC_FULLRANGE   4096U
#define LPADC_RESULTSHIFT 3U
#endif /* DEMO_LPADC_USE_HIGH_RESOLUTION */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitDebugConsole(void);

static void ADC_Configuration(void);
static void LowPowerTimerADCTrigger_Init(void);
static void EDMA_Configuration(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile uint32_t DMACounter                                              = 0U;
AT_NONCACHEABLE_SECTION_ALIGN_INIT(uint32_t destAddr[BUFFER_LENGTH], 32U) = {0x00U};

/*******************************************************************************
 * Code
 ******************************************************************************/


void DEMO_DMA_IRQ_HANDLER(void)
{
    if ((EDMA_GetChannelStatusFlags(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL_0) & kEDMA_InterruptFlag) != 0U)
    {
        DMACounter++;
        EDMA_ClearChannelStatusFlags(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, kEDMA_InterruptFlag);
        EDMA_EnableChannelRequest(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL_0);
    }
}

/*!
 * @brief Main function
 */

int main(void)
{
    uint32_t i                       = 0U;
    volatile uint32_t currentCounter = 0U;

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1U);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Enable INPUTMUX0 */
    CLOCK_EnableClock(kCLOCK_InputMux0);

    /* attach FRO HF to ADC0 */
    CLOCK_SetClkDiv(kCLOCK_DivAdc0Clk, 1U);
    CLOCK_AttachClk(kFRO12M_to_ADC0);

    /* Enable 12MHz FRO to LPTMR */
    CLOCK_SetupClockCtrl(kCLOCK_FRO12MHZ_ENA);

    /* enable VREF */
    SPC_EnableActiveModeAnalogModules(DEMO_SPC_BASE, kSPC_controlVref);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Connect ADC FIFO flag to DMA0 Channel 0 trigger */
    INPUTMUX_EnableSignal(INPUTMUX0, kINPUTMUX_Adc0FifoARequestToDma0Ch21Ena, true);

    /* Connect LPTMR trigger output to ADC trigger input */
    INPUTMUX_AttachSignal(INPUTMUX0, 0U, kINPUTMUX_Lptmr0ToAdc0Trigger);

    PRINTF("\r\nLPADC EDMA Example");

    /* Low Power Timer Initialization */
    LowPowerTimerADCTrigger_Init();

    /* ADC Initialization */
    PRINTF("\r\nConfiguring LPADC...");
    ADC_Configuration();

    /* EDMA Initialization */
    PRINTF("\r\nConfiguring LPADC EDMA...");
    EDMA_Configuration();

    /* Start LPTMR which will trigger ADC conversions */
    LPTMR_StartTimer(LPTMR_TRIG_BASE);

    while (1)
    {
        PRINTF("\r\nPress any key to print output buffer:\r\n");
        GETCHAR();

        /* Wait for DMA to be done */
        while (currentCounter == DMACounter)
        {
        }
        currentCounter = DMACounter;
        PRINTF("\n\r");
        for (i = 0; i < BUFFER_LENGTH; i++)
        {
            PRINTF("%d = %d\n\r", i, ((destAddr[i] & 0x7FFFU) >> LPADC_RESULTSHIFT));
        }
        PRINTF("\n\r");
    }
}

/*!
 * @brief EDMA configuration
 */
static void EDMA_Configuration(void)
{
    edma_transfer_config_t transferConfig;
    edma_channel_config_t lpadcDmaChnlConfig;
    edma_config_t userConfig;

    lpadcDmaChnlConfig.channelDataSignExtensionBitPosition             = 0U;
    lpadcDmaChnlConfig.channelPreemptionConfig.enableChannelPreemption = false;
    lpadcDmaChnlConfig.channelPreemptionConfig.enablePreemptAbility    = true;
    lpadcDmaChnlConfig.channelRequestSource                            = DEMO_DMA_REQUEST;
    lpadcDmaChnlConfig.protectionLevel                                 = kEDMA_ChannelProtectionLevelUser;
#if !(defined(FSL_FEATURE_EDMA_HAS_NO_CH_SBR_SEC) && FSL_FEATURE_EDMA_HAS_NO_CH_SBR_SEC)
    lpadcDmaChnlConfig.securityLevel = kEDMA_ChannelSecurityLevelNonSecure;
#endif /* !(defined(FSL_FEATURE_EDMA_HAS_NO_CH_SBR_SEC) && FSL_FEATURE_EDMA_HAS_NO_CH_SBR_SEC) */

    /* Configure EDMA channel for one shot transfer */
    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(DEMO_DMA_BASEADDR, &userConfig);

#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    void *srcAddr = (uint32_t *)&(DEMO_LPADC_BASE->RESFIFO[0U]);
#else
    void *srcAddr = (uint32_t *)&(DEMO_LPADC_BASE->RESFIFO);
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U)) */
    EDMA_PrepareTransfer(&transferConfig, srcAddr, sizeof(uint32_t), destAddr, sizeof(destAddr[0]), sizeof(destAddr[0]),
                         sizeof(destAddr), kEDMA_PeripheralToMemory);

    /* Used to change the destination address to the original value */
    transferConfig.dstMajorLoopOffset = (int32_t)((-1) * sizeof(destAddr));

    EDMA_SetTransferConfig(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, &transferConfig, NULL);
    EDMA_InitChannel(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, &lpadcDmaChnlConfig);

    EnableIRQ(DEMO_DMA_IRQ);
    EDMA_EnableChannelRequest(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL_0);
}

/*!
 * @brief ADC configuration
 */
static void ADC_Configuration(void)
{
    lpadc_config_t lpadcConfigStruct;
    lpadc_conv_trigger_config_t lpadcTriggerConfigStruct;
    lpadc_conv_command_config_t lpadcCommandConfigStruct;

    /* Sets the converter configuration structure with an available settings.
     * code
     *   config->enableInDozeMode        = true;
     *   config->conversionAverageMode   = kLPADC_ConversionAverage1;
     *   config->enableAnalogPreliminary = true;
     *   config->powerUpDelay            = 0x10;
     *   config->referenceVoltageSource  = kLPADC_ReferenceVoltageAlt3;
     *   config->powerLevelMode          = kLPADC_PowerLevelAlt1;
     *   config->triggerPriorityPolicy   = kLPADC_TriggerPriorityPreemptImmediately;
     *   config->enableConvPause         = false;
     *   config->convPauseDelay          = 0U;
     *   config->FIFO0Watermark          = 0U;
     *   config->FIFO1Watermark          = 0U;
     *   config->FIFOWatermark           = 0U;
     * endcode
     */
    LPADC_GetDefaultConfig(&lpadcConfigStruct);
    lpadcConfigStruct.enableAnalogPreliminary = true;
    lpadcConfigStruct.powerUpDelay            = 0x10U;
#if defined(DEMO_LPADC_VREF_SOURCE)
    lpadcConfigStruct.referenceVoltageSource = DEMO_LPADC_VREF_SOURCE;
#endif /* DEMO_LPADC_VREF_SOURCE */
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS
    lpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS */
    lpadcConfigStruct.enableInDozeMode = true;
    LPADC_Init(DEMO_LPADC_BASE, &lpadcConfigStruct);

    /* Request LPADC calibration. */
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CALOFSMODE) && FSL_FEATURE_LPADC_HAS_CTRL_CALOFSMODE
    LPADC_SetOffsetCalibrationMode(DEMO_LPADC_BASE, DEMO_LPADC_OFFSET_CALIBRATION_MODE);
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CALOFSMODE */

#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CALOFS) && FSL_FEATURE_LPADC_HAS_CTRL_CALOFS
#if defined(DEMO_LPADC_DO_OFFSET_CALIBRATION) && DEMO_LPADC_DO_OFFSET_CALIBRATION
    LPADC_DoOffsetCalibration(DEMO_LPADC_BASE); /* Request offset calibration, automatic update OFSTRIM register. */
#else                                           /* Update OFSTRIM register manually. */

#if defined(FSL_FEATURE_LPADC_HAS_OFSTRIM) && FSL_FEATURE_LPADC_HAS_OFSTRIM
#if defined(FSL_FEATURE_LPADC_OFSTRIM_COUNT) && (FSL_FEATURE_LPADC_OFSTRIM_COUNT == 2U)
    LPADC_SetOffsetValue(DEMO_LPADC_BASE, DEMO_LPADC_OFFSET_VALUE_A, DEMO_LPADC_OFFSET_VALUE_B);
#elif defined(FSL_FEATURE_LPADC_OFSTRIM_COUNT) && (FSL_FEATURE_LPADC_OFSTRIM_COUNT == 1U)
    LPADC_SetOffsetValue(DEMO_LPADC_BASE, DEMO_LPADC_OFFSET_VALUE);
#endif /* FSL_FEATURE_LPADC_OFSTRIM_COUNT */

#else  /* For other OFSTRIM register type. */
    if (DEMO_LPADC_OFFSET_CALIBRATION_MODE == kLPADC_OffsetCalibration12bitMode)
    {
        LPADC_SetOffset12BitValue(DEMO_LPADC_BASE, DEMO_LPADC_OFFSET_VALUE_A, DEMO_LPADC_OFFSET_VALUE_B);
    }
    else
    {
        LPADC_SetOffset16BitValue(DEMO_LPADC_BASE, DEMO_LPADC_OFFSET_VALUE_A, DEMO_LPADC_OFFSET_VALUE_B);
    }
#endif /* FSL_FEATURE_LPADC_HAS_OFSTRIM */

#endif /* DEMO_LPADC_DO_OFFSET_CALIBRATION */
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CALOFS */

#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_REQ) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_REQ
    /* Request auto calibration (including gain error calibration and linearity error calibration). */
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CAL_REQ */

#if (defined(FSL_FEATURE_LPADC_HAS_CFG_CALOFS) && FSL_FEATURE_LPADC_HAS_CFG_CALOFS)
    /* Do auto calibration. */
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CFG_CALOFS */

#if (defined(FSL_FEATURE_LPADC_HAS_CFG_CALOFS) && FSL_FEATURE_LPADC_HAS_CFG_CALOFS)
    /* Do auto calibration. */
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CFG_CALOFS */

    /* Sets the conversion command's configuration structure with an available settings.
     * code
     *   config->sampleScaleMode            = kLPADC_SampleFullScale;
     *   config->channelBScaleMode          = kLPADC_SampleFullScale;
     *   config->channelSampleMode          = kLPADC_SampleChannelSingleEndSideA;
     *   config->channelNumber              = 2U;
     *   config->alternateChannelNumber     = 0U;
     *   config->chainedNextCmdNumber       = 0U;
     *   config->enableAutoChannelIncrement = false;
     *   config->loopCount                  = 0U;
     *   config->hardwareAverageMode        = kLPADC_HardwareAverageCount1;
     *   config->sampleTimeMode             = kLPADC_SampleTimeADCK3;
     *   config->hardwareCompareMode        = kLPADC_HardwareCompareDisabled;
     *   config->hardwareCompareValueHigh   = 0U;
     *   config->hardwareCompareValueLow    = 0U;
     *   config->conversionResolutionMode   = kLPADC_ConversionResolutionStandard;
     *   config->enableWaitTrigger          = false;
     *   config->enableChannelB             = false;
     * endcode
     */
    LPADC_GetDefaultConvCommandConfig(&lpadcCommandConfigStruct);
    lpadcCommandConfigStruct.channelNumber = DEMO_LPADC_USER_CHANNEL;
#if defined(DEMO_LPADC_USE_HIGH_RESOLUTION) && DEMO_LPADC_USE_HIGH_RESOLUTION
    lpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
#endif /* DEMO_LPADC_USE_HIGH_RESOLUTION */
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &lpadcCommandConfigStruct);

    /* Sets the trigger's configuration structure with an available settings.
     *   config->targetCommandId       = 1U;
     *   config->delayPower            = 0U;
     *   config->priority              = 0U;
     *   config->enableHardwareTrigger = true;
     *   config->channelAFIFOSelect    = 0U;
     *   config->channelBFIFOSelect    = 0U;
     * endcode
     */
    LPADC_GetDefaultConvTriggerConfig(&lpadcTriggerConfigStruct);
    lpadcTriggerConfigStruct.targetCommandId = DEMO_LPADC_USER_CMDID; /* CMD1 is executed. */

    /* Enable the hardware trigger function in the ADC block */
    lpadcTriggerConfigStruct.enableHardwareTrigger = true;

    /* Configured the trigger0. */
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &lpadcTriggerConfigStruct);

    /* Enable the watermark DMA in the ADC block */
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    LPADC_EnableFIFO0WatermarkDMA(DEMO_LPADC_BASE, true);
#else
    LPADC_EnableFIFOWatermarkDMA(DEMO_LPADC_BASE, true);
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U)) */

    PRINTF("\r\nADC Full Range: %d", LPADC_FULLRANGE);
#if defined(FSL_FEATURE_LPADC_HAS_CMDL_CSCALE) && FSL_FEATURE_LPADC_HAS_CMDL_CSCALE
    if (kLPADC_SampleFullScale == lpadcCommandConfigStruct.sampleScaleMode)
    {
        PRINTF("Full channel scale (Factor of 1).\r\n");
    }
    else if (kLPADC_SamplePartScale == lpadcCommandConfigStruct.sampleScaleMode)
    {
        PRINTF("Divided input voltage signal. (Factor of 30/64).\r\n");
    }
#endif
}

/*!
 * @brief LPTMR configuration
 */
static void LowPowerTimerADCTrigger_Init(void)
{
    lptmr_config_t lptmrConfig;

    /* Configure LPTMR */
    /*
     * Default configuration:
     *
     * lptmrConfig.timerMode = kLPTMR_TimerModeTimeCounter;
     * lptmrConfig.pinSelect = kLPTMR_PinSelectInput_0;
     * lptmrConfig.pinPolarity = kLPTMR_PinPolarityActiveHigh;
     * lptmrConfig.enableFreeRunning = false;
     * lptmrConfig.bypassPrescaler = true;
     * lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;
     * lptmrConfig.value = kLPTMR_Prescale_Glitch_0;
     */
    LPTMR_GetDefaultConfig(&lptmrConfig);
    lptmrConfig.prescalerClockSource = ADC_LPTMR_TRIG_CLOCK;

    /* Initialize the LPTMR */
    LPTMR_Init(LPTMR_TRIG_BASE, &lptmrConfig);

    /*
     * Set timer period.
     * Note : the parameter "ticks" of LPTMR_SetTimerPeriod should be equal or greater than 1.
     */
    LPTMR_SetTimerPeriod(LPTMR_TRIG_BASE, USEC_TO_COUNT(LPTMR_TRIG_USEC_COUNT, LPTMR_TRIG_SOURCE_CLOCK));
}
