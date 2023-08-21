/*
 * Copyright 2021 NXP
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
#include "fsl_dac.h"

#include "fsl_power.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DAC_BASEADDR   DAC0
#define DEMO_DAC_VREF       kDAC_ReferenceVoltageSourceAlt1
#define DEMO_OPAMP_BASEADDR OPAMP0
#define DEMO_DAC_VOLT_STEP  0.806f

#define DAC_VALUE_TO_MVOLT(dacValue) ((double)(dacValue)*DEMO_DAC_VOLT_STEP)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void DEMO_InitDAC(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/



/*******************************************************************************
 * Code
 ******************************************************************************/

int main(void)
{
    opamp_config_t config;
    uint8_t ch;
    uint32_t dacValue;

    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    CLOCK_AttachClk(kFRO12M_to_DAC0);
    CLOCK_SetClkDiv(kCLOCK_DivDac0Clk, 1UL, true);
    POWER_DisablePD(kPDRUNCFG_PD_OPAMP0);
    POWER_DisablePD(kPDRUNCFG_PD_DAC0);

    PRINTF("OPAMP EXAMPLE!\r\n");

    DEMO_InitDAC();

    /*
     *  config->enable        = false;
     *  config->mode          = kOPAMP_LowNoiseMode;
     *  config->trimOption    = kOPAMP_TrimOptionDefault;
     *  config->intRefVoltage = kOPAMP_IntRefVoltVddaDiv2;
     *
     *  config->enablePosADCSw = false;
     *  config->posRefVoltage = kOPAMP_PosRefVoltVrefh3;
     *  config->posGain       = kOPAMP_PosGainReserved;
     *
     *  config->negGain       = kOPAMP_NegGainBufferMode;
     */
    OPAMP_GetDefaultConfig(&config);
    config.posGain = kOPAMP_PosGainNonInvert1X;
    config.negGain = kOPAMP_NegGainInvert1X;
    /* Connect REFP to DAC output. */
    config.posRefVoltage = kOPAMP_PosRefVoltVrefh3;
    config.enable        = true;

    OPAMP_Init(DEMO_OPAMP_BASEADDR, &config);

    while (1)
    {
        ch       = 0U;
        dacValue = 0UL;
        PRINTF("\r\nPlease input a value (0 - 4095) for DAC:");

        while (ch != 0x0D)
        {
            ch = GETCHAR();
            if ((ch >= '0') && (ch <= '9'))
            {
                PUTCHAR(ch);
                dacValue = dacValue * 10 + (ch - '0');
            }
        }

        PRINTF("\r\nInput DAC value is %d\r\n", dacValue);
        if (dacValue > 4095)
        {
            PRINTF("DAC value is out of range.\r\n");
            continue;
        }

        DAC_SetData(DEMO_DAC_BASEADDR, dacValue);

        PRINTF("Current OPAMP output is about %6.3f mV\r\n", (DAC_VALUE_TO_MVOLT(dacValue)));
    }
}

void DEMO_InitDAC(void)
{
    dac_config_t dacConfig;

    DAC_GetDefaultConfig(&dacConfig);
    dacConfig.referenceVoltageSource = DEMO_DAC_VREF;
    DAC_Init(DEMO_DAC_BASEADDR, &dacConfig);
    DAC_Enable(DEMO_DAC_BASEADDR, true);
}
