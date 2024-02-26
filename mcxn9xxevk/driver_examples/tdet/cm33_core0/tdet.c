/*
 * Copyright 2023 NXP
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

#include "fsl_tdet.h"

#include <string.h>

#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void VBAT0_DriverIRQHandler(void)
{
    NVIC_DisableIRQ(VBAT0_IRQn);

    PRINTF("TDET IRQ Reached!\r\n");

    PRINTF("Clear TDET IRQ r\n");
    TDET_ClearStatusFlags(TDET0, kTDET_StatusAll);

    NVIC_EnableIRQ(VBAT0_IRQn);
}

/*!
 * @brief Main function.
 */
int main(void)
{
    status_t result = kStatus_Fail;

    uint32_t flags;

    tdet_config_t myConfig;

    DIGTMP_Type *base = TDET0;

    /* Init hardware */
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("TDET Peripheral Driver Example\r\n\r\n");

    result = TDET_Init(base);
    if (result != kStatus_Success)
    {
        PRINTF("Error while TDET Init.\r\n");
        return 1;
    }

    flags = 0;

    TDET_SoftwareReset(base);

    result = TDET_GetStatusFlags(base, &flags);
    if ((result == kStatus_Success) && (kTDET_StatusTamperFlag & flags))
    {
        result = TDET_ClearStatusFlags(base, kTDET_StatusAll);
    }

    TDET_GetDefaultConfig(base, &myConfig);

    result = TDET_SetConfig(base, &myConfig);

    NVIC_EnableIRQ(VBAT0_IRQn);

    // TDET_EnableInterrupts(base, kTDET_InterruptAll);

    /* force Tamper Detect Flag to be set. */
    result = TDET_ForceTamper(base);

    flags  = 0;
    result = TDET_GetStatusFlags(base, &flags);
    if ((result == kStatus_Success) && (kTDET_StatusTamperFlag & flags))

    {
        PRINTF("Tampering detected Tamper Detect Flag is set \r\n\r\n");
    }

    result = TDET_ClearStatusFlags(base, kTDET_StatusAll);

    PRINTF("Passive tamper example \r\n");
    /* Set external tamper PIN 0*/

    tdet_pin_config_t tamper_cfg;

    TDET_PinGetDefaultConfig(base, &tamper_cfg);

    /* Set value as inverted so it should be triggered even without need to connecting anything */
    tamper_cfg.pinPolarity = kTDET_TamperPinPolarityExpectInverted; /*Configure tamper pin to be inverted*/

    result = TDET_PinSetConfig(base, &tamper_cfg, kTDET_ExternalTamper0);

    result = TDET_EnableTampers(base, kTDET_TamperTamperPin0);
    // result = TDET_EnableInterrupts(base, kTDET_InterruptTamperPinTamper0);

    /* Check if bit in Status register has been set for Tamper Pin flag*/

    flags  = 0;
    result = TDET_GetStatusFlags(base, &flags);
    if ((result == kStatus_Success) && (kTDET_StatusTamperPinTamper0 & flags))
    {
        PRINTF("Tampering detected on PIN0 \r\n\r\n");
    }
    else
    {
        PRINTF("No tampering detected on PIN0 \r\n\r\n");
    }
    result = TDET_DisableTampers(base, kTDET_TamperTamperPin0);

    result = TDET_ClearStatusFlags(base, kTDET_StatusTamperPinTamper0);

    PRINTF("Active tamper example \r\n");

    tdet_pin_config_t active_tamper_cfg_tx;
    tdet_pin_config_t active_tamper_cfg_rx;

    tdet_active_tamper_config_t active_tamper_seting;

    active_tamper_seting.activeTamperPolynomial = 111u;
    active_tamper_seting.activeTamperShift      = 2u;

    result = TDET_ActiveTamperSetConfig(base, &active_tamper_seting, kTDET_ActiveTamperRegister0);

    TDET_PinGetDefaultConfig(base, &active_tamper_cfg_tx);

    active_tamper_cfg_tx.pinDirection =
        kTDET_TamperPinDirectionOut; /*Set tamper pin to be output driving inverse of expected value*/

    active_tamper_cfg_tx.pinPolarity = kTDET_TamperPinPolarityExpectInverted; /*Configure tamper pin to be inverted*/
    active_tamper_cfg_tx.tamperPinExpected =
        kTDET_GlitchFilterExpectedActTamperOut0; /*Specify expected value for tamper pin as active tamper 0 output */

    result = TDET_PinSetConfig(base, &active_tamper_cfg_tx, kTDET_ExternalTamper1);

    TDET_PinGetDefaultConfig(base, &active_tamper_cfg_rx);

    active_tamper_cfg_rx.tamperPinExpected =
        kTDET_GlitchFilterExpectedActTamperOut0;  /*Specify expected value for tamper pin as active tamper 0 output */
    active_tamper_cfg_rx.tamperPullEnable = true; /*Enable pull-resistor on tamper pin*/
    active_tamper_cfg_rx.glitchFilterWidth =
        10; /*Specify glitch filter width of 22 [(10+1)*2] clock edges on tamper pin*/
    active_tamper_cfg_rx.glitchFilterEnable = true; /*Enable glitch filter on tamper pin*/

    result = TDET_PinSetConfig(base, &active_tamper_cfg_rx, kTDET_ExternalTamper2);

    result = TDET_EnableTampers(base, kTDET_TamperTamperPin1 | kTDET_TamperTamperPin2);

    /* if filter enable,wait some time  */
    for (uint32_t i = 0; i < 0xffff; i++)
    {
    }

    /* Check for tamper detection */

    flags  = 0;
    result = TDET_GetStatusFlags(base, &flags);

    if ((result == kStatus_Success) && (kTDET_StatusTamperPinTamper1 & flags))
    {
        PRINTF("Tampering detected on active tamper \r\n\r\n");
    }
    else
    {
        PRINTF("No tampering detected on active tamper \r\n\r\n");
    }

    /* Deinit TDET */
    TDET_Deinit(base);

    PRINTF("End of example\r\n");
    /* End of example */
    while (1)
    {
    }
}
