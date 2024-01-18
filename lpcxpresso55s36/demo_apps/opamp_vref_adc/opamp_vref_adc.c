/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_lpadc.h"
#include "fsl_vref.h"
#include "fsl_opamp.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_LPADC_BASE        ADC0
#define DEMO_LPADC_VREF_SOURCE kLPADC_ReferenceVoltageAlt3

#define DEMO_LPADC_USER_CHANNEL 1U
#define DEMO_LPADC_USER_CMDID   1U

#define DEMO_OPAMP_BASEADDR              OPAMP0
#define DEMO_VREF_BASE                   VREF
#define DEMO_LPADC_DO_OFFSET_CALIBRATION true
#define DEMO_LPADC_USE_HIGH_RESOLUTION   true
#define DEMO_LPADC_VREF_VOLTAGE          3.280f

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief Initialize the OPAMP.
 */
static void OPAMP_Configuration(void);
/*!
 * @brief Initialize the VREF.
 */
static void VREF_Configuration(void);
/*!
 * @brief Initialize the LPADC.
 */
static void LPADC_Configuration(void);

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
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    lpadc_conv_result_t mLpadcResultConfigStruct;
    uint8_t ch;
    uint8_t trimVal;
    uint16_t adcValue = 0U;
    float voltage     = 0.0f;

    /* Init board hardware. */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();
#if !defined(DONOT_ENABLE_FLASH_PREFETCH)
    /* enable flash prefetch for better performance */
    SYSCON->FMCCR |= SYSCON_FMCCR_PREFEN_MASK;
#endif

    /* Disable VREF power down */
    POWER_DisablePD(kPDRUNCFG_PD_VREF);
    POWER_DisablePD(kPDRUNCFG_PD_OPAMP0);

    CLOCK_SetClkDiv(kCLOCK_DivAdc0Clk, 2U, true);
    CLOCK_AttachClk(kFRO_HF_to_ADC0);

    PRINTF("OPAMP VREF LPADC DEMO\r\n");
    PRINTF("ADC Full Range: %d\r\n", g_LpadcFullRange);

    VREF_Configuration();  /* Initialize the VREF*/
    OPAMP_Configuration(); /* Initialize the OPAMP*/
    LPADC_Configuration(); /* Initialize the LPADC*/

    while (1)
    {
        ch      = 0U;
        trimVal = 0U;

        PRINTF("\r\nPlease input a trim value (0-11):");

        while (ch != 0x0D)
        {
            ch = GETCHAR();
            if ((ch >= '0') && (ch <= '9'))
            {
                PUTCHAR(ch);
                trimVal = trimVal * 10 + (ch - '0');
            }
        }
        /* Check the trim value range */
        if (trimVal > 11)
        {
            continue;
        }

        VREF_SetTrim21Val(DEMO_VREF_BASE, trimVal);
        PRINTF("\r\nUse trim value: %d", trimVal);
        PRINTF("\r\nExpected voltage on VREF_OUT: %.3fV\r\n", (1.000f + (0.100f * (double)trimVal)));

        PRINTF("Please press any key exluding key (R or r) to get user channel's ADC value.\r\n");

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
                PRINTF("ADC value: %d\r\n", adcValue);
                PRINTF("Actual voltage on OPAMP_OUT: %.3fV\r\n", (double)voltage);
            }
        }
    }
}

static void OPAMP_Configuration(void)
{
    opamp_config_t config;
    /*
     *  config->enable        = false;
     *  config->mode          = kOPAMP_LowNoiseMode;
     *  config->trimOption    = kOPAMP_TrimOptionDefault;
     *  config->intRefVoltage = kOPAMP_IntRefVoltVddaDiv2;
     *
     *  config->enablePosADCSw = false;
     *  config->posRefVoltage = kOPAMP_PosRefVoltVrefh3;
     *  config->posGain       = kOPAMP_PosGainReserved;
     *
     *  config->negGain       = kOPAMP_NegGainBufferMode;
     */
    OPAMP_GetDefaultConfig(&config);
    config.posGain = kOPAMP_PosGainNonInvert1X;
    config.negGain = kOPAMP_NegGainInvert1X;
    /* Connect REFP to VREF output. */
    config.posRefVoltage = kOPAMP_PosRefVoltVrefh1;
    config.enable        = true;

    OPAMP_Init(DEMO_OPAMP_BASEADDR, &config);
}

static void VREF_Configuration(void)
{
    vref_config_t vrefConfig;

    VREF_GetDefaultConfig(&vrefConfig);
    /* Initialize the VREF mode. */
    VREF_Init(DEMO_VREF_BASE, &vrefConfig);
}

static void LPADC_Configuration(void)
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

    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    mLpadcCommandConfigStruct.channelNumber = DEMO_LPADC_USER_CHANNEL;
#if defined(DEMO_LPADC_USE_HIGH_RESOLUTION) && DEMO_LPADC_USE_HIGH_RESOLUTION
    mLpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
#endif /* DEMO_LPADC_USE_HIGH_RESOLUTION */
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &mLpadcCommandConfigStruct);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
    mLpadcTriggerConfigStruct.targetCommandId       = DEMO_LPADC_USER_CMDID;
    mLpadcTriggerConfigStruct.enableHardwareTrigger = false;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */
}
