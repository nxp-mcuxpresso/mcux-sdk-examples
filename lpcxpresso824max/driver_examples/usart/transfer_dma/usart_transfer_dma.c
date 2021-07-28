/*
 * Copyright  2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_usart.h"
#include "fsl_dma.h"
#include "fsl_debug_console.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_USART              USART0
#define EXAMPLE_USART_CLK_SRC      kCLOCK_MainClk
#define EXAMPLE_USART_CLK_FREQ     CLOCK_GetFreq(EXAMPLE_USART_CLK_SRC)
#define EXAMPLE_USART_DMA_BASEADDR DMA0
#define USART_RX_DMA_CHANNEL       0
#define ECHO_BUFFER_LENGTH 8

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void EXAMPLE_UserCallback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds);
static void EXAMPLE_USARTInit(void);
static void EXAMPLE_DMAConfiguration(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
dma_handle_t g_DMA_Handle; /*!< The DMA RX Handles. */
uint8_t g_data_buffer[16];

/*******************************************************************************
 * Code
 ******************************************************************************/

static void EXAMPLE_UserCallback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    int i;
    if (tcds == kDMA_IntB)
    {
        PRINTF("\n\rCallBack B is triggered!\n\r");
        PRINTF("The received data is: \n\r");
        for (i = 8; i <= 15; i++)
        {
            PRINTF("%c", g_data_buffer[i]);
        }
    }
    if (tcds == kDMA_IntA)
    {
        PRINTF("\n\rCallBack A is triggered!\n\r");
        PRINTF("The received data is: \n\r");
        for (i = 0; i <= 7; i++)
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
    /* Enable clock of uart0. */
    CLOCK_EnableClock(kCLOCK_Uart0);
    /* Ser DIV of uart0. */
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Initialize the USART instance. */
    EXAMPLE_USARTInit();

    /* Configuration the DMA for receive data. */
    EXAMPLE_DMAConfiguration();

    PRINTF("USART DMA transfer Example.\r\nThe USART will echo the double buffer after each 8 bytes:\r\n");

    /* Start DMA for transfer. */
    DMA_StartTransfer(&g_DMA_Handle);

    while (1)
    {
        __WFI();
    }
}

static void EXAMPLE_USARTInit(void)
{
    usart_config_t config;

    /* Default config by using USART_GetDefaultConfig():
     * config.baudRate_Bps = 9600U;
     * config.parityMode = kUSART_ParityDisabled;
     * config.stopBitCount = kUSART_OneStopBit;
     * config.bitCountPerChar = kUSART_8BitsPerChar;
     * config.loopback = false;
     * config.enableRx = false;
     * config.enableTx = false;
     * config.syncMode = kUSART_SyncModeDisabled;
     */
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_USART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;

    USART_Init(EXAMPLE_USART, &config, EXAMPLE_USART_CLK_FREQ);
}

static void EXAMPLE_DMAConfiguration(void)
{
    dma_transfer_config_t transferConfig;

    /* Configure DMA. */
    DMA_Init(EXAMPLE_USART_DMA_BASEADDR);

    DMA_EnableChannel(EXAMPLE_USART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);
    DMA_CreateHandle(&g_DMA_Handle, EXAMPLE_USART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);
    DMA_SetCallback(&g_DMA_Handle, EXAMPLE_UserCallback, NULL);

    DMA_PrepareTransfer(&transferConfig, (void *)&EXAMPLE_USART->RXDAT, g_data_buffer, 1, 8, kDMA_PeripheralToMemory,
                        &g_pingpong_desc[1]);
    DMA_SubmitTransfer(&g_DMA_Handle, &transferConfig);

    transferConfig.xfercfg.intA = false;
    transferConfig.xfercfg.intB = true;
    DMA_CreateDescriptor(&g_pingpong_desc[1], &transferConfig.xfercfg, (void *)&EXAMPLE_USART->RXDAT, &g_data_buffer[8],
                         &g_pingpong_desc[0]);

    transferConfig.xfercfg.intA = true;
    transferConfig.xfercfg.intB = false;
    DMA_CreateDescriptor(&g_pingpong_desc[0], &transferConfig.xfercfg, (void *)&EXAMPLE_USART->RXDAT, &g_data_buffer[0],
                         &g_pingpong_desc[1]);
}
