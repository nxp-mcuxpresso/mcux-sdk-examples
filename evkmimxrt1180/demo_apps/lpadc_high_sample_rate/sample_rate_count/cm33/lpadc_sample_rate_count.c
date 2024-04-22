/*
 * Copyright 2020-2021, 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "fsl_common.h"
#include "fsl_lpadc.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#if ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0))
#include "fsl_lpit.h"
#endif /* ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0)) */
#if ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0))
#include "fsl_pit.h"
#endif /* ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0)) */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPADC_BASE        ADC1
#define DEMO_LPADC_CHANNEL_NUM 1U
#define DEMO_LPADC_IRQn        ADC1_IRQn
#define DEMO_LPADC_IRQ_HANDLER ADC1_IRQHandler

#define DEMO_LPIT_BASE        LPIT3
#define DEMO_LPIT_CHANNEL_NUM kLPIT_Chnl_0
#define DEMO_LPIT_IRQn        LPIT3_IRQn
#define DEMO_LPIT_IRQ_HANDLER LPIT3_IRQHandler
#define DEMO_LPIT_CLOCK_FREQ  CLOCK_GetRootClockFreq(kCLOCK_Root_Lpit3)

#define DEMO_CLOCK_FUNCTION_PARAMETER_COUNT 2U
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

    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd3, 19U);

    /* The ADC1 module of the RT1180 maximum operating clock frequency is 84MHz, in order to achieve the
       highest conversion rate, set the ADC1 operating clock to the configurable highest frequency 83.375MHz.
     */
    adc1ClkRoot.mux      = kCLOCK_ADC1_ClockRoot_MuxSysPll2Pfd3; /* Set clock source as SYS PLL2 pfd3 CLK. */
    adc1ClkRoot.div      = 6U;
    adc1ClkRoot.clockOff = false;
    CLOCK_SetRootClock(kCLOCK_Root_Adc1, &adc1ClkRoot);
}
#if ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0))
void DEMO_LPIT_IRQ_HANDLER(void)
#endif /* ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0)) */
#if ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0))
    void DEMO_PIT_IRQ_HANDLER(void)
#endif /* ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0)) */
{
    LPADC_Enable(DEMO_LPADC_BASE, false);

#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    LPADC_DisableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFO0WatermarkInterruptEnable);
#else
    LPADC_DisableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFOWatermarkInterruptEnable);
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
    DisableIRQ(DEMO_LPADC_IRQn);

#if ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0))
    LPIT_ClearStatusFlags(DEMO_LPIT_BASE, kLPIT_Channel0TimerFlag);
    DisableIRQ(DEMO_LPIT_IRQn);
    LPIT_StopTimer(DEMO_LPIT_BASE, DEMO_LPIT_CHANNEL_NUM);
#endif /* ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0)) */
#if ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0))
    PIT_ClearStatusFlags(DEMO_PIT_BASE, DEMO_PIT_CHANNEL_NUM, kPIT_TimerFlag);
    DisableIRQ(DEMO_PIT_IRQn);
    PIT_StopTimer(DEMO_PIT_BASE, DEMO_PIT_CHANNEL_NUM);
#endif /* ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0)) */

    g_timeOut = true;
    SDK_ISR_EXIT_BARRIER;
}

void DEMO_LPADC_IRQ_HANDLER(void)
{
    g_conversionCount += 16UL;

#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    /* Clear the FIFO. */
    LPADC_DoResetFIFO0(DEMO_LPADC_BASE);
    while ((LPADC_GetStatusFlags(DEMO_LPADC_BASE) & kLPADC_ResultFIFO0ReadyFlag) == kLPADC_ResultFIFO0ReadyFlag)
#else
    /* Clear the FIFO. */
    LPADC_DoResetFIFO(DEMO_LPADC_BASE);
    while ((LPADC_GetStatusFlags(DEMO_LPADC_BASE) & kLPADC_ResultFIFOReadyFlag) == kLPADC_ResultFIFOReadyFlag)
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U)) */
    {
    }
    LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1UL);
    SDK_ISR_EXIT_BARRIER;
}

int main(void)
{
    lpadc_sample_time_mode_t sampleTimeMode;

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitADCClock();

    PRINTF("ADC High Sample Rate Demo!\r\n");

#if (defined(DEMO_CLOCK_FUNCTION_PARAMETER_COUNT) && (DEMO_CLOCK_FUNCTION_PARAMETER_COUNT == 1U))
    PRINTF("Current ADC clock frequency is %d Hz!\r\n", CLOCK_GetFreqFromObs(CCM_OBS_ADC1_CLK_ROOT));
#else
    PRINTF("Current ADC clock frequency is %d Hz!\r\n", CLOCK_GetFreqFromObs(0, CCM_OBS_ADC1_CLK_ROOT));
#endif /* (defined(DEMO_CLOCK_FUNCTION_PARAMETER_COUNT) && (DEMO_CLOCK_FUNCTION_PARAMETER_COUNT == 1U)) */
    DEMO_ADCInit();
    DEMO_TimerInit();
    while (1)
    {
        sampleTimeMode = DEMO_SelectSampleTime();
        DEMO_ADCStartSample(sampleTimeMode);
#if ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0))
        EnableIRQ(DEMO_LPIT_IRQn);
#endif /* ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0)) */
#if ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0))
        EnableIRQ(DEMO_PIT_IRQn);
#endif /* ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0)) */
        g_timeOut         = false;
        g_conversionCount = 0UL;
        PRINTF("\r\nPlease press any keys to trigger ADC conversion!\r\n");
        (void)GETCHAR();
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1UL);
#if ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0))
        LPIT_StartTimer(DEMO_LPIT_BASE, DEMO_LPIT_CHANNEL_NUM);
#endif /* ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0)) */
#if ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0))
        PIT_StartTimer(DEMO_PIT_BASE, DEMO_PIT_CHANNEL_NUM);
#endif /* ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0)) */
        while (1)
        {
            if (g_timeOut)
            {
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
                if ((LPADC_GetStatusFlags(DEMO_LPADC_BASE) & kLPADC_ResultFIFO0ReadyFlag) ==
                    kLPADC_ResultFIFO0ReadyFlag)
#else
                if ((LPADC_GetStatusFlags(DEMO_LPADC_BASE) & kLPADC_ResultFIFOReadyFlag) == kLPADC_ResultFIFOReadyFlag)
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U)) */
                {
                    g_conversionCount += 16UL;
                }
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
                g_conversionCount += LPADC_GetConvResultCount(DEMO_LPADC_BASE, 0U);
#else
                g_conversionCount += LPADC_GetConvResultCount(DEMO_LPADC_BASE);
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U)) */
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
#if defined(FSL_FEATURE_LPADC_HAS_CFG_PWRSEL) && (FSL_FEATURE_LPADC_HAS_CFG_PWRSEL == 1U)
    adcConfig.powerLevelMode = kLPADC_PowerLevelAlt4; /* Highest power setting. */
#endif /* defined(FSL_FEATURE_LPADC_HAS_CFG_PWRSEL) && (FSL_FEATURE_LPADC_HAS_CFG_PWRSEL == 1U) */
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2))
    adcConfig.FIFO0Watermark = DEMO_ADC_FIFO_DEPTH;
    adcConfig.FIFO1Watermark = 0U;
#else
    adcConfig.FIFOWatermark = DEMO_ADC_FIFO_DEPTH;
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2)) */
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

#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    LPADC_DoResetFIFO0(DEMO_LPADC_BASE);
    LPADC_ClearStatusFlags(DEMO_LPADC_BASE, kLPADC_ResultFIFO0ReadyFlag | kLPADC_ResultFIFO0OverflowFlag);
    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFO0WatermarkInterruptEnable);
#else
    LPADC_DoResetFIFO(DEMO_LPADC_BASE);
    LPADC_ClearStatusFlags(DEMO_LPADC_BASE, kLPADC_ResultFIFOReadyFlag | kLPADC_ResultFIFOOverflowFlag);
    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFOWatermarkInterruptEnable);
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U)) */

    EnableIRQ(DEMO_LPADC_IRQn);
    /* Enable the module after setting configuration. */
    LPADC_Enable(DEMO_LPADC_BASE, true);
}

/*!
 * @brief Configures Timer, as period as 1s.
 */
static void DEMO_TimerInit(void)
{
#if ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0))
    lpit_config_t LPITConfig;

    LPIT_GetDefaultConfig(&LPITConfig);
    LPIT_Init(DEMO_LPIT_BASE, &LPITConfig);

    LPIT_SetTimerPeriod(DEMO_LPIT_BASE, DEMO_LPIT_CHANNEL_NUM, USEC_TO_COUNT(1000000U, DEMO_LPIT_CLOCK_FREQ));
    /* Enable timer interrupts for channel 0 */
    LPIT_EnableInterrupts(DEMO_LPIT_BASE, kLPIT_Channel0TimerInterruptEnable);
#endif /* ((defined FSL_FEATURE_SOC_LPIT_COUNT) && (FSL_FEATURE_SOC_LPIT_COUNT != 0)) */
#if ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0))
    pit_config_t pitConfig;

    PIT_GetDefaultConfig(&pitConfig);
    PIT_Init(DEMO_PIT_BASE, &pitConfig);

    PIT_SetTimerPeriod(DEMO_PIT_BASE, DEMO_PIT_CHANNEL_NUM, USEC_TO_COUNT(1000000U, DEMO_PIT_CLOCK_FREQ));
    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(DEMO_PIT_BASE, DEMO_PIT_CHANNEL_NUM, kPIT_TimerInterruptEnable);
#endif /* ((defined FSL_FEATURE_SOC_PIT_COUNT) && (FSL_FEATURE_SOC_PIT_COUNT != 0)) */
}
