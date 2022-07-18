/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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
#include "board.h"

#include "fsl_sha.h"

#include <string.h>

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define EXAMPLE_SHA       (SHA0)
#define EXAMPLE_SHA_CLOCK (kCLOCK_Sha0)
#define EXAMPLE_SHA_RESET (kSHA_RST_SHIFT_RSTn)


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t s_Digest[32] = {0x0U};

static __attribute__((aligned(4))) const uint8_t s_Input[] =
    "Hello SHA-256 world. Hello SHA-256 world. Hello SHA-256 world. Hello SHA-256 world.";
static const uint8_t s_DigestExpected[32] = {0xE6, 0x86, 0x75, 0xE9, 0xAC, 0xFF, 0x8B, 0x57, 0xE3, 0x86, 0x31,
                                             0x2E, 0x85, 0x37, 0x1E, 0x64, 0x10, 0xBE, 0xCC, 0xE2, 0xA2, 0x49,
                                             0x00, 0x19, 0x55, 0xC4, 0x5C, 0x98, 0xCB, 0xE5, 0x37, 0x3E};

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function.
 */
int main(void)
{
    status_t status;
    sha_ctx_t ctx;
    uint32_t i;
    size_t digestSize = sizeof(s_Digest);

    /* Init hardware */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Ungate clock to SHA engine and reset it */
    CLOCK_EnableClock(EXAMPLE_SHA_CLOCK);
    RESET_PeripheralReset(EXAMPLE_SHA_RESET);

    /* Calculate, compare and print SHA-256 */

    PRINTF("SHA Peripheral Driver Example\r\n\r\n");

    PRINTF("Calculating SHA-256 digest.\r\n");
    PRINTF("Input: %s\r\n", (const char *)&(s_Input[0]));

    status = SHA_Init(EXAMPLE_SHA, &ctx, kSHA_Sha256);
    if (status != kStatus_Success)
    {
        PRINTF("Failed to initialize SHA module.\r\n");
        return 1;
    }

    status = SHA_Update(EXAMPLE_SHA, &ctx, s_Input, sizeof(s_Input) - 1U);
    if (status != kStatus_Success)
    {
        PRINTF("Failed to update SHA.\r\n");
        return 1;
    }

    status = SHA_Finish(EXAMPLE_SHA, &ctx, s_Digest, &digestSize);
    if (status != kStatus_Success)
    {
        PRINTF("Failed to finish SHA calculation.\r\n");
        return 1;
    }

    PRINTF("Output: ");
    for (i = 0U; i < digestSize; i++)
    {
        PRINTF("%02X", s_Digest[i]);
    }
    PRINTF("\r\n");

    if (memcmp(s_Digest, s_DigestExpected, sizeof(s_DigestExpected)) != 0)
    {
        PRINTF("Output does not match expected one.\r\n");
        return 1;
    }

    while (1)
    {
    }
}
