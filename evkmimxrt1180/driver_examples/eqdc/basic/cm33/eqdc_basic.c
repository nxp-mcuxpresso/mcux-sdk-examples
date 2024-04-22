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
#define DEMO_EQDC EQDC1

#define DEMO_ENCODER_DISK_LINE 0xFFFFFFFFU
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
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
    
    PRINTF("\r\nEQDC Basic Example.\r\n");

    EQDC_GetDefaultConfig(&sEqdcConfig);
    sEqdcConfig.positionModulusValue               = DEMO_ENCODER_DISK_LINE;
    
    EQDC_Init(DEMO_EQDC, &sEqdcConfig);
    EQDC_SetOperateMode(DEMO_EQDC, kEQDC_QuadratureDecodeOperationMode);
    EQDC_DoSoftwareLoadInitialPositionValue(DEMO_EQDC);

    PRINTF("Press any key to get the encoder values ...\r\n");

    while (1)
    {
        GETCHAR();
        PRINTF("\r\n");

        /* This read operation would capture all the position counter to responding hold registers. */
        mCurPosValue = EQDC_GetPosition(DEMO_EQDC);
        
        PRINTF("Current position value: %ld\r\n", mCurPosValue);
        PRINTF("Position differential value: %d\r\n", (int16_t)EQDC_GetHoldPositionDifference(DEMO_EQDC));
        PRINTF("Position revolution value: %d\r\n", EQDC_GetHoldRevolution(DEMO_EQDC));
        PRINTF("\r\n");
    }
}
