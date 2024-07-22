/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_lpadc.h"

#include "fsl_clock.h"
#include "fsl_reset.h"
#include <stdbool.h>
#include "fsl_inputmux.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPADC_BASE                  ADC0
#define DEMO_LPADC_IRQn                  ADC0_IRQn
#define DEMO_LPADC_IRQ_HANDLER_FUNC      ADC0_IRQHandler
#define DEMO_LPADC_VREF_SOURCE           kLPADC_ReferenceVoltageAlt3
#define DEMO_LPADC_DO_OFFSET_CALIBRATION true

#define DEMO_SW_NAME BOARD_SW2_NAME

#define DEMO_HIGH_PRIORITY_TRIGGER_ID (1U)
#define DEMO_HIGH_PRIORITY_CMAD_ID    (2U)
#define DEMO_LPADC_VDD_CHANNEL        29U

#define DEMO_LOW_PRIORITY_TRIGGER_ID (0U)
#define DEMO_LOW_PRIORITY_CMAD_ID    (1U)
#define DEMO_LPADC_EXTERNAL_CHANNEL  0U

#define DEMO_LOOP_COUNT (4U)

#define SOFT_TRIGGER_MASK (1UL << DEMO_LOW_PRIORITY_TRIGGER_ID)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitTimer(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
bool g_trigger1CompletedFlag = false;
bool g_trigger2CompletedFlag = false;

uint16_t g_lowData[4];
uint16_t g_highData;
uint8_t g_cmdLoopCount = 0U;
/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_SW2_IRQ_HANDLER(void)
{
    GPIO_GpioClearInterruptFlags(BOARD_SW2_GPIO, 1U << BOARD_SW2_GPIO_PIN);
}


void DEMO_LPADC_IRQ_HANDLER_FUNC(void)
{
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2))
    if (LPADC_GetStatusFlags(DEMO_LPADC_BASE) & kLPADC_ResultFIFO0OverflowFlag)
#else
    if (LPADC_GetStatusFlags(DEMO_LPADC_BASE) & kLPADC_ResultFIFOOverflowFlag)
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2)) */
    {
        PRINTF("FIFO OverFlow!!! Will Disable ADC!\r\n");
        LPADC_Deinit(DEMO_LPADC_BASE);
    }

#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2))
    if (LPADC_GetStatusFlags(DEMO_LPADC_BASE) & kLPADC_ResultFIFO0ReadyFlag)
#else
    if (LPADC_GetStatusFlags(DEMO_LPADC_BASE) & kLPADC_ResultFIFOReadyFlag)
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2)) */
    {
        lpadc_conv_result_t tmpResult;
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2))
        LPADC_GetConvResult(DEMO_LPADC_BASE, &tmpResult, 0U);
#else
        (void)LPADC_GetConvResult(DEMO_LPADC_BASE, &tmpResult);
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2)) */
        if ((tmpResult.triggerIdSource == DEMO_LOW_PRIORITY_TRIGGER_ID) &&
            (tmpResult.commandIdSource == DEMO_LOW_PRIORITY_CMAD_ID))
        {
            g_lowData[g_cmdLoopCount] = tmpResult.convValue;
            g_cmdLoopCount++;
            if (g_cmdLoopCount == DEMO_LOOP_COUNT)
            {
                g_trigger1CompletedFlag = true;
                g_cmdLoopCount          = 0U;
            }
        }
        if ((tmpResult.triggerIdSource == DEMO_HIGH_PRIORITY_TRIGGER_ID) &&
            (tmpResult.commandIdSource == DEMO_HIGH_PRIORITY_CMAD_ID))
        {
            g_highData              = tmpResult.convValue;
            g_trigger2CompletedFlag = true;
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    lpadc_config_t adcConfig;
    lpadc_conv_trigger_config_t lowPriorityTriggerConfig;
    lpadc_conv_trigger_config_t highPriorityTriggerConfig;

    lpadc_conv_command_config_t lowPriorityCommandConfig;
    lpadc_conv_command_config_t highPriorityCommandConfig;

    RESET_PeripheralReset(kADC0_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kINPUTMUX0_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_SetClockDiv(kCLOCK_DivADC0, 1U);
    CLOCK_AttachClk(kFRO12M_to_ADC0);

    /* Map Input pin to ADC trigger1 input. */
    INPUTMUX_Init(INPUTMUX0);
    INPUTMUX_AttachSignal(INPUTMUX0, 1UL, kINPUTMUX_Gpio3PinEventTrig0ToAdc0Trigger);

    CLOCK_EnableClock(kCLOCK_GateGPIO3);

    gpio_pin_config_t sw_config = {
        kGPIO_DigitalInput,
        0,
    };
    GPIO_SetPinInterruptConfig(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN, kGPIO_ActiveLowTriggerOutputEnable);
    EnableIRQ(BOARD_SW2_IRQ);
    GPIO_PinInit(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN, &sw_config);

    PRINTF("\r\nLPADC Trigger Exception Example!\r\n");

    LPADC_GetDefaultConfig(&adcConfig);
    adcConfig.enableInDozeMode = false;
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS
    adcConfig.conversionAverageMode = kLPADC_ConversionAverage128;
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS */
    adcConfig.enableAnalogPreliminary = true;
    adcConfig.powerUpDelay            = 0x80U;
    adcConfig.referenceVoltageSource  = DEMO_LPADC_VREF_SOURCE;
    adcConfig.triggerPriorityPolicy   = kLPADC_ConvPreemptImmediatelyNotAutoResumed;
    adcConfig.enableConvPause         = false;
    adcConfig.convPauseDelay          = 0U;
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2))
    adcConfig.FIFO0Watermark = 0U; /*When FIFO is not empty, interrupt will be triggered. */
    adcConfig.FIFO1Watermark = 0U;
#else
    adcConfig.FIFOWatermark = 0U; /*When FIFO is not empty, interrupt will be triggered. */
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2)) */
    /* The high priority trigger will be scheduled until current conversion finish. */
    LPADC_Init(DEMO_LPADC_BASE, &adcConfig);

    /* Enable watermark interrupt and overflow interrupt. */
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    LPADC_EnableInterrupts(DEMO_LPADC_BASE,
                           kLPADC_FIFO0WatermarkInterruptEnable | kLPADC_ResultFIFO0OverflowInterruptEnable);
#else
    LPADC_EnableInterrupts(DEMO_LPADC_BASE,
                           kLPADC_FIFOWatermarkInterruptEnable | kLPADC_ResultFIFOOverflowInterruptEnable);
#endif /* (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U)) */
    EnableIRQ(DEMO_LPADC_IRQn);

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

    LPADC_GetDefaultConvCommandConfig(&lowPriorityCommandConfig);
    lowPriorityCommandConfig.channelNumber       = DEMO_LPADC_EXTERNAL_CHANNEL;
    lowPriorityCommandConfig.hardwareAverageMode = kLPADC_HardwareAverageCount128;
    lowPriorityCommandConfig.loopCount           = (DEMO_LOOP_COUNT - 1U); /* Command will execute 4 times. */
    lowPriorityCommandConfig.sampleTimeMode      = kLPADC_SampleTimeADCK131;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LOW_PRIORITY_CMAD_ID, &lowPriorityCommandConfig);

    LPADC_GetDefaultConvCommandConfig(&highPriorityCommandConfig);
    highPriorityCommandConfig.channelNumber = DEMO_LPADC_VDD_CHANNEL;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_HIGH_PRIORITY_CMAD_ID, &highPriorityCommandConfig);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&lowPriorityTriggerConfig);
    lowPriorityTriggerConfig.targetCommandId       = DEMO_LOW_PRIORITY_CMAD_ID; /* Trigger 0 execute command 1. */
    lowPriorityTriggerConfig.delayPower            = 0x8U;
    lowPriorityTriggerConfig.enableHardwareTrigger = false;
    lowPriorityTriggerConfig.priority              = 1U; /* Set to priority level2. */
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, DEMO_LOW_PRIORITY_TRIGGER_ID, &lowPriorityTriggerConfig);

    LPADC_GetDefaultConvTriggerConfig(&highPriorityTriggerConfig);
    highPriorityTriggerConfig.targetCommandId       = DEMO_HIGH_PRIORITY_CMAD_ID; /* Trigger 1 execute command 2. */
    highPriorityTriggerConfig.delayPower            = 0U;
    highPriorityTriggerConfig.enableHardwareTrigger = true;
    highPriorityTriggerConfig.priority              = 0U; /* Set to highest priority(level1). */
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, DEMO_HIGH_PRIORITY_TRIGGER_ID, &highPriorityTriggerConfig);

    PRINTF("Please Press any key to start example\r\n");
    PRINTF("Press %s to start high priority conversion.\r\n", DEMO_SW_NAME);
    GETCHAR();

    g_cmdLoopCount = 0U;
    /* Start low priority trigger. */
    LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, SOFT_TRIGGER_MASK); /* 1U is the trigger0 mask. */
    EnableIRQ(DEMO_LPADC_IRQn);
    while (1)
    {
        if (g_trigger1CompletedFlag)
        {
            uint32_t result = 0UL;
            for (uint8_t i = 0U; i < DEMO_LOOP_COUNT; i++)
            {
                result += g_lowData[i];
            }
            result /= 4UL;
            PRINTF("External Channel: %d\r\n", result);
            g_trigger1CompletedFlag = false;
            LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, SOFT_TRIGGER_MASK);
        }

        if (g_trigger2CompletedFlag)
        {
            PRINTF("Internal Channel VDD/4: %d\r\n", g_highData);
            g_trigger2CompletedFlag = false;
            LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, SOFT_TRIGGER_MASK);
        }
    }
}
