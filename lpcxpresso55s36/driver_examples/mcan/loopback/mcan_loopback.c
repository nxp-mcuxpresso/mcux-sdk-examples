/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_mcan.h"
#include "pin_mux.h"
#include "board.h"
#include "stdlib.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define USE_CANFD (1U)
/*
 *    CAN_DATASIZE   DLC    BYTES_IN_MB
 *    8              8      kMCAN_8ByteDatafield
 *    12             9      kMCAN_12ByteDatafield
 *    16             10     kMCAN_16ByteDatafield
 *    20             11     kMCAN_20ByteDatafield
 *    24             12     kMCAN_24ByteDatafield
 *    32             13     kMCAN_32ByteDatafield
 *    48             14     kMCAN_48ByteDatafield
 *    64             15     kMCAN_64ByteDatafield
 *
 *  CAN data size (pay load size), DLC and Bytes in Message buffer must align.
 *
 */
#define DLC         (15)
#define BYTES_IN_MB kMCAN_64ByteDatafield
/* If not define USE_CANFD or define it 0, CAN_DATASIZE should be 8. */
#define CAN_DATASIZE (64U)
/* If user need to auto execute the improved timming configuration. */
#define USE_IMPROVED_TIMING_CONFIG (1U)
#define EXAMPLE_MCAN_IRQHandler    CAN0_IRQ0_IRQHandler
#define EXAMPLE_MCAN_IRQn          CAN0_IRQ0_IRQn
#define EXAMPLE_MCAN               CAN0
#define MCAN_CLK_FREQ              CLOCK_GetMCanClkFreq()
#define STDID_OFFSET               (18U)
#define MSG_RAM_BASE               0x04000000U
#define STD_FILTER_OFS 0x0
#if (defined(USE_EXT_FILTER) && USE_EXT_FILTER)
#define EXT_FILTER_OFS 0x4
#endif
#define RX_FIFO0_OFS 0x10U
#if (defined(USE_CANFD) && USE_CANFD)
#define TX_BUFFER_OFS 0x60U
#else
#define TX_BUFFER_OFS 0x20U
#endif
#define MSG_RAM_SIZE (TX_BUFFER_OFS + 8 + CAN_DATASIZE)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool rxComplete = false;
mcan_rx_buffer_frame_t rxFrame;
mcan_tx_buffer_frame_t txFrame;
uint8_t tx_data[CAN_DATASIZE];
uint8_t rx_data[CAN_DATASIZE];
#ifndef MSG_RAM_BASE
/* The initial value of 1 is just to avoid variables being placed in the bss section (which will cause hole between data
 * section and bss section), Message RAM must be initialized with 0 before used. */
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
__attribute__((aligned(1U << CAN_MRBA_BA_SHIFT), section(".data"))) uint8_t msgRam[MSG_RAM_SIZE] = {1U};
#else
SDK_ALIGN(uint8_t msgRam[MSG_RAM_SIZE], 1U << CAN_MRBA_BA_SHIFT) = {1U};
#endif
#else
#define msgRam MSG_RAM_BASE
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

void CAN0_IRQ0_IRQHandler(void)
{
    MCAN_ClearStatusFlag(EXAMPLE_MCAN, CAN_IR_RF0N_MASK);
    MCAN_ReadRxFifo(EXAMPLE_MCAN, 0, &rxFrame);
    rxComplete = true;
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    mcan_config_t mcanConfig;
    mcan_memory_config_t memoryConfig          = {0};
    mcan_frame_filter_config_t rxFilter        = {0};
    mcan_std_filter_element_config_t stdFilter = {0};
#if (defined(USE_EXT_FILTER) && USE_EXT_FILTER)
    mcan_frame_filter_config_t extRxFilter     = {0};
    mcan_ext_filter_element_config_t extFilter = {0};
#endif
    mcan_rx_fifo_config_t rxFifo0    = {0};
    mcan_tx_buffer_config_t txBuffer = {0};

    /* Initialize board hardware. */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Set MCAN clock 100Mhz/5=20MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivCanClk, 5U, true);
    CLOCK_AttachClk(kMCAN_DIV_to_MCAN);

    BOARD_InitPins();
    BOARD_BootClockPLL100M();
    BOARD_InitDebugConsole();

    PRINTF("\r\n==MCAN loopback functional example -- Start.==\r\n\r\n");

    /* Get MCAN module default Configuration. */
    /*
     * mcanConfig.baudRate               = 500000U;
     * mcanConfig.baudRateFD             = 2000000U;
     * mcanConfig.enableCanfdNormal      = false;
     * mcanConfig.enableCanfdSwitch      = false;
     * mcanConfig.enableLoopBackInt      = false;
     * mcanConfig.enableLoopBackExt      = false;
     * mcanConfig.enableBusMon           = false;
     */
    MCAN_GetDefaultConfig(&mcanConfig);
    /* Enable external loop back mode.
     * Current node treats its own transmitted messages as received messages,
     * Other node can receive Current node transmitted messages,
     * Other node can't transmit messages or ACK signal to Current node.
     */
    mcanConfig.enableLoopBackExt = true;
#if (defined(USE_CANFD) && USE_CANFD)
    /* Enable Bit Rate Switch to make baudRateD make sense.*/
    mcanConfig.enableCanfdSwitch = true;
#endif

#if (defined(USE_IMPROVED_TIMING_CONFIG) && USE_IMPROVED_TIMING_CONFIG)
    mcan_timing_config_t timing_config;
    memset(&timing_config, 0, sizeof(timing_config));
#if (defined(USE_CANFD) && USE_CANFD)
    if (MCAN_FDCalculateImprovedTimingValues(mcanConfig.baudRateA, mcanConfig.baudRateD, MCAN_CLK_FREQ, &timing_config))
    {
        /* Update the improved timing configuration*/
        memcpy(&(mcanConfig.timingConfig), &timing_config, sizeof(mcan_timing_config_t));
    }
    else
    {
        PRINTF("No found Improved Timing Configuration. use default configuration\r\n\r\n");
    }
#else
    if (MCAN_CalculateImprovedTimingValues(mcanConfig.baudRateA, MCAN_CLK_FREQ, &timing_config))
    {
        /* Update the improved timing configuration*/
        memcpy(&(mcanConfig.timingConfig), &timing_config, sizeof(mcan_timing_config_t));
    }
    else
    {
        PRINTF("No found Improved Timing Configuration. use default configuration\r\n\r\n");
    }
#endif
#endif

    MCAN_Init(EXAMPLE_MCAN, &mcanConfig, MCAN_CLK_FREQ);

    memoryConfig.baseAddr = (uint32_t)msgRam;
    /* STD filter config. */
    rxFilter.address          = STD_FILTER_OFS;
    rxFilter.idFormat         = kMCAN_FrameIDStandard;
    rxFilter.listSize         = 1U;
    rxFilter.nmFrame          = kMCAN_reject0;
    rxFilter.remFrame         = kMCAN_rejectFrame;
    memoryConfig.stdFilterCfg = &rxFilter;
#if (defined(USE_EXT_FILTER) && USE_EXT_FILTER)
    /* EXT filter config. */
    extRxFilter.address       = EXT_FILTER_OFS;
    extRxFilter.idFormat      = kMCAN_FrameIDExtend;
    extRxFilter.listSize      = 1U;
    extRxFilter.nmFrame       = kMCAN_reject0;
    extRxFilter.remFrame      = kMCAN_rejectFrame;
    memoryConfig.extFilterCfg = &extRxFilter;
#endif
    /* RX fifo0 config. */
    rxFifo0.address       = RX_FIFO0_OFS;
    rxFifo0.elementSize   = 1U;
    rxFifo0.watermark     = 0;
    rxFifo0.opmode        = kMCAN_FifoBlocking;
    rxFifo0.datafieldSize = kMCAN_8ByteDatafield;
#if (defined(USE_CANFD) && USE_CANFD)
    rxFifo0.datafieldSize = BYTES_IN_MB;
#endif
    memoryConfig.rxFifo0Cfg = &rxFifo0;
    /* TX buffer config. */
    txBuffer.address       = TX_BUFFER_OFS;
    txBuffer.dedicatedSize = 1U;
    txBuffer.fqSize        = 0;
    txBuffer.datafieldSize = kMCAN_8ByteDatafield;
#if (defined(USE_CANFD) && USE_CANFD)
    txBuffer.datafieldSize = BYTES_IN_MB;
#endif
    memoryConfig.txBufferCfg = &txBuffer;
    /* Set Message RAM config and clear memory to avoid BEU/BEC error. */
    memset((void *)msgRam, 0, MSG_RAM_SIZE * sizeof(uint8_t));
    if (kStatus_Success != MCAN_SetMessageRamConfig(EXAMPLE_MCAN, &memoryConfig))
    {
        PRINTF("MCAN Message RAM configuration failed, please check parameters!\r\n");
        return -1;
    }

    /* Filling Standard ID filter element with Classic filter mode, which only filter matching ID. */
    stdFilter.sfec  = kMCAN_storeinFifo0;
    stdFilter.sft   = kMCAN_classic;
    stdFilter.sfid1 = 0x123U;
    stdFilter.sfid2 = 0x7FFU;
    MCAN_SetSTDFilterElement(EXAMPLE_MCAN, &rxFilter, &stdFilter, 0);
#if (defined(USE_EXT_FILTER) && USE_EXT_FILTER)
    /* Filling extended ID filter element with range filter (0x0 ~ 0x1FFFFFFF) mode, which make it actually accept all
     * extended frame. */
    extFilter.efec  = kMCAN_storeinFifo0;
    extFilter.eft   = kMCAN_range;
    extFilter.efid1 = 0x0U;
    extFilter.efid2 = 0x1FFFFFFFU;
    MCAN_SetEXTFilterElement(EXAMPLE_MCAN, &extRxFilter, &extFilter, 0);
#endif

    /* Enable RX fifo0 new message interrupt using interrupt line 0. */
    MCAN_EnableInterrupts(EXAMPLE_MCAN, 0, CAN_IE_RF0NE_MASK);
    EnableIRQ(CAN0_IRQ0_IRQn);

    /* Config TX frame data. */
    uint8_t cnt = 0;
    for (cnt = 0; cnt < CAN_DATASIZE; cnt++)
    {
        tx_data[cnt] = cnt;
    }
    txFrame.xtd  = kMCAN_FrameIDStandard;
    txFrame.rtr  = kMCAN_FrameTypeData;
    txFrame.fdf  = 0;
    txFrame.brs  = 0;
    txFrame.dlc  = 8U;
    txFrame.id   = 0x123U << STDID_OFFSET;
    txFrame.data = tx_data;
    txFrame.size = CAN_DATASIZE;
#if (defined(USE_CANFD) && USE_CANFD)
    txFrame.fdf = 1;
    txFrame.brs = 1;
    txFrame.dlc = DLC;
#endif
    MCAN_TransferSendBlocking(EXAMPLE_MCAN, 0, &txFrame);

    while (!rxComplete)
    {
    }

    rxFrame.size = CAN_DATASIZE;
    /* After call the API of rMCAN_TransferReceiveFifoNonBlocking success, we can
     * only get a point (rxFrame.data) to the fifo reading entrance.
     * Copy the received frame data from the FIFO by the pointer(rxFrame.data). */
    memcpy(rx_data, rxFrame.data, rxFrame.size);

    PRINTF("Received Frame ID: 0x%x\r\n", rxFrame.id >> STDID_OFFSET);
    PRINTF("Received Frame DATA: ");
    cnt = 0;
    while (cnt < rxFrame.size)
    {
        PRINTF("0x%x ", rx_data[cnt++]);
    }

#if (defined(USE_EXT_FILTER) && USE_EXT_FILTER)
    /* Use Extend frame ID. */
    txFrame.xtd = kMCAN_FrameIDExtend;
    txFrame.id  = 0x12345678U;
    rxComplete  = false;
    MCAN_TransferSendBlocking(EXAMPLE_MCAN, 0, &txFrame);

    while (!rxComplete)
    {
    }

    rxFrame.size = CAN_DATASIZE;
    /* After call the API of rMCAN_TransferReceiveFifoNonBlocking success, we can
     * only get a point (rxFrame.data) to the fifo reading entrance.
     * Copy the received frame data from the FIFO by the pointer(rxFrame.data). */
    memcpy(rx_data, rxFrame.data, rxFrame.size);

    PRINTF("\r\nReceived Extend Frame ID: 0x%x\r\n", rxFrame.id);
    PRINTF("Received Frame DATA: ");
    cnt = 0;
    while (cnt < rxFrame.size)
    {
        PRINTF("0x%x ", rx_data[cnt++]);
    }
#endif
    PRINTF("\r\n\r\n==MCAN loopback functional example -- Finish.==\r\n");

    while (1)
    {
    }
}
