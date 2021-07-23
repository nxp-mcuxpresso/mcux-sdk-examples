/*
 * Copyright 2019 NXP
 * All rights reserved.
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

#define EXAMPLE_TIMEOUT_PERIOD_MS (10U)
#define EXAMPLE_RING_BUFFER_SIZE 32

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Start ring buffer with DMA used. */
static void EXAMPLE_StartRingBufferDMA(void);

/* Get how many characters stored in ring buffer. */
static uint32_t EXAMPLE_GetRingBufferLengthDMA(void);

/* Reading the cahracters from ring buffer. */
static void EXAMPLE_ReadRingBufferDMA(uint8_t *ringBuffer, uint8_t *receiveBuffer, uint32_t length);

/* Initialize the USART module. */
static void EXAMPLE_USARTInit(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
dma_handle_t g_uartRxDmaHandle;
uint8_t g_ringBuffer[EXAMPLE_RING_BUFFER_SIZE] = {0};
uint8_t g_rxBuffer[EXAMPLE_RING_BUFFER_SIZE]   = {0};
volatile bool txOnGoing                        = false;
volatile uint32_t ringBufferIndex              = 0U;
volatile uint32_t receivedBytes                = 0U;
volatile uint32_t g_systickCounter             = 0U;

/* Allocate descriptor memory pointer with ring buffer used. */
AT_NONCACHEABLE_SECTION_ALIGN(static dma_descriptor_t g_descriptorPtr[1], 16);
/*******************************************************************************
 * Code
 ******************************************************************************/
void SysTick_Handler(void)
{
    if (g_systickCounter != 0U)
    {
        g_systickCounter--;
    }
}

void SysTick_DelayTicks(uint32_t n)
{
    g_systickCounter = n;
    while (g_systickCounter != 0U)
    {
    }
}

/* Start ring buffer with DMA used. */
static void EXAMPLE_StartRingBufferDMA(void)
{
    dma_transfer_config_t xferConfig;

    /* Configure DMA. */
    DMA_Init(EXAMPLE_USART_DMA_BASEADDR);

    /* Create handle and enable associated channel. */
    DMA_EnableChannel(EXAMPLE_USART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);
    DMA_CreateHandle(&g_uartRxDmaHandle, EXAMPLE_USART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);

    /* Prepare transfer and link the descriptor. */
    DMA_PrepareTransfer(&xferConfig, ((void *)((uint32_t)&EXAMPLE_USART->RXDAT)), g_ringBuffer, sizeof(uint8_t),
                        EXAMPLE_RING_BUFFER_SIZE, kDMA_PeripheralToMemory, &g_descriptorPtr[0]);

    /* Submit transfer. */
    (void)DMA_SubmitTransfer(&g_uartRxDmaHandle, &xferConfig);

    /* Config the descriptor and link the descriptor to itself for continuous transfer. */
    xferConfig.xfercfg.intA = true;
    xferConfig.xfercfg.intB = false;
    DMA_CreateDescriptor(&g_descriptorPtr[0], &xferConfig.xfercfg, (void *)&EXAMPLE_USART->RXDAT, g_ringBuffer,
                         &g_descriptorPtr[0]);

    /* Start transfer. */
    DMA_StartTransfer(&g_uartRxDmaHandle);
}

/* Get how many characters stored in ring buffer. */
static uint32_t EXAMPLE_GetRingBufferLengthDMA(void)
{
    uint32_t regPrimask     = 0U;
    uint32_t RemainingBytes = 0U;

    /* Disable IRQ, protect ring buffer. */
    regPrimask     = DisableGlobalIRQ();
    RemainingBytes = DMA_GetRemainingBytes(EXAMPLE_USART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);

    /* When all transfers are completed, XFERCOUNT value changes from 0 to 0x3FF. The last value
     * 0x3FF does not mean there are 1024 transfers left to complete.It means all data transfer
     * has completed.
     */
    if (RemainingBytes == 1024)
    {
        RemainingBytes = 0U;
    }

    receivedBytes = EXAMPLE_RING_BUFFER_SIZE - RemainingBytes;

    /* If the received bytes is less than index value, it means the ring buffer has reached it boundary. */
    if (receivedBytes < ringBufferIndex)
    {
        receivedBytes += EXAMPLE_RING_BUFFER_SIZE;
    }

    receivedBytes -= ringBufferIndex;

    /* Enable IRQ if previously enabled. */
    EnableGlobalIRQ(regPrimask);

    return receivedBytes;
}

/* Reading the cahracters from ring buffer. */
static void EXAMPLE_ReadRingBufferDMA(uint8_t *ringBuffer, uint8_t *receiveBuffer, uint32_t length)
{
    assert(ringBuffer);
    assert(receiveBuffer);
    assert(length);

    uint32_t index = length;

    /* If length if larger than ring buffer size, it means overflow occurred, need to reset the ringBufferIndex. */
    if (length > EXAMPLE_RING_BUFFER_SIZE)
    {
        ringBufferIndex = ((ringBufferIndex + length) % EXAMPLE_RING_BUFFER_SIZE);
        index           = EXAMPLE_RING_BUFFER_SIZE;
    }

    while (index)
    {
        *(receiveBuffer++) = ringBuffer[ringBufferIndex++];
        if (ringBufferIndex == EXAMPLE_RING_BUFFER_SIZE)
        {
            ringBufferIndex = 0U;
        }
        index--;
    }
}

/* Initialize the USART module. */
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

/*!
 * @brief Main function
 */
int main(void)
{
    size_t byteCount = 0U;
    uint32_t i       = 0U;

    /* Initialize the pins. */
    BOARD_InitBootPins();

    /* Enable clock to 30MHz. */
    BOARD_BootClockFRO30M();

    /* Select the main clock as source clock of USART0. */
    CLOCK_Select(kUART0_Clk_From_MainClk);

    /* Intialize the debug console. */
    BOARD_InitDebugConsole();

    /* Initialize the USART module. */
    EXAMPLE_USARTInit();

    PRINTF("USART DMA ring buffer example\r\nBoard will send back received data\r\n");

    /* Start DMA ring buffer. */
    EXAMPLE_StartRingBufferDMA();

    /* Set systick reload value to generate 1ms interrupt */
    SysTick_Config(SystemCoreClock / 1000U);

    while (1)
    {
        byteCount = 0U;

        /* Delay function. */
        SysTick_DelayTicks(EXAMPLE_TIMEOUT_PERIOD_MS);

        /* Get the received bytes number stored in DMA ring buffer. */
        byteCount = EXAMPLE_GetRingBufferLengthDMA();

        if (0U != byteCount)
        {
            /* Move the data from ring buffer to given buffer section. */
            EXAMPLE_ReadRingBufferDMA(g_ringBuffer, g_rxBuffer, byteCount);

            for (i = 0U; i < byteCount; i++)
            {
                PRINTF("%c", g_rxBuffer[i]);
            }
        }
    }
}
