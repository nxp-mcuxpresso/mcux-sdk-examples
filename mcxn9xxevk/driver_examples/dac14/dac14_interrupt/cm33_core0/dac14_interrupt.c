/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_dac14.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DAC14_BASEADDR         DAC2
#define DEMO_DAC14_IRQ_ID           DAC2_IRQn
#define DEMO_DAC14_IRQ_HANDLER_FUNC DAC2_IRQHandler
#define DEMO_DAC14_VALUE_ARRAY_SIZE 32U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_Dac14InputIndex    = 0U;
volatile uint32_t g_Dac14OutputIndex   = 0U;
volatile uint32_t g_Dac14InterruptDone = false;
/* User-defined array  for DAC output. */
const uint32_t g_Dac14Values[DEMO_DAC14_VALUE_ARRAY_SIZE] = {
    500,   1000,  2000,  3000,  4000,  5000,  6000,  7000, 8000, 9000, 10000, 11000, 12000, 13000, 14000, 15000,
    16000, 15000, 14000, 13000, 12000, 11000, 10000, 9000, 8000, 7000, 6000,  5000,  4000,  3000,  2000,  1000};

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    dac14_config_t dac14ConfigStruct;

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO HF to DAC2 */
    CLOCK_SetClkDiv(kCLOCK_DivDac2Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_DAC2);

    /* enable analog module */
    SPC0->ACTIVE_CFG1 |= 0x41;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\nDAC14 Interrupt Example.\r\n");

    /* Configure the DAC. */
    DAC14_GetDefaultConfig(&dac14ConfigStruct);
    dac14ConfigStruct.TriggerSource     = kDAC14_SoftwareTriggerSource; /* Software trigger. */
    dac14ConfigStruct.WorkMode          = kDAC14_FIFOWorkMode;          /* Normal FIFO mode. */
    dac14ConfigStruct.enableOpampBuffer = true;
    dac14ConfigStruct.enableDAC         = true;
    DAC14_Init(DEMO_DAC14_BASEADDR, &dac14ConfigStruct);

    /* Enable DAC interrupts. */
    DAC14_EnableInterrupts(DEMO_DAC14_BASEADDR, kDAC14_FIFOEmptyInterruptEnable);
    EnableIRQ(DEMO_DAC14_IRQ_ID); /* Enable interrupt in NVIC. */

    PRINTF("Press any key to trigger the DAC...\r\n");

    while (1)
    {
        /* Wait  */
        while (!g_Dac14InterruptDone)
        {
        }
        g_Dac14InterruptDone = false;

        /* Trigger the buffer and move the pointer. */
        GETCHAR();
        DAC14_DoSoftwareTrigger(DEMO_DAC14_BASEADDR);
        PRINTF("DAC14 next output: %d\r\n", g_Dac14Values[g_Dac14OutputIndex]);
        if (g_Dac14OutputIndex >= DEMO_DAC14_VALUE_ARRAY_SIZE - 1U)
        {
            g_Dac14OutputIndex = 0U;
        }
        else
        {
            g_Dac14OutputIndex++;
        }
    }
}

/*!
 * @brief IRQ function for DAC buffer interrupt
 */
void DEMO_DAC14_IRQ_HANDLER_FUNC(void)
{
    uint32_t flags = DAC14_GetStatusFlags(DEMO_DAC14_BASEADDR);

    if (0U != (kDAC14_FIFOEmptyFlag & flags))
    {
        DAC14_SetData(DEMO_DAC14_BASEADDR, g_Dac14Values[g_Dac14InputIndex]);
        if (g_Dac14InputIndex >= (DEMO_DAC14_VALUE_ARRAY_SIZE - 1U))
        {
            g_Dac14InputIndex = 0U;
        }
        else
        {
            g_Dac14InputIndex++;
        }
    }
    g_Dac14InterruptDone = true;
    SDK_ISR_EXIT_BARRIER;
}
