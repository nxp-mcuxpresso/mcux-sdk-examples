/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_eqdc.h"
#include "fsl_debug_console.h"

#include "fsl_xbar.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_EQDC              EQDC1
#define DEMO_EQDC_INDEX_IRQ_ID EQDC1_IRQn
#define ENC_INDEX_IRQHandler   EQDC1_IRQHandler
#define DEMO_ENCODER_DISK_LINE 0xFFFFFFFFU

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
volatile uint32_t g_EqdcIndexCounter = 0U;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void NVIC_Configuration(void)
{
    EnableIRQ(DEMO_EQDC_INDEX_IRQ_ID); /* Enable the interrupt for EQDC_INDEX. */
}

void ENC_INDEX_IRQHandler(void)
{
    g_EqdcIndexCounter++;
    EQDC_ClearStatusFlags(DEMO_EQDC, kEQDC_IndexPresetPulseFlag);
    SDK_ISR_EXIT_BARRIER;
}

int main(void)
{
    eqdc_config_t sEqdcConfig;
    uint32_t mCurPosValue;
    
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    XBAR_Init(kXBAR_DSC1);
    XBAR_SetSignalsConnection(kXBAR1_InputIomuxXbarInout22, kXBAR1_OutputEqdc1Phasea);
    XBAR_SetSignalsConnection(kXBAR1_InputIomuxXbarInout20, kXBAR1_OutputEqdc1Phaseb);
    XBAR_SetSignalsConnection(kXBAR1_InputIomuxXbarInout18, kXBAR1_OutputEqdc1Index);

    PRINTF("\r\nEQDC INDEX Interrupt Example.\r\n");

    EQDC_GetDefaultConfig(&sEqdcConfig);
    sEqdcConfig.positionModulusValue               = DEMO_ENCODER_DISK_LINE;
    sEqdcConfig.indexPresetInitPosCounterMode      = kEQDC_IndexInitPosCounterOnFallingEdge;
    
    EQDC_Init(DEMO_EQDC, &sEqdcConfig);
    EQDC_SetOperateMode(DEMO_EQDC, kEQDC_QuadratureDecodeOperationMode);
    EQDC_DoSoftwareLoadInitialPositionValue(DEMO_EQDC);

    PRINTF("Press any key to get the encoder values ...\r\n");
    
    NVIC_Configuration();
    EQDC_ClearStatusFlags(DEMO_EQDC, kEQDC_IndexPresetPulseFlag);
    EQDC_EnableInterrupts(DEMO_EQDC, kEQDC_IndexPresetPulseInterruptEnable);

    while (1)
    {
        GETCHAR();
        PRINTF("\r\n");

        /* This read operation would capture all the position counter to responding hold registers. */
        mCurPosValue = EQDC_GetPosition(DEMO_EQDC);
        
        PRINTF("Current position value: %ld\r\n", mCurPosValue);
        PRINTF("Current position differential value: %d\r\n", (int16_t)EQDC_GetHoldPositionDifference(DEMO_EQDC));
        PRINTF("Current position revolution value: %d\r\n", EQDC_GetHoldRevolution(DEMO_EQDC));
        PRINTF("g_EqdcIndexCounter: %d\r\n", g_EqdcIndexCounter);
    }
}
