/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_tpm.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* define instance */
#define DEMO_TPM_BASEADDR TPM2

/* Interrupt to enable and flag to read; depends on the TPM channel used */
#define BOARD_TPM_INPUT_CAPTURE_CHANNEL kTPM_Chnl_0

/* Interrupt to enable and flag to read; depends on the TPM channel used */
#define TPM_CHANNEL_INTERRUPT_ENABLE kTPM_Chnl0InterruptEnable
#define TPM_CHANNEL_FLAG             kTPM_Chnl0Flag

/* Interrupt number and interrupt handler for the TPM instance used */
#define TPM_INTERRUPT_NUMBER      TPM2_IRQn
#define TPM_INPUT_CAPTURE_HANDLER TPM2_IRQHandler

/* Get source clock for TPM driver */
#define LPTPM_CLOCK_ROOT kCLOCK_Root_Tpm2
#define LPTPM_CLOCK_GATE kCLOCK_Tpm2
#define TPM_SOURCE_CLOCK CLOCK_GetIpFreq(LPTPM_CLOCK_ROOT)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool tpmIsrFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void TPM_INPUT_CAPTURE_HANDLER(void)
{
    tpmIsrFlag = true;

    /* Clear interrupt flag.*/
    TPM_ClearStatusFlags(DEMO_TPM_BASEADDR, TPM_CHANNEL_FLAG);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    tpm_config_t tpmInfo;

    /* Board pin, clock, debug console init */
    /* clang-format off */

    const clock_root_config_t lptpmClkCfg = {
        .clockOff = false,
	.mux = 0,
	.div = 1
    };
    /* clang-format on */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(LPTPM_CLOCK_ROOT, &lptpmClkCfg);
    CLOCK_EnableClock(LPTPM_CLOCK_GATE);

    /* Print a note to terminal */
    PRINTF("\r\nTPM input capture example\r\n");
    PRINTF("\r\nOnce the input signal is received the input capture value is printed\r\n");

    TPM_GetDefaultConfig(&tpmInfo);
    /* Initialize TPM module */
    TPM_Init(DEMO_TPM_BASEADDR, &tpmInfo);

    /* Setup input capture on a TPM channel */
    TPM_SetupInputCapture(DEMO_TPM_BASEADDR, BOARD_TPM_INPUT_CAPTURE_CHANNEL, kTPM_FallingEdge);

    /* Set the timer to be in free-running mode */
    TPM_SetTimerPeriod(DEMO_TPM_BASEADDR, TPM_MAX_COUNTER_VALUE(DEMO_TPM_BASEADDR));

    /* Enable channel interrupt when the second edge is detected */
    TPM_EnableInterrupts(DEMO_TPM_BASEADDR, TPM_CHANNEL_INTERRUPT_ENABLE);

    /* Enable at the NVIC */
    EnableIRQ(TPM_INTERRUPT_NUMBER);

    TPM_StartTimer(DEMO_TPM_BASEADDR, kTPM_SystemClock);

    while (tpmIsrFlag != true)
    {
    }

    PRINTF("\r\nCapture value C(n)V=%x\r\n", TPM_GetChannelValue(DEMO_TPM_BASEADDR, BOARD_TPM_INPUT_CAPTURE_CHANNEL));
    while (1)
    {
    }
}
