/*
 * Copyright 2022 NXP
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

#include "fsl_itrc.h"

#include <string.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ITRC ITRC0

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ITRC_Demo_Status_Print(void);
/*******************************************************************************
 * Code
 ******************************************************************************/

void ITRC0_DriverIRQHandler(void)
{
    NVIC_DisableIRQ(ITRC0_IRQn);
    PRINTF("ITRC IRQ Reached!\r\n");

    ITRC_Demo_Status_Print();

    PRINTF("Clear ITRC IRQ and SW Event 0 STATUS\r\n\r\n");
    ITRC_ClearStatus(ITRC, ((uint32_t)kITRC_Irq | (uint32_t)kITRC_SwEvent0));

    NVIC_EnableIRQ(ITRC0_IRQn);
}

void ITRC_Demo_Status_Print(void)
{
    uint32_t status_word = ITRC_GetStatus(ITRC);

    PRINTF("ITRC STATUS:\r\n");
    /* Input Event signals */
    if (ITRC_STATUS_IN0_STATUS_MASK & status_word)
        PRINTF("Digital glitch detector from CSSv2!\r\n");
    if (ITRC_STATUS_IN1_STATUS_MASK & status_word)
        PRINTF("Tamper pins detector present inside RTC module!\r\n");
    if (ITRC_STATUS_IN2_STATUS_MASK & status_word)
        PRINTF("Code Watch Dog which detects anomalies in code flow integrity!\r\n");
    if (ITRC_STATUS_IN3_STATUS_MASK & status_word)
        PRINTF("Low voltage detector (BoD) for VBAT rail!\r\n");
    if (ITRC_STATUS_IN4_STATUS_MASK & status_word)
        PRINTF("Low voltage detector (BoD) for VDD_CORE rail!\r\n");
    if (ITRC_STATUS_IN5_STATUS_MASK & status_word)
        PRINTF("Raw interrupt status of watchdog timer!\r\n");
    if (ITRC_STATUS_IN6_STATUS_MASK & status_word)
        PRINTF("Flash ECC exception event signal!\r\n");
    if (ITRC_STATUS_IN7_STATUS_MASK & status_word)
        PRINTF("AHB secure bus illegal access event!\r\n");
    if (ITRC_STATUS_IN8_STATUS_MASK & status_word)
        PRINTF("CSS reported error event!\r\n");
#if defined(FSL_FEATURE_ITRC_HAS_SYSCON_GLITCH) && (FSL_FEATURE_ITRC_HAS_SYSCON_GLITCH > 0)
    if (ITRC_STATUS_IN9_STATUS_MASK & status_word)
        PRINTF("Analog glitch sensor which is configured using SYSCON registers!\r\n");
#endif /* FSL_FEATURE_ITRC_HAS_SYSCON_GLITCH */
    if (ITRC_STATUS_IN10_STATUS_MASK & status_word)
        PRINTF("Error flag from PKC module!\r\n");
    if (ITRC_STATUS_IN14_STATUS_MASK & status_word)
        PRINTF("SW Event0 occured!\r\n");
    if (ITRC_STATUS_IN15_STATUS_MASK & status_word)
        PRINTF("SW Event1 occured!\r\n");
    /* Output Action signals */
    if (ITRC_STATUS_OUT0_STATUS_MASK & status_word)
        PRINTF("Generated ITRC interrupt!\r\n");
    if (ITRC_STATUS_OUT1_STATUS_MASK & status_word)
        PRINTF("Generated CSSv2 reset and key destroy!\r\n");
    if (ITRC_STATUS_OUT2_STATUS_MASK & status_word)
        PRINTF("Generated PUF Zeroize which disable PUF from outputting keys!\r\n");
    if (ITRC_STATUS_OUT3_STATUS_MASK & status_word)
        PRINTF("RAM Zeroized (including PKC RAM)!\r\n");
    if (ITRC_STATUS_OUT4_STATUS_MASK & status_word)
        PRINTF("Generated Chip reset! (Should be asserted only after PUF/RAM Zeroize response events completed)\r\n");
    if (ITRC_STATUS_OUT5_STATUS_MASK & status_word)
        PRINTF("Generated Tamper Out (INMUX)!\r\n");

    PRINTF("\r\n");
}

/*!
 * @brief Main function.
 */
int main(void)
{
    status_t result = kStatus_Fail;

    /* Init hardware */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("ITRC Peripheral Driver Example\r\n\r\n");

    /* Enable ITRC IRQ */
    result = ITRC_Init(ITRC);
    if (result != kStatus_Success)
    {
        PRINTF("Error while ITRC Init.\r\n");
        return 1;
    }

    /* Clear all possible pending Event/Action statuses */
    result = ITRC_ClearAllStatus(ITRC);
    if (result != kStatus_Success)
    {
        PRINTF("Error while ITRC STATUS Clear.\r\n");
        return 1;
    }

    /* Test if event or action already occured */
    if ((ITRC_GetStatus(ITRC) & (IN_0_15_EVENTS_MASK | OUT_ACTIONS_MASK)) == 0u)
    {
        PRINTF("Pass: No Event/Action triggered in STATUS after Init\r\n\r\n");
    }
    else
    {
        PRINTF("Fail: Action Triggered after Init!!");
    }

#if defined(ITRC_STATUS1_IN16_STATUS_MASK)
    /* Test if event or action already occured */
    if ((ITRC_GetStatus1(ITRC) & (IN_16_47_EVENTS_MASK)) == 0u)
    {
        PRINTF("Pass: No Event triggered in STATUS1 after Init\r\n\r\n");
    }
    else
    {
        PRINTF("Fail: Action Triggered after Init!!");
    }
#endif /* ITRC_STATUS1_IN16_STATUS_MASK */

    /* Set ITRC IRQ action upon SW Event 0 */
    PRINTF("Enable ITRC IRQ Action response to SW Event 0\r\n\r\n");
    ITRC_SetActionToEvent(ITRC, kITRC_Irq, kITRC_SwEvent0, kITRC_Unlock, kITRC_Enable);
    if (result != kStatus_Success)
    {
        PRINTF("Error seting ITRC.\r\n");
        return 1;
    }

    /* Trigger SW Event 0 */
    PRINTF("Trigger SW Event 0\r\n\r\n");
    ITRC_SetSWEvent0(ITRC);

    /* Wait a few tics for IRQ */
    __NOP();
    __NOP();

    /* Disable ITRC IRQ action upon SW Event 0 */
    PRINTF("Disable ITRC IRQ Action response to SW Event 0\r\n\r\n");
    ITRC_SetActionToEvent(ITRC, kITRC_Irq, kITRC_SwEvent0, kITRC_Unlock, kITRC_Disable);
    if (result != kStatus_Success)
    {
        PRINTF("Error seting ITRC.\r\n");
        return 1;
    }

    /* Trigger SW Event 0 when action is disabled */
    PRINTF("Trigger SW Event 0\r\n\r\n");
    ITRC_SetSWEvent0(ITRC);

    /* Wait a few tics for IRQ */
    __NOP();
    __NOP();

    /* Test if event occured after disabling */
    if ((ITRC_GetStatus(ITRC) & (OUT_ACTIONS_MASK)) == 0u)
    {
        PRINTF("Pass: No Action triggered\r\n\r\n");
    }
    else
    {
        PRINTF("Error: Action triggered by Event even if not selected!!");
    }

    /* Deinit ITRC by disable IRQ */
    ITRC_Deinit(ITRC);

    PRINTF("End of example\r\n");
    /* End of example */
    while (1)
    {
    }
}
