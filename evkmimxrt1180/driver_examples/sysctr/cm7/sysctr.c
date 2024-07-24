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
#include "fsl_sysctr.h"
#include "fsl_str.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SYSCTR_IRQ_ID                   SYS_CTR1_IRQn
#define EXAMPLE_SYSCTR_COMPARE          SYS_CTR_COMPARE
#define EXAMPLE_SYSCTR_CONTROL          SYS_CTR_CONTROL
#define EXAMPLE_SYSCTR_READ             SYS_CTR_READ
#define EXAMPLE_SYSCTR_IRQHandler       SYS_CTR1_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_sysctrIsrFlag = false;
volatile uint64_t g_counterValue = 0U;
/*******************************************************************************
 * Code
 ******************************************************************************/
void EXAMPLE_SYSCTR_IRQHandler(void)
{
    uint32_t statusFlag;

    statusFlag = SYSCTR_GetStatusFlags(EXAMPLE_SYSCTR_COMPARE);

    /*
     * Due to Compare0 and Compare1 Interrupt Status flag cannot be cleared directly,
     * it is necessary to test them at the same time.
     * Otherwise code cannot jump to else if branch when Compare1 Interrupt happens.
     */
    if ((statusFlag & kSYSCTR_Compare0Flag) && !(statusFlag & kSYSCTR_Compare1Flag))
    {
        g_sysctrIsrFlag = true;
        g_counterValue = SYSCTR_GetCounterlValue(EXAMPLE_SYSCTR_READ);
        SYSCTR_DisableInterrupts(EXAMPLE_SYSCTR_COMPARE, kSYSCTR_Compare0InterruptEnable);
    }
    else if (statusFlag & kSYSCTR_Compare1Flag)
    {
        g_sysctrIsrFlag = true;
        g_counterValue = SYSCTR_GetCounterlValue(EXAMPLE_SYSCTR_READ);
        SYSCTR_DisableInterrupts(EXAMPLE_SYSCTR_COMPARE, kSYSCTR_Compare1InterruptEnable);
    }
    else
    {
        /* Intentional empty */
    }

    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    sysctr_config_t sysctrConfig;
    SYSCTR_GetDefaultConfig(&sysctrConfig);

    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Initialize SYSCTR module */
    SYSCTR_Init(EXAMPLE_SYSCTR_CONTROL, EXAMPLE_SYSCTR_COMPARE, &sysctrConfig);

    SYSCTR_SetCounterClockSource(EXAMPLE_SYSCTR_CONTROL, kSYSCTR_BaseFrequency);

    /* Set SYSCTR module Compare Value */
    SYSCTR_SetCompareValue(EXAMPLE_SYSCTR_CONTROL, EXAMPLE_SYSCTR_COMPARE, kSYSCTR_CompareFrame_0, 0x44AA200U);
    SYSCTR_SetCompareValue(EXAMPLE_SYSCTR_CONTROL, EXAMPLE_SYSCTR_COMPARE, kSYSCTR_CompareFrame_1, 0x7270E00U);

    /* Enable SYSCTR module Compare0 and Compare1 */
    SYSCTR_EnableCompare(EXAMPLE_SYSCTR_COMPARE, kSYSCTR_CompareFrame_0, true);
    SYSCTR_EnableCompare(EXAMPLE_SYSCTR_COMPARE, kSYSCTR_CompareFrame_1, true);

    /* Enable SYSCTR Compare0 and Compare1 interrupt */
    SYSCTR_EnableInterrupts(EXAMPLE_SYSCTR_COMPARE, kSYSCTR_Compare0InterruptEnable | kSYSCTR_Compare1InterruptEnable);

    /* Enable at the Interrupt */
    EnableIRQ(SYSCTR_IRQ_ID);

    /* Start Timer */
    PRINTF("\r\nStarting System Counter ...");
    SYSCTR_StartCounter(EXAMPLE_SYSCTR_CONTROL);

    while (true)
    {
        if (true == g_sysctrIsrFlag)
        {
            PRINTF("\r\n System Counter compare interrupt is occurred !");
            PRINTF("\r\n System Counter upper 24 bits is %x ", (uint32_t)(g_counterValue >> 32U));
            PRINTF("\r\n System Counter lower 32 bits is %x ", (uint32_t)g_counterValue);
            g_sysctrIsrFlag = false;
        }
    }
}
