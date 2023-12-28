/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
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

#include "fsl_common.h"
#include "fsl_reset.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPADC_BASE             ADC1
#define DEMO_LPADC_IRQn             ADC1_IRQn
#define DEMO_LPADC_USER_CHANNEL     5U
#define DEMO_LPADC_USER_CMDID       15U
#define DEMO_LPADC_IRQ_HANDLER_FUNC ADC1_IRQHandler


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_LpadcConversionCompletedFlag = false;
lpadc_conv_result_t g_LpadcResultConfigStruct;
const uint32_t g_Lpadc_12bitFullRange = 4096U;

/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_LPADC_IRQ_HANDLER_FUNC(void)
{
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    if (LPADC_GetConvResult(DEMO_LPADC_BASE, &g_LpadcResultConfigStruct, 0U))
#else
    if (LPADC_GetConvResult(DEMO_LPADC_BASE, &g_LpadcResultConfigStruct))
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
    {
        g_LpadcConversionCompletedFlag = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    lpadc_config_t mLpadcConfigStruct;
    lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
    lpadc_conv_command_config_t mLpadcCommandConfigStruct;

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

    PRINTF("LPADC Single Interrupt Example\r\n");

    LPADC_GetDefaultConfig(&mLpadcConfigStruct);
    mLpadcConfigStruct.enableAnalogPreliminary = true;
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2))
    mLpadcConfigStruct.FIFO0Watermark = 0U;
    mLpadcConfigStruct.FIFO1Watermark = 0U;
#else
    mLpadcConfigStruct.FIFOWatermark = 0U;
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
    LPADC_Init(DEMO_LPADC_BASE, &mLpadcConfigStruct);

    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    mLpadcCommandConfigStruct.channelNumber = DEMO_LPADC_USER_CHANNEL;
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

    PRINTF("ADC Full Range: %d\r\n", g_Lpadc_12bitFullRange);
#if defined(FSL_FEATURE_LPADC_HAS_CMDL_CSCALE) && FSL_FEATURE_LPADC_HAS_CMDL_CSCALE
    if (kLPADC_SampleFullScale == mLpadcCommandConfigStruct.sampleScaleMode)
    {
        PRINTF("Full channel scale (Factor of 1).\r\n");
    }
    else if (kLPADC_SamplePartScale == mLpadcCommandConfigStruct.sampleScaleMode)
    {
        PRINTF("Divided input voltage signal. (Factor of 30/64).\r\n");
    }
#endif

    /* When the number of datawords stored in the ADC Result FIFO is greater
     * than watermark value(0U), LPADC watermark interrupt would be triggered.
     */
    PRINTF("Please press any key to get user channel's ADC value.\r\n");
    GETCHAR();
    LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U);
    while (!g_LpadcConversionCompletedFlag)
    {
    }
    PRINTF("ADC value: %d\r\n", ((g_LpadcResultConfigStruct.convValue) >> 3U));
    while (1)
    {
    }
}
