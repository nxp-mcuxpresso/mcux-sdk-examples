/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void copy_to_ram(void);
static void clock_change(void);

#if defined(__ICCARM__)
#pragma section = "ramfunc_section"
#pragma section = "ramfunc_section_init"
void clock_change(void) @"ramfunc_section";
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Load$$EXEC_m_ramfunc$$Base[];   /* Base address for loading ram function*/
extern uint32_t Load$$EXEC_m_ramfunc$$Length[]; /* Size of ram function */
extern uint32_t Image$$EXEC_m_ramfunc$$Base[];
void clock_change(void) __attribute__((section("ramfunc_section")));
#elif defined(__GNUC__)
extern uint32_t ramfunc_load_address[];
extern uint32_t ramfunc_length[];
extern uint32_t ramfunc_execution_address[];
void clock_change(void) __attribute__((section("ramfunc_section")));
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    char ch;

    /* Init board hardware. */
    BOARD_InitPins();
    uint32_t uartClkSrcFreq;

    /* SIM_SOPT2[27:26]:
     *  00: Clock Disabled
     *  01: MCGFLLCLK, or MCGPLLCLK, or IRC48M
     *  10: OSCERCLK
     *  11: MCGIRCCLK
     */
    CLOCK_SetLpuartClock(1);

    uartClkSrcFreq = CLOCK_GetFreq(kCLOCK_PllFllSelClk);

    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);

    PRINTF("hello world.\r\n");

    /* Change QSPI clock frequency. Any clock change or QSPI configure change shall put into RAM */
    PRINTF("\r\nBegin to change QSPI clock frequency\r\n");
    copy_to_ram();
    clock_change();

    PRINTF("\r\nQSPI clock change finished!\r\n");

    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}

void copy_to_ram()
{
    uint8_t *codeRelocateRomStart;
    uint32_t codeRelocateSize;
    uint8_t *codeReloocateRamStart;
#if defined(__ICCARM__)
    codeRelocateRomStart  = (uint8_t *)__section_begin("ramfunc_section_init");
    codeRelocateSize      = (uint32_t)__section_size("ramfunc_section_init");
    codeReloocateRamStart = (uint8_t *)__section_begin("ramfunc_section");
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
    codeRelocateRomStart  = (uint8_t *)Load$$EXEC_m_ramfunc$$Base;
    codeRelocateSize      = (uint32_t)Load$$EXEC_m_ramfunc$$Length;
    codeReloocateRamStart = (uint8_t *)Image$$EXEC_m_ramfunc$$Base;
#elif defined(__GNUC__)
    codeRelocateRomStart  = (uint8_t *)ramfunc_load_address;
    codeRelocateSize      = (uint32_t)ramfunc_length;
    codeReloocateRamStart = (uint8_t *)ramfunc_execution_address;
#endif

    while (codeRelocateSize)
    {
        *codeReloocateRamStart++ = *codeRelocateRomStart++;
        codeRelocateSize--;
    }
}

/* This function changes the QSPI frequency */
void clock_change()
{
    uint32_t val = QuadSPI0->MCR;

    /* Disable QSPI */
    QuadSPI0->MCR |= QuadSPI_MCR_MDIS_MASK;

    /* Change clock frequency */
    val &= ~QuadSPI_MCR_SCLKCFG_MASK;
    val |= QuadSPI_MCR_SCLKCFG(1);
    QuadSPI0->MCR = val;

    /* Re-enable QSPI */
    QuadSPI0->MCR &= (uint32_t)~QuadSPI_MCR_MDIS_MASK;
}
