/*
 * Copyright 2022 NXP
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
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_OPAMP_BASEADDR OPAMP0

#define DEMO_LPADC_BASE          ADC1
#define DEMO_LPADC_USER_CHANNELA 1U
#define DEMO_LPADC_USER_CHANNELB 24U
#define DEMO_LPADC_USER_CMDID    1U
#define DEMO_LPADC_VREF_SOURCE   kLPADC_ReferenceVoltageAlt3
#define DEMO_LPADC_VREF_VOLTAGE  3.300f

#define DEMO_DAC_BASEADDR  DAC0
#define DEMO_DAC_VREF      kDAC_ReferenceVoltageSourceAlt1
#define DEMO_DAC_VOLT_STEP 0.806f

#define DAC_VALUE_TO_MVOLT(dacValue) ((float)(dacValue)*DEMO_DAC_VOLT_STEP)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitDebugConsole(void);
void DAC_Configuration(void);
void OPAMP_Configuration(void);
void ADC_Configuration(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (defined(DEMO_LPADC_USE_HIGH_RESOLUTION) && DEMO_LPADC_USE_HIGH_RESOLUTION)
const uint32_t g_LpadcFullRange   = 65536U;
const uint32_t g_LpadcResultShift = 0U;
#else
const uint32_t g_LpadcFullRange   = 4096U;
const uint32_t g_LpadcResultShift = 3U;
#endif /* DEMO_LPADC_USE_HIGH_RESOLUTION */
lpadc_conv_result_t mLpadcResultConfigStruct;
/*******************************************************************************
 * Code
 ******************************************************************************/
int main(void)
{
    uint8_t ch;
    uint32_t dacValue;
    uint16_t adcValue = 0U;
    float voltage     = 0.0f;

    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO HF to DAC0 */
    CLOCK_SetClkDiv(kCLOCK_DivDac0Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_DAC0);

    /* attach FRO HF to ADC1 */
    CLOCK_SetClkDiv(kCLOCK_DivAdc1Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_ADC1);

    /* enable analog module */
    SPC0->ACTIVE_CFG1 |= 0x111;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    DAC_Configuration();
    ADC_Configuration();
    OPAMP_Configuration();
    PRINTF("OPAMP DAC ADC EXAMPLE!\r\n");

    while (1)
    {
        ch       = 0U;
        dacValue = 0UL;
        PRINTF("\r\nPlease input a value (0 - 4095) for DAC:");

        while (ch != 0x0D)
        {
            ch = GETCHAR();
            if ((ch >= '0') && (ch <= '9'))
            {
                PUTCHAR(ch);
                dacValue = dacValue * 10 + (ch - '0');
            }
        }

        PRINTF("\r\nInput DAC value is %d\r\n", dacValue);
        if (dacValue > 4095)
        {
            PRINTF("DAC value is out of range.\r\n");
            continue;
        }

        DAC_SetData(DEMO_DAC_BASEADDR, dacValue);
        PRINTF("Current DAC output is about %6.3f mV\r\n", (double)(DAC_VALUE_TO_MVOLT(dacValue)));

        PRINTF("Please press any key excluding key (R or r) to get user channel's ADC value.\r\n");

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
                while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &mLpadcResultConfigStruct, 0U))
                {
                }
                adcValue = (mLpadcResultConfigStruct.convValue) >> g_LpadcResultShift;
                voltage  = (DEMO_LPADC_VREF_VOLTAGE * ((float)adcValue / (float)g_LpadcFullRange));
                PRINTF("Vsw1 ADC value: %d\r\n", adcValue);
                PRINTF("Actual voltage on Vsw1: %.3fV\r\n", (double)voltage);

                while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &mLpadcResultConfigStruct, 1U))
                {
                }
                adcValue = (mLpadcResultConfigStruct.convValue) >> g_LpadcResultShift;
                voltage  = (DEMO_LPADC_VREF_VOLTAGE * ((float)adcValue / (float)g_LpadcFullRange));
                PRINTF("Vref ADC value: %d\r\n", adcValue);
                PRINTF("Actual voltage on Vref: %.3fV\r\n", (double)voltage);
            }
        }
    }
}

void DAC_Configuration(void)
{
    dac_config_t dacConfig;
    DAC_GetDefaultConfig(&dacConfig);
    dacConfig.referenceVoltageSource = DEMO_DAC_VREF;
    DAC_Init(DEMO_DAC_BASEADDR, &dacConfig);
    DAC_Enable(DEMO_DAC_BASEADDR, true);
}

void OPAMP_Configuration(void)
{
    opamp_config_t config;
    /*
     *  config->enable        = false;
     *  config->enablePosADCSw1 = false;
     *  config->mode          = kOPAMP_LowNoiseMode;
     *  config->trimOption    = kOPAMP_TrimOptionDefault;
     *  config->intRefVoltage = kOPAMP_IntRefVoltVddaDiv2;
     *  config->enablePosADCSw1 = false;
     *  config->enablePosADCSw2 = false;
     *  config->posRefVoltage = kOPAMP_PosRefVoltVrefh3;
     *  config->posGain       = kOPAMP_PosGainReserved;
     *  config->negGain       = kOPAMP_NegGainBufferMode;
     *  config->enableRefBuffer = false;
     *  config->PosInputChannelSelection  = kOPAMP_PosInputChannel0
     *  config->enableTriggerMode = false;
     *  config->enableOutputSwitch = true;
     */

    OPAMP_GetDefaultConfig(&config);
    config.posGain         = kOPAMP_PosGainNonInvert1X;
    config.negGain         = kOPAMP_NegGainInvert1X;
    config.posRefVoltage   = kOPAMP_PosRefVoltVrefh3;
    config.enable          = true;
    config.enableRefBuffer = true;
    config.enablePosADCSw1 = true;
    config.enablePosADCSw2 = true;
    OPAMP_Init(DEMO_OPAMP_BASEADDR, &config);
}
void ADC_Configuration(void)
{
    lpadc_config_t mLpadcConfigStruct;
    lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
    lpadc_conv_command_config_t mLpadcCommandConfigStruct;

    LPADC_GetDefaultConfig(&mLpadcConfigStruct);
    mLpadcConfigStruct.enableAnalogPreliminary = true;
#if defined(DEMO_LPADC_VREF_SOURCE)
    mLpadcConfigStruct.referenceVoltageSource = DEMO_LPADC_VREF_SOURCE;
#endif /* DEMO_LPADC_VREF_SOURCE */
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS
    mLpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS */
    LPADC_Init(DEMO_LPADC_BASE, &mLpadcConfigStruct);

    LPADC_DoOffsetCalibration(DEMO_LPADC_BASE);
    /* Request gain calibration. */
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);

    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelDualSingleEndBothSide;
    mLpadcCommandConfigStruct.channelNumber     = DEMO_LPADC_USER_CHANNELA;
#if defined(DEMO_LPADC_USE_HIGH_RESOLUTION) && DEMO_LPADC_USE_HIGH_RESOLUTION
    mLpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
#endif /* DEMO_LPADC_USE_HIGH_RESOLUTION */
    mLpadcCommandConfigStruct.channelBNumber      = DEMO_LPADC_USER_CHANNELB;
    mLpadcCommandConfigStruct.enableChannelB      = true;
    mLpadcCommandConfigStruct.hardwareAverageMode = kLPADC_HardwareAverageCount4;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &mLpadcCommandConfigStruct);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
    mLpadcTriggerConfigStruct.targetCommandId       = DEMO_LPADC_USER_CMDID;
    mLpadcTriggerConfigStruct.enableHardwareTrigger = false;
    mLpadcTriggerConfigStruct.channelBFIFOSelect    = 1U;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */
}
