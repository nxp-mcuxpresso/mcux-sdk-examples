/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_dma.h"
#include "fsl_inputmux.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DMA_CHANNEL1                1
#define DEMO_DMA_CHANNEL0                0
#define DEMO_DMA_CHANNEL2                2
#define DEMO_DMA_CHANNEL_TRIGGER_INPUT   kINPUTMUX_OtrigAToDma0
#define DEMO_DMA_CHANNEL_TRIGGER_OUTPUT0 kINPUTMUX_Dma0FlexSpiRxTrigoutToTriginChannels
#define BUFF_LENGTH        4U
#define DMA_DESCRIPTOR_NUM 3U
#define DEST_BUFFER_LENGTH 16U
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static dma_handle_t s_DMA_Handle0;
static dma_handle_t s_DMA_Handle1;
static dma_handle_t s_DMA_Handle2;
static volatile bool s_Transfer0_Done = false;
static volatile bool s_Transfer1_Done = false;
static volatile bool s_Transfer2_Done = false;

DMA_ALLOCATE_LINK_DESCRIPTORS(s_dma_table1, 1U);
DMA_ALLOCATE_DATA_TRANSFER_BUFFER(static uint32_t s_srcBuffer0[BUFF_LENGTH], sizeof(uint32_t)) = {1, 2, 3, 4};
DMA_ALLOCATE_DATA_TRANSFER_BUFFER(static uint32_t s_srcBuffer1[BUFF_LENGTH], sizeof(uint32_t)) = {11, 22, 33, 44};
DMA_ALLOCATE_DATA_TRANSFER_BUFFER(static uint32_t s_srcBuffer2[BUFF_LENGTH], sizeof(uint32_t)) = {111, 222, 333, 444};
DMA_ALLOCATE_DATA_TRANSFER_BUFFER(static uint32_t s_srcBuffer3[BUFF_LENGTH], sizeof(uint32_t)) = {1111, 2222, 3333,
                                                                                                  4444};
DMA_ALLOCATE_DATA_TRANSFER_BUFFER(static uint32_t s_destBuffer[BUFF_LENGTH * 4], sizeof(uint32_t)) = {0x00};
static dma_channel_trigger_t s_channelTrigger                                                      = {
    .type  = kDMA_FallingEdgeTrigger,
    .burst = kDMA_SingleTransfer,
    .wrap  = kDMA_NoWrap,
};
/*******************************************************************************
 * Code
 ******************************************************************************/

/* User callback function for DMA transfer. */
void DMA0_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        s_Transfer0_Done = true;
    }
}

/* User callback function for DMA transfer. */
void DMA1_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        s_Transfer1_Done = true;
    }
}

/* User callback function for DMA transfer. */
void DMA2_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        s_Transfer2_Done = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0;

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    /* Print source buffer */
    PRINTF("DMA channel chain example begin.\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < DEST_BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", s_destBuffer[i]);
    }

    DMA_Init(DMA0);

    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, DEMO_DMA_CHANNEL1, DEMO_DMA_CHANNEL_TRIGGER_INPUT);
    INPUTMUX_AttachSignal(INPUTMUX, DEMO_DMA_CHANNEL2, DEMO_DMA_CHANNEL_TRIGGER_INPUT);
    INPUTMUX_AttachSignal(INPUTMUX, 0, DEMO_DMA_CHANNEL_TRIGGER_OUTPUT0);
    INPUTMUX_AttachSignal(INPUTMUX, DEMO_DMA_CHANNEL0, DEMO_DMA_CHANNEL_TRIGGER_INPUT);

    /* configuration for channel0 */
    DMA_CreateHandle(&s_DMA_Handle0, DMA0, DEMO_DMA_CHANNEL0);
    DMA_EnableChannel(DMA0, DEMO_DMA_CHANNEL0);
    DMA_SetCallback(&s_DMA_Handle0, DMA0_Callback, NULL);
    /* configuration for channel1 */
    DMA_CreateHandle(&s_DMA_Handle1, DMA0, DEMO_DMA_CHANNEL1);
    DMA_EnableChannel(DMA0, DEMO_DMA_CHANNEL1);
    DMA_SetCallback(&s_DMA_Handle1, DMA1_Callback, NULL);
    /* configuration for channel2 */
    DMA_CreateHandle(&s_DMA_Handle2, DMA0, DEMO_DMA_CHANNEL2);
    DMA_EnableChannel(DMA0, DEMO_DMA_CHANNEL2);
    DMA_SetCallback(&s_DMA_Handle2, DMA2_Callback, NULL);

    /* DMA channel0 trigger configurations */
    DMA_SetChannelConfig(DMA0, DEMO_DMA_CHANNEL0, &s_channelTrigger, false);
    DMA_SetChannelConfig(DMA0, DEMO_DMA_CHANNEL1, &s_channelTrigger, false);
    DMA_SetChannelConfig(DMA0, DEMO_DMA_CHANNEL2, &s_channelTrigger, false);

    DMA_SetupDescriptor(&(s_dma_table1[0]),
                        DMA_CHANNEL_XFER(false, false, false, true, 4U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave1xWidth, 16U),
                        s_srcBuffer3, &s_destBuffer[12], NULL);
    /* submit channel0 transfer parameter */
    DMA_SubmitChannelTransferParameter(&s_DMA_Handle0,
                                       DMA_CHANNEL_XFER(true, true, true, false, 4U, kDMA_AddressInterleave1xWidth,
                                                        kDMA_AddressInterleave1xWidth, 16U),
                                       s_srcBuffer0, &s_destBuffer[0], &(s_dma_table1[0]));

    /* submit channel1 transfer parameter */
    DMA_SubmitChannelTransferParameter(&s_DMA_Handle1,
                                       DMA_CHANNEL_XFER(false, false, true, false, 4, kDMA_AddressInterleave1xWidth,
                                                        kDMA_AddressInterleave1xWidth, 16U),
                                       s_srcBuffer1, &s_destBuffer[4], NULL);

    /* submit channel2 transfer parameter */
    DMA_SubmitChannelTransferParameter(&s_DMA_Handle2,
                                       DMA_CHANNEL_XFER(false, false, true, false, 4, kDMA_AddressInterleave1xWidth,
                                                        kDMA_AddressInterleave1xWidth, 16U),
                                       s_srcBuffer2, &s_destBuffer[8], NULL);

    /* software trigger channl0 transfer start firstly */
    DMA_DoChannelSoftwareTrigger(DMA0, DEMO_DMA_CHANNEL0);

    /* Wait channel1 DMA transfer finish */
    while (s_Transfer1_Done != true)
    {
    }
    /* Wait channel2 DMA transfer finish */
    while (s_Transfer2_Done != true)
    {
    }
    /* Wait channel0 DMA transfer finish */
    while (s_Transfer0_Done != true)
    {
    }

    /* Print destination buffer */
    PRINTF("\r\nDMA channel chain example finish.\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < DEST_BUFFER_LENGTH; i++)
    {
        PRINTF("%d\t", s_destBuffer[i]);
    }
    while (1)
    {
    }
}
