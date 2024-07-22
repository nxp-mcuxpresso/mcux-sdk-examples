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

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_CAN           CAN2
#define TX_MESSAGE_BUFFER_NUM (0)

#define FLEXCAN_CLOCK_ROOT   (kCLOCK_Root_Can2)
#define FLEXCAN_CLOCK_GATE   kCLOCK_Can2
#define EXAMPLE_CAN_CLK_FREQ CLOCK_GetIpFreq(FLEXCAN_CLOCK_ROOT)

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
 * @brief CAN transceiver configuration function
 */
static void FLEXCAN_PHY_Config(void)
{
#if (defined(USE_PHY_TJA1152) && USE_PHY_TJA1152)
    /* Setup Tx Message Buffer. */
    FLEXCAN_SetFDTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);

    /* Initialize TJA1152. */
    /* STB=H, configuration CAN messages are expected from the local host via TXD pin. */
    RGPIO_PortSet(EXAMPLE_STB_RGPIO, 1u << EXAMPLE_STB_RGPIO_PIN);   

    /* Classical CAN messages with standard identifier 0x555 must be transmitted 
     * by the local host controller until acknowledged by the TJA1152 for
     * automatic bit rate detection. Do not set frame.brs = 1U to keep nominal
     * bit rate in CANFD frame data phase. */
    txFrame.id     = FLEXCAN_ID_STD(0x555);
    txFrame.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
    txFrame.type   = (uint8_t)kFLEXCAN_FrameTypeData;
    txFrame.length = 0U;
    txXfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
    txXfer.framefd = &txFrame;
    (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXfer);
    while (!txComplete)
    {
    };
    txComplete = false;

    /* Configuration of spoofing protection. */
    /* Add 0x123 to 0x126 to Transmission Whitelist. */
    /* Set mask 0x007 to allow 0x123 to 0x126 transfer. */
    txFrame.id     = FLEXCAN_ID_EXT(0x18DA00F1);
    txFrame.format = (uint8_t)kFLEXCAN_FrameFormatExtend;
    txFrame.type   = (uint8_t)kFLEXCAN_FrameTypeData;
    txFrame.length = 6U;
    txFrame.dataWord[0] = CAN_WORD_DATA_BYTE_0(0x10) | CAN_WORD_DATA_BYTE_1(0x00) | CAN_WORD_DATA_BYTE_2(0x51) |
                          CAN_WORD_DATA_BYTE_3(0x23);
    txFrame.dataWord[1] = CAN_WORD_DATA_BYTE_4(0x00) | CAN_WORD_DATA_BYTE_5(0x07);
    (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXfer);
    while (!txComplete)
    {
    };
    txComplete = false;

    /* Configuration of command message ID. */
    /* Reconfiguration is only accepted locally. Keep CONFIG_ID as default value 0x18DA00F1. */
    txFrame.length = 5U;
    txFrame.dataWord[0] = CAN_WORD_DATA_BYTE_0(0x60) | CAN_WORD_DATA_BYTE_1(0x98) | CAN_WORD_DATA_BYTE_2(0xDA) |
                          CAN_WORD_DATA_BYTE_3(0x00);
    txFrame.dataWord[1] = CAN_WORD_DATA_BYTE_4(0xF1);
    (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXfer);
    while (!txComplete)
    {
    };
    txComplete = false;

    /* Leaving configuration mode. */
    /* Configuration into volatile memory only. */
    txFrame.length = 8U;
    txFrame.dataWord[0] = CAN_WORD_DATA_BYTE_0(0x71) | CAN_WORD_DATA_BYTE_1(0x02) | CAN_WORD_DATA_BYTE_2(0x03) |
                          CAN_WORD_DATA_BYTE_3(0x04);
    txFrame.dataWord[1] = CAN_WORD_DATA_BYTE_4(0x05) | CAN_WORD_DATA_BYTE_5(0x06) | CAN_WORD_DATA_BYTE_6(0x07) |
                          CAN_WORD_DATA_BYTE_7(0x08);
    (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXfer);
    while (!txComplete)
    {
    };
    txComplete = false;

    LOG_INFO("Initialize TJA1152 successfully!\r\n\r\n");

    /* STB=L, TJA1152 switch from secure standby mode to normal mode. */
    RGPIO_PortClear(EXAMPLE_STB_RGPIO, 1u << EXAMPLE_STB_RGPIO_PIN);
    /* Initialize TJA1152 end. */
#endif
}

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
    /* clang-format off */

    const clock_root_config_t flexcanClkCfg = {
        .clockOff = false,
	.mux = 2,
	.div = 10
    };
    const clock_root_config_t lpi2cClkCfg = {
        .clockOff = false,
        .mux = 0, // 24MHz oscillator source
        .div = 1
    };
    /* clang-format on */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(FLEXCAN_CLOCK_ROOT, &flexcanClkCfg);
    CLOCK_EnableClock(FLEXCAN_CLOCK_GATE);
    CLOCK_SetRootClock(BOARD_ADP5585_I2C_CLOCK_ROOT, &lpi2cClkCfg);
    CLOCK_EnableClock(BOARD_ADP5585_I2C_CLOCK_GATE);

    /* Select CAN2 signals */
    adp5585_handle_t handle1;
    BOARD_InitADP5585(&handle1);
    ADP5585_SetDirection(&handle1, (1 << BOARD_ADP5585_EXP_SEL), kADP5585_Output);
    ADP5585_ClearPins(&handle1, (1 << BOARD_ADP5585_EXP_SEL));

    /* Select CAN_STBY signals */
    adp5585_handle_t handle;
    BOARD_InitADP5585(&handle);
    ADP5585_SetDirection(&handle, (1 << BOARD_ADP5585_CAN_STBY), kADP5585_Output);
    ADP5585_ClearPins(&handle, (1 << BOARD_ADP5585_CAN_STBY));

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

    flexcanConfig.bitRate = 500000U;

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

    /* Configure CAN transceiver */
    FLEXCAN_PHY_Config();

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
