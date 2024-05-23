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
#define EXAMPLE_DMA_CHANNEL  0
#define BUFF_LENGTH      8U
#define HALF_BUFF_LENGTH (BUFF_LENGTH / 2)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_ALIGN_INIT(uint32_t srcAddr[HALF_BUFF_LENGTH], 16) = {0x01U, 0x02U, 0x03U, 0x04};
AT_NONCACHEABLE_SECTION_ALIGN_INIT(uint32_t destAddr[BUFF_LENGTH], 16)     = {0x00U};
edma_handle_t g_EDMA_Handle;
volatile bool g_Transfer_Done = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

/* User callback function for EDMA transfer. */
void EDMA_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds)
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
    PRINTF("EDMA wrap transfer example begin.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFF_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }

    /* Configure EDMA channel for one shot transfer */
    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_DMA_BASEADDR, &userConfig);

    EDMA_CreateHandle(&g_EDMA_Handle, EXAMPLE_DMA_BASEADDR, EXAMPLE_DMA_CHANNEL);
    EDMA_SetCallback(&g_EDMA_Handle, EDMA_Callback, NULL);
    EDMA_PrepareTransfer(&transferConfig, srcAddr, sizeof(srcAddr[0]), destAddr, sizeof(destAddr[0]),
                         sizeof(srcAddr[0]) * BUFF_LENGTH, sizeof(srcAddr[0]) * BUFF_LENGTH, kEDMA_MemoryToMemory);
    transferConfig.srcAddrModulo = kEDMA_Modulo16bytes;
    EDMA_SubmitTransfer(&g_EDMA_Handle, &transferConfig);
    EDMA_StartTransfer(&g_EDMA_Handle);
    /* Wait for EDMA transfer finish */
    while (g_Transfer_Done != true)
    {
    }
    /* Print destination buffer */
    PRINTF("\r\n\r\nEDMA wrap transfer example finish.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFF_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }
    while (1)
    {
    }
}
