/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_cdog.h"

#include <string.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CDOG_AppIRQHandler CDOG_DriverIRQHandler

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile uint32_t s_start = 0x00U;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void CDOG_AppIRQHandler(void)
{
    NVIC_DisableIRQ(CDOG_IRQn);

    PRINTF("CDOG IRQ Reached \r\n");

    if ((CDOG->FLAGS & CDOG_FLAGS_TO_FLAG_MASK))
    {
        PRINTF("* Timeout fault occured *\r\n\r\n");
    }
    if ((CDOG->FLAGS & CDOG_FLAGS_MISCOM_FLAG_MASK))
    {
        PRINTF("* Miscompare fault occured *\r\n\r\n");
    }
    if ((CDOG->FLAGS & CDOG_FLAGS_SEQ_FLAG_MASK))
    {
        PRINTF("* Sequence fault occured *\r\n\r\n");
    }
    if ((CDOG->FLAGS & CDOG_FLAGS_CNT_FLAG_MASK))
    {
        PRINTF("* Control fault occured *\r\n\r\n");
    }
    if ((CDOG->FLAGS & CDOG_FLAGS_STATE_FLAG_MASK))
    {
        PRINTF("* State fault occured *\r\n\r\n");
    }
    if ((CDOG->FLAGS & CDOG_FLAGS_ADDR_FLAG_MASK))
    {
        PRINTF("* Address fault occured *\r\n\r\n");
    }
    if ((CDOG->FLAGS & CDOG_FLAGS_POR_FLAG_MASK))
    {
        PRINTF("* POR occured *\r\n\r\n");
    }

    CDOG_Stop(CDOG, s_start);
    CDOG->FLAGS = 0x0U;
    CDOG_Start(CDOG, 0xFFFFFFU, s_start);
    NVIC_EnableIRQ(CDOG_IRQn);
}

#if defined(CDOG1)

void CDOG1_DriverIRQHandler(void)
{
    /* CDOG1 is not used in example */
}
#endif

void SecureCounterExample()
{
    uint32_t reload = 0xFFFFFFU;

    /* Sets start value for secure counter and reload value for instruction timer, */
    /* which changes CDOG status to active */
    CDOG_Start(CDOG, reload, s_start);

    s_start += 42U;
    /* Add 42 to secure counter */
    CDOG_Add(CDOG, s_start);

    /* Check if secure counter have value is 42 */
    CDOG_Check(CDOG, s_start);

    s_start -= 2U;
    /* Substract 2 from secure counter */
    CDOG_Sub(CDOG, 2U);

    /* Check if secure counter have value 40 */
    CDOG_Check(CDOG, s_start);

    s_start++;
    /* Add 1 to secure counter */
    CDOG_Add1(CDOG);

    /* Check if secure counter have value 41, and stops CDOG */
    CDOG_Stop(CDOG, s_start);

    /* Start CDOG again with new secure counter value */
    CDOG_Start(CDOG, reload, 0U);

    /* Check if secure counter have value 0 */
    CDOG_Check(CDOG, 0U);

    /* Check if secure counter have value 1, which should trigger FAULT */
    CDOG_Check(CDOG, 1U);
}

/*!
 * @brief Main function.
 */
int main(void)
{
    status_t result = kStatus_Fail;
    cdog_config_t conf;

    /* Init hardware */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("CDOG Peripheral Driver Example\r\n\r\n");

    CDOG_GetDefaultConfig(&conf);

    conf.timeout    = kCDOG_FaultCtrl_EnableInterrupt;
    conf.miscompare = kCDOG_FaultCtrl_EnableInterrupt;
    conf.sequence   = kCDOG_FaultCtrl_EnableInterrupt;
    conf.state      = kCDOG_FaultCtrl_EnableInterrupt;
    conf.address    = kCDOG_FaultCtrl_EnableInterrupt;
    conf.irq_pause  = kCDOG_IrqPauseCtrl_Pause;
    conf.debug_halt = kCDOG_DebugHaltCtrl_Pause;
    conf.lock       = kCDOG_LockCtrl_Unlock;

    /* Clears pending FLAGS and sets CONTROL register */
    result = CDOG_Init(CDOG, &conf);
    if (result != kStatus_Success)
    {
        PRINTF("Error while CDOG Init. CDOG was probably not in IDLE mode due SW reset.\r\n");
        return 1;
    }

    SecureCounterExample();

    /* Test if timeout fault already occured */
    /* Note only POR reset clears these bits */
    while ((CDOG->STATUS & CDOG_STATUS_NUMTOF_MASK) <= 0x0U)
    {
        PRINTF("intruction timer:%08x\r\n", CDOG->INSTRUCTION_TIMER);
    }

    CDOG_Stop(CDOG, s_start);
    CDOG_Deinit(CDOG);

    PRINTF("End of example\r\n");
    /* End of example */
    while (1)
    {
    }
}
