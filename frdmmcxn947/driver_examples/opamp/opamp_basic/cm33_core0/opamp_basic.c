/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_opamp.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_OPAMP_BASEADDR OPAMP0
void OPAMP_Configuration(void);
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitDebugConsole(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
int main(void)
{
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* enable analog module */
    SPC0->ACTIVE_CFG1 |= 0x100;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    PRINTF("OPAMP BASIC EXAMPLE!\r\n");
    OPAMP_Configuration();
    while (1)
    {
    }
}

void OPAMP_Configuration(void)
{
    opamp_config_t config;
    /*
     *  config->enable        = false;
     *  config->enablePosADCSw1 = false;
     *  config->mode          = kOPAMP_LowNoiseMode;
     *  config->trimOption    = kOPAMP_TrimOptionDefault;
     *  config->intRefVoltage = kOPAMP_IntRefVoltVddaDiv2;
     *  config->enablePosADCSw1 = false;
     *  config->enablePosADCSw2 = false;
     *  config->posRefVoltage = kOPAMP_PosRefVoltVrefh3;
     *  config->posGain       = kOPAMP_PosGainReserved;
     *  config->negGain       = kOPAMP_NegGainBufferMode;
     *  config->enableRefBuffer = false;
     *  config->PosInputChannelSelection  = kOPAMP_PosInputChannel0
     *  config->enableTriggerMode = false;
     *  config->enableOutputSwitch = true;
     */
    OPAMP_GetDefaultConfig(&config);
    config.posGain = kOPAMP_PosGainNonInvertDisableBuffer2X;
    config.negGain = kOPAMP_NegGainInvert1X;
    config.enable  = true;
    OPAMP_Init(DEMO_OPAMP_BASEADDR, &config);
}
