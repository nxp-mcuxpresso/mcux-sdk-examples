/*
 * Copyright 2020, 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_acomp.h"

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_ACOMP_IRQHANDLER  GAU_ACOMP_INT_FUNC11_IRQHandler
#define EXAMPLE_ACOMP_ID          kACOMP_Acomp0
#define EXAMPLE_ACOMP_BASE        GAU_ACOMP
#define EXAMPLE_ACOMP_POS_CHANNEL kACOMP_PosCh0
#define EXAMPLE_ACOMP_NEG_CHANNEL kACOMP_NegChVIO_0P50


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
    acomp_config_t acompConfig;
    acomp_positive_input_config_t posInputConfig = {
        .channel     = EXAMPLE_ACOMP_POS_CHANNEL,
        .hysterLevel = kACOMP_Hyster0MV,
    };
    acomp_negative_input_config_t negInputConfig = {
        .channel     = EXAMPLE_ACOMP_NEG_CHANNEL,
        .hysterLevel = kACOMP_Hyster0MV,
    };

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_AttachClk(kMAIN_CLK_to_GAU_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivGauClk, 1U);
    CLOCK_EnableClock(kCLOCK_Gau);
    RESET_PeripheralReset(kGAU_RST_SHIFT_RSTn);

    POWER_PowerOnGau();

    PRINTF("\r\nACOMP Basic Example!\r\n");

    /*
     *  config->id = kACOMP_Acomp0;
     *  config->enable = false;
     *  config->warmupTime = kACOMP_WarmUpTime1us;
     *  config->responseMode = kACOMP_SlowResponseMode;
     *  config->inactiveValue = kACOMP_ResultLogicLow;
     *  config->intTrigType = kACOMP_HighLevelTrig;
     *  config->edgeDetectTrigSrc = kACOMP_EdgePulseDis;
     *  config->outPinMode = kACOMP_PinOutDisable;
     *  config->posInput = NULL;
     *  config->negInput = NULL;
     */
    ACOMP_GetDefaultConfig(&acompConfig);
    acompConfig.enable       = true;
    acompConfig.responseMode = kACOMP_FastResponseMode;
    acompConfig.posInput     = &posInputConfig;
    acompConfig.negInput     = &negInputConfig;
    ACOMP_Init(EXAMPLE_ACOMP_BASE, &acompConfig);

    PRINTF("Please press any key to get ACOMP execute result.\r\n");

    while (1)
    {
        GETCHAR();

        if (ACOMP_GetResult(EXAMPLE_ACOMP_BASE, EXAMPLE_ACOMP_ID))
        {
            PRINTF("\r\nThe positive input voltage is greater than negative input voltage!\r\n");
        }
        else
        {
            PRINTF("\r\nThe positive input voltage is less than negative input voltage!\r\n");
        }
    }
}
