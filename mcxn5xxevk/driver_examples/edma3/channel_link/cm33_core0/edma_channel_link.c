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
#define DEMO_DMA_CHANNEL_1   1
#define DEMO_DMA_CHANNEL_2   2
#define APP_DMA_IRQ_HANDLER  EDMA_0_CH2_IRQHandler
#define APP_DMA_IRQ          EDMA_0_CH2_IRQn

#define BUFFER_LENGTH 4U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_Transfer_Done = false;

AT_NONCACHEABLE_SECTION_INIT(uint32_t srcAddr[BUFFER_LENGTH])   = {0x01U, 0x02U, 0x03U, 0x04U};
AT_NONCACHEABLE_SECTION_INIT(uint32_t destAddr0[BUFFER_LENGTH]) = {0x00U, 0x00U, 0x00U, 0x00U};
AT_NONCACHEABLE_SECTION_INIT(uint32_t destAddr1[BUFFER_LENGTH]) = {0x00U, 0x00U, 0x00U, 0x00U};
AT_NONCACHEABLE_SECTION_INIT(uint32_t destAddr2[BUFFER_LENGTH]) = {0x00U, 0x00U, 0x00U, 0x00U};

/*******************************************************************************
 * Code
 ******************************************************************************/

/* EDMA transfer channel 0 IRQ handler */
void APP_DMA_IRQ_HANDLER(void)
{
    if ((EDMA_GetChannelStatusFlags(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_2) & kEDMA_InterruptFlag) != 0U)
    {
        EDMA_ClearChannelStatusFlags(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_2, kEDMA_InterruptFlag);
        g_Transfer_Done = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i;
    edma_transfer_config_t transferConfig0;
    edma_transfer_config_t transferConfig1;
    edma_transfer_config_t transferConfig2;
    edma_config_t userConfig;

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Print destination buffer */
    PRINTF("EDMA channel link example begin.\r\n\r\n");
    PRINTF("Destination Buffer 0:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr0[i]);
    }
    PRINTF("\r\n\r\nDestination Buffer 1:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr1[i]);
    }
    PRINTF("\r\n\r\nDestination Buffer 2:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr2[i]);
    }

    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_DMA_BASEADDR, &userConfig);

    EDMA_PrepareTransfer(&transferConfig0, srcAddr, sizeof(srcAddr[0]), destAddr0, sizeof(destAddr0[0]),
                         sizeof(destAddr0) / 2, sizeof(destAddr0), kEDMA_MemoryToMemory);
    transferConfig0.enabledInterruptMask       = 0U;
    transferConfig0.enableChannelMinorLoopLink = true;
    transferConfig0.enableChannelMajorLoopLink = true;
    transferConfig0.minorLoopLinkChannel       = DEMO_DMA_CHANNEL_1;
    transferConfig0.majorLoopLinkChannel       = DEMO_DMA_CHANNEL_2;

    EDMA_PrepareTransfer(&transferConfig1, srcAddr, sizeof(srcAddr[0]), destAddr1, sizeof(destAddr0[0]),
                         sizeof(uint32_t) * BUFFER_LENGTH, sizeof(destAddr1), kEDMA_MemoryToMemory);
    transferConfig1.enabledInterruptMask = 0U;
    EDMA_PrepareTransfer(&transferConfig2, srcAddr, sizeof(srcAddr[0]), destAddr2, sizeof(destAddr0[0]),
                         sizeof(uint32_t) * BUFFER_LENGTH, sizeof(destAddr2), kEDMA_MemoryToMemory);

    EDMA_SetTransferConfig(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, &transferConfig0, NULL);
    EDMA_SetTransferConfig(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_1, &transferConfig1, NULL);
    EDMA_SetTransferConfig(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_2, &transferConfig2, NULL);

    EnableIRQ(APP_DMA_IRQ);

    EDMA_TriggerChannelStart(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0);
    EDMA_TriggerChannelStart(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0);

    /* Wait for EDMA transfer finish */
    while (g_Transfer_Done != true)
    {
    }

    /* Print destination buffer */
    PRINTF("\r\n\r\nEDMA channel link example finish.\r\n\r\n");
    PRINTF("Destination Buffer 0:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr0[i]);
    }
    PRINTF("\r\n\r\nDestination Buffer 1:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr1[i]);
    }
    PRINTF("\r\n\r\nDestination Buffer 2:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr2[i]);
    }

    while (1)
    {
    }
}
