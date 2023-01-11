/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdint.h>

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern uint32_t __bss_ocram_start__;
extern uint32_t __bss_ocram_end__;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Initialization of OCRAM BSS section.
 *
 * @param void
 *
 * @return None
 */
void SystemInitHook(void)
{
    uint32_t startAddr; /* Address of the source memory. */
    uint32_t endAddr;   /* End of copied memory. */

    /* Get the addresses for the OCRAM BSS section (zero-initialized data). */
    startAddr = (uint32_t)&__bss_ocram_start__;
    endAddr   = (uint32_t)&__bss_ocram_end__;

    /* Reset the .bss section. */
    while (startAddr < endAddr)
    {
        /* Clear one byte. */
        *((uint8_t *)startAddr) = 0U;

        /* Increment the pointer. */
        startAddr++;
    }
}
