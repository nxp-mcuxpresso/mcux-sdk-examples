/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "fsl_common.h"

#include "fsl_lpadc.h"
#include "fsl_pit.h"

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

#define DEMO_PIT_BASE        PIT1
#define DEMO_PIT_CHANNEL_NUM kPIT_Chnl_0
#define DEMO_PIT_IRQn        PIT1_IRQn
#define DEMO_PIT_IRQ_HANDLER PIT1_IRQHandler
#define DEMO_PIT_CLOCK_FREQ  CLOCK_GetRootClockFreq(kCLOCK_Root_Bus)
#define DEMO_ADC_FIFO_DEPTH 0xFU

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitADCClock(void);
static lpadc_sample_time_mode_t DEMO_SelectSampleTime(void);
static void DEMO_ADCStartSample(lpadc_sample_time_mode_t sampleTimeMode);
static void DEMO_TimerInit(void);
static void DEMO_ADCInit(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_timeOut             = false;
volatile uint32_t g_conversionCount = 0UL;

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

void DEMO_PIT_IRQ_HANDLER(void)
{
    LPADC_Enable(DEMO_LPADC_BASE, false);
    LPADC_DisableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFOWatermarkInterruptEnable);
    DisableIRQ(DEMO_LPADC_IRQn);
    PIT_ClearStatusFlags(DEMO_PIT_BASE, DEMO_PIT_CHANNEL_NUM, kPIT_TimerFlag);
    DisableIRQ(DEMO_PIT_IRQn);
    PIT_StopTimer(DEMO_PIT_BASE, DEMO_PIT_CHANNEL_NUM);
    g_timeOut = true;
    __DSB();
}

void DEMO_LPADC_IRQ_HANDLER(void)
{
    g_conversionCount += 16UL;
    /* Clear the FIFO. */
    LPADC_DoResetFIFO(DEMO_LPADC_BASE);
    while ((LPADC_GetStatusFlags(DEMO_LPADC_BASE) & kLPADC_ResultFIFOReadyFlag) == kLPADC_ResultFIFOReadyFlag)
    {
    }
    LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1UL);
    __DSB();
}

int main(void)
{
    lpadc_sample_time_mode_t sampleTimeMode;

    clock_root_config_t busRootCfg = {0};
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Set bus clock root mux to system PLL3. */
    busRootCfg.mux = kCLOCK_BUS_ClockRoot_MuxSysPll3Out;
    busRootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &busRootCfg);
    BOARD_InitADCClock();

    PRINTF("ADC High Sample Rate Demo!\r\n");
    PRINTF("Current ADC clock frequency is %d Hz!\r\n", CLOCK_GetFreqFromObs(CCM_OBS_ADC1_CLK_ROOT));

    DEMO_ADCInit();
    DEMO_TimerInit();
    while (1)
    {
        sampleTimeMode = DEMO_SelectSampleTime();
        DEMO_ADCStartSample(sampleTimeMode);
        EnableIRQ(DEMO_PIT_IRQn);
        g_timeOut         = false;
        g_conversionCount = 0UL;
        PRINTF("\r\nPlease press any keys to trigger ADC conversion!\r\n");
        (void)GETCHAR();
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1UL);
        PIT_StartTimer(DEMO_PIT_BASE, DEMO_PIT_CHANNEL_NUM);
        while (1)
        {
            if (g_timeOut)
            {
                if ((LPADC_GetStatusFlags(DEMO_LPADC_BASE) & kLPADC_ResultFIFOReadyFlag) == kLPADC_ResultFIFOReadyFlag)
                {
                    g_conversionCount += 16UL;
                }
                g_conversionCount += LPADC_GetConvResultCount(DEMO_LPADC_BASE);
                /* Due to hardware arverage set as 128, so the result should multiply by 128. */
                PRINTF("Sample Rate: %d SPS\r\n", (g_conversionCount * 128U));
                g_timeOut = false;
                break;
            }
        }
    }
}

/*!
 * @brief Selects sample time from user input.
 *
 * @return The sample time mode in type of lpadc_sample_time_mode_t.
 */
static lpadc_sample_time_mode_t DEMO_SelectSampleTime(void)
{
    char ch;

    PRINTF("\r\nPlease select sample time!\r\n");
    PRINTF("\tA -- 3 ADCK cycles total sample time\r\n");
    PRINTF("\tB -- 5 ADCK cycles total sample time\r\n");
    PRINTF("\tC -- 7 ADCK cycles total sample time\r\n");
    PRINTF("\tD -- 11 ADCK cycles total sample time\r\n");
    PRINTF("\tE -- 19 ADCK cycles total sample time\r\n");
    PRINTF("\tF -- 35 ADCK cycles total sample time\r\n");
    PRINTF("\tG -- 67 ADCK cycles total sample time\r\n");
    PRINTF("\tH -- 131 ADCK cycles total sample time\r\n");

    ch = GETCHAR();
    PUTCHAR(ch);

    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch -= 'a' - 'A';
    }

    return (lpadc_sample_time_mode_t)(ch - 'A');
}

/*!
 * @brief Configures ADC module.
 */
static void DEMO_ADCInit(void)
{
    lpadc_config_t adcConfig;
    lpadc_conv_trigger_config_t softwareTriggerConfig;

    LPADC_GetDefaultConfig(&adcConfig);
    adcConfig.enableAnalogPreliminary = true;
    adcConfig.powerLevelMode          = kLPADC_PowerLevelAlt4; /* Highest power setting. */
    adcConfig.FIFOWatermark           = DEMO_ADC_FIFO_DEPTH;
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

    LPADC_GetDefaultConvTriggerConfig(&softwareTriggerConfig);
    softwareTriggerConfig.targetCommandId = 1U;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &softwareTriggerConfig);
}

/*!
 * @brief Configures ADC sample mode.
 *
 * @param sampleTimeMode The sample time mode to be set.
 */
static void DEMO_ADCStartSample(lpadc_sample_time_mode_t sampleTimeMode)
{
    lpadc_conv_command_config_t commandConfig;

    /* Disenable the module after setting configuration. */
    LPADC_Enable(DEMO_LPADC_BASE, false);
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

    commandConfig.hardwareAverageMode = kLPADC_HardwareAverageCount128;

    commandConfig.loopCount      = DEMO_ADC_FIFO_DEPTH;
    commandConfig.sampleTimeMode = sampleTimeMode;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, 1U, &commandConfig);

    LPADC_DoResetFIFO(DEMO_LPADC_BASE);
    LPADC_ClearStatusFlags(DEMO_LPADC_BASE, kLPADC_ResultFIFOReadyFlag | kLPADC_ResultFIFOOverflowFlag);

    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFOWatermarkInterruptEnable);
    EnableIRQ(DEMO_LPADC_IRQn);
    /* Enable the module after setting configuration. */
    LPADC_Enable(DEMO_LPADC_BASE, true);
}

/*!
 * @brief Configures Timer, as period as 1s.
 */
static void DEMO_TimerInit(void)
{
    pit_config_t pitConfig;

    PIT_GetDefaultConfig(&pitConfig);
    PIT_Init(DEMO_PIT_BASE, &pitConfig);

    PIT_SetTimerPeriod(DEMO_PIT_BASE, DEMO_PIT_CHANNEL_NUM, USEC_TO_COUNT(1000000U, DEMO_PIT_CLOCK_FREQ));
    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(DEMO_PIT_BASE, DEMO_PIT_CHANNEL_NUM, kPIT_TimerInterruptEnable);
}
