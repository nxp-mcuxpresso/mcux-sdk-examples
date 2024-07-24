/*
 * Copyright 2019-2022, 2023-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpadc.h"

#include "fsl_reset.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPADC_BASE                ADC1
#define DEMO_LPADC_IRQn                ADC1_IRQn
#define DEMO_LPADC_IRQ_HANDLER_FUNC    ADC1_IRQHandler
#define DEMO_LPADC_USER_CMDID          15U
#define DEMO_LPADC_TEMP_SENS_CHANNEL   9U
#define DEMO_LPADC_USE_HIGH_RESOLUTION 0U
#define DEMO_LPADC_SAMPLE_CHANNEL_MODE kLPADC_SampleChannelDiffBothSideAB
#define DEMO_LPADC_VREEFP              1.8f /* LPADC positive reference voltage value with the unit as V. */
#define DEMO_LPADC_VREFN               0.0f /* LPADC negative reference voltage value with the unit as V. */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
float DEMO_MeasureTemperature(ADC_Type *base, uint32_t commandId, uint32_t index);
static void ADC_Configuration(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
lpadc_conv_command_config_t g_LpadcCommandConfigStruct; /* Structure to configure conversion command. */
volatile bool g_LpadcConversionCompletedFlag = false;
float g_CurrentTemperature                   = 0.0f;
#if (defined(DEMO_LPADC_USE_HIGH_RESOLUTION) && (DEMO_LPADC_USE_HIGH_RESOLUTION))
const uint32_t g_LpadcFullRange = 65536U;
#else
const uint32_t g_LpadcFullRange = 4096U;
#endif /* DEMO_LPADC_USE_HIGH_RESOLUTION */

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * brief Measure Temperature sensor.
 */
float DEMO_MeasureTemperature(ADC_Type *base, uint32_t commandId, uint32_t index)
{
    lpadc_conv_result_t convResultStruct;

    float Vtemp              = 0.0f;
    uint32_t convResultShift = 0U;
    float temperature        = -273.15f; /* Absolute zero degree as the incorrect return value. */

    if (true == LPADC_GetConvResult(base, &convResultStruct))
    {
        Vtemp       = (((convResultStruct.convValue >> convResultShift) *
                  ((DEMO_LPADC_VREEFP - DEMO_LPADC_VREFN) * 1000.0f / g_LpadcFullRange)));
        temperature = 500.94832f - (0.6190881f * Vtemp);
    }

    return temperature;
}

void DEMO_LPADC_IRQ_HANDLER_FUNC(void)
{
    g_CurrentTemperature           = DEMO_MeasureTemperature(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, 0U);
    g_LpadcConversionCompletedFlag = true;
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    CLOCK_SetIpSrc(kCLOCK_Adc1, kCLOCK_Pcc1BusIpSrcCm33Bus);
    RESET_PeripheralReset(kRESET_Adc1);

    if (BOARD_IsLowPowerBootType() != true) /* not low power boot type */
    {
        BOARD_HandshakeWithUboot(); /* Must handshake with uboot, unless will get issues(such as: SoC reset all the
                                       time) */
    }
    else                            /* low power boot type */
    {
        BOARD_SetTrdcGlobalConfig();
    }

    PRINTF("LPADC Temperature Measurement Example\r\n");

    ADC_Configuration();

    PRINTF("ADC Full Range: %d\r\n", g_LpadcFullRange);
#if defined(FSL_FEATURE_LPADC_HAS_CMDL_CSCALE) && FSL_FEATURE_LPADC_HAS_CMDL_CSCALE
    if (kLPADC_SampleFullScale == g_LpadcCommandConfigStruct.sampleScaleMode)
    {
        PRINTF("Full channel scale (Factor of 1).\r\n");
    }
    else if (kLPADC_SamplePartScale == g_LpadcCommandConfigStruct.sampleScaleMode)
    {
        PRINTF("Divided input voltage signal. (Factor of 30/64).\r\n");
    }
#endif

    PRINTF("Please press any key to get temperature from the internal temperature sensor.\r\n");

    while (1)
    {
        GETCHAR();
        g_LpadcConversionCompletedFlag = false;
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U); /* 1U is trigger0 mask. */
        while (false == g_LpadcConversionCompletedFlag)
        {
        }
        PRINTF("Current temperature: %6.3f\r\n", (double)g_CurrentTemperature);
    }
}

static void ADC_Configuration(void)
{
    lpadc_config_t lpadcConfigStruct;
    lpadc_conv_trigger_config_t lpadcTriggerConfigStruct;

    /* Init ADC peripheral. */
    LPADC_GetDefaultConfig(&lpadcConfigStruct);
    lpadcConfigStruct.enableAnalogPreliminary = true;
#if defined(FSL_FEATURE_LPADC_HAS_CFG_PWRSEL) && (FSL_FEATURE_LPADC_HAS_CFG_PWRSEL == 1U)
    lpadcConfigStruct.powerLevelMode = kLPADC_PowerLevelAlt4;
#endif /* defined(FSL_FEATURE_LPADC_HAS_CFG_PWRSEL) && (FSL_FEATURE_LPADC_HAS_CFG_PWRSEL == 1U) */
#if defined(DEMO_LPADC_VREF_SOURCE)
    lpadcConfigStruct.referenceVoltageSource = DEMO_LPADC_VREF_SOURCE;
#endif /* DEMO_LPADC_VREF_SOURCE */
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS
    lpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS */
#if defined(FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE)
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2))
    lpadcConfigStruct.FIFO0Watermark = FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE - 1U;
    lpadcConfigStruct.FIFO1Watermark = 0U;
#else
    lpadcConfigStruct.FIFOWatermark = FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE - 1U;
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2)) */
#endif /* FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE */
    LPADC_Init(DEMO_LPADC_BASE, &lpadcConfigStruct);
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    LPADC_DoResetFIFO0(DEMO_LPADC_BASE);
#else
    LPADC_DoResetFIFO(DEMO_LPADC_BASE);
#endif

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

    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&g_LpadcCommandConfigStruct);
    g_LpadcCommandConfigStruct.channelNumber       = DEMO_LPADC_TEMP_SENS_CHANNEL;
    g_LpadcCommandConfigStruct.sampleChannelMode   = DEMO_LPADC_SAMPLE_CHANNEL_MODE;
    g_LpadcCommandConfigStruct.sampleTimeMode      = kLPADC_SampleTimeADCK131;
    g_LpadcCommandConfigStruct.hardwareAverageMode = kLPADC_HardwareAverageCount128;
#if defined(FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE)
    g_LpadcCommandConfigStruct.loopCount = FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE - 1U;
#endif /* FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE */
#if defined(FSL_FEATURE_LPADC_HAS_CMDL_MODE) && FSL_FEATURE_LPADC_HAS_CMDL_MODE
    g_LpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
#endif /* FSL_FEATURE_LPADC_HAS_CMDL_MODE */
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &g_LpadcCommandConfigStruct);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&lpadcTriggerConfigStruct);
    lpadcTriggerConfigStruct.targetCommandId = DEMO_LPADC_USER_CMDID;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &lpadcTriggerConfigStruct); /* Configurate the trigger0. */

    /* Enable the watermark interrupt. */
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFO0WatermarkInterruptEnable);
#else
    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFOWatermarkInterruptEnable);
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
    EnableIRQ(DEMO_LPADC_IRQn);

    /* Eliminate the first two inaccurate results. */
    LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U); /* 1U is trigger0 mask. */
    while (false == g_LpadcConversionCompletedFlag)
    {
    }
}
