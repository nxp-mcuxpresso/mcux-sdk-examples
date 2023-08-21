/*
 * Copyright 2020-2021 NXP
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
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

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

    PRINTF("Memory data invariant compare:");
    if (data_invariant_memory_compare() == EXIT_CODE_OK)
    {
        pass++;
        PRINTF("pass \r\n");
    }
    else
    {
        fail++;
        PRINTF("fail \r\n");
    }

    PRINTF("Memory data invariant copy:");
    if (data_invariant_memory_compare() == EXIT_CODE_OK)
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

//    PRINTF("ELS power down wake-up init:");
//    if (ELS_PowerDownWakeupInit(ELS) == kStatus_Success)
//    {
//        pass++;
//        PRINTF("pass \r\n");
//    }
//    else
//    {
//        fail++;
//        PRINTF("fail \r\n");
//    }

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
