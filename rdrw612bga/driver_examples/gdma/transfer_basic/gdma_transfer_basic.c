/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_gdma.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_GDMA            GDMA
#define APP_GDMA_CH         0
#define APP_GDMA_IRQn       GDMA_IRQn
#define APP_GDMA_IRQHandler GDMA_IRQHandler
#define APP_XFER_DATA_LEN 1024U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void APP_InitGdma(void);
static void APP_InitData(uint8_t *txData, uint8_t *rxData, uint32_t len);
static void APP_GdmaCopyData(uint8_t *txData, uint8_t *rxData, uint32_t len);
static bool APP_VerifyData(uint8_t *txData, uint8_t *rxData, uint32_t len);

static void APP_GdmaCallback(gdma_handle_t *handle, void *userData, uint32_t interrupts);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t s_gdmaTxData[APP_XFER_DATA_LEN];
static uint8_t s_gdmaRxData[APP_XFER_DATA_LEN];
static gdma_handle_t s_gdmaHandle;
static volatile bool s_gdmaXferDone;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void APP_GdmaCallback(gdma_handle_t *handle, void *userData, uint32_t interrupts)
{
    if (0UL != (interrupts & kGDMA_TransferDoneFlag))
    {
        s_gdmaXferDone = true;
    }
}

int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Reset GDMA */
    RESET_PeripheralReset(kGDMA_RST_SHIFT_RSTn);

    PRINTF("GDMA transfer basic example start\r\n");

    APP_InitGdma();

    APP_InitData(s_gdmaTxData, s_gdmaRxData, APP_XFER_DATA_LEN);

    APP_GdmaCopyData(s_gdmaTxData, s_gdmaRxData, APP_XFER_DATA_LEN);

    /* Verify the result. */
    if (true == APP_VerifyData(s_gdmaTxData, s_gdmaRxData, APP_XFER_DATA_LEN))
    {
        PRINTF("GDMA transfer basic example succeed\r\n");
    }
    else
    {
        PRINTF("GDMA transfer basic example failed\r\n");
    }

    while (1)
    {
    }
}

static void APP_InitGdma(void)
{
    GDMA_Init(APP_GDMA);

    GDMA_CreateHandle(&s_gdmaHandle, APP_GDMA, APP_GDMA_CH);

    GDMA_SetCallback(&s_gdmaHandle, APP_GdmaCallback, NULL);
}

static void APP_InitData(uint8_t *txData, uint8_t *rxData, uint32_t len)
{
    uint32_t i;

    /* txData set to 0, 1, 2, 3, ... */
    for (i = 0; i < len; i++)
    {
        txData[i] = (uint8_t)i;
    }

    /* rxData set to 0. */
    memset(rxData, 0, len);
}

static void APP_GdmaCopyData(uint8_t *txData, uint8_t *rxData, uint32_t len)
{
    gdma_channel_xfer_config_t xferConfig = {0};

    xferConfig.srcAddr        = (uint32_t)txData;
    xferConfig.destAddr       = (uint32_t)rxData;
    xferConfig.linkListAddr   = 0; /* Don't use LLI */
    xferConfig.ahbProt        = kGDMA_ProtPrevilegedMode;
    xferConfig.srcBurstSize   = kGDMA_BurstSize1;
    xferConfig.destBurstSize  = kGDMA_BurstSize1;
    xferConfig.srcWidth       = kGDMA_TransferWidth1Byte;
    xferConfig.destWidth      = kGDMA_TransferWidth1Byte;
    xferConfig.srcAddrInc     = true;
    xferConfig.destAddrInc    = true;
    xferConfig.transferLen    = len;
    xferConfig.enableLinkList = false;

    GDMA_SubmitTransfer(&s_gdmaHandle, &xferConfig);

    s_gdmaXferDone = false;

    GDMA_StartTransfer(&s_gdmaHandle);

    /* Wait for transfer done. */
    while (!s_gdmaXferDone)
    {
    }
}

static bool APP_VerifyData(uint8_t *txData, uint8_t *rxData, uint32_t len)
{
    return (0 == memcmp(txData, rxData, len));
}
