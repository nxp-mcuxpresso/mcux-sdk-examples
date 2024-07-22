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

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_DMA_BASEADDR   DMA4
#define EXAMPLE_DMA_CLOCK_ROOT kCLOCK_Root_WakeupAxi
#define EXAMPLE_DMA_CLOCK_GATE kCLOCK_Edma2
#define DEMO_DMA_CHANNEL_0     0U
#define APP_DMA_IRQ            DMA4_0_1_IRQn
#define APP_DMA_IRQ_HANDLER    DMA4_0_1_IRQHandler
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

    /* clang-format off */
    /* 250MHz DMA clock */
    const clock_root_config_t dmaClkCfg = {
        .clockOff = false,
	.mux = kCLOCK_WAKEUPAXI_ClockRoot_MuxSysPll1Pfd0, // 1000MHz
	.div = 4
    };
    /* clang-format on */

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(EXAMPLE_DMA_CLOCK_ROOT, &dmaClkCfg);
    CLOCK_EnableClock(EXAMPLE_DMA_CLOCK_GATE);
    /* Print destination buffer */
    PRINTF("EDMA memory to memory example begin.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }

    /* Configure EDMA channel for one shot transfer */
    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(EXAMPLE_DMA_BASEADDR, &userConfig);
    EDMA_PrepareTransfer(&transferConfig, srcAddr, sizeof(srcAddr[0]), destAddr, sizeof(destAddr[0]), sizeof(destAddr),
                         sizeof(destAddr), kEDMA_MemoryToMemory);
    EDMA_SetTransferConfig(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0, &transferConfig, NULL);
    EnableIRQ(APP_DMA_IRQ);
    EDMA_TriggerChannelStart(EXAMPLE_DMA_BASEADDR, DEMO_DMA_CHANNEL_0);

    /* Wait for EDMA transfer finish */
    while (g_Transfer_Done != true)
    {
    }
    /* Print destination buffer */
    PRINTF("\r\n\r\nEDMA memory to memory example finish.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }
    while (1)
    {
    }
}
