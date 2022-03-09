/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdlib.h>
#include <string.h>
#include "pin_mux.h"
#include "board.h"
#include "fsl_codec_common.h"
#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "fsl_dmic.h"
#include "fsl_dmic_dma.h"
#include "fsl_i2c.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"
#include "fsl_inputmux.h"
#include "fsl_power.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "fsl_sx1502.h"
#include <stdbool.h>
#include "fsl_gpio.h"
#include "fsl_codec_adapter.h"
#include "fsl_cs42448.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ENABLE_DMIC_0 1 /* 1: dmic 0 enabled, 0: dmic 0 disabled */
#define DEMO_ENABLE_DMIC_1 1
#define DEMO_ENABLE_DMIC_2 1
#define DEMO_ENABLE_DMIC_3 1

#define DEMO_ENABLE_DMIC_4 1
#define DEMO_ENABLE_DMIC_5 1
#define DEMO_ENABLE_DMIC_6 1
#define DEMO_ENABLE_DMIC_7 1

#define DEMO_I2S_MASTER_CLOCK_FREQUENCY CLOCK_GetMclkClkFreq()
#define DEMO_I2S_TX                     (I2S3)
#define DEMO_I2S_SAMPLE_RATE            48000
#define I2S_CLOCK_DIVIDER               (24576000 / DEMO_I2S_SAMPLE_RATE / 32 / 8)
#define DEMO_DMA_MEMCPY_LEFT_CHANNEL    0U
#define DEMO_DMA_MEMCPY_RIGHT_CHANNEL   1U

#define DEMO_RECORD_PLAYBACK_TIME_MS       10000U
#define DEMO_AUTO_ENTER_SLEEP_MODE_TIME_MS 20000U

#define DEMO_DMA_CHANNEL_TRIGGER_INPUT_A   kINPUTMUX_Dma0TrigOutAToDma0
#define DEMO_DMA_CHANNEL_TRIGGER_OUTPUT_A  kINPUTMUX_Dma0OtrigChannel16ToTriginChannels
#define DEMO_DMA_CHANNEL_OUT_TRIGGER_INDEX 0

#define DEMO_DMA            (DMA0)
#define DEMO_I2S_TX_CHANNEL (7)

#define DEMO_DMIC_DMA_RX_CHANNEL_0 16U
#define DEMO_DMIC_DMA_RX_CHANNEL_1 17U
#define DEMO_DMIC_DMA_RX_CHANNEL_2 18U
#define DEMO_DMIC_DMA_RX_CHANNEL_3 19U
#define DEMO_DMIC_DMA_RX_CHANNEL_4 20U
#define DEMO_DMIC_DMA_RX_CHANNEL_5 21U
#define DEMO_DMIC_DMA_RX_CHANNEL_6 22U
#define DEMO_DMIC_DMA_RX_CHANNEL_7 23U
#define DEMO_DMIC_NUMS             (8U)

#define DEMO_DMA_MEMCPY_CHANNEL_0 0
#define DEMO_DMA_MEMCPY_CHANNEL_1 1

#define DEMO_DMIC_CHANNEL_0       kDMIC_Channel0
#define DEMO_DMIC_CHANNEL_1       kDMIC_Channel1
#define DEMO_DMIC_CHANNEL_2       kDMIC_Channel2
#define DEMO_DMIC_CHANNEL_3       kDMIC_Channel3
#define DEMO_DMIC_CHANNEL_4       kDMIC_Channel4
#define DEMO_DMIC_CHANNEL_5       kDMIC_Channel5
#define DEMO_DMIC_CHANNEL_6       kDMIC_Channel6
#define DEMO_DMIC_CHANNEL_7       kDMIC_Channel7
#define DEMO_CODEC_I2C_BASEADDR   I2C2
#define DEMO_CODEC_I2C_INSTANCE   2U
#define DEMO_CODEC_I2C_CLOCK_FREQ CLOCK_GetFlexCommClkFreq(2U)

#define DEMO_SX1502_I2C_INSTANCE 1
#define FIFO_DEPTH           (15U)
#define PLAYBACK_BUFFER_SIZE (1024)
#define PLAYBACK_BUFFER_NUM  (2U)
#define RECORD_BUFFER_SIZE   (64)
#define RECORD_BUFFER_NUM    (DEMO_DMIC_NUMS * PLAYBACK_BUFFER_NUM)
#if DEMO_ENABLE_DMIC_0
#define DEMO_DMIC_MAX_ID 0
#endif
#if DEMO_ENABLE_DMIC_1
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 1
#endif
#if DEMO_ENABLE_DMIC_2
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 2
#endif
#if DEMO_ENABLE_DMIC_3
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 3
#endif
#if DEMO_ENABLE_DMIC_4
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 4
#endif
#if DEMO_ENABLE_DMIC_5
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 5
#endif
#if DEMO_ENABLE_DMIC_6
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 6
#endif
#if DEMO_ENABLE_DMIC_7
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 7
#endif
#ifndef DEMO_DMIC_MAX_ID
#error "Please enable at least one DMIC"
#endif
/*! @brief Task stack size. */
#define MENU_TASK_STACK_SIZE (1024U)
/*! @brief Task stack priority. */
#define MENU_TASK_PRIORITY (configMAX_PRIORITIES - 1U)

/*! @brief Task stack size. */
#define SX1502_TASK_STACK_SIZE (1024U)
/*! @brief Task stack priority. */
#define SX1502_TASK_PRIORITY (configMAX_PRIORITIES - 2U)

/*! @brief Task stack size. */
#define RECORD_PLAYBACK_TASK_STACK_SIZE (1024U)
/*! @brief Task stack priority. */
#define RECORD_PLAYBACK_TASK_PRIORITY (configMAX_PRIORITIES - 2U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitCodec(void);
void BOARD_InitSX1502(void);
static void i2s_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData);
static void memcpy_channel_callback(struct _dma_handle *handle, void *userData, bool transferDone, uint32_t intmode);
static void DEMO_DMAChannelConfigurations(void);
static void DEMO_PreparingRecordDMIC(void);
static void DEMO_DMICChannelConfigurations(void);
static void ReceiverTransferConfig(dmic_transfer_t *startAddress,
                                   void *dataAddr,
                                   uint32_t xferNum,
                                   uint32_t dataWidth,
                                   uint32_t dataSizePerTransfer,
                                   uint32_t interleaveSize);
extern void BORAD_CodecReset(bool state);
static void DEMO_DMAMemcpyChannelConfigurations(void);
static void dmic_Callback(DMIC_Type *base, dmic_dma_handle_t *handle, status_t status, void *userData);
static void DEMO_I2SConfigurations(void);
static void DEMO_EnterSleepMode(void);
static void DEMO_AbortRecordPlayback(void);
static void MenuTask(void *pvParameters);
static void SX1502Task(void *pvParameters);
static void RecordPlayBackTask(void *pvParameters);
/*******************************************************************************
 * Variables
 ******************************************************************************/
cs42448_config_t cs42448Config = {
    .DACMode      = kCS42448_ModeSlave,
    .ADCMode      = kCS42448_ModeSlave,
    .reset        = NULL,
    .master       = false,
    .i2cConfig    = {.codecI2CInstance = DEMO_CODEC_I2C_INSTANCE},
    .format       = {.sampleRate = 48000U, .bitWidth = 24U},
    .bus          = kCS42448_BusTDM,
    .slaveAddress = CS42448_I2C_ADDR,
};

codec_config_t boardCodecConfig = {.codecDevType = kCODEC_CS42448, .codecDevConfig = &cs42448Config};

sx1502_config_t sx1502Config = {
    .initRegDataValue  = 0xFFU,
    .initRegDirValue   = 0,
    .sx1502I2CInstance = DEMO_SX1502_I2C_INSTANCE,
};

sx1502_handle_t sx1502Handle;
codec_handle_t s_codecHandle;
static i2s_config_t tx_config;
static uint32_t volatile s_RecordEmptyBlock = PLAYBACK_BUFFER_NUM;
/* DMIC dma handle for 8 channel */
static dmic_dma_handle_t s_dmicDmaHandle[DEMO_DMIC_NUMS];
/* dma handle for 8 channel */
static dma_handle_t s_dmaHandle[DEMO_DMIC_NUMS];
/* i2s dma handle */
static dma_handle_t s_i2sTxDmaHandle;
static dma_handle_t s_memcpyDmaHandle0;
static dma_handle_t s_memcpyDmaHandle1;
static i2s_dma_handle_t s_i2sTxHandle;
/* ping pong descriptor */
SDK_ALIGN(dma_descriptor_t s_dmaDescriptorPingpong[RECORD_BUFFER_NUM], 16);
SDK_ALIGN(dma_descriptor_t s_memcpyDescriptor[PLAYBACK_BUFFER_NUM * 2U], 16);
/* dmic transfer configurations */
static dmic_transfer_t s_receiveXfer0[RECORD_BUFFER_NUM / 2];
static dmic_transfer_t s_receiveXfer1[RECORD_BUFFER_NUM / 2];
/* dmic channel configurations */
static dmic_channel_config_t s_dmicChannelConfig = {
    .divhfclk            = kDMIC_PdmDiv1,
    .osr                 = 32U,
    .gainshft            = 3U,
    .preac2coef          = kDMIC_CompValueZero,
    .preac4coef          = kDMIC_CompValueZero,
    .dc_cut_level        = kDMIC_DcCut155,
    .post_dc_gain_reduce = 1U,
    .saturate16bit       = 1U,
    .sample_rate         = kDMIC_PhyFullSpeed,
};

/*!@breif data buffer */
SDK_ALIGN(static uint8_t s_processedAudio[RECORD_BUFFER_SIZE * RECORD_BUFFER_NUM * 4U], 4U);
SDK_ALIGN(static uint8_t s_recordBuffer0[RECORD_BUFFER_SIZE * RECORD_BUFFER_NUM / 2], 4U);
SDK_ALIGN(static uint8_t s_recordBuffer1[RECORD_BUFFER_SIZE * RECORD_BUFFER_NUM / 2], 4U);
static SemaphoreHandle_t s_menuTasksSemaphore      = NULL;
static SemaphoreHandle_t s_recordPlaybackSemaphore = NULL;
static SemaphoreHandle_t s_sx1502TasksSemaphore    = NULL;

static volatile bool s_m2mBufferReady         = false;
static volatile bool s_buffer1Ready           = false;
static volatile bool s_i2sTransferFinish      = false;
static volatile uint8_t s_dmicBufferIndex     = 0U;
static volatile uint8_t s_m2mBufferIndex      = 0U;
static volatile bool s_isDMICTriggerred       = false;
static dma_channel_trigger_t s_channelTrigger = {
    .type  = kDMA_FallingEdgeTrigger,
    .burst = kDMA_SingleTransfer,
    .wrap  = kDMA_NoWrap,
};
extern sx1502_handle_t sx1502Handle;
/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_InitCodec(void)
{
    if (CODEC_Init(&s_codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("CODEC_Init failed!\r\n");
        assert(false);
    }
}

void BOARD_InitSX1502(void)
{
    if (SX1502_Init(&sx1502Handle, &sx1502Config) != kStatus_Success)
    {
        PRINTF("SX1502_Init failed!\r\n");
        assert(false);
    }
}

/*!
 * @brief Main function
 */

int main(void)
{
    /* Board pin, clock, debug console init */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_InputMux);

    /* attach AUDIO PLL clock to FLEXCOMM3 (I2S3) */
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM3);
    /* I2C */
    CLOCK_AttachClk(kFFRO_to_FLEXCOMM2);
    CLOCK_AttachClk(kSFRO_to_FLEXCOMM1);

    /* attach AUDIO PLL clock to MCLK */
    CLOCK_AttachClk(kAUDIO_PLL_to_MCLK_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivMclkClk, 2);
    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;

    CLOCK_AttachClk(kAUDIO_PLL_to_DMIC_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 8);

    cs42448Config.i2cConfig.codecI2CSourceClock = CLOCK_GetFlexCommClkFreq(2);
    cs42448Config.format.mclk_HZ                = CLOCK_GetMclkClkFreq();

    sx1502Config.sx1502I2CSourceClock = CLOCK_GetFlexCommClkFreq(1U);
    BOARD_InitCodec();
    BOARD_InitSX1502();
    DEMO_DMAChannelConfigurations();
    DEMO_DMICChannelConfigurations();
    DEMO_I2SConfigurations();

    PRINTF("\r\ndmic i2s hwvad example.\r\n");

    if (pdPASS != xTaskCreate(MenuTask, "MenuTask", MENU_TASK_STACK_SIZE, NULL, MENU_TASK_PRIORITY, NULL))
    {
        return -1;
    }

    if (pdPASS != xTaskCreate(SX1502Task, "SX1502Task", SX1502_TASK_STACK_SIZE, NULL, SX1502_TASK_PRIORITY, NULL))
    {
        return -1;
    }

    if (pdPASS != xTaskCreate(RecordPlayBackTask, "RecordPlayBackTask", RECORD_PLAYBACK_TASK_STACK_SIZE, NULL,
                              RECORD_PLAYBACK_TASK_PRIORITY, NULL))
    {
        return -1;
    }

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* Scheduler should never reach this point. */
    while (true)
    {
    }
}

static void DMIC0_HWVAD_Callback(void)
{
    volatile int i;

    /* trigger LED  */
    xSemaphoreGiveFromISR(s_sx1502TasksSemaphore, NULL);
    DisableIRQ(HWVAD0_IRQn);

    /* reset hwvad internal interrupt */
    DMIC_CtrlClrIntrHwvad(DMIC0, true);
    /* wait for HWVAD to settle */
    for (i = 0; i <= 500U; i++)
    {
    }
    /*HWVAD Normal operation */
    DMIC_CtrlClrIntrHwvad(DMIC0, false);
}

static void MenuTask(void *pvParameters)
{
    char input;
    uint32_t startTicks       = 0;
    s_menuTasksSemaphore      = xSemaphoreCreateBinary();
    s_recordPlaybackSemaphore = xSemaphoreCreateBinary();
    s_sx1502TasksSemaphore    = xSemaphoreCreateBinary();

    while (1)
    {
        PRINTF("\r\n    1: Record and playback\r\n");
        PRINTF("\r\n    2: Enter sleep mode\r\n");

        startTicks = xTaskGetTickCount();
        do
        {
            if (DbgConsole_TryGetchar(&input) == kStatus_Success)
            {
                if (input == '1')
                {
                    xSemaphoreGive(s_recordPlaybackSemaphore);
                    break;
                }
                else if (input == '2')
                {
                    DEMO_EnterSleepMode();
                    break;
                }
                else
                {
                    PRINTF("\r\nInvalid input, please retry.\r\n");
                    continue;
                }
            }
            else
            {
                /* no input, sysem going to sleep automatically */
                if (xTaskGetTickCount() - startTicks > DEMO_AUTO_ENTER_SLEEP_MODE_TIME_MS)
                {
                    DEMO_EnterSleepMode();
                    break;
                }
            }
        } while (1);

        xSemaphoreTake(s_menuTasksSemaphore, portMAX_DELAY);
    }
}

static void RecordPlayBackTask(void *pvParameters)
{
    uint32_t startTicks = 0U;
    i2s_transfer_t i2sTxTransfer;
    s_recordPlaybackSemaphore = xSemaphoreCreateBinary();

    xSemaphoreGive(s_recordPlaybackSemaphore);

    while (1)
    {
        xSemaphoreTake(s_recordPlaybackSemaphore, portMAX_DELAY);

        s_RecordEmptyBlock = PLAYBACK_BUFFER_NUM;
        s_isDMICTriggerred = false;
        s_m2mBufferReady   = false;
        s_dmicBufferIndex  = 0U;
        s_m2mBufferIndex   = 0U;
        DEMO_PreparingRecordDMIC();
        DEMO_DMAMemcpyChannelConfigurations();

        startTicks = xTaskGetTickCount();

        while (1)
        {
            if (s_RecordEmptyBlock < PLAYBACK_BUFFER_NUM)
            {
                i2sTxTransfer.data     = s_processedAudio + (s_m2mBufferIndex == 0U ? 1U : 0U) * PLAYBACK_BUFFER_SIZE;
                i2sTxTransfer.dataSize = PLAYBACK_BUFFER_SIZE;
                I2S_TxTransferSendDMA(DEMO_I2S_TX, &s_i2sTxHandle, i2sTxTransfer);
            }

            if ((s_isDMICTriggerred) && (s_m2mBufferReady))
            {
                s_isDMICTriggerred = false;
                s_m2mBufferReady   = false;
                DMA_StartTransfer(&s_memcpyDmaHandle0);
            }

            /* only playack 10000 ticks */
            if (xTaskGetTickCount() - startTicks > DEMO_RECORD_PLAYBACK_TIME_MS)
            {
                DEMO_AbortRecordPlayback();
                break;
            }
        }

        /* Acitve menu task */
        xSemaphoreGive(s_menuTasksSemaphore);
    }
}

static void SX1502Task(void *pvParameters)
{
    while (1)
    {
        xSemaphoreTake(s_sx1502TasksSemaphore, portMAX_DELAY);

        SX1502_IO_OutputControl(&sx1502Handle, kSX1502_IO_All, 0x00U);

        vTaskDelay(1000U);

        SX1502_IO_OutputControl(&sx1502Handle, kSX1502_IO_All, 0xFFU);

        /* Acitve menu task */
        xSemaphoreGive(s_menuTasksSemaphore);
    }
}

static void DEMO_EnterSleepMode(void)
{
    uint32_t i = 0;

    PRINTF("\r\nGoing to sleep.\r\n");

    /* reset hwvad internal interrupt */
    DMIC_CtrlClrIntrHwvad(DMIC0, true);
    /* wait for HWVAD to settle */
    for (i = 0; i <= 500U; i++)
    {
    }
    /*HWVAD Normal operation */
    DMIC_CtrlClrIntrHwvad(DMIC0, false);
    NVIC_ClearPendingIRQ(HWVAD0_IRQn);
    /* enable voice detect event */
    EnableIRQ(HWVAD0_IRQn);

    __WFI();
}

static void DEMO_I2SConfigurations(void)
{
    /* i2s configurations */
    /*
     * masterSlave = kI2S_MasterSlaveNormalMaster;
     * mode = kI2S_ModeI2sClassic;
     * rightLow = false;
     * leftJust = false;
     * pdmData = false;
     * sckPol = false;
     * wsPol = false;
     * divider = 1;
     * oneChannel = false;
     * dataLength = 16;
     * frameLength = 32;
     * position = 0;
     * fifoLevel = 4;
     */
    I2S_TxGetDefaultConfig(&tx_config);
    tx_config.divider     = I2S_CLOCK_DIVIDER;
    tx_config.mode        = kI2S_ModeDspWsShort;
    tx_config.wsPol       = true;
    tx_config.dataLength  = 32U;
    tx_config.frameLength = 32 * 8U;

    I2S_TxInit(DEMO_I2S_TX, &tx_config);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel1, false, 64);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel2, false, 128);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel3, false, 192);

    I2S_TxTransferCreateHandleDMA(DEMO_I2S_TX, &s_i2sTxHandle, &s_i2sTxDmaHandle, i2s_Callback, NULL);
}

static void DEMO_AbortRecordPlayback(void)
{
    /* Abort DMIC */
    DMIC_TransferAbortReceiveDMA(DMIC0, &s_dmicDmaHandle[0]);
    DMIC_TransferAbortReceiveDMA(DMIC0, &s_dmicDmaHandle[1]);
    DMIC_TransferAbortReceiveDMA(DMIC0, &s_dmicDmaHandle[2]);
    DMIC_TransferAbortReceiveDMA(DMIC0, &s_dmicDmaHandle[3]);
    DMIC_TransferAbortReceiveDMA(DMIC0, &s_dmicDmaHandle[4]);
    DMIC_TransferAbortReceiveDMA(DMIC0, &s_dmicDmaHandle[5]);
    DMIC_TransferAbortReceiveDMA(DMIC0, &s_dmicDmaHandle[6]);
    DMIC_TransferAbortReceiveDMA(DMIC0, &s_dmicDmaHandle[7]);

    /* Abort I2S */
    I2S_TransferAbortDMA(DEMO_I2S_TX, &s_i2sTxHandle);

    /* Abort M2M channel */
    DMA_AbortTransfer(&s_memcpyDmaHandle0);
    DMA_AbortTransfer(&s_memcpyDmaHandle1);
}

static void DEMO_DMAChannelConfigurations(void)
{
    uint32_t i = 0;

    /* DMA configurations */
    DMA_Init(DEMO_DMA);

    for (i = 0; i < DEMO_DMIC_NUMS; i++)
    {
        DMA_EnableChannel(DEMO_DMA, i + DEMO_DMIC_DMA_RX_CHANNEL_0);
        /* configurations for DMIC channel */
        DMA_SetChannelPriority(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_0 + i, kDMA_ChannelPriority1);
        DMA_CreateHandle(&s_dmaHandle[i], DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_0 + i);
    }

    /* configurations for I2S channel */
    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_CreateHandle(&s_i2sTxDmaHandle, DEMO_DMA, DEMO_I2S_TX_CHANNEL);

    /* configurations for memcpy channel channel */
    DMA_EnableChannel(DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_0);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_0, kDMA_ChannelPriority0);
    DMA_CreateHandle(&s_memcpyDmaHandle0, DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_0);
    DMA_SetCallback(&s_memcpyDmaHandle0, NULL, NULL);

    DMA_EnableChannel(DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_1);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_1, kDMA_ChannelPriority0);
    DMA_CreateHandle(&s_memcpyDmaHandle1, DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_1);
    DMA_SetCallback(&s_memcpyDmaHandle1, memcpy_channel_callback, NULL);
    DMA_SetChannelConfig(DMA0, DEMO_DMA_MEMCPY_CHANNEL_1, &s_channelTrigger, false);

    /* connect two memory to memoery channels, channel0 trigger channel 1 after one descriptor exhaust */
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, DEMO_DMA_MEMCPY_CHANNEL_1, kINPUTMUX_Dma0TrigOutAToDma0);
    INPUTMUX_AttachSignal(INPUTMUX, DEMO_DMA_MEMCPY_CHANNEL_0, kINPUTMUX_Dma0OtrigChannel0ToTriginChannels);

    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_0);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_1);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_2);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_3);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_4);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_5);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_6);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_7);

    /* Only enable the maximum dmic channel's dma interrupt */
#if DEMO_DMIC_MAX_ID == 0
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_0);
#endif
#if DEMO_DMIC_MAX_ID == 1
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_1);
#endif
#if DEMO_DMIC_MAX_ID == 2
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_2);
#endif
#if DEMO_DMIC_MAX_ID == 3
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_3);
#endif
#if DEMO_DMIC_MAX_ID == 4
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_4);
#endif
#if DEMO_DMIC_MAX_ID == 5
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_5);
#endif
#if DEMO_DMIC_MAX_ID == 6
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_6);
#endif
#if DEMO_DMIC_MAX_ID == 7
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_7);
#endif
}

static void DEMO_DMAMemcpyChannelConfigurations(void)
{
    DMA_SetupDescriptor(&(s_memcpyDescriptor[0]),
                        DMA_CHANNEL_XFER(true, true, false, true, 2U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave4xWidth, RECORD_BUFFER_SIZE * sizeof(uint32_t)),
                        s_recordBuffer0, &s_processedAudio[2], &(s_memcpyDescriptor[1]));

    DMA_SetupDescriptor(&(s_memcpyDescriptor[1]),
                        DMA_CHANNEL_XFER(true, true, false, true, 2U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave4xWidth, RECORD_BUFFER_SIZE * sizeof(uint32_t)),
                        &s_recordBuffer0[RECORD_BUFFER_SIZE * sizeof(uint32_t)],
                        &s_processedAudio[RECORD_BUFFER_SIZE * sizeof(uint32_t) * 4U + 2], &(s_memcpyDescriptor[0]));

    /* must make sure the memcpy buffer index is not confilct with the DMIC filling buffer index */
    if (s_dmicBufferIndex == 1U)
    {
        DMA_SubmitChannelDescriptor(&s_memcpyDmaHandle0, &(s_memcpyDescriptor[0]));
    }
    else
    {
        DMA_SubmitChannelDescriptor(&s_memcpyDmaHandle0, &(s_memcpyDescriptor[1]));
    }

    DMA_SetupDescriptor(&(s_memcpyDescriptor[2]),
                        DMA_CHANNEL_XFER(true, true, false, true, 2U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave4xWidth, RECORD_BUFFER_SIZE * sizeof(uint32_t)),
                        s_recordBuffer1, &s_processedAudio[6], &(s_memcpyDescriptor[3]));

    DMA_SetupDescriptor(&(s_memcpyDescriptor[3]),
                        DMA_CHANNEL_XFER(true, true, false, true, 2U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave4xWidth, RECORD_BUFFER_SIZE * sizeof(uint32_t)),
                        &s_recordBuffer1[RECORD_BUFFER_SIZE * sizeof(uint32_t)],
                        &s_processedAudio[RECORD_BUFFER_SIZE * sizeof(uint32_t) * 4U + 6], &(s_memcpyDescriptor[2]));

    /* must make sure the memcpy buffer index is not confilct with the DMIC filling buffer index */
    if (s_dmicBufferIndex == 1U)
    {
        DMA_SubmitChannelDescriptor(&s_memcpyDmaHandle1, &(s_memcpyDescriptor[2]));
    }
    else
    {
        DMA_SubmitChannelDescriptor(&s_memcpyDmaHandle1, &(s_memcpyDescriptor[3]));
    }

    DMA_StartTransfer(&s_memcpyDmaHandle0);
}

static void DEMO_DMICChannelConfigurations(void)
{
    uint32_t i = 0;

    /* dmic channel configurations */
    DMIC_Init(DMIC0);
    DMIC_Use2fs(DMIC0, true);
    bool rightChannel = false;

    for (i = 0U; i < DEMO_DMIC_NUMS; i++)
    {
        if (i % 2)
        {
            rightChannel = true;
        }
        else
        {
            rightChannel = false;
        }

        DMIC_ConfigChannel(DMIC0, (dmic_channel_t)i, (stereo_side_t)rightChannel, &s_dmicChannelConfig);
        DMIC_FifoChannel(DMIC0, i, FIFO_DEPTH, true, true);

        DMIC_TransferCreateHandleDMA(DMIC0, &s_dmicDmaHandle[i], dmic_Callback, NULL, &s_dmaHandle[i]);
        DMIC_InstallDMADescriptorMemory(&s_dmicDmaHandle[i], &s_dmaDescriptorPingpong[i * PLAYBACK_BUFFER_NUM],
                                        PLAYBACK_BUFFER_NUM);
    }

    /*Gain of the noise estimator */
    DMIC_SetGainNoiseEstHwvad(DMIC0, 0x02U);

    /*Gain of the signal estimator */
    DMIC_SetGainSignalEstHwvad(DMIC0, 0x01U);

    /* 00 = first filter by-pass, 01 = hpf_shifter=1, 10 = hpf_shifter=4 */
    DMIC_SetFilterCtrlHwvad(DMIC0, 0x01U);

    /*input right-shift of (GAIN x 2 -10) bits (from -10bits (0000) to +14bits (1100)) */
    DMIC_SetInputGainHwvad(DMIC0, 0x04U);

    DisableDeepSleepIRQ(HWVAD0_IRQn);
    DisableIRQ(HWVAD0_IRQn);

    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH0(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH1(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH2(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH3(1));

    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH4(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH5(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH6(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH7(1));

    DMIC_FilterResetHwvad(DMIC0, true);
    DMIC_FilterResetHwvad(DMIC0, false);
    /* reset hwvad internal interrupt */
    DMIC_CtrlClrIntrHwvad(DMIC0, true);
    /* Delay to clear first spurious interrupt and let the filter converge */
    SDK_DelayAtLeastUs(20000, SystemCoreClock);
    /*HWVAD Normal operation */
    DMIC_CtrlClrIntrHwvad(DMIC0, false);
    DMIC_HwvadEnableIntCallback(DMIC0, DMIC0_HWVAD_Callback);
    EnableDeepSleepIRQ(HWVAD0_IRQn);
    DisableIRQ(HWVAD0_IRQn);
}

static void memcpy_channel_callback(struct _dma_handle *handle, void *userData, bool transferDone, uint32_t intmode)
{
    if (s_RecordEmptyBlock)
    {
        s_RecordEmptyBlock--;
    }
    s_m2mBufferIndex = s_m2mBufferIndex == 0U ? 1U : 0U;
    s_m2mBufferReady = true;
}

static void dmic_Callback(DMIC_Type *base, dmic_dma_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_DMIC_Idle == status)
    {
        s_dmicBufferIndex  = s_dmicBufferIndex == 0U ? 1U : 0U;
        s_isDMICTriggerred = true;
    }
}

static void i2s_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    if (s_RecordEmptyBlock < PLAYBACK_BUFFER_NUM)
    {
        s_RecordEmptyBlock++;
    }
}

static void ReceiverTransferConfig(dmic_transfer_t *startAddress,
                                   void *dataAddr,
                                   uint32_t xferNum,
                                   uint32_t dataWidth,
                                   uint32_t dataSizePerTransfer,
                                   uint32_t interleaveSize)
{
    uint32_t i = 0, offset = 0U;

    for (i = 0; i < xferNum; i += 2)
    {
        startAddress[i].data                   = (uint8_t *)((uint32_t)dataAddr + offset);
        startAddress[i].dataWidth              = dataWidth;
        startAddress[i].dataSize               = dataSizePerTransfer;
        startAddress[i].dataAddrInterleaveSize = interleaveSize;
        startAddress[i].linkTransfer           = &startAddress[i + 1];

        startAddress[i + 1].data      = (uint8_t *)((uint32_t)dataAddr + dataSizePerTransfer * interleaveSize + offset);
        startAddress[i + 1].dataWidth = dataWidth;
        startAddress[i + 1].dataSize  = dataSizePerTransfer;
        startAddress[i + 1].dataAddrInterleaveSize = interleaveSize;
        startAddress[i + 1].linkTransfer           = &startAddress[i];

        offset += dataWidth;
    }
}

static void DEMO_PreparingRecordDMIC(void)
{
    ReceiverTransferConfig(s_receiveXfer0, s_recordBuffer0, RECORD_BUFFER_NUM / 2, sizeof(uint16_t), RECORD_BUFFER_SIZE,
                           kDMA_AddressInterleave4xWidth);
    ReceiverTransferConfig(s_receiveXfer1, s_recordBuffer1, RECORD_BUFFER_NUM / 2, sizeof(uint16_t), RECORD_BUFFER_SIZE,
                           kDMA_AddressInterleave4xWidth);

#if DEMO_ENABLE_DMIC_0
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[0], &s_receiveXfer0[0], DEMO_DMIC_CHANNEL_0);
#endif

#if DEMO_ENABLE_DMIC_1
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[1], &s_receiveXfer0[2], DEMO_DMIC_CHANNEL_1);
#endif

#if DEMO_ENABLE_DMIC_2
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[2], &s_receiveXfer0[4], DEMO_DMIC_CHANNEL_2);
#endif

#if DEMO_ENABLE_DMIC_3
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[3], &s_receiveXfer0[6], DEMO_DMIC_CHANNEL_3);
#endif

#if DEMO_ENABLE_DMIC_4
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[4], &s_receiveXfer1[0], DEMO_DMIC_CHANNEL_4);
#endif

#if DEMO_ENABLE_DMIC_5
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[5], &s_receiveXfer1[2], DEMO_DMIC_CHANNEL_5);
#endif

#if DEMO_ENABLE_DMIC_6
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[6], &s_receiveXfer1[4], DEMO_DMIC_CHANNEL_6);
#endif

#if DEMO_ENABLE_DMIC_7
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[7], &s_receiveXfer1[6], DEMO_DMIC_CHANNEL_7);
#endif
}
