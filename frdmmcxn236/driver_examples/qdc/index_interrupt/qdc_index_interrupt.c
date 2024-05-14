/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_qdc.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_QDC_BASEADDR     QDC0
#define DEMO_QDC_INDEX_IRQ_ID QDC0_IDX_IRQn
#define QDC_INDEX_IRQHandler  QDC0_IDX_IRQHandler


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_EncIndexCounter = 0U;

/*******************************************************************************
 * Code
 ******************************************************************************/

void NVIC_Configuration(void)
{
    EnableIRQ(DEMO_QDC_INDEX_IRQ_ID); /* Enable the interrupt for QDC_INDEX. */
}

/*!
 * @brief ISR for INDEX event
 */
void QDC_INDEX_IRQHandler(void)
{
    g_EncIndexCounter++;
    QDC_ClearStatusFlags(DEMO_QDC_BASEADDR, kQDC_INDEXPulseFlag);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    qdc_config_t mEncConfigStruct;
    uint32_t mCurPosValue;

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\nQDC INDEX Interrupt Example.\r\n");

    /* Initialize the QDC module. */
    QDC_GetDefaultConfig(&mEncConfigStruct);
    /* Configure the INDEX action. */
    mEncConfigStruct.INDEXTriggerMode = kQDC_INDEXTriggerOnRisingEdge;
#if (defined(FSL_FEATURE_QDC_HAS_CTRL3) && FSL_FEATURE_QDC_HAS_CTRL3)
    /*
     * If there is CTRL3, the period measurement is enabled by default,
     * with this setting, the POSD is loaded to POSDH only when POSD
     * is read (calling QDC_GetPositionDifferenceValue).
     * In this project, the POSD is desired to be loaded to POSDH when
     * UPOS is read (calling QDC_GetPositionValue), so disable the
     * period measurement here.
     */
    mEncConfigStruct.enablePeriodMeasurementFunction = false;
#endif
    /* Setup the hardware. */
    QDC_Init(DEMO_QDC_BASEADDR, &mEncConfigStruct);
    /* Update position counter from initial position register. */
    QDC_DoSoftwareLoadInitialPositionValue(DEMO_QDC_BASEADDR);

    PRINTF("Press any key to get the encoder values ...\r\n");

    /* Interrupts. */
    NVIC_Configuration();
    QDC_ClearStatusFlags(DEMO_QDC_BASEADDR, kQDC_INDEXPulseFlag);
    QDC_EnableInterrupts(DEMO_QDC_BASEADDR, kQDC_INDEXPulseInterruptEnable);

    while (1)
    {
        GETCHAR();
        PRINTF("\r\n");

        /* This read operation would capture all the positon counter to responding hold registers. */
        mCurPosValue = QDC_GetPositionValue(DEMO_QDC_BASEADDR);

        PRINTF("Current position value: %ld\r\n", mCurPosValue);
        PRINTF("Current position differential value: %d\r\n",
               (int16_t)QDC_GetHoldPositionDifferenceValue(DEMO_QDC_BASEADDR));
        PRINTF("Current position revolution value: %d\r\n", QDC_GetHoldRevolutionValue(DEMO_QDC_BASEADDR));
        PRINTF("g_EncIndexCounter: %d\r\n", g_EncIndexCounter);
    }
}
