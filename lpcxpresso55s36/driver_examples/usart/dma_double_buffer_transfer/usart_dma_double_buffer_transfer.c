/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_usart.h"
#include "fsl_usart_dma.h"
#include "fsl_dma.h"
#include "fsl_debug_console.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_USART                USART0
#define DEMO_USART_CLK_SRC        kCLOCK_Flexcomm0
#define DEMO_USART_CLK_FREQ       CLOCK_GetFlexCommClkFreq(0U)
#define USART_RX_DMA_CHANNEL      4
#define USART_TX_DMA_CHANNEL      5
#define EXAMPLE_UART_DMA_BASEADDR DMA0


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*!< The DMA RX Handles. */
dma_handle_t g_DMA_Handle;
uint8_t g_data_buffer[16];

/*******************************************************************************
 * Code
 ******************************************************************************/
void DMA_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    int i;
    if (tcds == kDMA_IntB)
    {
        PRINTF("\n\r CallBack B \n\r");
        for (i = 0; i <= 7; i++)
        {
            PRINTF("%c", g_data_buffer[i]);
        }
        PRINTF(" ");
        for (i = 8; i <= 15; i++)
        {
            PRINTF("%c", g_data_buffer[i]);
        }
    }
    if (tcds == kDMA_IntA)
    {
        PRINTF("\n\r CallBack A \n\r");
        for (i = 0; i <= 7; i++)
        {
            PRINTF("%c", g_data_buffer[i]);
        }
        PRINTF(" ");
        for (i = 8; i <= 15; i++)
        {
            PRINTF("%c", g_data_buffer[i]);
        }
    }
}

/*! @brief Static table of descriptors */
#if defined(__ICCARM__)
#pragma data_alignment              = 16
dma_descriptor_t g_pingpong_desc[2] = {0};
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
__attribute__((aligned(16))) dma_descriptor_t g_pingpong_desc[2] = {0};
#elif defined(__GNUC__)
__attribute__((aligned(16))) dma_descriptor_t g_pingpong_desc[2] = {0};
#endif

/*!
 * @brief Main function
 */
int main(void)
{
    dma_transfer_config_t transferConfig;
    usart_config_t config;

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);

    BOARD_InitPins();
    BOARD_BootClockFRO12M();
    BOARD_InitDebugConsole();

    /* Initialize the USART. */
    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUSART_ParityDisabled;
     * config.stopBitCount = kUSART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 1;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;

    USART_Init(DEMO_USART, &config, DEMO_USART_CLK_FREQ);
    USART_EnableRxDMA(DEMO_USART, true);

    PRINTF("USART Receive Double Buffer Example. The USART will echo the double buffer after each 8 bytes :\n\r");

    /* Configure DMA. */
    DMA_Init(EXAMPLE_UART_DMA_BASEADDR);
    DMA_EnableChannel(EXAMPLE_UART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);
    DMA_CreateHandle(&g_DMA_Handle, EXAMPLE_UART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);
    DMA_SetCallback(&g_DMA_Handle, DMA_Callback, NULL);
    DMA_PrepareTransfer(&transferConfig, (void *)&DEMO_USART->FIFORD, g_data_buffer, 1, 8, kDMA_PeripheralToMemory,
                        &g_pingpong_desc[1]);
    DMA_SubmitTransfer(&g_DMA_Handle, &transferConfig);
    transferConfig.xfercfg.intA = false;
    transferConfig.xfercfg.intB = true;
    DMA_CreateDescriptor(&g_pingpong_desc[1], &transferConfig.xfercfg, (void *)&DEMO_USART->FIFORD, &g_data_buffer[8],
                         &g_pingpong_desc[0]);
    transferConfig.xfercfg.intA = true;
    transferConfig.xfercfg.intB = false;
    DMA_CreateDescriptor(&g_pingpong_desc[0], &transferConfig.xfercfg, (void *)&DEMO_USART->FIFORD, &g_data_buffer[0],
                         &g_pingpong_desc[1]);

    DMA_StartTransfer(&g_DMA_Handle);

    while (1)
    {
        __WFI();
    }
}
