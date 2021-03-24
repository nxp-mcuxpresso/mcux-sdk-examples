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
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ADC_CHANNEL_GROUP0 0U
#define DEMO_ADC_CHANNEL_GROUP1 1U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile uint16_t u16Result0A[256]    = {0};
volatile uint16_t u16Result0B[256]    = {0};
volatile uint16_t u16Result1A[256]    = {0};
volatile uint16_t u16Result1B[256]    = {0};
volatile uint16_t u16CycleTimes       = 0;
const uint32_t g_Adc16_12bitFullRange = 4096U;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief ISR for ADC16 interrupt function
 */
void DEMO_ADC0_IRQHANDLER(void)
{
    /* Read to clear COCO flag. */
    if (ADC16_GetChannelStatusFlags(DEMO_ADC0_PERIPHERAL, DEMO_ADC_CHANNEL_GROUP0))
    {
        u16Result0A[u16CycleTimes] = ADC16_GetChannelConversionValue(DEMO_ADC0_PERIPHERAL, DEMO_ADC_CHANNEL_GROUP0);
    }

    if (ADC16_GetChannelStatusFlags(DEMO_ADC0_PERIPHERAL, DEMO_ADC_CHANNEL_GROUP1))
    {
        u16Result0B[u16CycleTimes] = ADC16_GetChannelConversionValue(DEMO_ADC0_PERIPHERAL, DEMO_ADC_CHANNEL_GROUP1);
    }
    __DSB();
}

void DEMO_ADC1_IRQHANDLER(void)
{
    /* Read to clear COCO flag. */
    if (ADC16_GetChannelStatusFlags(DEMO_ADC1_PERIPHERAL, DEMO_ADC_CHANNEL_GROUP0))
    {
        u16Result1A[u16CycleTimes] = ADC16_GetChannelConversionValue(DEMO_ADC1_PERIPHERAL, DEMO_ADC_CHANNEL_GROUP0);
    }

    if (ADC16_GetChannelStatusFlags(DEMO_ADC1_PERIPHERAL, DEMO_ADC_CHANNEL_GROUP1))
    {
        u16Result1B[u16CycleTimes] = ADC16_GetChannelConversionValue(DEMO_ADC1_PERIPHERAL, DEMO_ADC_CHANNEL_GROUP1);

        if (u16CycleTimes < 256)
        {
            u16CycleTimes++;
        }
        else
        {
            u16CycleTimes = 0;
        }
    }
    __DSB();
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0U;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitBootPeripherals();

    PRINTF("\r\nRun pdb trig adc with flextimer demo.\r\n");
    PRINTF("ADC Full Range: %d\r\n", g_Adc16_12bitFullRange);
    while (1)
    {
        PRINTF("\r\nInput any character to start demo.\r\n");
        GETCHAR();
        /* Reset counter */
        DEMO_FTM_PERIPHERAL->CNT &= ~FTM_CNT_COUNT_MASK;

        /* Enable FTM0 channel0 trigger as external trigger */
        DEMO_FTM_PERIPHERAL->EXTTRIG |= FTM_EXTTRIG_CH0TRIG_MASK;

        while (u16CycleTimes < 256)
        {
        }
        /* Disable FTM0 channel0 trigger and Clear FTM0 channel external trigger flag, it will not trig PDB */
        if (FTM_GetStatusFlags(DEMO_FTM_PERIPHERAL) & kFTM_ChnlTriggerFlag)
        {
            DEMO_FTM_PERIPHERAL->EXTTRIG &= ~FTM_EXTTRIG_CH0TRIG_MASK;
            FTM_ClearStatusFlags(DEMO_FTM_PERIPHERAL, kFTM_ChnlTriggerFlag);
        }

        for (i = 0; i < 256; i++)
        {
            PRINTF("\r\n%d, %d, %d, %d\r\n", u16Result0A[i], u16Result0B[i], u16Result1A[i], u16Result1B[i]);
        }

        for (i = 0; i < 256; i++)
        {
            u16Result0A[i] = 0;
            u16Result0B[i] = 0;
            u16Result1A[i] = 0;
            u16Result1B[i] = 0;
        }

        u16CycleTimes = 0;
    }
}
