/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_dma.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define BUFF_LENGTH             4U
#define DMA_DESCRIPTOR_NUM      2U
#define DMA_LINK_TRANSFER_COUNT 10
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static dma_handle_t s_DMA_Handle;
static volatile bool s_Transfer_Done = false;

DMA_ALLOCATE_LINK_DESCRIPTORS(s_dma_table, DMA_DESCRIPTOR_NUM);
DMA_ALLOCATE_DATA_TRANSFER_BUFFER(static uint32_t s_srcBuffer1[BUFF_LENGTH], sizeof(uint32_t)) = {1, 2, 3, 4};
DMA_ALLOCATE_DATA_TRANSFER_BUFFER(static uint32_t s_srcBuffer2[4], sizeof(uint32_t))           = {11, 22, 33, 44};
DMA_ALLOCATE_DATA_TRANSFER_BUFFER(static uint32_t s_destBuffer[BUFF_LENGTH * DMA_DESCRIPTOR_NUM],
                                  sizeof(uint32_t))                                            = {0x00};
static volatile uint32_t s_transferCount                                                       = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/

/* User callback function for DMA transfer. */
void DMA_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        if (++s_transferCount > DMA_LINK_TRANSFER_COUNT)
        {
            s_Transfer_Done = true;
            DMA_DisableChannel(DMA0, 0);
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Print source buffer */
    PRINTF("DMA linked transfer example begin.\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFF_LENGTH * DMA_DESCRIPTOR_NUM; i++)
    {
        PRINTF("%d\t", s_destBuffer[i]);
    }

    DMA_Init(DMA0);
    DMA_CreateHandle(&s_DMA_Handle, DMA0, 0);
    DMA_EnableChannel(DMA0, 0);
    DMA_SetCallback(&s_DMA_Handle, DMA_Callback, NULL);

    DMA_SetupDescriptor(&(s_dma_table[0]),
                        DMA_CHANNEL_XFER(true, false, false, true, 4U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave1xWidth, 16U),
                        s_srcBuffer1, &s_destBuffer[0], &(s_dma_table[1]));

    DMA_SetupDescriptor(&(s_dma_table[1]),
                        DMA_CHANNEL_XFER(true, false, false, true, 4U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave1xWidth, 16U),
                        s_srcBuffer2, &s_destBuffer[4], &(s_dma_table[0]));

    DMA_SubmitChannelDescriptor(&s_DMA_Handle, &(s_dma_table[0]));
    DMA_StartTransfer(&s_DMA_Handle);
    /* Wait for DMA transfer finish */
    while (s_Transfer_Done != true)
    {
    }
    /* Print destination buffer */
    PRINTF("\r\nDMA linked transfer example finish.\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFF_LENGTH * DMA_DESCRIPTOR_NUM; i++)
    {
        PRINTF("%d\t", s_destBuffer[i]);
    }
    while (1)
    {
    }
}
