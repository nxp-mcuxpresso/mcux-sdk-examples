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
#define DEMO_DMA_CHANNEL_0   0U
#define APP_DMA_IRQ          EDMA0_CH0_IRQn
#define APP_DMA_IRQ_HANDLER  EDMA0_CH0_IRQHandler
#define BUFFER_LENGTH 4U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_INIT(uint32_t srcAddr[BUFFER_LENGTH])  = {0x01, 0x02, 0x03, 0x04};
AT_NONCACHEABLE_SECTION_INIT(uint32_t destAddr[BUFFER_LENGTH]) = {0x00, 0x00, 0x00, 0x00};
volatile bool g_Transfer_Done                                  = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

/* EDMA transfer channel 0 IRQ handler */
void APP_DMA_IRQ_HANDLER(void)
{
    if ((EDMA_GetChannelStatusFlags(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0) & kEDMA_InterruptFlag) != 0U)
    {
        EDMA_ClearChannelStatusFlags(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, kEDMA_InterruptFlag);
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

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    RESET_ClearPeripheralReset(kDMA0_RST_SHIFT_RSTn);
    /* Print destination buffer */
    PRINTF("EDMA memset begin.\r\n\r\n");
    PRINTF("Source Buffer:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", srcAddr[i]);
    }
    PRINTF("\r\n\r\nDestination Buffer:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }

    /* Configure EDMA channel for one shot transfer */
    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_DMA_BASEADDR, &userConfig);
    EDMA_PrepareTransferConfig(&transferConfig, srcAddr, sizeof(srcAddr[0]), 0U, destAddr, sizeof(destAddr[0]),
                               sizeof(destAddr[0]), sizeof(destAddr), sizeof(destAddr));
    EDMA_SetTransferConfig(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, &transferConfig, NULL);
    EnableIRQ(APP_DMA_IRQ);

    EDMA_TriggerChannelStart(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0);

    /* Wait for EDMA transfer finish */
    while (g_Transfer_Done != true)
    {
    }
    /* Print destination buffer */
    PRINTF("\r\n\r\nEDMA memset finish.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }
    while (1)
    {
    }
}
