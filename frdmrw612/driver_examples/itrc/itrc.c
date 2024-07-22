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

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ITRC_DISABLE_VOLTAGE_SENSORS (1u)

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

void ITRC_DriverIRQHandler(void)
{
    NVIC_DisableIRQ(ITRC_IRQn);
    PRINTF("ITRC IRQ Reached!\r\n");

    ITRC_Demo_Status_Print();

    PRINTF("Clear ITRC IRQ and SW Event 0 STATUS\r\n\r\n");
    ITRC_ClearOutActionStatus(ITRC, kITRC_Irq);
    ITRC_ClearInEventStatus(ITRC, kITRC_SwEvent0);

    EnableIRQ(ITRC_IRQn);
}

void ITRC_Demo_Status_Print(void)
{
    PRINTF("ITRC STATUS0:\r\n");
    /* Input Event signals STATUS0 */
    if (ITRC_GetInEventStatus(ITRC, kITRC_CauTemeprature))
        PRINTF("CAU Temeprature Sensor detector event occurred!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_PmipTemperature))
        PRINTF("PMIP Temperature Sensor detector event occurred!\r\n");
#if (ITRC_DISABLE_VOLTAGE_SENSORS != 1u) /* Disable checking of voltage sensors which may catch some events */
    if (ITRC_GetInEventStatus(ITRC, kITRC_VddCore))
        PRINTF("Voltage Sensor detector event occured on VDD_CORE rail!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_Vdd18))
        PRINTF("Voltage Sensor detector event occured on VDD_18 rail!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_Vdd33))
        PRINTF("Voltage Sensor detector event occured on VDD_33 rail!\r\n");
#endif
    if (ITRC_GetInEventStatus(ITRC, kITRC_VddCoreGlitch))
        PRINTF("CAU Analog glitch sensor event occurred on VDD_CORE rail!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_AnalogSensor))
        PRINTF("Analog Sensor configuration control anamoly detected!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_Ahb))
        PRINTF("AHB secure bus checkers detected illegal access!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_Cwd))
        PRINTF("Code watchdog detected an code execution anomaly!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_Css))
        PRINTF("CSS error event occurred!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_Pkc))
        PRINTF("PKC module detected an error event!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_Otp))
        PRINTF("OTP module detected an error event!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_Prince))
        PRINTF("Prince IP module detected an error event!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_ClockGlitch))
        PRINTF("Digital Clock glitch detector module detected an error event!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_SecurityIP))
        PRINTF("Security IP Command violation error event!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_Trng))
        PRINTF("True Random Number generator error event!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_PmipGlitch))
        PRINTF("PMIP Analog glitch sensor event occurred on VDD_18 rail!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_PmipVddCoreGlitch))
        PRINTF("PMIP Analog glitch sensor event occurred on VDD_CORE rail!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_TcpuPll))
        PRINTF("TCPU PLL UnLock Error occurred!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_T3Pll))
        PRINTF("T3 PLL UnLock Error occurred!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_SwEvent0))
        PRINTF("Software event 0 occurred!\r\n");
    if (ITRC_GetInEventStatus(ITRC, kITRC_SwEvent1))
        PRINTF("Software event 1 occurred!\r\n");

    /* Output Action signals */
    if (ITRC_GetOutActionStatus(ITRC, kITRC_Irq))
        PRINTF("ITRC triggered ITRC_IRQ output!\r\n");
    if (ITRC_GetOutActionStatus(ITRC, kITRC_ChipReset))
        PRINTF("ITRC triggered CHIP_RESET to reset the chip after all other response process finished!\r\n");

    PRINTF("\r\n");
}

/*!
 * @brief Main function.
 */
int main(void)
{
    status_t result = kStatus_Fail;

    /* Init hardware */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
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
    if (ITRC_GetInEventStatus(ITRC, kITRC_SwEvent0))
    {
        PRINTF("Fail: Action Triggered after Init!!");
    }
    else
    {
        PRINTF("Pass: No Event/Action triggered after Init\r\n\r\n");
    }

    /* Set ITRC IRQ action upon SW Event 0 */
    PRINTF("Enable ITRC IRQ Action response to SW Event 0\r\n\r\n");
    result = ITRC_SetActionToEvent(ITRC, kITRC_Irq, kITRC_SwEvent0, kITRC_Unlock, kITRC_Enable);
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
    result = ITRC_SetActionToEvent(ITRC, kITRC_Irq, kITRC_SwEvent0, kITRC_Unlock, kITRC_Disable);
    if (result != kStatus_Success)
    {
        PRINTF("Error seting ITRC.\r\n");
        return 1;
    }

    /* Clear all possible pending Event/Action statuses */
    result = ITRC_ClearAllStatus(ITRC);
    if (result != kStatus_Success)
    {
        PRINTF("Error while ITRC STATUS Clear.\r\n");
        return 1;
    }

    /* Trigger SW Event 0 when action is disabled */
    PRINTF("Trigger SW Event 0\r\n\r\n");
    ITRC_SetSWEvent0(ITRC);

    /* Wait a few tics for IRQ */
    __NOP();
    __NOP();

    /* Test if event occured after disabling */
    if (ITRC_GetOutActionStatus(ITRC, kITRC_Irq))
    {
        PRINTF("Error: Action triggered by Event even if not selected!!");
    }
    else
    {
        PRINTF("Pass: No Action triggered after disabling\r\n\r\n");
    }

    /* Deinit ITRC by disable IRQ */
    ITRC_Deinit(ITRC);

    PRINTF("End of example\r\n");
    /* End of example */
    while (1)
    {
    }
}
