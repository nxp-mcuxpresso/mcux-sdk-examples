/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <xtensa/config/core.h>
#include <xtensa/xos.h>

#include "audio_stream.h"
#include "fsl_debug_console.h"
#include "fsl_dmic.h"
#include "fsl_dmic_dma.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"
#include "fsl_inputmux.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_I2S_MASTER_CLOCK_FREQUENCY CLOCK_GetMclkClkFreq()
#define DEMO_AUDIO_PROTOCOL             kCODEC_BusI2S
#define DEMO_AUDIO_BIT_WIDTH            (16)
#define DEMO_AUDIO_SAMPLE_RATE          (16000)
#define DEMO_I2S_TX                     (I2S3)
#define DEMO_I2S_CLOCK_DIVIDER \
    (24576000U / 16000U / 16U / 1) /* I2S source clock 24.576 MHz, sample rate 16 kHz, bits width 16, 1 channel, \
                                      so bitclock should be 16 kHz * 16 = 256 kHz, divider should be 24.576 MHz / 256 kHz */

#define DEMO_DMA                 (DMA1)
#define DEMO_DMIC_RX_CHANNEL     16U
#define DEMO_I2S_TX_CHANNEL      (7)
#define DEMO_I2S_TX_MODE         kI2S_MasterSlaveNormalMaster
#define DEMO_DMIC_CHANNEL        kDMIC_Channel0
#define DEMO_DMIC_CHANNEL_ENABLE DMIC_CHANEN_EN_CH0(1)
#define FIFO_DEPTH  (15U)
#define BUFFER_SIZE (640 * sizeof(int16_t)) /* Audio buffers for 16 kHz * 640 = 40 ms frames */
#define BUFFER_NUM  (25)         /* Audio buffers for 25 * 40 ms = 1 second of audio */

#define BOARD_XTAL_SYS_CLK_HZ 24000000U /*!< Board xtal_sys frequency in Hz */
#define BOARD_XTAL32K_CLK_HZ  32768U    /*!< Board xtal32K frequency in Hz */

/*******************************************************************************
 * Variables
 ******************************************************************************/

static i2s_config_t s_i2sTxConfig;
static dmic_dma_handle_t s_dmicDmaHandle;
static dma_handle_t s_dmicRxDmaHandle;
static dma_handle_t s_i2sTxDmaHandle;
static i2s_dma_handle_t s_i2sTxHandle;
static i2s_transfer_t s_i2sTxTransfer;
static dmic_transfer_t s_dmicRxTransfer[BUFFER_NUM];

#if (XCHAL_DCACHE_SIZE > 0)
extern int NonCacheable_start, NonCacheable_end;
extern int NonCacheable_init_start, NonCacheable_init_end;

/* Audio buffer located in non-cachable memory block */
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_buffer[BUFFER_SIZE * (BUFFER_NUM + 1)], 4);
AT_NONCACHEABLE_SECTION_ALIGN(static dma_descriptor_t s_dmaDescriptorPingpong[BUFFER_NUM], 16);
#else
SDK_ALIGN(static uint8_t s_buffer[BUFFER_SIZE * (BUFFER_NUM + 1)], 4);
SDK_ALIGN(static dma_descriptor_t s_dmaDescriptorPingpong[BUFFER_NUM], 16);
#endif

typedef struct
{
    int head;
    int tail;
    uint8_t* start;
} audio_queue_t;

/* Audio buffer queue info */
static audio_queue_t s_queue;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void DMIC_Callback(DMIC_Type *base, dmic_dma_handle_t *handle, status_t status, void *userData)
{
    s_i2sTxTransfer.data = s_queue.start + s_queue.head * BUFFER_SIZE;
    s_i2sTxTransfer.dataSize = BUFFER_SIZE;

    if (I2S_TxTransferSendDMA(DEMO_I2S_TX, &s_i2sTxHandle, s_i2sTxTransfer) != kStatus_Success)
    {
        PRINTF("I2S_TxTransferSendDMA failed!\r\n");
    }

    if (s_queue.head == 0)
    {
        /* Prepend last buffer to the queue start to allow continuous processing
           of sliding window up to BUFFER_SIZE */
        memcpy(s_buffer + (BUFFER_SIZE * BUFFER_NUM), s_buffer, BUFFER_SIZE);
    }

    s_queue.head = (s_queue.head + 1) % BUFFER_NUM;

    /* Drop oldest unprocessed buffers when overflowing */
    if (s_queue.head == s_queue.tail)
    {
        s_queue.tail = (s_queue.tail + 1) % BUFFER_NUM;
    }
}

static void I2S_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
}

void AUDIO_Init(void)
{
    dmic_channel_config_t dmicChannelConfig;

#if (XCHAL_DCACHE_SIZE > 0)
    /* Disable DSP cache for non-cacheable sections. */
    xthal_set_region_attribute((uint32_t *)&NonCacheable_start,
        (uint32_t)&NonCacheable_end - (uint32_t)&NonCacheable_start, XCHAL_CA_BYPASS, 0);
    xthal_set_region_attribute((uint32_t *)&NonCacheable_init_start,
        (uint32_t)&NonCacheable_init_end - (uint32_t)&NonCacheable_init_start, XCHAL_CA_BYPASS, 0);
#endif

    xos_set_clock_freq(XOS_CLOCK_FREQ);
    xos_start_system_timer(-1, 0);

    /* Set external XTAL OSC freq */
    CLOCK_SetXtalFreq(BOARD_XTAL_SYS_CLK_HZ);

    CLOCK_EnableClock(kCLOCK_InputMux);

    /* DSP_INT0_SEL18 = DMA1 */
    INPUTMUX_AttachSignal(INPUTMUX, 18U, kINPUTMUX_Dmac1ToDspInterrupt);

    /* Map DMA IRQ handler to INPUTMUX selection DSP_INT0_SEL18
     * EXTINT19 = DSP INT 23 */
    xos_register_interrupt_handler(XCHAL_EXTINT19_NUM, (XosIntFunc *)DMA_IRQHandle, DMA1);
    xos_interrupt_enable(XCHAL_EXTINT19_NUM);

    DMA_Init(DEMO_DMA);
    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_EnableChannel(DEMO_DMA, DEMO_DMIC_RX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_DMIC_RX_CHANNEL, kDMA_ChannelPriority2);

    memset(&dmicChannelConfig, 0U, sizeof(dmic_channel_config_t));

    dmicChannelConfig.divhfclk            = kDMIC_PdmDiv3;
    dmicChannelConfig.osr                 = 32U;
    dmicChannelConfig.gainshft            = 0U;
    dmicChannelConfig.preac2coef          = kDMIC_CompValueZero;
    dmicChannelConfig.preac4coef          = kDMIC_CompValueZero;
    dmicChannelConfig.dc_cut_level        = kDMIC_DcNoRemove;
    dmicChannelConfig.post_dc_gain_reduce = 0U;
    dmicChannelConfig.saturate16bit       = 1U;
    dmicChannelConfig.sample_rate         = kDMIC_PhyFullSpeed;

    DMIC_Init(DMIC0);
#if !(defined(FSL_FEATURE_DMIC_HAS_NO_IOCFG) && FSL_FEATURE_DMIC_HAS_NO_IOCFG)
    DMIC_SetIOCFG(DMIC0, kDMIC_PdmDual);
#endif
    DMIC_Use2fs(DMIC0, true);
    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL, true);
    DMIC_ConfigChannel(DMIC0, DEMO_DMIC_CHANNEL, kDMIC_Left, &dmicChannelConfig);

    DMIC_FifoChannel(DMIC0, DEMO_DMIC_CHANNEL, FIFO_DEPTH, true, true);
    DMIC_EnableChannnel(DMIC0, DEMO_DMIC_CHANNEL_ENABLE);

    xos_start_main("main", 7, 0);

    DMA_CreateHandle(&s_i2sTxDmaHandle, DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_CreateHandle(&s_dmicRxDmaHandle, DEMO_DMA, DEMO_DMIC_RX_CHANNEL);

    I2S_TxGetDefaultConfig(&s_i2sTxConfig);
    s_i2sTxConfig.divider     = DEMO_I2S_CLOCK_DIVIDER;
    s_i2sTxConfig.masterSlave = DEMO_I2S_TX_MODE;
    s_i2sTxConfig.oneChannel  = true;
    I2S_TxInit(DEMO_I2S_TX, &s_i2sTxConfig);
    I2S_TxTransferCreateHandleDMA(DEMO_I2S_TX, &s_i2sTxHandle, &s_i2sTxDmaHandle, I2S_Callback, NULL);

    s_queue.start = s_buffer + BUFFER_SIZE;

    /* Transfer configurations for channel0 */
    for (int i = 0; i < BUFFER_NUM; i++)
    {
        s_dmicRxTransfer[i].data                   = s_queue.start + i * BUFFER_SIZE;
        s_dmicRxTransfer[i].dataWidth              = sizeof(uint16_t);
        s_dmicRxTransfer[i].dataSize               = BUFFER_SIZE;
        s_dmicRxTransfer[i].dataAddrInterleaveSize = kDMA_AddressInterleave1xWidth;
        s_dmicRxTransfer[i].linkTransfer           = &s_dmicRxTransfer[i + 1];
    }
    s_dmicRxTransfer[BUFFER_NUM - 1].linkTransfer = &s_dmicRxTransfer[0];

    DMIC_TransferCreateHandleDMA(DMIC0, &s_dmicDmaHandle, DMIC_Callback, NULL, &s_dmicRxDmaHandle);
    DMIC_InstallDMADescriptorMemory(&s_dmicDmaHandle, s_dmaDescriptorPingpong, BUFFER_NUM);
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle, s_dmicRxTransfer, DEMO_DMIC_CHANNEL);
}

bool AUDIO_GetNextFrame(int16_t** buffer)
{
    if (s_queue.tail != s_queue.head)
    {
        *buffer = (int16_t*)(s_queue.start + s_queue.tail * BUFFER_SIZE);
        s_queue.tail = (s_queue.tail + 1) % BUFFER_NUM;
        return true;
    }
    return false;
}
