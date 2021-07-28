/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpadc.h"
#include "fsl_adc_etc.h"
#include "fsl_pit.h"
#include "fsl_xbara.h"
#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ADC_BASE           LPADC1
#define DEMO_ADC_USER_CHANNEL   0U
#define DEMO_ADC_USER_CMDID     1U
#define DEMO_ADC_CHANNEL_GROUP0 0U
#define DEMO_ADC_CHANNEL_GROUP1 1U

#define DEMO_ADC_ETC_BASE          ADC_ETC
#define DEMO_ADC_ETC_CHAIN_LENGTH  1U /* Chain length is 2. */
#define DEMO_ADC_ETC_CHANNEL0      0U
#define DEMO_ADC_ETC_CHANNEL1      1U
#define DEMO_ADC_ETC_TRIGGER_GROUP 0U
#define DEMO_ADC_ETC_DONE0_Handler ADC_ETC_IRQ0_IRQHandler
#define DEMO_ADC_ETC_DONE1_Handler ADC_ETC_IRQ1_IRQHandler

#define DEMO_XBARA_BASE           XBARA1
#define DEMO_XBARA_INPUT_PITCH0   kXBARA1_InputPit1Trigger0
#define DEMO_XBARA_OUTPUT_ADC_ETC kXBARA1_OutputAdcEtc0Coco0

/*
 * Clock source:
 * PIT1: Bus clock
 * PIT2: Bus LPSR clock
 */
#define DEMO_PIT_BASE         PIT1
#define DEMO_PIT_CLOCK_SOURCE CLOCK_GetRootClockFreq(kCLOCK_Root_Bus)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ADC_ETC_Configuration(void);
void LPADC_Configuration(void);
void XBARA_Configuration(void);
void PIT_Configuration(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_AdcConversionValue0;
volatile uint32_t g_AdcConversionValue1;
const uint32_t g_Adc_12bitFullRange = 4096U;

/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_ADC_ETC_DONE0_Handler(void)
{
    ADC_ETC_ClearInterruptStatusFlags(DEMO_ADC_ETC_BASE, kADC_ETC_Trg0TriggerSource, kADC_ETC_Done0StatusFlagMask);
    /* Get result from the trigger source chain 0. */
    g_AdcConversionValue0 = ADC_ETC_GetADCConversionValue(DEMO_ADC_ETC_BASE, DEMO_ADC_ETC_TRIGGER_GROUP, 0U);
    __DSB();
}

void DEMO_ADC_ETC_DONE1_Handler(void)
{
    ADC_ETC_ClearInterruptStatusFlags(DEMO_ADC_ETC_BASE, kADC_ETC_Trg0TriggerSource, kADC_ETC_Done1StatusFlagMask);
    /* Get result from the trigger source chain 1. */
    g_AdcConversionValue1 = ADC_ETC_GetADCConversionValue(DEMO_ADC_ETC_BASE, DEMO_ADC_ETC_TRIGGER_GROUP, 1U);
    __DSB();
}

/*!
 * @brief Main function.
 */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("ADC_ETC_Hardware_Trigger_Conv Example Start!\r\n");

    LPADC_Configuration();
    XBARA_Configuration();
    PIT_Configuration();
    ADC_ETC_Configuration();

    /* Enable the NVIC. */
    EnableIRQ(ADC_ETC_IRQ0_IRQn);
    EnableIRQ(ADC_ETC_IRQ1_IRQn);

    /* Start PIT channel0. */
    PIT_StartTimer(DEMO_PIT_BASE, kPIT_Chnl_0);

    PRINTF("ADC Full Range: %d\r\n", g_Adc_12bitFullRange);
    PRINTF("Please press any key to get user channel's ADC value.\r\n");

    while (1)
    {
        GETCHAR();
        PRINTF("ADC conversion value is %d and %d\r\n", g_AdcConversionValue0, g_AdcConversionValue1);
    }
}

void ADC_ETC_Configuration(void)
{
    adc_etc_config_t adcEtcConfig;
    adc_etc_trigger_config_t adcEtcTriggerConfig;
    adc_etc_trigger_chain_config_t adcEtcTriggerChainConfig;

    /* Initialize the ADC_ETC. */
    ADC_ETC_GetDefaultConfig(&adcEtcConfig);
    adcEtcConfig.XBARtriggerMask = 1U; /* Enable the external XBAR trigger0. */
    ADC_ETC_Init(DEMO_ADC_ETC_BASE, &adcEtcConfig);

    /* Set the external XBAR trigger0 configuration. */
    adcEtcTriggerConfig.enableSyncMode      = false;
    adcEtcTriggerConfig.enableSWTriggerMode = false;
    adcEtcTriggerConfig.triggerChainLength  = 1U; /* Chain length 2. */
    adcEtcTriggerConfig.triggerPriority     = 0U;
    adcEtcTriggerConfig.sampleIntervalDelay = 0U;
    adcEtcTriggerConfig.initialDelay        = 0U;
    ADC_ETC_SetTriggerConfig(DEMO_ADC_ETC_BASE, 0U, &adcEtcTriggerConfig);

    /* Set the external XBAR trigger0 chain configuration. */
    adcEtcTriggerChainConfig.enableB2BMode       = true;
    adcEtcTriggerChainConfig.ADCHCRegisterSelect = 1U
                                                   << DEMO_ADC_CHANNEL_GROUP0; /* Select ADC_HC0 register to trigger. */
    adcEtcTriggerChainConfig.ADCChannelSelect =
        DEMO_ADC_ETC_CHANNEL0; /* ADC_HC0 will be triggered to sample Corresponding channel. */
    adcEtcTriggerChainConfig.InterruptEnable = kADC_ETC_Done0InterruptEnable; /* Enable the Done0 interrupt. */
#if defined(FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN) && FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN
    adcEtcTriggerChainConfig.enableIrq = true; /* Enable the IRQ. */
#endif                                         /* FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN */
    ADC_ETC_SetTriggerChainConfig(DEMO_ADC_ETC_BASE, DEMO_ADC_ETC_TRIGGER_GROUP, 0U,
                                  &adcEtcTriggerChainConfig); /* Configure the trigger0 chain0. */
    adcEtcTriggerChainConfig.ADCHCRegisterSelect = 1U
                                                   << DEMO_ADC_CHANNEL_GROUP1; /* Select ADC_HC1 register to trigger. */
    adcEtcTriggerChainConfig.ADCChannelSelect =
        DEMO_ADC_ETC_CHANNEL1; /* ADC_HC1 will be triggered to sample Corresponding channel. */
    adcEtcTriggerChainConfig.InterruptEnable = kADC_ETC_Done1InterruptEnable; /* Enable the Done1 interrupt. */
#if defined(FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN) && FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN
    adcEtcTriggerChainConfig.enableIrq = true; /* Enable the IRQ. */
#endif                                         /* FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN */
    /* Configure the trigger group chain 1. */
    ADC_ETC_SetTriggerChainConfig(DEMO_ADC_ETC_BASE, DEMO_ADC_ETC_TRIGGER_GROUP, 1U, &adcEtcTriggerChainConfig);
}

/*!
 * @brief Configure LPADC to working with ADC_ETC.
 */
void LPADC_Configuration(void)
{
    lpadc_config_t lpadcConfig;
    lpadc_conv_command_config_t lpadcCommandConfig;
    lpadc_conv_trigger_config_t lpadcTriggerConfig;

    /* Initialize the ADC module. */
    LPADC_GetDefaultConfig(&lpadcConfig);
    LPADC_Init(DEMO_ADC_BASE, &lpadcConfig);

#if (defined(FSL_FEATURE_LPADC_HAS_CFG_CALOFS) && FSL_FEATURE_LPADC_HAS_CFG_CALOFS)
    /* Do offset calibration. */
    LPADC_DoOffsetCalibration(DEMO_ADC_BASE, SystemCoreClock);
#endif /* FSL_FEATURE_LPADC_HAS_CFG_CALOFS */

    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&lpadcCommandConfig);
    lpadcCommandConfig.channelNumber = DEMO_ADC_USER_CHANNEL;
    LPADC_SetConvCommandConfig(DEMO_ADC_BASE, DEMO_ADC_USER_CMDID, &lpadcCommandConfig);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&lpadcTriggerConfig);
    lpadcTriggerConfig.targetCommandId       = DEMO_ADC_USER_CMDID;
    lpadcTriggerConfig.enableHardwareTrigger = true;
    LPADC_SetConvTriggerConfig(DEMO_ADC_BASE, 0U, &lpadcTriggerConfig);
    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&lpadcTriggerConfig);
    lpadcTriggerConfig.targetCommandId       = DEMO_ADC_USER_CMDID;
    lpadcTriggerConfig.enableHardwareTrigger = true;
    LPADC_SetConvTriggerConfig(DEMO_ADC_BASE, 1U, &lpadcTriggerConfig);
}

/*!
 * @brief Configure XBARA to work with ADC_ETC.
 */
void XBARA_Configuration(void)
{
    /* Init xbara module. */
    XBARA_Init(DEMO_XBARA_BASE);

    /* Configure the XBARA signal connections. */
    XBARA_SetSignalsConnection(DEMO_XBARA_BASE, DEMO_XBARA_INPUT_PITCH0, DEMO_XBARA_OUTPUT_ADC_ETC);
}

/*!
 * @brief Configuration PIT to trigger ADC_ETC.
 */
void PIT_Configuration(void)
{
    /* Structure of initialize PIT */
    pit_config_t pitConfig;

    /* Init pit module */
    PIT_GetDefaultConfig(&pitConfig);
    PIT_Init(DEMO_PIT_BASE, &pitConfig);

    /* Set timer period for channel 0 */
    PIT_SetTimerPeriod(DEMO_PIT_BASE, kPIT_Chnl_0, USEC_TO_COUNT(1000000U, DEMO_PIT_CLOCK_SOURCE));
}
