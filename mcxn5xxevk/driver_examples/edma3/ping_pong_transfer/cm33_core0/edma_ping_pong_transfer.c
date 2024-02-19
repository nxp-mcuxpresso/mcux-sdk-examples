/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_edma.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_DMA_BASEADDR DMA0
#define DEMO_DMA_CHANNEL_0   0

#define BUFFER_LENGTH      8U
#define HALF_BUFFER_LENGTH (BUFFER_LENGTH / 2U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
edma_handle_t g_DMA_Handle;
volatile bool g_transferDone = false;

AT_NONCACHEABLE_SECTION_INIT(uint32_t srcAddr[BUFFER_LENGTH])  = {0x01U, 0x02U, 0x03U, 0x04U,
                                                                 0x05U, 0x06U, 0x07U, 0x08U};
AT_NONCACHEABLE_SECTION_INIT(uint32_t destAddr[BUFFER_LENGTH]) = {0x00U, 0x00U, 0x00U, 0x00U,
                                                                  0x00U, 0x00U, 0x00U, 0x00U};
AT_NONCACHEABLE_SECTION_ALIGN(edma_tcd_t g_DMA_Tcd[2], sizeof(edma_tcd_t));

/*******************************************************************************
 * Code
 ******************************************************************************/

/* User callback function for EDMA transfer. */
void DMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        g_transferDone = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0;
    edma_transfer_config_t transferConfig[2];
    edma_config_t userConfig;

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("EDMA ping pong transfer example begin.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }

    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_DMA_BASEADDR, &userConfig);

    EDMA_CreateHandle(&g_DMA_Handle, EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0);
    EDMA_SetCallback(&g_DMA_Handle, DMA_Callback, NULL);
    EDMA_InstallTCDMemory(&g_DMA_Handle, g_DMA_Tcd, 2);
    EDMA_PrepareTransfer(&transferConfig[0], &srcAddr[0], sizeof(srcAddr[0]), &destAddr[0], sizeof(destAddr[0]),
                         sizeof(uint32_t) * HALF_BUFFER_LENGTH, sizeof(uint32_t) * HALF_BUFFER_LENGTH,
                         kEDMA_MemoryToMemory);
    EDMA_PrepareTransfer(&transferConfig[1], &srcAddr[4], sizeof(srcAddr[0]), &destAddr[4], sizeof(destAddr[0]),
                         sizeof(uint32_t) * HALF_BUFFER_LENGTH, sizeof(uint32_t) * HALF_BUFFER_LENGTH,
                         kEDMA_MemoryToMemory);
    EDMA_SubmitLoopTransfer(&g_DMA_Handle, transferConfig, 2);
    EDMA_StartTransfer(&g_DMA_Handle);

    /* Wait for EDMA transfer finish */
    while (!g_transferDone)
    {
    }

    g_transferDone = false;

    EDMA_StartTransfer(&g_DMA_Handle);

    /* Wait for EDMA transfer finish */
    while (!g_transferDone)
    {
    }

    /* Print destination buffer */
    PRINTF("\r\n\r\nEDMA ping pong transfer example finish.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }
    /* Free the memory space allocated */
    while (1)
    {
    }
}
