/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_dac.h"
#include "fsl_dma.h"

#include "fsl_power.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DAC_BASE          DAC0
#define DEMO_DAC_COUNTER_VALUE 1000U
#define DEMO_DMA_DAC_CHANNEL   22U
#define DEMO_DAC_DATA_REG_ADDR 0x40014000

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void DMA_Configfuation(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
dma_handle_t gDmaHandleStruct; /* Handler structure for using DMA. */
const uint32_t g_waveform[] = {0x0000, 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888, 0x9999, 0xAAAA,
                               0xBBBB, 0xCCCC, 0xDDDD, 0xEEEE, 0xFFFF, 0xFFFF, 0xEEEE, 0xDDDD, 0xCCCC, 0xBBBB, 0xAAAA,
                               0x9999, 0x8888, 0x7777, 0x6666, 0x5555, 0x4444, 0x3333, 0x2222, 0x1111, 0x0000};
/*******************************************************************************
 * Code
 ******************************************************************************/
/* Software ISR for DMA transfer done. */
void DEMO_DMA_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    DMA_StartTransfer(&gDmaHandleStruct); /* Enable the DMA every time for each transfer. */
}

/*!
 * @brief Main function
 */
int main(void)
{
    dac_config_t dacConfigStruct;

    /* Attach 12 MHz clock to USART0 (debug console) */
    CLOCK_Select(BOARD_DEBUG_USART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_BootClockFRO30M();
    BOARD_InitDebugConsole();

    /* Power on the DAC0.*/
    POWER_DisablePD(kPDRUNCFG_PD_DAC0);

    NVIC_EnableIRQ(DMA0_IRQn);
    DMA_Configfuation();

    PRINTF("\r\nDAC dma Example.\r\n");

    /* Configure the DAC. */
    DAC_GetDefaultConfig(&dacConfigStruct);
    DAC_Init(DEMO_DAC_BASE, &dacConfigStruct);
    /* Enable the DAC DMA. */
    DAC_EnableDMA(DEMO_DAC_BASE, true);
    /* Configure the frequency of DAC DMA. */
    DAC_SetCounterValue(DEMO_DAC_BASE, DEMO_DAC_COUNTER_VALUE);
    DAC_EnableDoubleBuffering(DEMO_DAC_BASE, true);

    PRINTF("Please probe the signal using an oscilloscope.\r\n");

    while (1)
    {
    }
}

static void DMA_Configfuation(void)
{
    dma_transfer_config_t dmaTransferConfigStruct;

    /* Configure DMA. */
    DMA_Init(DMA0);
    DMA_EnableChannel(DMA0, DEMO_DMA_DAC_CHANNEL);
    DMA_CreateHandle(&gDmaHandleStruct, DMA0, DEMO_DMA_DAC_CHANNEL);
    DMA_SetCallback(&gDmaHandleStruct, DEMO_DMA_Callback, NULL);

    /* Prepare and submit the transfer. */
    DMA_PrepareTransfer(&dmaTransferConfigStruct,       /* To keep the configuration. */
                        (void *)g_waveform,             /* DMA transfer source address. */
                        (void *)DEMO_DAC_DATA_REG_ADDR, /* DMA transfer destination address. */
                        sizeof(uint32_t),               /* DMA transfer destination address width(bytes). */
                        sizeof(g_waveform),             /* DMA transfer bytes to be transferred. */
                        kDMA_MemoryToPeripheral,        /* DMA transfer type. */
                        NULL                            /* nextDesc Chain custom descriptor to transfer. */
    );
    DMA_SubmitTransfer(&gDmaHandleStruct, &dmaTransferConfigStruct);
    DMA_StartTransfer(&gDmaHandleStruct);
}
