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

#include "fsl_cwt.h"

#include <string.h>

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


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

void CodeWDG_DriverIRQHandler(void)
{
    NVIC_DisableIRQ(CodeWDG_IRQn);

    PRINTF("CWT IRQ Reached \r\n");

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

    CWT_Stop(CDOG, s_start);
    CDOG->FLAGS = 0x0U;
    CWT_Start(CDOG, 0xFFFFFU, s_start);
    NVIC_EnableIRQ(CodeWDG_IRQn);
}

void SecureCounterExample()
{
    uint32_t reload = 0xFFFFFU;

    /* Sets start value for secure counter and reload value for instruction timer, */
    /* which changes CWT status to active */
    CWT_Start(CDOG, reload, s_start);

    s_start += 42U;
    /* Add 42 to secure counter */
    CWT_Add(CDOG, s_start);

    /* Check if secure counter have value is 42 */
    CWT_Check(CDOG, s_start);

    s_start -= 2U;
    /* Substract 2 from secure counter */
    CWT_Sub(CDOG, 2U);

    /* Check if secure counter have value 40 */
    CWT_Check(CDOG, s_start);

    s_start++;
    /* Add 1 to secure counter */
    CWT_Add1(CDOG);

    /* Check if secure counter have value 41, and stops CWT */
    CWT_Stop(CDOG, s_start);

    /* Start CWT again with new secure counter value */
    CWT_Start(CDOG, reload, 0U);

    /* Check if secure counter have value 0 */
    CWT_Check(CDOG, 0U);

    /* Check if secure counter have value 1, which should trigger FAULT */
    CWT_Check(CDOG, 1U);
}

/*!
 * @brief Main function.
 */
int main(void)
{
    status_t result = kStatus_Fail;
    cwt_config_t conf;

    /* Init hardware */
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("CWT Peripheral Driver Example\r\n\r\n");

    CWT_GetDefaultConfig(&conf);

    conf.timeout    = kCDOG_FaultCtrl_EnableInterrupt;
    conf.miscompare = kCDOG_FaultCtrl_EnableInterrupt;
    conf.sequence   = kCDOG_FaultCtrl_EnableInterrupt;
    conf.control    = kCDOG_FaultCtrl_EnableReset; /* Note: Control can generate only reset */
    conf.state      = kCDOG_FaultCtrl_EnableInterrupt;
    conf.address    = kCDOG_FaultCtrl_EnableInterrupt;
    conf.irq_pause  = kCDOG_IrqPauseCtrl_Pause;
    conf.debug_halt = kCDOG_DebugHaltCtrl_Pause;
    conf.lock       = kCDOG_LockCtrl_Unlock;

    /* Clears pending FLAGS and sets CONTROL register */
    result = CWT_Init(CDOG, &conf);
    if (result != kStatus_Success)
    {
        PRINTF("Error while CWT Init. CWT was probably not in IDLE mode due SW reset.\r\n");
        return 1;
    }

    SecureCounterExample();

    /* Test if timeout fault already occured */
    /* Note only POR reset clears these bits */
    while ((CDOG->STATUS & CDOG_STATUS_NUMTOF_MASK) <= 0x0U)
    {
        PRINTF("intruction timer:%08x\r\n", CDOG->INSTRUCTION_TIMER);
    }

    CWT_Stop(CDOG, s_start);
    CWT_Deinit(CDOG);

    PRINTF("End of example\r\n");
    /* End of example */
    while (1)
    {
    }
}
