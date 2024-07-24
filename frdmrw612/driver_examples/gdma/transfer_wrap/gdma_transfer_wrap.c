/*
 * Copyright 2021-2022 NXP
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
#define APP_XFER_WRAPPED_DATA_LEN   16U
#define APP_XFER_UNWRAPPED_DATA_LEN (4U * APP_XFER_WRAPPED_DATA_LEN)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void APP_InitGdma(void);
static bool APP_GdmaSrcWrap(void);
static bool APP_GdmaDestWrap(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*
 * The wrapped data: WIDTH=1, BURST_SIZE=16, becasue wrap used, so address
 * align to WIDTH*BURST_SIZE = 16.
 * The unwrapped data: WIDTH=4, BURST_SIZE=4, becasue wrap not used, so address
 * align to WIDTH = 4.
 */
SDK_ALIGN(static uint8_t s_wrappedData[APP_XFER_WRAPPED_DATA_LEN], 16);
SDK_ALIGN(static uint8_t s_unwrappedData[APP_XFER_UNWRAPPED_DATA_LEN], 4);

static gdma_handle_t s_gdmaHandle;

static volatile bool s_gdmaXferDone = false;

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

    PRINTF("GDMA wrap example start\r\n");

    APP_InitGdma();

    if (APP_GdmaSrcWrap() && APP_GdmaDestWrap())
    {
        PRINTF("GDMA wrap example succeed\r\n");
    }
    else
    {
        PRINTF("GDMA wrap example failed\r\n");
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

static bool APP_GdmaSrcWrap(void)
{
    uint32_t i;

    gdma_channel_xfer_config_t xferConfig = {0};

    PRINTF("%s started\r\n", __func__);

    /*
     * Prepare the data
     *
     * Fill the s_wrappedData, clear the s_unwrappedData.
     */
    for (i = 0; i < ARRAY_SIZE(s_wrappedData); i++)
    {
        s_wrappedData[i] = (uint8_t)i;
    }

    memset(s_unwrappedData, 0, sizeof(s_unwrappedData));

    /*
     * GDMA data transfer
     *
     * s_wrappedData will be copied to s_unwrappedData.
     */
    xferConfig.srcAddr        = (uint32_t)s_wrappedData;
    xferConfig.destAddr       = (uint32_t)s_unwrappedData;
    xferConfig.ahbProt        = kGDMA_ProtPrevilegedMode;
    xferConfig.srcBurstSize   = kGDMA_BurstSizeWrap16;
    xferConfig.destBurstSize  = kGDMA_BurstSize4;
    xferConfig.srcWidth       = kGDMA_TransferWidth1Byte;
    xferConfig.destWidth      = kGDMA_TransferWidth4Byte;
    xferConfig.srcAddrInc     = true;
    xferConfig.destAddrInc    = true;
    xferConfig.transferLen    = APP_XFER_UNWRAPPED_DATA_LEN;
    xferConfig.enableLinkList = false;

    GDMA_SubmitTransfer(&s_gdmaHandle, &xferConfig);

    s_gdmaXferDone = false;

    GDMA_StartTransfer(&s_gdmaHandle);

    /* Wait for transfer done. */
    while (!s_gdmaXferDone)
    {
    }

    /*
     * Verify the result
     */
    for (i = 0; i < APP_XFER_UNWRAPPED_DATA_LEN; i++)
    {
        if (s_unwrappedData[i] != s_wrappedData[i % APP_XFER_WRAPPED_DATA_LEN])
        {
            PRINTF("%s error\r\n", __func__);

            return false;
        }
    }

    PRINTF("%s succeed\r\n", __func__);

    return true;
}

static bool APP_GdmaDestWrap(void)
{
    uint32_t i;

    gdma_channel_xfer_config_t xferConfig = {0};

    PRINTF("%s started\r\n", __func__);

    /*
     * Prepare the data
     *
     * Fill the s_unwrappedData, clear the s_wrappedData.
     */
    for (i = 0; i < ARRAY_SIZE(s_unwrappedData); i++)
    {
        s_unwrappedData[i] = (uint8_t)i;
    }

    memset(s_wrappedData, 0, sizeof(s_wrappedData));

    /*
     * GDMA data transfer
     *
     * s_unwrappedData will be copied to s_wrappedData.
     */
    xferConfig.srcAddr        = (uint32_t)s_unwrappedData;
    xferConfig.destAddr       = (uint32_t)s_wrappedData;
    xferConfig.ahbProt        = kGDMA_ProtPrevilegedMode;
    xferConfig.srcBurstSize   = kGDMA_BurstSize4;
    xferConfig.destBurstSize  = kGDMA_BurstSizeWrap16;
    xferConfig.srcWidth       = kGDMA_TransferWidth4Byte;
    xferConfig.destWidth      = kGDMA_TransferWidth1Byte;
    xferConfig.srcAddrInc     = true;
    xferConfig.destAddrInc    = true;
    xferConfig.transferLen    = APP_XFER_UNWRAPPED_DATA_LEN;
    xferConfig.enableLinkList = false;

    GDMA_SubmitTransfer(&s_gdmaHandle, &xferConfig);

    s_gdmaXferDone = false;

    GDMA_StartTransfer(&s_gdmaHandle);

    /* Wait for transfer done. */
    while (!s_gdmaXferDone)
    {
    }

    /*
     * Verify the result
     */
    if (0 != memcmp(s_wrappedData, &s_unwrappedData[APP_XFER_UNWRAPPED_DATA_LEN - APP_XFER_WRAPPED_DATA_LEN],
                    APP_XFER_WRAPPED_DATA_LEN))
    {
        PRINTF("%s error\r\n", __func__);
        return false;
    }
    else
    {
        PRINTF("%s succeed\r\n", __func__);
        return true;
    }
}
