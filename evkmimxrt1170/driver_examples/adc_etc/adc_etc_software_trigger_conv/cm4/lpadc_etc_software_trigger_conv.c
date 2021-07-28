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
#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ADC_BASE          LPADC1
#define DEMO_ADC_USER_CHANNEL  0U
#define DEMO_ADC_USER_CMDID    1U
#define DEMO_ADC_CHANNEL_GROUP 0U

#define DEMO_ADC_ETC_BASE          ADC_ETC
#define DEMO_ADC_ETC_TRIGGER_GROUP 0U
#define DEMO_ADC_ETC_CHANNEL       0U
#define DEMO_ADC_ETC_DONE0_Handler ADC_ETC_IRQ0_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void LPADC_Configuration(void);
void ADC_ETC_Configuration(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_AdcConversionDoneFlag;
volatile uint32_t g_AdcConversionValue;
const uint32_t g_Adc_12bitFullRange = 4096U;

/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_ADC_ETC_DONE0_Handler(void)
{
    ADC_ETC_ClearInterruptStatusFlags(DEMO_ADC_ETC_BASE, kADC_ETC_Trg0TriggerSource, kADC_ETC_Done0StatusFlagMask);
    g_AdcConversionDoneFlag = true;
    /* Get result from the trigger source chain 0. */
    g_AdcConversionValue = ADC_ETC_GetADCConversionValue(DEMO_ADC_ETC_BASE, DEMO_ADC_ETC_TRIGGER_GROUP, 0U);
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

    PRINTF("ADC_ETC_Software_Trigger_Conv Example Start!\r\n");

    LPADC_Configuration();
    ADC_ETC_Configuration();
    /* Enable the NVIC. */
    EnableIRQ(ADC_ETC_IRQ0_IRQn);

    PRINTF("ADC Full Range: %d\r\n", g_Adc_12bitFullRange);
    while (1)
    {
        g_AdcConversionDoneFlag = false;
        PRINTF("Press any key to get user channel's ADC value.\r\n");
        GETCHAR();
        ADC_ETC_DoSoftwareTrigger(DEMO_ADC_ETC_BASE, DEMO_ADC_ETC_TRIGGER_GROUP); /* Do software XBAR trigger0. */
        while (!g_AdcConversionDoneFlag)
        {
        }
        PRINTF("ADC conversion value is %d\r\n", g_AdcConversionValue);
    }
}

/*!
 * @brief Configure ADC_ETC.
 */
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
    adcEtcTriggerConfig.enableSWTriggerMode = true;
    adcEtcTriggerConfig.triggerChainLength  = 0U; /* Chain length 1. */
    adcEtcTriggerConfig.triggerPriority     = 0U;
    adcEtcTriggerConfig.sampleIntervalDelay = 0U;
    adcEtcTriggerConfig.initialDelay        = 0U;
    ADC_ETC_SetTriggerConfig(DEMO_ADC_ETC_BASE, DEMO_ADC_ETC_TRIGGER_GROUP, &adcEtcTriggerConfig);

    /* Set the external XBAR trigger0 chain0 configuration. */
    adcEtcTriggerChainConfig.enableB2BMode       = false;
    adcEtcTriggerChainConfig.ADCHCRegisterSelect = 1U
                                                   << DEMO_ADC_CHANNEL_GROUP; /* Select ADC_HC0 register to trigger. */
    adcEtcTriggerChainConfig.ADCChannelSelect =
        DEMO_ADC_ETC_CHANNEL; /* ADC_HC0 will be triggered to sample Corresponding channel. */
    adcEtcTriggerChainConfig.InterruptEnable = kADC_ETC_Done0InterruptEnable; /* Enable the Done0 interrupt. */
#if defined(FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN) && FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN
    adcEtcTriggerChainConfig.enableIrq = true; /* Enable the IRQ. */
#endif                                         /* FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN */
    /* Configure the trigger group chain 0. */
    ADC_ETC_SetTriggerChainConfig(DEMO_ADC_ETC_BASE, DEMO_ADC_ETC_TRIGGER_GROUP, 0U, &adcEtcTriggerChainConfig);
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
}
