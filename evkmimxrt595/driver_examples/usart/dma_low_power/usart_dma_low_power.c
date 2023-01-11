/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_usart.h"
#include "fsl_usart_dma.h"
#include "fsl_dma.h"
#include "fsl_power.h"

#include "fsl_inputmux.h"
#include "fsl_rtc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_USART          USART0
#define EXAMPLE_USART_CLK_FREQ CLK_RTC_32K_CLK
#define USART_RX_DMA_CHANNEL   0
#define USART_TX_DMA_CHANNEL   1

#define EXAMPLE_UART_DMA_BASEADDR DMA0
#define EXAMPLE_UART_DMA_IRQn     DMA0_IRQn

#define EXAMPLE_UART_BAUDRATE (9600U)

/*!< Power down all unnecessary blocks and enable RBB during deep sleep. */
#define EXAMPLE_DEEPSLEEP_RUNCFG0 (SYSCTL0_PDRUNCFG0_RBBSRAM_PD_MASK | SYSCTL0_PDRUNCFG0_RBB_PD_MASK)
#define EXAMPLE_DEEPSLEEP_RAM_APD 0xFFC00000U /* 0x280000 - 0x4FFFFF keep powered */
#define EXAMPLE_DEEPSLEEP_RAM_PPD 0x0U

#define EXAMPLE_EXCLUDE_FROM_DEEPSLEEP                                                                       \
    (((const uint32_t[]){EXAMPLE_DEEPSLEEP_RUNCFG0,                                                          \
                         (SYSCTL0_PDSLEEPCFG1_FLEXSPI0_SRAM_APD_MASK | SYSCTL0_PDSLEEPCFG1_SRAM_SLEEP_MASK), \
                         EXAMPLE_DEEPSLEEP_RAM_APD, EXAMPLE_DEEPSLEEP_RAM_PPD}))
#define ECHO_BUFFER_LENGTH 8

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void EXAMPLE_EnterDeepSleep(void);
void EXAMPLE_EnableDmaWakeup(void);

/* UART user callback */
void UART_UserCallback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/

usart_dma_handle_t g_uartDmaHandle;
dma_handle_t g_uartTxDmaHandle;
dma_handle_t g_uartRxDmaHandle;
const uint8_t g_tipString[] =
    "USART DMA low power example\r\nSend back received data\r\nEcho every 8 characters (1st char \'q\' leads to "
    "end).";
const uint8_t g_tipInput[] = "\r\n------------------\r\nInput any key to start.\r\n";
const uint8_t g_tipStart[] = "Now enter deep sleep and wait for 8 characters.\r\n";
uint8_t g_buffer[ECHO_BUFFER_LENGTH];
volatile bool txOnGoing, rxOnGoing;

/*******************************************************************************
 * Code
 ******************************************************************************/
void EXAMPLE_EnterDeepSleep(void)
{
    /* Enable deep sleep IRQ. */
    EnableDeepSleepIRQ(EXAMPLE_UART_DMA_IRQn);
    /* Enter deep sleep mode by using power API. */
    POWER_EnterDeepSleep(EXAMPLE_EXCLUDE_FROM_DEEPSLEEP);
}

void EXAMPLE_EnableDmaWakeup(void)
{
    /* Enable DMA wake up by RX FIFO. */
    SYSCTL0->HWWAKE |= SYSCTL0_HWWAKE_FCWAKE_MASK;
    EXAMPLE_USART->FIFOCFG |= USART_FIFOCFG_WAKERX_MASK;
}

static void USART_Print(const uint8_t *str, uint32_t size)
{
    usart_transfer_t xfer;

    xfer.data     = (uint8_t *)str;
    xfer.dataSize = size;
    txOnGoing     = true;
    USART_TransferSendDMA(EXAMPLE_USART, &g_uartDmaHandle, &xfer);

    /* Wait send finished */
    while (txOnGoing)
    {
    }
}

/* UART user callback */
void USART_UserCallback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_USART_TxIdle == status)
    {
        txOnGoing = false;
    }

    if (kStatus_USART_RxIdle == status)
    {
        rxOnGoing = false;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    usart_config_t config;
    uint8_t ch;
    bool success = true;
    usart_transfer_t sendXfer;
    usart_transfer_t receiveXfer;
    uint32_t irqMask;

    BOARD_InitPins();
    BOARD_BootClockRUN();

    /* Enable OSC32K clock for FLEXCOMM0 */
    CLOCK_EnableOsc32K(true);
    RTC_Init(RTC);

    /* Configure DMAMUX. */
    RESET_PeripheralReset(kINPUTMUX_RST_SHIFT_RSTn);

    INPUTMUX_Init(INPUTMUX);
    /* Enable DMA request */
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Flexcomm0RxToDmac0Ch0RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Flexcomm0TxToDmac0Ch1RequestEna, true);
    /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);

    /* Initialize the UART. */
    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUART_ParityDisabled;
     * config.stopBitCount = kUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 1;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = EXAMPLE_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;

    USART_Init(EXAMPLE_USART, &config, EXAMPLE_USART_CLK_FREQ);
    EXAMPLE_USART->CFG |= USART_CFG_MODE32K_MASK;

    /* Configure DMA. */
    DMA_Init(EXAMPLE_UART_DMA_BASEADDR);
    DMA_EnableChannel(EXAMPLE_UART_DMA_BASEADDR, USART_TX_DMA_CHANNEL);
    DMA_EnableChannel(EXAMPLE_UART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);

    DMA_CreateHandle(&g_uartTxDmaHandle, EXAMPLE_UART_DMA_BASEADDR, USART_TX_DMA_CHANNEL);
    DMA_CreateHandle(&g_uartRxDmaHandle, EXAMPLE_UART_DMA_BASEADDR, USART_RX_DMA_CHANNEL);

    /* Create UART DMA handle. */
    USART_TransferCreateHandleDMA(EXAMPLE_USART, &g_uartDmaHandle, USART_UserCallback, NULL, &g_uartTxDmaHandle,
                                  &g_uartRxDmaHandle);

    /* Send g_tipString out. */
    USART_Print(g_tipString, sizeof(g_tipString) - 1);

    /* Start to echo. */
    sendXfer.data        = g_buffer;
    sendXfer.dataSize    = sizeof(g_buffer);
    receiveXfer.data     = g_buffer;
    receiveXfer.dataSize = sizeof(g_buffer);

    EXAMPLE_EnableDmaWakeup();

    while (1)
    {
        USART_Print(g_tipInput, sizeof(g_tipInput) - 1);
        if (USART_ReadBlocking(EXAMPLE_USART, &ch, 1) == kStatus_Success)
        {
            USART_Print(g_tipStart, sizeof(g_tipStart) - 1);
        }
        else
        {
            success = false;
            break;
        }
        rxOnGoing = true;
        /* First receive 8 characters. */
        USART_TransferReceiveDMA(EXAMPLE_USART, &g_uartDmaHandle, &receiveXfer);

        irqMask = DisableGlobalIRQ();
        if (rxOnGoing)
        {
            /* Enter deep sleep only when the DMA interrupt not occurs. */
            EXAMPLE_EnterDeepSleep();
        }
        EnableGlobalIRQ(irqMask);
        __ISB();
        assert(rxOnGoing == false);

        /* Now echo the characters */
        txOnGoing = true;
        USART_TransferSendDMA(EXAMPLE_USART, &g_uartDmaHandle, &sendXfer);
        /* Wait send finished */
        while (txOnGoing)
        {
        }

        if (g_buffer[0] == 'q')
        {
            break;
        }
    }

    strncpy((char *)g_buffer, success ? "\r\nDone!" : "\r\nFail!", sizeof(g_buffer));
    USART_TransferSendDMA(EXAMPLE_USART, &g_uartDmaHandle, &sendXfer);
    while (1)
    {
    }
}
