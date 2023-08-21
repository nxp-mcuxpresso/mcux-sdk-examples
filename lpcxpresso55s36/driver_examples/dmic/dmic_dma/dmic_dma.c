/*
 * Copyright 2016, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_dmic.h"
#include "fsl_dma.h"
#include "fsl_dmic_dma.h"
#include <stdlib.h>
#include <string.h>

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DMAREQ_DMIC0            23U
#define DMAREQ_DMIC1            24U
#define APP_DMAREQ_CHANNEL      DMAREQ_DMIC0
#define APP_DMIC_CHANNEL        kDMIC_Channel0
#define APP_DMIC_CHANNEL_ENABLE DMIC_CHANEN_EN_CH0(1) | DMIC_CHANEN_EN_CH1(1)
#define FIFO_DEPTH    15U
#define BUFFER_LENGTH 32U
#if defined(FSL_FEATURE_DMIC_CHANNEL_HAS_SIGNEXTEND) && (FSL_FEATURE_DMIC_CHANNEL_HAS_SIGNEXTEND)
#define DEMO_DMIC_DATA_WIDTH uint32_t
#else
#define DEMO_DMIC_DATA_WIDTH uint16_t
#endif
/*******************************************************************************

 * Prototypes
 ******************************************************************************/
/* DMIC user callback */
void DMIC_UserCallback(DMIC_Type *base, dmic_dma_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
DEMO_DMIC_DATA_WIDTH g_rxBuffer[BUFFER_LENGTH] = {0};

dmic_dma_handle_t g_dmicDmaHandle;
dma_handle_t g_dmicRxDmaHandle;
volatile bool g_Transfer_Done = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
/* DMIC user callback */
void DMIC_UserCallback(DMIC_Type *base, dmic_dma_handle_t *handle, status_t status, void *userData)
{
    userData = userData;
    if (status == kStatus_DMIC_Idle)
    {
        g_Transfer_Done = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    dmic_channel_config_t dmic_channel_cfg;

    dmic_transfer_t receiveXfer;

    uint32_t i;
    /* Board pin, clock, debug console init */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 8U, false);

    CLOCK_AttachClk(kEXT_CLK_to_DMIC);

    dmic_channel_cfg.divhfclk            = kDMIC_PdmDiv1;
    dmic_channel_cfg.osr                 = 25U;
    dmic_channel_cfg.gainshft            = 2U;
    dmic_channel_cfg.preac2coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.preac4coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.dc_cut_level        = kDMIC_DcCut155;
    dmic_channel_cfg.post_dc_gain_reduce = 1;
    dmic_channel_cfg.saturate16bit       = 1U;
    dmic_channel_cfg.sample_rate         = kDMIC_PhyFullSpeed;
#if defined(FSL_FEATURE_DMIC_CHANNEL_HAS_SIGNEXTEND) && (FSL_FEATURE_DMIC_CHANNEL_HAS_SIGNEXTEND)
    dmic_channel_cfg.enableSignExtend = true;
#endif

    DMIC_Init(DMIC0);
#if !(defined(FSL_FEATURE_DMIC_HAS_NO_IOCFG) && FSL_FEATURE_DMIC_HAS_NO_IOCFG)
    DMIC_SetIOCFG(DMIC0, kDMIC_PdmDual);
#endif

    DMIC_Use2fs(DMIC0, true);
    DMIC_EnableChannelDma(DMIC0, APP_DMIC_CHANNEL, true);
#if defined(BOARD_DMIC_CHANNEL_STEREO_SIDE_SWAP) && (BOARD_DMIC_CHANNEL_STEREO_SIDE_SWAP)
    DMIC_ConfigChannel(DMIC0, APP_DMIC_CHANNEL, kDMIC_Right, &dmic_channel_cfg);
#else
    DMIC_ConfigChannel(DMIC0, APP_DMIC_CHANNEL, kDMIC_Left, &dmic_channel_cfg);
#endif

    DMIC_FifoChannel(DMIC0, APP_DMIC_CHANNEL, FIFO_DEPTH, true, true);

    DMIC_EnableChannnel(DMIC0, APP_DMIC_CHANNEL_ENABLE);
    PRINTF("Configure DMA\r\n");

    DMA_Init(DMA0);

    DMA_EnableChannel(DMA0, APP_DMAREQ_CHANNEL);

    /* Request dma channels from DMA manager. */
    DMA_CreateHandle(&g_dmicRxDmaHandle, DMA0, APP_DMAREQ_CHANNEL);

    /* Create DMIC DMA handle. */
    DMIC_TransferCreateHandleDMA(DMIC0, &g_dmicDmaHandle, DMIC_UserCallback, NULL, &g_dmicRxDmaHandle);
    receiveXfer.dataSize               = 2 * BUFFER_LENGTH;
    receiveXfer.data                   = g_rxBuffer;
    receiveXfer.dataAddrInterleaveSize = kDMA_AddressInterleave1xWidth;
    receiveXfer.dataWidth              = sizeof(DEMO_DMIC_DATA_WIDTH);
    receiveXfer.linkTransfer           = NULL;
    PRINTF("Buffer Data before transfer \r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\r\n", g_rxBuffer[i]);
    }
    DMIC_TransferReceiveDMA(DMIC0, &g_dmicDmaHandle, &receiveXfer, APP_DMIC_CHANNEL);

    /* Wait for DMA transfer finish */
    while (g_Transfer_Done == false)
    {
    }

    PRINTF("Transfer completed\r\n");
    PRINTF("Buffer Data after transfer \r\n");
    for (i = 0; i < BUFFER_LENGTH; i++)
    {
        PRINTF("%d\r\n", g_rxBuffer[i]);
    }
    while (1)
    {
    }
}
