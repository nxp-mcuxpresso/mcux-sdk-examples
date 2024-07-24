/*
 * Copyright 2020-2021,2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "mcuxCsslExamples.h"
#include "els_pkc_common.h"
#include "mcux_els.h"
#include "mcux_pkc.h"

#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if (defined(PKC0))
#define PKC PKC0
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    char ch;
    uint8_t pass = 0;
    uint8_t fail = 0;

    /* Init board hardware. */
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Enable ELS and related clocks */
    if (ELS_PowerDownWakeupInit(ELS) != kStatus_Success)
    {
        PRINTF("\r\nELS init failed\r\n");
        return kStatus_Fail;
    }

    PRINTF("\r\nELS PKC common example\r\n");
    PRINTF("\r\n============================\r\n");

    PRINTF("ELS get info:");
    if (mcuxClEls_Common_Get_Info_example() == true)
    {
        pass++;
        PRINTF("pass \r\n");
    }
    else
    {
        fail++;
        PRINTF("fail \r\n");
    }

    PRINTF("RNG PRNG random:");
    if (mcuxClEls_Rng_Prng_Get_Random_example() == true)
    {
        pass++;
        PRINTF("pass \r\n");
    }
    else
    {
        fail++;
        PRINTF("fail \r\n");
    }

    PRINTF("Flow protection:");
    if (mcuxCsslFlowProtection_example() == true)
    {
        pass++;
        PRINTF("pass \r\n");
    }
    else
    {
        fail++;
        PRINTF("fail \r\n");
    }

    PRINTF("Memory compare:");
    if (mcuxCsslMemory_Compare_example() == EXIT_CODE_OK)
    {
        pass++;
        PRINTF("pass \r\n");
    }
    else
    {
        fail++;
        PRINTF("fail \r\n");
    }

    PRINTF("Memory copy:");
    if (mcuxCsslMemory_Copy_example() == EXIT_CODE_OK)
    {
        pass++;
        PRINTF("pass \r\n");
    }
    else
    {
        fail++;
        PRINTF("fail \r\n");
    }

    PRINTF("Key component operations:");
    if (mcuxClKey_example() == true)
    {
        pass++;
        PRINTF("pass \r\n");
    }
    else
    {
        fail++;
        PRINTF("fail \r\n");
    }

    PRINTF("ELS power down wake-up init:");
    if (ELS_PowerDownWakeupInit(ELS) == kStatus_Success)
    {
        pass++;
        PRINTF("pass \r\n");
    }
    else
    {
        fail++;
        PRINTF("fail \r\n");
    }

    PRINTF("PKC power down wake-up init:");
    if (PKC_PowerDownWakeupInit(PKC) == kStatus_Success)
    {
        pass++;
        PRINTF("pass \r\n");
    }
    else
    {
        fail++;
        PRINTF("fail \r\n");
    }

    PRINTF("\r\n============================\r\n");
    PRINTF("RESULT: ");
    if (fail == 0)
    {
        PRINTF("All %d test PASS!!\r\n", pass);
    }
    else
    {
        PRINTF("%d / %d test PASSED, %d FAILED!!\r\n", pass, pass + fail, fail);
    }

    PRINTF("ELS example END \r\n");
    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}
