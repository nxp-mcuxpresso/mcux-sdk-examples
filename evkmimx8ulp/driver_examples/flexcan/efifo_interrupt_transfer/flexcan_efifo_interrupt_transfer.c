/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_flexcan.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_reset.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_CAN           CAN0
#define TX_MESSAGE_BUFFER_NUM (0)

#define EXAMPLE_CAN_CLOCK_NAME   (kCLOCK_Flexcan)
#define EXAMPLE_CAN_CLOCK_SOURCE (kCLOCK_Pcc1BusIpSrcSysOscDiv2)
#define EXAMPLE_CAN_CLK_FREQ     (CLOCK_GetFlexcanClkFreq())

/* Set USE_IMPROVED_TIMING_CONFIG macro to use api to calculates the improved CAN / CAN FD timing values. */
#define USE_IMPROVED_TIMING_CONFIG (1U)
/* Fix MISRA_C-2012 Rule 17.7. */
#define LOG_INFO (void)PRINTF
#ifndef RX_MESSAGE_COUNT
#define RX_MESSAGE_COUNT (4U)
#endif
/*
 *    DWORD_IN_MB    DLC    BYTES_IN_MB             Maximum MBs
 *    2              8      kFLEXCAN_8BperMB        64
 *    4              10     kFLEXCAN_16BperMB       42
 *    8              13     kFLEXCAN_32BperMB       25
 *    16             15     kFLEXCAN_64BperMB       14
 *
 * Dword in each message buffer, Length of data in bytes, Payload size must align,
 * and the Message Buffers are limited corresponding to each payload configuration:
 */
#define DLC         (15)
#define BYTES_IN_MB kFLEXCAN_64BperMB
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
flexcan_handle_t flexcanHandle;
volatile bool txComplete = false;
volatile bool rxComplete = false;
flexcan_mb_transfer_t txXfer;
flexcan_fifo_transfer_t rxFifoXfer;
flexcan_fd_frame_t txFrame = {0};
flexcan_fd_frame_t rxFrame[RX_MESSAGE_COUNT];
/* Config fifo filters to make it accept std frame with ID 0x123 ~ 0x 126. */
uint32_t rxEnFifoFilter[] = {FLEXCAN_ENHANCED_RX_FIFO_STD_MASK_AND_FILTER(0x123, 0, 0x3F, 0),
                             FLEXCAN_ENHANCED_RX_FIFO_STD_MASK_AND_FILTER(0x124, 0, 0x3F, 0),
                             FLEXCAN_ENHANCED_RX_FIFO_STD_MASK_AND_FILTER(0x125, 0, 0x3F, 0),
                             FLEXCAN_ENHANCED_RX_FIFO_STD_MASK_AND_FILTER(0x126, 0, 0x3F, 0)};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief FlexCAN Call Back function
 */
static FLEXCAN_CALLBACK(flexcan_callback)
{
    switch (status)
    {
        case kStatus_FLEXCAN_RxFifoIdle:
            rxComplete = true;
            break;

        case kStatus_FLEXCAN_TxIdle:
            if (TX_MESSAGE_BUFFER_NUM == result)
            {
                txComplete = true;
            }
            break;

        default:
            break;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    flexcan_config_t flexcanConfig;
    flexcan_enhanced_rx_fifo_config_t rxEhFifoConfig;
#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
    uint8_t node_type;
#endif
    uint32_t i;

    /* Initialize board hardware. */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetIpSrc(EXAMPLE_CAN_CLOCK_NAME, EXAMPLE_CAN_CLOCK_SOURCE);
    RESET_PeripheralReset(kRESET_Flexcan);
    CLOCK_SetIpSrc(kCLOCK_Lpi2c0, kCLOCK_Pcc1BusIpSrcSysOscDiv2);
    RESET_PeripheralReset(kRESET_Lpi2c0);

    pca6416a_handle_t handle;
    BOARD_InitPCA6416A(&handle);
    PCA6416A_SetDirection(&handle, 1 << 4U, kPCA6416A_Output);
    PCA6416A_ClearPins(&handle, 1 << 4U);

    LOG_INFO("FlexCAN Enhanced Rx FIFO interrupt example.\r\n");
#if (defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
    LOG_INFO("Loopback mode, Message buffer %d used for Tx, Enhanced Rx FIFO used for Rx.\r\n\r\n",
             TX_MESSAGE_BUFFER_NUM);
#else
    LOG_INFO("Board to board mode.\r\n");
    LOG_INFO("Node B Enhanced Rx FIFO used for Rx.\r\n");
    LOG_INFO("Node A Message buffer %d used for Tx.\r\n", TX_MESSAGE_BUFFER_NUM);

    do
    {
        LOG_INFO("Please select local node as A or B:\r\n");
        LOG_INFO("Note: Node B should start first.\r\n");
        LOG_INFO("Node:");
        node_type = GETCHAR();
        LOG_INFO("%c", node_type);
        LOG_INFO("\r\n");
    } while ((node_type != 'A') && (node_type != 'B') && (node_type != 'a') && (node_type != 'b'));
#endif
    /* Get FlexCAN module default Configuration. */
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

#if (defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
    flexcanConfig.enableLoopBack = true;
#endif

#if (defined(USE_IMPROVED_TIMING_CONFIG) && USE_IMPROVED_TIMING_CONFIG)
    flexcan_timing_config_t timing_config;
    memset(&timing_config, 0, sizeof(flexcan_timing_config_t));
    if (FLEXCAN_FDCalculateImprovedTimingValues(EXAMPLE_CAN, flexcanConfig.bitRate, flexcanConfig.bitRateFD,
                                                EXAMPLE_CAN_CLK_FREQ, &timing_config))
    {
        /* Update the improved timing configuration*/
        memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
    }
    else
    {
        LOG_INFO("No found Improved Timing Configuration. Just used default configuration\r\n\r\n");
    }
#endif

    FLEXCAN_FDInit(EXAMPLE_CAN, &flexcanConfig, EXAMPLE_CAN_CLK_FREQ, BYTES_IN_MB, true);

    /* Create FlexCAN handle structure and set call back function. */
    FLEXCAN_TransferCreateHandle(EXAMPLE_CAN, &flexcanHandle, flexcan_callback, NULL);

#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
    if ((node_type == 'A') || (node_type == 'a') || (node_type == 'T') || (node_type == 't'))
    {
#endif
        /* Setup Tx Message Buffer. */
        FLEXCAN_SetFDTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
        txFrame.dataWord[0] = 0;
        txFrame.dataWord[1] = 0x55;
        txFrame.format      = (uint8_t)kFLEXCAN_FrameFormatStandard;
        txFrame.type        = (uint8_t)kFLEXCAN_FrameTypeData;
        txFrame.length      = (uint8_t)DLC;
        txFrame.brs         = (uint8_t)1U;
        txFrame.edl         = (uint8_t)1U;
        txXfer.mbIdx        = (uint8_t)TX_MESSAGE_BUFFER_NUM;
        txXfer.framefd      = &txFrame;
#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
    }
    else
    {
#endif
        /* Setup Enhanced Rx FIFO. */
        rxEhFifoConfig.idFilterTable     = rxEnFifoFilter;
        rxEhFifoConfig.idFilterPairNum   = sizeof(rxEnFifoFilter) / sizeof(rxEnFifoFilter[0]) / 2U;
        rxEhFifoConfig.extendIdFilterNum = 0;
        rxEhFifoConfig.fifoWatermark     = RX_MESSAGE_COUNT - 1U; /* Reduce the frequency to enter IRQ. */
        rxEhFifoConfig.dmaPerReadLength  = kFLEXCAN_19WordPerRead;
        rxEhFifoConfig.priority          = kFLEXCAN_RxFifoPrioHigh;
        FLEXCAN_SetEnhancedRxFifoConfig(EXAMPLE_CAN, &rxEhFifoConfig, true);
        rxFifoXfer.framefd  = &rxFrame[0];
        rxFifoXfer.frameNum = RX_MESSAGE_COUNT;
#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
        LOG_INFO("Start to Wait data from Node A.\r\n\r\n");
    }
#endif

    while (true)
    {
#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
        if ((node_type == 'A') || (node_type == 'a'))
        {
#endif
            LOG_INFO("Press any key to trigger %d transmission.\r\n\r\n", RX_MESSAGE_COUNT);
            GETCHAR();
            for (i = 0; i < RX_MESSAGE_COUNT; i++)
            {
                txFrame.id = FLEXCAN_ID_STD(0x123U + i);
                (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXfer);

                while (!txComplete)
                {
                };
                txComplete = false;
                LOG_INFO("Send Msg%d to Enhanced Rx FIFO: word0 = 0x%x, word1 = 0x%x, id = 0x%x.\r\n", i,
                         txFrame.dataWord[0], txFrame.dataWord[1], 0x123 + i);
                txFrame.dataWord[0]++;
            }
            LOG_INFO("\r\n");
#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
        }
        else
        {
#endif
            /* Receive data through Enhanced Rx FIFO. */
            if (FLEXCAN_TransferReceiveEnhancedFifoNonBlocking(EXAMPLE_CAN, &flexcanHandle, &rxFifoXfer) !=
                kStatus_Success)
            {
                LOG_INFO("Receive CAN message from Enhanced Rx FIFO failed!\r\n");
                return -1;
            }
            else
            {
                while (!rxComplete)
                {
                }
                rxComplete = false;
                for (i = 0; i < RX_MESSAGE_COUNT; i++)
                {
                    LOG_INFO(
                        "Receive Msg%d from Enhanced Rx FIFO: word0 = 0x%x, word1 = 0x%x, ID Filter Hit: %d, Time "
                        "stamp: %d.\r\n",
                        i, rxFrame[i].dataWord[0], rxFrame[i].dataWord[1], rxFrame[i].idhit, rxFrame[i].timestamp);
                }
                LOG_INFO("\r\n");
            }
#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
            LOG_INFO("Wait for the next %d messages!\r\n\r\n", RX_MESSAGE_COUNT);
        }
#endif
    }
}
