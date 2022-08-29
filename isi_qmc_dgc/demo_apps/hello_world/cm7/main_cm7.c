/*
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    main_cm7.c
 * @brief   Application entry point.
 */
#include "main_cm7.h"

/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */
__RAMFUNC(SRAM_ITC_cm7) void SoftwareHandler(void);
static void cm4Release(void);

/*
 * @brief   Application entry point.
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    cm4Release();

#if defined(ENABLE_RAM_VECTOR_TABLE)
    InstallIRQHandler(Reserved177_IRQn, (uint32_t)SoftwareHandler);
#endif

    EnableIRQ(BOARD_USER_BUTTON1_IRQ);
    EnableIRQ(BOARD_DIG_IN0_IRQ);
    EnableIRQ(BOARD_DIG_IN2_IRQ);
    EnableIRQ(BOARD_DIG_IN3_IRQ);
    EnableIRQ(BOARD_SLOW_DIG_IN4_IRQ);

    PRINTF("ISI QMC code started \r\n");

    SoftwareHandler();

    vTaskStartScheduler();

    return 0;
}

void SoftwareHandler(void)
{
}

static void cm4Release(void)
{
    PRINTF("Releasing CM4\r\n");
    IOMUXC_LPSR_GPR->GPR0 = ((uint32_t)(0x0000FFFF & (uint32_t)CORE1_REAL_BOOT_ADDRESS));
    IOMUXC_LPSR_GPR->GPR1 = IOMUXC_LPSR_GPR_GPR1_CM4_INIT_VTOR_HIGH((uint32_t)CORE1_REAL_BOOT_ADDRESS >> 16);

    SRC->CTRL_M4CORE = SRC_CTRL_M4CORE_SW_RESET_MASK;
    SRC->SCR |= SRC_SCR_BT_RELEASE_M4_MASK;
}
