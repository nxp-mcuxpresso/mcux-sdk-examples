/*
 * Copyright 2018, 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_cmp.h"

#include <stdbool.h>
#include "fsl_power.h"
#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_CMP_P_CHANNEL   0
#define DEMO_CMP_N_CHANNEL   4
#define DEMO_CMP_VREF_SOURCE KCMP_VREFSourceVDDA

#define DEMO_GPIO_BASE GPIO
#define DEMO_GPIO_PORT 1U
#define DEMO_GPIO_PIN  28U
#define DEMO_LED_ON    1U
#define DEMO_LED_OFF   0U


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    cmp_config_t mCmpConfigStruct;
    cmp_vref_config_t mCmpVrefConfigStruct;

    /* Initialize hardware. */
    /* attach clock for USART(debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* reset FLEXCOMM for USART */
    RESET_PeripheralReset(kFC0_RST_SHIFT_RSTn);

    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();
    BOARD_InitPins();

    POWER_DisablePD(kPDRUNCFG_PD_COMP);

    PRINTF("CMP polling driver example\r\n");

    /*
     * config->enableHysteresis    = true;
     * config->enableLowPower      = true;
     * config->filterClockDivider  = kCMP_FilterClockDivide1;
     * config->filterSampleMode    = kCMP_FilterSampleMode0;
     */
    CMP_GetDefaultConfig(&mCmpConfigStruct);
    CMP_Init(&mCmpConfigStruct);

    /* Set VREF source. */
    mCmpVrefConfigStruct.vrefSource = KCMP_VREFSourceVDDA;
    mCmpVrefConfigStruct.vrefValue  = 15U; /* Select half of the reference voltage. */
    CMP_SetVREF(&mCmpVrefConfigStruct);

    /* Set P-side and N-side input channels. */
    CMP_SetInputChannels(DEMO_CMP_P_CHANNEL, DEMO_CMP_N_CHANNEL);

    while (1)
    {
        if (true == CMP_GetOutput())
        {
            /* Turn on LED when P-side voltage is greater than that of N-side. */
            GPIO_PinWrite(DEMO_GPIO_BASE, DEMO_GPIO_PORT, DEMO_GPIO_PIN, DEMO_LED_ON);
        }
        else
        {
            /* Turn off LED when P-side voltage is lower than that of N-side. */
            GPIO_PinWrite(DEMO_GPIO_BASE, DEMO_GPIO_PORT, DEMO_GPIO_PIN, DEMO_LED_OFF);
        }
    }
}
