/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_flexcan.h"
#include "fsl_flexcan_edma.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif
#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_CAN           CAN1
#define TX_MESSAGE_BUFFER_NUM (9)

#define EXAMPLE_CAN_DMA            (DMA3)
#define EXAMPLE_CAN_DMA_CHANNEL    kDma3RequestMuxCAN1
#define FLEXCAN_DMA_REQUEST_SOURCE (-1)
#define FLEXCAN_CLOCK_ROOT         (kCLOCK_Root_Can1)
#define FLEXCAN_CLOCK_GATE         kCLOCK_Can1
#define EXAMPLE_CAN_CLK_FREQ       CLOCK_GetIpFreq(FLEXCAN_CLOCK_ROOT)
#define EXAMPLE_DMA_CLOCK_ROOT     kCLOCK_Root_WakeupAxi
#define EXAMPLE_DMA_CLOCK_GATE     kCLOCK_Edma1
#define USE_IMPROVED_TIMING_CONFIG (1U)
/* Fix MISRA_C-2012 Rule 17.7. */
#define LOG_INFO (void)PRINTF
#ifndef EXAMPLE_RX_MESSAGE_NUM
#define EXAMPLE_RX_MESSAGE_NUM (4)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool txComplete = false;
volatile bool rxComplete = false;
flexcan_handle_t flexcanHandle;
flexcan_edma_handle_t flexcanEdmaHandle;
edma_handle_t flexcanRxFifoEdmaHandle;
flexcan_mb_transfer_t txXfer;
flexcan_fifo_transfer_t rxFifoXfer;
flexcan_frame_t txFrame;
AT_NONCACHEABLE_SECTION(flexcan_frame_t rxFrame[EXAMPLE_RX_MESSAGE_NUM]);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief FlexCAN Call Back function
 */
static FLEXCAN_CALLBACK(flexcan_callback)
{
    /* Process FlexCAN Tx event. */
    if ((kStatus_FLEXCAN_TxIdle == status) && (TX_MESSAGE_BUFFER_NUM == result))
    {
        txComplete = true;
    }
}

/*!
 * @brief FlexCAN DMA Call Back function
 */
static void flexcan_dma_callback(CAN_Type *base, flexcan_edma_handle_t *handle, status_t status, void *userData)
{
    /* Process FlexCAN Rx event. */
    if (kStatus_FLEXCAN_RxFifoIdle == status)
    {
        rxComplete = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    edma_config_t edmaConfig;
    flexcan_config_t flexcanConfig;
    flexcan_rx_fifo_config_t rxFifoConfig;
    uint32_t rxFifoFilter[] = {
        FLEXCAN_RX_FIFO_STD_FILTER_TYPE_A(0x321, 0, 0), FLEXCAN_RX_FIFO_STD_FILTER_TYPE_A(0x321, 1, 0),
        FLEXCAN_RX_FIFO_STD_FILTER_TYPE_A(0x123, 0, 0), FLEXCAN_RX_FIFO_STD_FILTER_TYPE_A(0x123, 1, 0)};

    /* Initialize board hardware. */
    /* clang-format off */
    /* 250MHz DMA clock */
    const clock_root_config_t dmaClkCfg = {
        .clockOff = false,
        .mux = kCLOCK_WAKEUPAXI_ClockRoot_MuxSysPll1Pfd0, // 1000MHz
        .div = 4
    };
    const clock_root_config_t flexcanClkCfg = {
        .clockOff = false,
	.mux = 0,
	.div = 1
    };
    const clock_root_config_t lpi2cClkCfg = {
        .clockOff = false,
        .mux = 0, // 24MHZ oscillator source
        .div = 1
    };
    /* clang-format on */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(EXAMPLE_DMA_CLOCK_ROOT, &dmaClkCfg);
    CLOCK_EnableClock(EXAMPLE_DMA_CLOCK_GATE);
    CLOCK_SetRootClock(FLEXCAN_CLOCK_ROOT, &flexcanClkCfg);
    CLOCK_EnableClock(FLEXCAN_CLOCK_GATE);
    CLOCK_SetRootClock(kCLOCK_Root_Lpi2c1, &lpi2cClkCfg);
    CLOCK_EnableClock(kCLOCK_Lpi2c1);

    LOG_INFO("\r\n==FlexCAN loopback edma example -- Start.==\r\n\r\n");

    /* Init FlexCAN module. */
    /*
     * flexcanConfig.clkSrc                 = kFLEXCAN_ClkSrc0;
     * flexcanConfig.bitRate                = 1000000U;
     * flexcanConfig.bitRateFD              = 2000000U;
     * flexcanConfig.maxMbNum               = 16;
     * flexcanConfig.enableLoopBack         = false;
     * flexcanConfig.enableSelfWakeup       = false;
     * flexcanConfig.enableIndividMask      = false;
     * flexcanConfig.disableSelfReception   = false;
     * flexcanConfig.enableListenOnlyMode   = false;
     * flexcanConfig.enableDoze             = false;
     */
    FLEXCAN_GetDefaultConfig(&flexcanConfig);

#if defined(EXAMPLE_CAN_CLK_SOURCE)
    flexcanConfig.clkSrc = EXAMPLE_CAN_CLK_SOURCE;
#endif
    /* Enable internal loopback. */
    flexcanConfig.enableLoopBack = true;

#if (defined(USE_IMPROVED_TIMING_CONFIG) && USE_IMPROVED_TIMING_CONFIG)
    flexcan_timing_config_t timing_config;
    memset(&timing_config, 0, sizeof(flexcan_timing_config_t));
    if (FLEXCAN_CalculateImprovedTimingValues(EXAMPLE_CAN, flexcanConfig.bitRate, EXAMPLE_CAN_CLK_FREQ, &timing_config))
    {
        /* Update the improved timing configuration*/
        memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
    }
    else
    {
        LOG_INFO("No found Improved Timing Configuration. Just used default configuration\r\n\r\n");
    }
#endif

    FLEXCAN_Init(EXAMPLE_CAN, &flexcanConfig, EXAMPLE_CAN_CLK_FREQ);

#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /* Configure DMA. */
    DMAMUX_Init(EXAMPLE_CAN_DMAMUX);
    DMAMUX_SetSource(EXAMPLE_CAN_DMAMUX, EXAMPLE_CAN_DMA_CHANNEL, EXAMPLE_CAN_DMA_REQUEST);
    DMAMUX_EnableChannel(EXAMPLE_CAN_DMAMUX, EXAMPLE_CAN_DMA_CHANNEL);
#endif

    /*
     * edmaConfig.enableRoundRobinArbitration = false;
     * edmaConfig.enableHaltOnError = true;
     * edmaConfig.enableContinuousLinkMode = false;
     * edmaConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&edmaConfig);
#if defined(BOARD_GetEDMAConfig)
    BOARD_GetEDMAConfig(edmaConfig);
#endif
    EDMA_Init(EXAMPLE_CAN_DMA, &edmaConfig);

    /* Create EDMA handle. */
    EDMA_CreateHandle(&flexcanRxFifoEdmaHandle, EXAMPLE_CAN_DMA, EXAMPLE_CAN_DMA_CHANNEL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(EXAMPLE_CAN_DMA, EXAMPLE_CAN_DMA_CHANNEL, FLEXCAN_DMA_REQUEST_SOURCE);
#endif

    /* Setup Tx Message Buffer. */
    FLEXCAN_SetTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);

    /* Setup Rx FIFO. */
    rxFifoConfig.idFilterTable = rxFifoFilter;
    rxFifoConfig.idFilterType  = kFLEXCAN_RxFifoFilterTypeA;
    rxFifoConfig.idFilterNum   = sizeof(rxFifoFilter) / sizeof(rxFifoFilter[0]);
    rxFifoConfig.priority      = kFLEXCAN_RxFifoPrioHigh;
    FLEXCAN_SetRxFifoConfig(EXAMPLE_CAN, &rxFifoConfig, true);

    /* Create FlexCAN handle structure and set call back function. */
    FLEXCAN_TransferCreateHandle(EXAMPLE_CAN, &flexcanHandle, flexcan_callback, NULL);

    /* Create FlexCAN EDMA handle structure and set call back function. */
    FLEXCAN_TransferCreateHandleEDMA(EXAMPLE_CAN, &flexcanEdmaHandle, flexcan_dma_callback, NULL,
                                     &flexcanRxFifoEdmaHandle);
    /* Receive data through Rx FIFO. */
    rxFifoXfer.frame    = &rxFrame[0];
    rxFifoXfer.frameNum = EXAMPLE_RX_MESSAGE_NUM;
    (void)FLEXCAN_TransferReceiveFifoEDMA(EXAMPLE_CAN, &flexcanEdmaHandle, &rxFifoXfer);

    for (uint32_t i = 1; i <= EXAMPLE_RX_MESSAGE_NUM; i++)
    {
        /* Send message through Tx Message Buffer. */
        txFrame.format    = (uint8_t)kFLEXCAN_FrameFormatStandard;
        txFrame.type      = (uint8_t)kFLEXCAN_FrameTypeData;
        txFrame.id        = FLEXCAN_ID_STD(0x123);
        txFrame.length    = 8U;
        txFrame.dataWord0 = CAN_WORD0_DATA_BYTE_0(0x11 * i) | CAN_WORD0_DATA_BYTE_1(0x11 * i) |
                            CAN_WORD0_DATA_BYTE_2(0x11 * i) | CAN_WORD0_DATA_BYTE_3(0x11 * i);
        txFrame.dataWord1 = CAN_WORD1_DATA_BYTE_4(0x11 * i) | CAN_WORD1_DATA_BYTE_5(0x11 * i) |
                            CAN_WORD1_DATA_BYTE_6(0x11 * i) | CAN_WORD1_DATA_BYTE_7(0x11 * i);

        txXfer.frame = &txFrame;
        txXfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
        (void)FLEXCAN_TransferSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXfer);
        while (!txComplete)
        {
        }
        txComplete = false;
        LOG_INFO("Send Msg%d to Rx FIFO: word0 = 0x%x, word1 = 0x%x.\r\n", i, txFrame.dataWord0, txFrame.dataWord1);
    }

    while (!rxComplete)
    {
    }
    rxComplete = false;
    for (uint32_t i = 0; i < EXAMPLE_RX_MESSAGE_NUM; i++)
    {
        LOG_INFO("Receive Msg%d from FIFO: word0 = 0x%x, word1 = 0x%x, ID Filter Hit%d.\r\n", i + 1,
                 rxFrame[i].dataWord0, rxFrame[i].dataWord1, rxFrame[i].idhit);
    }

    LOG_INFO("\r\n==FlexCAN loopback EDMA example -- Finish.==\r\n");

    while (true)
    {
        __WFI();
    }
}
