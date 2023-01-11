/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_dma.h"
#include "fsl_inputmux.h"

#include "fsl_pint.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DMA           DMA1
#define DEMO_DMA_CHANNEL   0
#define DEMO_INPUT_MUX_SRC kINPUTMUX_GpioInt0ToDma1
#define BUFF_LENGTH 4U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void DMA_HardwareTriggerConfig();

/*******************************************************************************
 * Variables
 ******************************************************************************/
dma_channel_trigger_t s_channelTrigger = {
    .type  = kDMA_RisingEdgeTrigger,
    .burst = kDMA_SingleTransfer,
    .wrap  = kDMA_NoWrap,
};
dma_handle_t g_DMA_Handle;
volatile bool g_Transfer_Done                                                           = false;
DMA_ALLOCATE_DATA_TRANSFER_BUFFER(uint32_t s_srcBuffer[BUFF_LENGTH], sizeof(uint32_t))  = {0x01, 0x02, 0x03, 0x04};
DMA_ALLOCATE_DATA_TRANSFER_BUFFER(uint32_t s_destBuffer[BUFF_LENGTH], sizeof(uint32_t)) = {0x00, 0x00, 0x00, 0x00};
extern dma_channel_trigger_t s_channelTrigger;

/*******************************************************************************
 * Code
 ******************************************************************************/
void PINT_Callback(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    PRINTF("\r\n\r\nSW1 is pressed.");
}


void DMA_HardwareTriggerConfig()
{
    PINT_Init(PINT);

    /* Connect trigger sources to PINT */
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_SecPatternMatchInp0Src, kINPUTMUX_GpioPort0Pin25ToPintsel); /* SW1 */

    /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);

    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableRiseEdge, PINT_Callback);

    PINT_EnableCallbackByIndex(PINT, kPINT_PinInt0);

    PRINTF("\r\n\r\nPress SW1 to trigger one shot DMA transfer.");
}

/* User callback function for DMA transfer. */
void DMA_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        g_Transfer_Done = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0;
    dma_channel_config_t transferConfig;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("DMA hardware trigger example begin.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFF_LENGTH; i++)
    {
        PRINTF("%d\t", s_destBuffer[i]);
    }

    DMA_Init(DEMO_DMA);

    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, DEMO_DMA_CHANNEL, DEMO_INPUT_MUX_SRC);

    DMA_CreateHandle(&g_DMA_Handle, DEMO_DMA, DEMO_DMA_CHANNEL);
    DMA_EnableChannel(DEMO_DMA, DEMO_DMA_CHANNEL);
    DMA_SetCallback(&g_DMA_Handle, DMA_Callback, NULL);
    DMA_PrepareChannelTransfer(&transferConfig, s_srcBuffer, &s_destBuffer[0],
                               DMA_CHANNEL_XFER(false, false, true, false, 4, kDMA_AddressInterleave1xWidth,
                                                kDMA_AddressInterleave1xWidth, 16),
                               kDMA_MemoryToMemory, &s_channelTrigger, NULL);
    DMA_SubmitChannelTransfer(&g_DMA_Handle, &transferConfig);

    DMA_StartTransfer(&g_DMA_Handle);

    DMA_HardwareTriggerConfig();

    while (g_Transfer_Done != true)
    {
    }

    PRINTF("\r\n\r\nDMA hardware trigger example finish.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFF_LENGTH; i++)
    {
        PRINTF("%d\t", s_destBuffer[i]);
    }
    while (1)
    {
    }
}
