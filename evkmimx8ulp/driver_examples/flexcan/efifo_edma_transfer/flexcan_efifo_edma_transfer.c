/*
 * Copyright 2021-2022 NXP
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
#include "fsl_reset.h"
#include "fsl_upower.h"
#include "fsl_trdc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_CAN           CAN0
#define TX_MESSAGE_BUFFER_NUM (0U)

#define EXAMPLE_CAN_DMA            (DMA0)
#define EXAMPLE_CAN_DMA_CHANNEL    0
#define FLEXCAN_DMA_REQUEST_SOURCE kDmaRequestMux0CAN

#define EXAMPLE_CAN_CLOCK_NAME   (kCLOCK_Flexcan)
#define EXAMPLE_CAN_CLOCK_SOURCE (kCLOCK_Pcc1BusIpSrcSysOscDiv2)
#define EXAMPLE_CAN_CLK_FREQ     (CLOCK_GetIpFreq(EXAMPLE_CAN_CLOCK_NAME))
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
volatile bool txComplete = false;
volatile bool rxComplete = false;
flexcan_handle_t flexcanHandle;
flexcan_edma_handle_t flexcanEdmaHandle;
edma_handle_t flexcanRxFifoEdmaHandle;
flexcan_mb_transfer_t txXfer;
flexcan_mb_transfer_t txXferCfg;
flexcan_fifo_transfer_t rxFifoXfer;
flexcan_fd_frame_t txFrame = {0};
flexcan_fd_frame_t txFrameCfg = {0};
AT_NONCACHEABLE_SECTION(flexcan_fd_frame_t rxFrame[RX_MESSAGE_COUNT]);
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
    txFrameCfg.id     = FLEXCAN_ID_STD(0x555);
    txFrameCfg.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
    txFrameCfg.type   = (uint8_t)kFLEXCAN_FrameTypeData;
    txFrameCfg.length = 0U;
    txXferCfg.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
    txXferCfg.framefd = &txFrameCfg;
    (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXferCfg);
    while (!txComplete)
    {
    };
    txComplete = false;

    /* Configuration of spoofing protection. */
    /* Add 0x123 to 0x126 to Transmission Whitelist. */
    /* Set mask 0x007 to allow 0x123 to 0x126 transfer. */
    txFrameCfg.id     = FLEXCAN_ID_EXT(0x18DA00F1);
    txFrameCfg.format = (uint8_t)kFLEXCAN_FrameFormatExtend;
    txFrameCfg.type   = (uint8_t)kFLEXCAN_FrameTypeData;
    txFrameCfg.length = 6U;
    txFrameCfg.dataWord[0] = CAN_WORD_DATA_BYTE_0(0x10) | CAN_WORD_DATA_BYTE_1(0x00) | CAN_WORD_DATA_BYTE_2(0x51) |
                             CAN_WORD_DATA_BYTE_3(0x23);
    txFrameCfg.dataWord[1] = CAN_WORD_DATA_BYTE_4(0x00) | CAN_WORD_DATA_BYTE_5(0x07);
    (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXferCfg);
    while (!txComplete)
    {
    };
    txComplete = false;

    /* Configuration of command message ID. */
    /* Reconfiguration is only accepted locally. Keep CONFIG_ID as default value 0x18DA00F1. */
    txFrameCfg.length = 5U;
    txFrameCfg.dataWord[0] = CAN_WORD_DATA_BYTE_0(0x60) | CAN_WORD_DATA_BYTE_1(0x98) | CAN_WORD_DATA_BYTE_2(0xDA) |
                             CAN_WORD_DATA_BYTE_3(0x00);
    txFrameCfg.dataWord[1] = CAN_WORD_DATA_BYTE_4(0xF1);
    (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXferCfg);
    while (!txComplete)
    {
    };
    txComplete = false;

    /* Leaving configuration mode. */
    /* Configuration into volatile memory only. */
    txFrameCfg.length = 8U;
    txFrameCfg.dataWord[0] = CAN_WORD_DATA_BYTE_0(0x71) | CAN_WORD_DATA_BYTE_1(0x02) | CAN_WORD_DATA_BYTE_2(0x03) |
                             CAN_WORD_DATA_BYTE_3(0x04);
    txFrameCfg.dataWord[1] = CAN_WORD_DATA_BYTE_4(0x05) | CAN_WORD_DATA_BYTE_5(0x06) | CAN_WORD_DATA_BYTE_6(0x07) |
                             CAN_WORD_DATA_BYTE_7(0x08);
    (void)FLEXCAN_TransferFDSendNonBlocking(EXAMPLE_CAN, &flexcanHandle, &txXferCfg);
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
    flexcan_enhanced_rx_fifo_config_t rxEhFifoConfig;
#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
    uint8_t node_type;
#endif
    uint32_t i;

    /* Initialize board hardware. */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    UPOWER_PowerOnMemPart(0U, (uint32_t)(kUPOWER_MP1_DMA0));
    CLOCK_EnableClock(kCLOCK_Dma0Ch0);

    CLOCK_SetIpSrc(EXAMPLE_CAN_CLOCK_NAME, EXAMPLE_CAN_CLOCK_SOURCE);
    RESET_PeripheralReset(kRESET_Flexcan);
    CLOCK_SetIpSrc(kCLOCK_Lpi2c0, kCLOCK_Pcc1BusIpSrcSysOscDiv2);
    RESET_PeripheralReset(kRESET_Lpi2c0);

    pca6416a_handle_t handle;
    BOARD_InitPCA6416A(&handle);
    PCA6416A_SetDirection(&handle, 1 << 4U, kPCA6416A_Output);
    PCA6416A_ClearPins(&handle, 1 << 4U);

    if (BOARD_IsLowPowerBootType() != true) /* not low power boot type */
    {
        BOARD_HandshakeWithUboot(); /* Must handshake with uboot, unless will get issues(such as: SoC reset all the
                                       time) */
    }
    else                            /* low power boot type */
    {
        BOARD_SetTrdcGlobalConfig();
    }

    LOG_INFO("FlexCAN Enhanced Rx FIFO edma example.\r\n");
#if (defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
    LOG_INFO("Loopback mode, Message buffer %d used for Tx, Enhanced Rx FIFO used for Rx.\r\n", TX_MESSAGE_BUFFER_NUM);
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

#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
    if ((node_type == 'A') || (node_type == 'a') || (node_type == 'T') || (node_type == 't'))
    {
#endif
        /* Setup Tx Message Buffer. */
        FLEXCAN_SetFDTxMbConfig(EXAMPLE_CAN, TX_MESSAGE_BUFFER_NUM, true);
        /* Send messages through Tx Message Buffer. */
        txFrame.dataWord[0] = 0;
        txFrame.dataWord[1] = 0x55;
        txFrame.format      = (uint8_t)kFLEXCAN_FrameFormatStandard;
        txFrame.type        = (uint8_t)kFLEXCAN_FrameTypeData;
        txFrame.id          = FLEXCAN_ID_STD(0x123);
        txFrame.brs         = 1U;
        txFrame.edl         = 1U;
        txFrame.length      = (uint8_t)DLC;
        txXfer.framefd      = &txFrame;
        txXfer.mbIdx        = (uint8_t)TX_MESSAGE_BUFFER_NUM;
#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
    }
    else
    {
#endif
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

        /* Setup Enhanced Rx FIFO. */
        rxEhFifoConfig.idFilterTable     = rxEnFifoFilter;
        rxEhFifoConfig.idFilterPairNum   = sizeof(rxEnFifoFilter) / sizeof(rxEnFifoFilter[0]) / 2U;
        rxEhFifoConfig.extendIdFilterNum = 0;
        /*!< To reduce the complexity of DMA software configuration, the fifoWatermark value must be set to 1 when using
            DMA handling the Enhanced Rx FIFO. Because a DMA transfer cannot be dynamically changed, Number of words
           read per transfer (dmaPerReadLength) should be programmed so that the Enhanced Rx FIFO element can store the
           largest CAN message present on the CAN bus. */
        rxEhFifoConfig.fifoWatermark    = 0U;
        rxEhFifoConfig.dmaPerReadLength = kFLEXCAN_19WordPerRead;
        rxEhFifoConfig.priority         = kFLEXCAN_RxFifoPrioHigh;
        FLEXCAN_SetEnhancedRxFifoConfig(EXAMPLE_CAN, &rxEhFifoConfig, true);
        rxFifoXfer.framefd  = &rxFrame[0];
        rxFifoXfer.frameNum = RX_MESSAGE_COUNT;
        /* Create FlexCAN EDMA handle structure and set call back function. */
        FLEXCAN_TransferCreateHandleEDMA(EXAMPLE_CAN, &flexcanEdmaHandle, flexcan_dma_callback, NULL,
                                         &flexcanRxFifoEdmaHandle);
#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
        LOG_INFO("Start to Wait data from Node A.\r\n\r\n");
    }
#endif

    /* Create FlexCAN handle structure and set call back function. */
    FLEXCAN_TransferCreateHandle(EXAMPLE_CAN, &flexcanHandle, flexcan_callback, NULL);

    /* Configure CAN transceiver */
    FLEXCAN_PHY_Config();

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
                         txFrame.dataWord[0], txFrame.dataWord[1], 0x123U + i);
                txFrame.dataWord[0]++;
            }
            LOG_INFO("\r\n");
#if !(defined(ENABLE_LOOPBACK) && ENABLE_LOOPBACK)
        }
        else
        {
#endif
            /* Receive data through Enhanced Rx FIFO. */
            if (FLEXCAN_TransferReceiveEnhancedFifoEDMA(EXAMPLE_CAN, &flexcanEdmaHandle, &rxFifoXfer) !=
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
