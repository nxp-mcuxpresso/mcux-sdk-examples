/*
 * Copyright 2021 NXP
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

#include "fsl_power.h"
#include "fsl_iap.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPADC_BASE             ADC0
#define DEMO_LPADC_IRQn             ADC0_IRQn
#define DEMO_LPADC_IRQ_HANDLER_FUNC ADC0_IRQHandler
#define DEMO_LPADC_USER_CHANNEL     7U
#define DEMO_LPADC_USER_CMDID       1U /* The available command number are 1-15 */
#define TSENS_CAL_OTP_FUSE_INDEX \
    93U /* TSENS_CAL is an 8-bit signed calibration constant retrieved from non-volatile memory.*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
float DEMO_GetTempsenorValue(void);
status_t DEMO_TempsenorInit(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t tsensorCal = 0;
volatile bool g_LpadcConversionCompletedFlag = false;
volatile uint16_t g_LpadcConvValue           = 0U;
lpadc_conv_result_t g_LpadcResultConfigStruct;
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
status_t DEMO_TempsenorInit(void)
{
    status_t status = kStatus_Success;
    /* Config to use PMC temperature sensor. */
    SYSCTL0->TEMPSENSORCTL |= SYSCTL0_TEMPSENSORCTL_TSSRC_MASK;

    POWER_DisablePD(kPDRUNCFG_PD_PMC_TEMPSNS);
    POWER_ApplyPD();

    /* Another 40us for the temperature sensor to sattle. */
    SDK_DelayAtLeastUs(40U, SystemCoreClock);

    status = IAP_OtpInit(SystemCoreClock);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Read TSENS_CAL calibration constant value from OTP Fuse. */
    status = IAP_OtpFuseRead(TSENS_CAL_OTP_FUSE_INDEX, &tsensorCal);
    return status;
}

float DEMO_GetTempsenorValue(void)
{
    /* TSENSOR[TSENSM] value set. */
    uint8_t tsensmSel[15]     = {0U, 1U, 3U, 2U, 6U, 7U, 5U, 4U, 5U, 7U, 6U, 2U, 3U, 1U, 0U};
    uint16_t tsensorValue[15] = {0U};
    float t, cm_vref, cm_ctat, cm_temp;
    int8_t calibration = 0;
    uint8_t i;

    for (i = 0; i < 15U; i++)
    {
        PMC->TSENSOR = PMC_TSENSOR_TSENSM(tsensmSel[i]);

        /* Do ADC convert */
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U); /* 1U is trigger0 mask. */
        while (!g_LpadcConversionCompletedFlag)
        {
        }
        tsensorValue[i]                = g_LpadcConvValue;
        g_LpadcConversionCompletedFlag = false;
    }

    /* Calculate temperature. */
    /* CM_CTAT = (2*C1_001 - C1_011 + 2*C2_001 - C2_011 + 2*C1_101 - C1_111 + 2*C2_101 - C2_111)/4 */
    cm_ctat = (float)(2 * tsensorValue[1] - tsensorValue[2] + 2 * tsensorValue[13] - tsensorValue[12] +
                      2 * tsensorValue[6] - tsensorValue[5] + 2 * tsensorValue[8] - tsensorValue[9]) /
              4.0;
    /* CM_TEMP = (2*C1_000 - C1_010 + 2*C2_000 - C2_010 + 4*CM_100 - C1_110 - C2_110)/4 */
    cm_temp = (float)(2 * tsensorValue[0] - tsensorValue[3] + 2 * tsensorValue[14] - tsensorValue[11] +
                      4 * tsensorValue[7] - tsensorValue[4] - tsensorValue[10]) /
              4.0;

    calibration = (int8_t)(tsensorCal & 0xFF);
    cm_vref     = cm_ctat + (953.36 + calibration) * cm_temp / 2048;

    t = 370.98 * (cm_temp / cm_vref) - 273.15;
    return t;
}

void DEMO_LPADC_IRQ_HANDLER_FUNC(void)
{
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    if (LPADC_GetConvResult(DEMO_LPADC_BASE, &g_LpadcResultConfigStruct, 0U))
#else
    if (LPADC_GetConvResult(DEMO_LPADC_BASE, &g_LpadcResultConfigStruct))
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
    {
        g_LpadcConvValue               = (g_LpadcResultConfigStruct.convValue) >> g_LpadcResultShift;
        g_LpadcConversionCompletedFlag = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    status_t ret;
    lpadc_config_t mLpadcConfigStruct;
    lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
    lpadc_conv_command_config_t mLpadcCommandConfigStruct;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    POWER_DisablePD(kPDRUNCFG_PD_ADC);
    POWER_DisablePD(kPDRUNCFG_LP_ADC);

    RESET_PeripheralReset(kADC0_RST_SHIFT_RSTn);
    CLOCK_AttachClk(kFRO_DIV4_to_ADC_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivAdcClk, 1);

    PRINTF("PMC Temperature Sensor Demo\r\n");

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
#if defined(FSL_FEATURE_LPADC_HAS_CMDL_CSCALE) && FSL_FEATURE_LPADC_HAS_CMDL_CSCALE
    mLpadcCommandConfigStruct.sampleScaleMode = kLPADC_SampleFullScale;
#endif
    mLpadcCommandConfigStruct.sampleChannelMode =
        kLPADC_SampleChannelDiffBothSideAB; /* ADC Temperature: differential mode. */
    mLpadcCommandConfigStruct.hardwareAverageMode =
        kLPADC_HardwareAverageCount128;                                  /* ADC Temperature: Maximum averaging. */
    mLpadcCommandConfigStruct.sampleTimeMode = kLPADC_SampleTimeADCK131; /* ADC Temperature: Maximum sample time. */
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &mLpadcCommandConfigStruct);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
    mLpadcTriggerConfigStruct.targetCommandId       = DEMO_LPADC_USER_CMDID;
    mLpadcTriggerConfigStruct.enableHardwareTrigger = false;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */

/* Enable the watermark interrupt. */
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFO0WatermarkInterruptEnable);
#else
    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFOWatermarkInterruptEnable);
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
    EnableIRQ(DEMO_LPADC_IRQn);

    PRINTF("ADC Full Range: %d\r\n", g_LpadcFullRange);

    /* Init temperature sensor */
    ret = DEMO_TempsenorInit();
    if (kStatus_Success != ret)
    {
        PRINTF("Temperature sensor initilizes failed.\r\n");
    }

    /* When the number of datawords stored in the ADC Result FIFO is greater
     * than watermark value(0U), LPADC watermark interrupt would be triggered.
     */
    PRINTF("Please press any key to get temperature from the internal temperature sensor.\r\n");

    while (1)
    {
        GETCHAR();

        PRINTF("Current temperature: %0.2f\r\n", DEMO_GetTempsenorValue());
    }
}
