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
#define DEMO_ADC16_CHANNEL_GROUP 0U

#define DAC_1_0_VOLTS 1241U
#define DAC_1_5_VOLTS 1862U
#define DAC_2_0_VOLTS 2482U
#define DAC_2_5_VOLTS 3103U
#define DAC_3_0_VOLTS 3724U

#define VREF_BRD 3.300
#define SE_12BIT 4096.0

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_Adc16ConversionDoneFlag  = false;
volatile uint32_t g_Adc16ConversionValue = 0;
const uint32_t g_Adc16_12bitFullRange    = 4096U;

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_ADC16_IRQ_HANDLER_FUNC(void)
{
    g_Adc16ConversionDoneFlag = true;
    /* Read conversion result to clear the conversion completed flag. */
    g_Adc16ConversionValue = ADC16_GetChannelConversionValue(BOARD_ADC16_PERIPHERAL, DEMO_ADC16_CHANNEL_GROUP);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t msg = ' ';
    float voltRead;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitPeripherals();
#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
    if (kStatus_Success == ADC16_DoAutoCalibration(BOARD_ADC16_PERIPHERAL))
    {
        PRINTF("\r\nADC16_DoAutoCalibration() Done.");
    }
    else
    {
        PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
    }
#endif /* FSL_FEATURE_ADC16_HAS_CALIBRATION */
    PRINTF("\r\nDAC ADC Peripheral Demo!\r\n");
    PRINTF("\r\nADC Full Range: %d.\r\n", g_Adc16_12bitFullRange);

    PRINTF("\r\nPress any key to start demo.\r\n");
    GETCHAR();
    PRINTF("\r\nDemo begin...\r\n");

    while (1)
    {
        PRINTF(
            "\r\n\r\nSelect DAC output level:\r\n\t1. 1.0 V\r\n\t2. 1.5 V\r\n\t3. 2.0 V\r\n\t4. 2.5 V\r\n\t5. 3.0 "
            "V\r\n-->");
        msg = ' ';
        msg = GETCHAR();
        PUTCHAR(msg);
        switch (msg)
        {
            case '1':
                DAC_SetBufferValue(BOARD_DAC_PERIPHERAL, 0U, DAC_1_0_VOLTS);
                break;
            case '2':
                DAC_SetBufferValue(BOARD_DAC_PERIPHERAL, 0U, DAC_1_5_VOLTS);
                break;
            case '3':
                DAC_SetBufferValue(BOARD_DAC_PERIPHERAL, 0U, DAC_2_0_VOLTS);
                break;
            case '4':
                DAC_SetBufferValue(BOARD_DAC_PERIPHERAL, 0U, DAC_2_5_VOLTS);
                break;
            case '5':
                DAC_SetBufferValue(BOARD_DAC_PERIPHERAL, 0U, DAC_3_0_VOLTS);
                break;
            default:
                PRINTF("\r\nPlease input a valid number: 1-5 \r\n");
                break;
        }

        g_Adc16ConversionDoneFlag = false;
        ADC16_SetChannelConfig(BOARD_ADC16_PERIPHERAL, DEMO_ADC16_CHANNEL_GROUP, &BOARD_ADC16_channelsConfig[0]);

        while (!g_Adc16ConversionDoneFlag)
        {
        }
        PRINTF("\r\n\r\nADC Value: %d\r\n", g_Adc16ConversionValue);

        /* Convert ADC value to a voltage based on 3.3V VREFH on board */
        voltRead = (float)(g_Adc16ConversionValue * (VREF_BRD / SE_12BIT));
        PRINTF("\r\nADC Voltage: %0.3f\r\n", voltRead);

        /* Determine what to do next based on user's request */
        PRINTF("\r\nWhat next?:\r\n\t1. Test another DAC output value.\r\n\t2. Terminate demo.\r\n-->");
        msg = ' ';
        while ((msg < '1') || (msg > '2'))
        {
            msg = GETCHAR();
            PUTCHAR(msg);
            PUTCHAR('\b');
        }

        /* Set next state */
        if (msg == '2')
        {
            DAC_Deinit(BOARD_DAC_PERIPHERAL);
            ADC16_Deinit(BOARD_ADC16_PERIPHERAL);
            break;
        }
    }
    PRINTF("\r\nDemo terminated! Reset device to begin again.\r\n");
    while (1)
    {
    }
}
