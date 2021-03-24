/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "peripherals.h"
#include "board.h"
#include "fsl_trgmux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* For ADC. */
#define DEMO_ADC_CHANNEL_GROUP 0U

/* For FTM. */
#define DEMO_FTM_COUNTER_CLOCK_SOURCE kFTM_SystemClock

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_adc12ConvValue;
volatile uint32_t g_adc12InterruptCounter;
volatile bool g_adc12InterruptFlag;
const uint32_t g_Adc12_8bitFullRange = 256U;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief ISR for ADC12 interrupt function
 */
void DEMO_ADC_IRQHANDLER(void)
{
    if (0U != (kADC12_ChannelConversionCompletedFlag &
               ADC12_GetChannelStatusFlags(DEMO_ADC_PERIPHERAL, DEMO_ADC_CHANNEL_GROUP)))
    {
        /* Read to clear COCO flag. */
        g_adc12ConvValue = ADC12_GetChannelConversionValue(DEMO_ADC_PERIPHERAL, DEMO_ADC_CHANNEL_GROUP);
        g_adc12InterruptCounter++;
    }
    g_adc12InterruptFlag = true;
    FTM_StopTimer(DEMO_FTM_PERIPHERAL); /* Stop the FTM counter. */
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    g_adc12InterruptCounter = 0U;
    g_adc12InterruptFlag    = false;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitBootPeripherals();

    CLOCK_SetIpSrc(kCLOCK_Adc0, kCLOCK_IpSrcFircAsync);
    CLOCK_SetIpSrc(kCLOCK_Ftm0, kCLOCK_IpSrcFircAsync);
    TRGMUX_SetTriggerSource(TRGMUX0, kTRGMUX_Pdb0, kTRGMUX_TriggerInput0, kTRGMUX_SourceFtm0);

    /* Calibrate ADC12. */
    if (kStatus_Success != ADC12_DoAutoCalibration(DEMO_ADC_PERIPHERAL))
    {
        PRINTF("ADC calibration failed!\r\n");
    }
    ADC12_SetChannelConfig(DEMO_ADC_PERIPHERAL, DEMO_ADC_CHANNEL_GROUP, DEMO_ADC_channelsConfig);

    PRINTF("\r\nftm_pdb_adc12_peripheral demo.\r\n");
    PRINTF("ADC Full Range: %d\r\n", g_Adc12_8bitFullRange);
    PRINTF("\r\nInput any key to trigger the ADC conversion.\r\n");
    while (1)
    {
        GETCHAR();
        /*
         * Start the FTM counter and finally trigger the ADC12's conversion.
         * FTM_StartTimer() -> PDB PreTrigger -> ADC conversion done interrupt -> FTM_StopTimer().
         */
        FTM_StartTimer(DEMO_FTM_PERIPHERAL, DEMO_FTM_COUNTER_CLOCK_SOURCE);
        while (false == g_adc12InterruptFlag)
        {
        }
        g_adc12InterruptFlag = false;

        PRINTF("g_adc12InterruptCounter: %d\r\n", g_adc12InterruptCounter);
    }
}
