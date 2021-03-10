/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_usart.h"
#include "fsl_usart_dma.h"
#include "fsl_dma.h"
#include "fsl_adapter_timer.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_USART             USART0
#define EXAMPLE_USART_CLK_SRC     kCLOCK_Flexcomm0
#define EXAMPLE_USART_CLK_FREQ    CLOCK_GetFlexCommClkFreq(0)
#define USART_RX_DMA_CHANNEL      0
#define USART_TX_DMA_CHANNEL      1
#define EXAMPLE_UART_DMA_BASEADDR DMA0

#define EXAMPLE_TIMEOUT_PERIOD_US    (10U)
#define EXAMPLE_TIMEOUT_PERIOD_COUNT (1000 * EXAMPLE_TIMEOUT_PERIOD_US)
#define EXAMPLE_TIMER_CLK_FREQ       CLOCK_GetFreq(kCLOCK_BusClk)
#define EXAMPLE_TIMER_INSTANCE       (0U)
#define EXAMPLE_RING_BUFFER_SIZE 32

typedef struct _hal_timer_handle_struct_t
{
    uint32_t timeout;
    uint32_t timerClock_Hz;
    hal_timer_callback_t callback;
    void *callbackParam;
    uint8_t instance;
} hal_timer_handle_struct_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Start ring buffer with DMA used. */
static void EXAMPLE_StartRingBufferDMA(void);

/* Get how many characters stored in ring buffer. */
static uint32_t EXAMPLE_GetRingBufferLengthDMA(void);

/* Read out the characters from ring buffer. */
static void EXAMPLE_ReadRingBufferDMA(uint8_t *ringBuffer, uint8_t *receiveBuffer, uint32_t length);

/* Initialzie the USART module. */
static void EXAMPLE_InitUSART(void);

/* Initialize the DMA configuration. */
static void EXAMPLE_InitDMA(void);

/* Initialize a hardware timer. */
static void EXAMPLE_InitTimer(void);

/* UART user callback */
void USART_UserCallback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData);

/* TIMER user callback. */
void TIMER_UserCallback(void *param);

/*******************************************************************************
 * Variables
 ******************************************************************************/
usart_dma_handle_t g_usartDmaHandle;
dma_handle_t g_usartTxDmaHandle;
dma_handle_t g_usartRxDmaHandle;
uint8_t g_tipString[] = "USART DMA ring buffer example\r\nBoard will send back received data\r\n";
uint8_t g_ringBuffer[EXAMPLE_RING_BUFFER_SIZE] = {0};
uint8_t g_rxBuffer[EXAMPLE_RING_BUFFER_SIZE]   = {0};
volatile bool txOnGoing                        = false;
volatile bool timeoutFlag                      = false;
volatile uint32_t ringBufferIndex              = 0U;
hal_timer_handle_struct_t g_timerHandle;

/* Allocate descriptor memory pointer with ring buffer used. */
AT_NONCACHEABLE_SECTION_ALIGN(static dma_descriptor_t g_descriptorPtr[1], 16);

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Start ring buffer with DMA used. */
static void EXAMPLE_StartRingBufferDMA(void)
{
    dma_transfer_config_t xferConfig;

    /* Prepare transfer and link the descriptor. */
    DMA_PrepareTransfer(&xferConfig, ((void *)((uint32_t)&EXAMPLE_USART->FIFORD)), g_ringBuffer, sizeof(uint8_t),
                        EXAMPLE_RING_BUFFER_SIZE, kDMA_PeripheralToMemory, &g_descriptorPtr[0]);

    /* Submit transfer. */
    (void)DMA_SubmitTransfer(&g_usartRxDmaHandle, &xferConfig);

    /* Config the descriptor and link the descriptor to itself for continuous transfer. */
    xferConfig.xfercfg.intA = true;
    xferConfig.xfercfg.intB = false;
    DMA_CreateDescriptor(&g_descriptorPtr[0], &xferConfig.xfercfg, (void *)&EXAMPLE_USART->FIFORD, g_ringBuffer,
                         &g_descriptorPtr[0]);

    /* Start transfer. */
    DMA_StartTransfer(&g_usartRxDmaHandle);

    /* Enable DMA request from rxFIFO */
    USART_EnableRxDMA(EXAMPLE_USART, true);
}

/* Get how many characters stored in ring buffer. */
static uint32_t EXAMPLE_GetRingBufferLengthDMA(void)
{
    uint32_t regPrimask     = 0U;
    uint32_t RemainingBytes = 0U;
    uint32_t receivedBytes  = 0U;

    /* Disable IRQ, protect ring buffer. */
    regPrimask     = DisableGlobalIRQ();
    RemainingBytes = DMA_GetRemainingBytes(EXAMPLE_UART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);

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

/* Read out the characters from ring buffer. */
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

/* Initialzie the USART module. */
static void EXAMPLE_InitUSART(void)
{
    usart_config_t config;

    /* Initialize the UART.
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUART_ParityDisabled;
     * config.stopBitCount = kUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 1;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;

    USART_Init(EXAMPLE_USART, &config, EXAMPLE_USART_CLK_FREQ);
}

/* Initialize the DMA configuration. */
static void EXAMPLE_InitDMA(void)
{
    /* Configure DMA. */
    DMA_Init(EXAMPLE_UART_DMA_BASEADDR);
    DMA_EnableChannel(EXAMPLE_UART_DMA_BASEADDR, USART_TX_DMA_CHANNEL);
    DMA_EnableChannel(EXAMPLE_UART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);

    DMA_CreateHandle(&g_usartTxDmaHandle, EXAMPLE_UART_DMA_BASEADDR, USART_TX_DMA_CHANNEL);
    DMA_CreateHandle(&g_usartRxDmaHandle, EXAMPLE_UART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);

    /* Create UART DMA handle. */
    USART_TransferCreateHandleDMA(EXAMPLE_USART, &g_usartDmaHandle, USART_UserCallback, NULL, &g_usartTxDmaHandle,
                                  NULL);
}

/* Initialize a hardware timer. */
static void EXAMPLE_InitTimer(void)
{
    hal_timer_config_t timerConfig;

    timeoutFlag             = false;
    timerConfig.timeout     = EXAMPLE_TIMEOUT_PERIOD_COUNT;
    timerConfig.srcClock_Hz = EXAMPLE_TIMER_CLK_FREQ;
    timerConfig.instance    = EXAMPLE_TIMER_INSTANCE;

    (void)memset(&g_timerHandle, 0, sizeof(g_timerHandle));

    /* Initialize the timer. */
    HAL_TimerInit(&g_timerHandle, &timerConfig);
    /* Install call back function. */
    HAL_TimerInstallCallback(&g_timerHandle, TIMER_UserCallback, NULL);
}

/* UART user callback */
void USART_UserCallback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_USART_TxIdle == status)
    {
        txOnGoing = false;
    }
}

/* Timer call back. */
void TIMER_UserCallback(void *param)
{
    timeoutFlag = true;
}

/*!
 * @brief Main function
 */
int main(void)
{
    usart_transfer_t sendXfer;
    size_t byteCount = 0U;

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);

    /* reset FLEXCOMM for USART */
    RESET_PeripheralReset(kFC0_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    /* Initialzie the USART module. */
    EXAMPLE_InitUSART();

    /* Initialzie the DMA configuration. */
    EXAMPLE_InitDMA();

    /* Initialize the hardware timer. */
    EXAMPLE_InitTimer();

    /* Send g_tipString out. */
    sendXfer.data     = g_tipString;
    sendXfer.dataSize = sizeof(g_tipString) - 1;
    txOnGoing         = true;
    USART_TransferSendDMA(EXAMPLE_USART, &g_usartDmaHandle, &sendXfer);

    /* Wait send finished */
    while (txOnGoing)
    {
    }

    /* Start ring buffer with DMA used. */
    EXAMPLE_StartRingBufferDMA();

    /* Start timer. */
    HAL_TimerEnable(&g_timerHandle);

    while (1)
    {
        byteCount = 0U;

        /* Wait for timer timeout occurred. Timeout period is defined by EXAMPLE_TIMEOUT_PERIOD_MS*/
        while (!timeoutFlag)
        {
        }

        timeoutFlag = false;
        /* Get the received bytes number stored in DMA ring buffer. */
        byteCount = EXAMPLE_GetRingBufferLengthDMA();

        if (0U != byteCount)
        {
            /* Move the data from ring buffer to given buffer section. */
            EXAMPLE_ReadRingBufferDMA(g_ringBuffer, g_rxBuffer, byteCount);

            /* Wait for sending finished */
            while (txOnGoing)
            {
            }

            /* Start to echo. */
            txOnGoing         = true;
            sendXfer.data     = g_rxBuffer;
            sendXfer.dataSize = byteCount;
            USART_TransferSendDMA(EXAMPLE_USART, &g_usartDmaHandle, &sendXfer);
        }
    }
}
