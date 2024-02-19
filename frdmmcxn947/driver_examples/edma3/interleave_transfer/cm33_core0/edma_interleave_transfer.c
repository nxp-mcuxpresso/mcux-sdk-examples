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
#define BUFFER_LENGTH    16U
#define HALF_BUFF_LENGTH (BUFFER_LENGTH / 2)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_INIT(uint32_t srcAddr[HALF_BUFF_LENGTH]) = {0x01U, 0x02U, 0x03U, 0x04U,
                                                                    0x05U, 0x06U, 0x07U, 0x08U};
AT_NONCACHEABLE_SECTION_INIT(uint32_t destAddr[BUFFER_LENGTH])   = {0x00U};
edma_handle_t g_DMA_Handle;
volatile bool g_Transfer_Done = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

/* User callback function for EDMA transfer. */
void DMA_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        g_Transfer_Done = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i;
    edma_transfer_config_t transferConfig;
    edma_config_t userConfig;

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    /* Print destination buffer */
    PRINTF("EDMA interleave transfer example begin.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }

    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_DMA_BASEADDR, &userConfig);

    EDMA_CreateHandle(&g_DMA_Handle, EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0);
    EDMA_SetCallback(&g_DMA_Handle, DMA_Callback, NULL);
    EDMA_PrepareTransfer(&transferConfig, srcAddr, sizeof(srcAddr[0]), destAddr, sizeof(destAddr[0]),
                         sizeof(uint32_t) * HALF_BUFF_LENGTH, sizeof(srcAddr), kEDMA_MemoryToMemory);
    transferConfig.srcOffset  = 4U;
    transferConfig.destOffset = 8U;
    EDMA_SubmitTransfer(&g_DMA_Handle, &transferConfig);

    EDMA_StartTransfer(&g_DMA_Handle);

    /* Wait for EDMA transfer finish */
    while (g_Transfer_Done != true)
    {
    }

    /* Print destination buffer */
    PRINTF("\r\n\r\nEDMA interleave transfer example finish.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }
    while (1)
    {
    }
}
