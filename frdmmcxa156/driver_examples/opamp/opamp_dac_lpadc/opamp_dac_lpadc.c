/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_opamp.h"
#include "fsl_dac.h"
#include "fsl_lpadc.h"

#include "fsl_common.h"
#include "fsl_spc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_OPAMP_BASE OPAMP0

#define DEMO_LPADC_BASE                  ADC0
#define DEMO_LPADC_USER_CHANNEL1         3U  /* LPADC channel 3 connected to the OPAMP0_INT */
#define DEMO_LPADC_USER_CHANNEL2         28U /* LPADC channel 28 connected to the OPAMP0_OBS */
#define DEMO_LPADC_USER_CMDID1           1U
#define DEMO_LPADC_USER_CMDID2           2U
#define DEMO_LPADC_VREF_SOURCE           kLPADC_ReferenceVoltageAlt3
#define DEMO_LPADC_VREF_VOLTAGE          3.300F
#define DEMO_LPADC_DO_OFFSET_CALIBRATION true

#define DEMO_DAC_BASE      DAC0
#define DEMO_DAC_VREF      kDAC_ReferenceVoltageSourceAlt1
#define DEMO_DAC_VOLT_STEP 0.806F
#define DAC_VALUE_TO_MVOLT(dacValue) ((float)(dacValue)*DEMO_DAC_VOLT_STEP)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitDebugConsole(void);
void DEMO_DoDacConfig(void);
void DEMO_DoOpampConfig(void);
void DEMO_DoLpadcConfig(void);
void DEMO_DoDacValueSet(void);
void DEMO_DoNgainValueSet(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (defined(DEMO_LPADC_USE_HIGH_RESOLUTION) && DEMO_LPADC_USE_HIGH_RESOLUTION)
const uint32_t g_LpadcFullRange   = 65536U;
const uint32_t g_LpadcResultShift = 0U;
#else
const uint32_t g_LpadcFullRange   = 4096U;
const uint32_t g_LpadcResultShift = 3U;
#endif /* (defined(DEMO_LPADC_USE_HIGH_RESOLUTION) && DEMO_LPADC_USE_HIGH_RESOLUTION) */

lpadc_conv_result_t mLpadcResultConfigStruct;

/*******************************************************************************
 * Code
 ******************************************************************************/
int main(void)
{
    int ch            = 0U;
    uint16_t adcValue = 0U;
    float voltage     = 0.0f;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* attach FRO HF to DAC0 */
    CLOCK_AttachClk(kFRO12M_to_DAC0);
    CLOCK_SetClockDiv(kCLOCK_DivDAC0, 1U);

    /* attach FRO HF to ADC0 */
    CLOCK_AttachClk(kFRO12M_to_ADC0);
    CLOCK_SetClockDiv(kCLOCK_DivADC0, 1U);

    /* Enable OPAMP and DAC */
    SPC_EnableActiveModeAnalogModules(SPC0, (kSPC_controlOpamp0 | kSPC_controlDac0));
    DEMO_DoDacConfig();
    DEMO_DoOpampConfig();
    DEMO_DoLpadcConfig();

    PRINTF("\r\n OPAMP DAC LPADC EXAMPLE.");

    while (1)
    {
        DEMO_DoDacValueSet();
        DEMO_DoNgainValueSet();

        PRINTF("\r\n Please press any key excluding key (R or r) to get the user channel's ADC value.");
        while (1)
        {
            ch = GETCHAR();

            /* The "r" and "R" is used to reset the trim value */
            if (ch == 'R' || ch == 'r')
            {
                break;
            }
            else
            {
                LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U); /* 1U is trigger0 mask. */

                while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &mLpadcResultConfigStruct))
                {
                }
                adcValue = (mLpadcResultConfigStruct.convValue) >> g_LpadcResultShift;
                voltage  = (DEMO_LPADC_VREF_VOLTAGE * ((float)adcValue / (float)g_LpadcFullRange));
                if (mLpadcResultConfigStruct.commandIdSource == DEMO_LPADC_USER_CMDID1)
                {
                    PRINTF("\r\n Vsw1 ADC value: %d", adcValue);
                    PRINTF("\r\n Actual voltage on Vsw1: %.3fV", (double)voltage);
                }

                while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &mLpadcResultConfigStruct))
                {
                }
                adcValue = (mLpadcResultConfigStruct.convValue) >> g_LpadcResultShift;
                voltage  = (DEMO_LPADC_VREF_VOLTAGE * ((float)adcValue / (float)g_LpadcFullRange));
                if (mLpadcResultConfigStruct.commandIdSource == DEMO_LPADC_USER_CMDID2)
                {
                    PRINTF("\r\n Vpref ADC value: %d", adcValue);
                    PRINTF("\r\n Actual voltage on Vpref: %.3fV", (double)voltage);
                }
            }
            PRINTF("\r\n");
            break;
        }
    }
}

/*! @brief This function is used to configure DAC. */
void DEMO_DoDacConfig(void)
{
    dac_config_t dacConfig;
    DAC_GetDefaultConfig(&dacConfig);
    dacConfig.referenceVoltageSource = DEMO_DAC_VREF;
    DAC_Init(DEMO_DAC_BASE, &dacConfig);
    DAC_Enable(DEMO_DAC_BASE, true);
}

/*! @brief This function is used to configure OPAMP. */
void DEMO_DoOpampConfig(void)
{
    opamp_config_t config;

    OPAMP_GetDefaultConfig(&config);
    config.posGain            = kOPAMP_PosGainNonInvert1X;
    config.negGain            = kOPAMP_NegGainInvert1X;
    config.posRefVoltage      = kOPAMP_PosRefVoltVrefh3;
    config.enableRefBuffer    = true;
    config.enablePosADCSw1    = true;
    config.enablePosADCSw2    = true;
    config.enableOutputSwitch = true;
    config.enable             = true;
    OPAMP_Init(DEMO_OPAMP_BASE, &config);
}

/*! @brief This function is used to configure LPADC.  */
void DEMO_DoLpadcConfig(void)
{
    lpadc_config_t mLpadcConfigStruct;
    lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
    lpadc_conv_command_config_t mLpadcCommandConfigStruct;

    /* Do LPADC configuration. */
    LPADC_GetDefaultConfig(&mLpadcConfigStruct);
    mLpadcConfigStruct.enableAnalogPreliminary = true;
    mLpadcConfigStruct.referenceVoltageSource  = DEMO_LPADC_VREF_SOURCE;
#if (defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS)
    mLpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
#endif /* (defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) */
    LPADC_Init(DEMO_LPADC_BASE, &mLpadcConfigStruct);

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
    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    mLpadcCommandConfigStruct.channelNumber     = DEMO_LPADC_USER_CHANNEL1;
    mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;
#if (defined(FSL_FEATURE_LPADC_HAS_CMDL_MODE) && FSL_FEATURE_LPADC_HAS_CMDL_MODE)
    mLpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionStandard;
#endif /* (defined(FSL_FEATURE_LPADC_HAS_CMDL_MODE) && FSL_FEATURE_LPADC_HAS_CMDL_MODE) */
    mLpadcCommandConfigStruct.chainedNextCommandNumber = DEMO_LPADC_USER_CMDID2;
    mLpadcCommandConfigStruct.hardwareAverageMode      = kLPADC_HardwareAverageCount4;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID1, &mLpadcCommandConfigStruct);

    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    mLpadcCommandConfigStruct.channelNumber     = DEMO_LPADC_USER_CHANNEL1;
    mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;
#if (defined(FSL_FEATURE_LPADC_HAS_CMDL_MODE) && FSL_FEATURE_LPADC_HAS_CMDL_MODE)
    mLpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionStandard;
#endif /* (defined(FSL_FEATURE_LPADC_HAS_CMDL_MODE) && FSL_FEATURE_LPADC_HAS_CMDL_MODE) */
    mLpadcCommandConfigStruct.chainedNextCommandNumber = 0U;
    mLpadcCommandConfigStruct.hardwareAverageMode      = kLPADC_HardwareAverageCount4;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID2, &mLpadcCommandConfigStruct);

    /* Do LPADC trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
    mLpadcTriggerConfigStruct.targetCommandId       = DEMO_LPADC_USER_CMDID1;
    mLpadcTriggerConfigStruct.enableHardwareTrigger = false;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &mLpadcTriggerConfigStruct);
}

/*! @brief This function is used to set DAC input value. */
void DEMO_DoDacValueSet(void)
{
    int ch            = 0U;
    uint32_t dacValue = 0UL;

    PRINTF("\r\n Please input a value (0 - 4095) for DAC, press the entry button to indicate the end of input:");

    while (ch != 0x0D)
    {
        ch = GETCHAR();
        if ((ch >= '0') && (ch <= '9'))
        {
            PUTCHAR(ch);
            dacValue = dacValue * 10 + (ch - '0');
        }
        else if (((ch < '0') || (ch > '9')) && (ch != 0x0D))
        {
            PRINTF("\r\n The input DAC value is not allowed!");
            assert(false);
        }
        else
        {
            /* Branch to avoid MISRC-2012 issue. */
        }
    }

    PRINTF("\r\n Input DAC value is %d", dacValue);
    if (dacValue > 4095)
    {
        PRINTF("\r\n DAC value is out of range.");
        assert(false);
    }

    DAC_SetData(DEMO_DAC_BASE, dacValue);
    PRINTF("\r\n The Current DAC output is about %6.3f mV", (double)(DAC_VALUE_TO_MVOLT(dacValue)));
}

/*! @brief This function is used to set OPAMP negative gain. */
void DEMO_DoNgainValueSet(void)
{
    int ch = 0;

    PRINTF(
        "\r\n Please select OPAMP negative port gain from the following options.\
            \r\n Input 0 means select 1x. \
            \r\n Input 1 means select 2x. \
            \r\n Input 2 means select 4x. \
            \r\n Input 3 means select 8x. \
            \r\n Input 4 means select 16x. \
            \r\n Input 5 means select 33x. \
            \r\n Input 6 means select 64x.");

    ch = GETCHAR();
    if ((ch >= '0') && (ch <= '6'))
    {
        PRINTF("\r\n %d", ch - '0');
        switch (ch)
        {
            case '0':
                PRINTF("\r\n Input OPAMP negative port gain is 1x");
                OPAMP_DoNegGainConfig(DEMO_OPAMP_BASE, kOPAMP_NegGainInvert1X);
                break;
            case '1':
                PRINTF("\r\n Input OPAMP negative port gain is 2x");
                OPAMP_DoNegGainConfig(DEMO_OPAMP_BASE, kOPAMP_NegGainInvert2X);
                break;
            case '2':
                PRINTF("\r\n Input OPAMP negative port gain is 4x");
                OPAMP_DoNegGainConfig(DEMO_OPAMP_BASE, kOPAMP_NegGainInvert4X);
                break;
            case '3':
                PRINTF("\r\n Input OPAMP negative port gain is 8x");
                OPAMP_DoNegGainConfig(DEMO_OPAMP_BASE, kOPAMP_NegGainInvert8X);
                break;
            case '4':
                PRINTF("\r\n Input OPAMP negative port gain is 16x");
                OPAMP_DoNegGainConfig(DEMO_OPAMP_BASE, kOPAMP_NegGainInvert16X);
                break;
            case '5':
                PRINTF("\r\n Input OPAMP negative port gain is 33x");
                OPAMP_DoNegGainConfig(DEMO_OPAMP_BASE, kOPAMP_NegGainInvert33X);
                break;
            case '7':
                PRINTF("\r\n Input OPAMP negative port gain is 64x");
                OPAMP_DoNegGainConfig(DEMO_OPAMP_BASE, kOPAMP_NegGainInvert64X);
                break;
            default:
                assert(false);
                break;
        }
    }
    else
    {
        PRINTF("\r\n The selected option is not allowed!");
        assert(false);
    }
}
