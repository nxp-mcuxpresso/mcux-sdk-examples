/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_xbara.h"
#include "fsl_xbarb.h"

#include "pin_mux.h"
#include "peripherals.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_XBARA_BASEADDR XBARA1
#define DEMO_XBARB_BASEADDR XBARB2

#define DEMO_XBARA_IRQ_HANDLER_FUNC XBAR1_IRQ_0_1_IRQHandler
#define DEMO_XBARA_IRQ_ID           XBAR1_IRQ_0_1_IRQn

#define DEMO_XBARB_INPUT_CMP_SIGNAL    kXBARB2_InputAcmp1Out
#define DEMO_XBARB_OUTPUT_AOI_SIGNAL_1 kXBARB2_OutputAoi1In00

#define DEMO_XBARB_INPUT_PIT_SIGNAL    kXBARB2_InputPitTrigger0
#define DEMO_XBARB_OUTPUT_AOI_SIGNAL_2 kXBARB2_OutputAoi1In01

#define DEMO_XBARA_INPUT_AOI_SIGNAL kXBARA1_InputAoi1Out0
#define DEMO_XBARA_OUTPUT_SIGNAL    kXBARA1_OutputDmaChMuxReq30


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief Initialize the XBAR module.
 *
 */
static void XBAR_Configuration(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_xbaraInterrupt = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

static void XBAR_Configuration(void)
{
    xbara_control_config_t xbaraConfig;

    /* Init XBAR module. */
    XBARA_Init(DEMO_XBARA_BASEADDR);
    XBARB_Init(DEMO_XBARB_BASEADDR);

    /* Configure the XBAR signal connections */
    XBARB_SetSignalsConnection(DEMO_XBARB_BASEADDR, DEMO_XBARB_INPUT_CMP_SIGNAL, DEMO_XBARB_OUTPUT_AOI_SIGNAL_1);
    XBARB_SetSignalsConnection(DEMO_XBARB_BASEADDR, DEMO_XBARB_INPUT_PIT_SIGNAL, DEMO_XBARB_OUTPUT_AOI_SIGNAL_2);
    XBARA_SetSignalsConnection(DEMO_XBARA_BASEADDR, DEMO_XBARA_INPUT_AOI_SIGNAL, DEMO_XBARA_OUTPUT_SIGNAL);

    /* Configure the XBARA interrupt */
    xbaraConfig.activeEdge  = kXBARA_EdgeRising;
    xbaraConfig.requestType = kXBARA_RequestInterruptEnalbe;
    XBARA_SetOutputSignalConfig(DEMO_XBARA_BASEADDR, DEMO_XBARA_OUTPUT_SIGNAL, &xbaraConfig);

    /* Enable at the NVIC. */
    EnableIRQ(DEMO_XBARA_IRQ_ID);
}

void DEMO_XBARA_IRQ_HANDLER_FUNC(void)
{
    /* Clear interrupt flag */
    XBARA_ClearStatusFlags(DEMO_XBARA_BASEADDR, kXBARA_EdgeDetectionOut0);
    g_xbaraInterrupt = true;
    SDK_ISR_EXIT_BARRIER;
}

int main(void)
{
    /* Init board hardware */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitBootPeripherals();
    /* Init XBAR */
    XBAR_Configuration();

    PRINTF("XBAR and AOI Peripheral Demo: Start...\r\n");

    while (1)
    {
        if (g_xbaraInterrupt)
        {
            g_xbaraInterrupt = false;
            PRINTF("XBAR interrupt occurred\r\n\r\n");
        }
    }
}
