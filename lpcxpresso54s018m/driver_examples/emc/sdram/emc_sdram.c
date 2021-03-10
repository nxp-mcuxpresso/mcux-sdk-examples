/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_emc.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SDRAM_BASE_ADDR  0xa0000000
#define SDRAM_SIZE_BYTES (8 * 1024 * 1024)
#define SDRAM_EXAMPLE_DATALEN (SDRAM_SIZE_BYTES / 4)
#define SDRAM_TEST_PATTERN    (2)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t SDRAM_DataBusCheck(volatile uint32_t *address)
{
    uint32_t data = 0;

    /* Write the walking 1's data test. */
    for (data = 1; data != 0; data <<= 1)
    {
        *address = data;

        /* Read the data out of the address and check. */
        if (*address != data)
        {
            return kStatus_Fail;
        }
    }
    return kStatus_Success;
}

status_t SDRAM_AddressBusCheck(volatile uint32_t *address, uint32_t bytes)
{
    uint32_t pattern = 0x55555555;
    uint32_t size    = bytes / 4;
    uint32_t offset;
    uint32_t checkOffset;

    /* write the pattern to the power-of-two address. */
    for (offset = 1; offset < size; offset <<= 1)
    {
        address[offset] = pattern;
    }
    address[0] = ~pattern;

    /* Read and check. */
    for (offset = 1; offset < size; offset <<= 1)
    {
        if (address[offset] != pattern)
        {
            return kStatus_Fail;
        }
    }

    if (address[0] != ~pattern)
    {
        return kStatus_Fail;
    }

    /* Change the data to the revert one address each time
     * and check there is no effect to other address. */
    for (offset = 1; offset < size; offset <<= 1)
    {
        address[offset] = ~pattern;
        for (checkOffset = 1; checkOffset < size; checkOffset <<= 1)
        {
            if ((checkOffset != offset) && (address[checkOffset] != pattern))
            {
                return kStatus_Fail;
            }
        }
        address[offset] = pattern;
    }
    return kStatus_Success;
}

int main(void)
{
    uint32_t index;
    uint32_t *sdram = (uint32_t *)SDRAM_BASE_ADDR; /* SDRAM start address. */

    /* Hardware Initialization */
    CLOCK_EnableClock(kCLOCK_InputMux);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL180M();
    BOARD_InitDebugConsole();
    BOARD_InitSDRAM();
    /* Data/address bus check. */
    if (SDRAM_DataBusCheck(sdram) != kStatus_Success)
    {
        PRINTF("\r\n SDRAM data bus check is failure.\r\n");
    }

    if (SDRAM_AddressBusCheck(sdram, SDRAM_SIZE_BYTES) != kStatus_Success)
    {
        PRINTF("\r\n SDRAM address bus check is failure.\r\n");
    }

    PRINTF("\r\n Start EMC SDRAM access example.\r\n");
    PRINTF("\r\n SDRAM Write Start, Start Address 0x%x, Data Length %d !\r\n", sdram, SDRAM_EXAMPLE_DATALEN);
    /* Prepare data and write to SDRAM. */
    for (index = 0; index < SDRAM_EXAMPLE_DATALEN; index++)
    {
        *(uint32_t *)(sdram + index) = index;
    }
    PRINTF("\r\n SDRAM Write finished!\r\n");

    PRINTF("\r\n SDRAM Read/Check Start, Start Address 0x%x, Data Length %d !\r\n", sdram, SDRAM_EXAMPLE_DATALEN);
    /* Read data from the SDRAM. */
    for (index = 0; index < SDRAM_EXAMPLE_DATALEN; index++)
    {
        if (*(uint32_t *)(sdram + index) != index)
        {
            PRINTF("\r\n SDRAM Write Data and Read Data Check Error!\r\n");
            break;
        }
    }

    PRINTF("\r\n SDRAM Write Data and Read Data Succeed.\r\n");

    EMC_Deinit(EMC);

    PRINTF("\r\n SDRAM Example End.\r\n");

    while (1)
    {
    }
}
