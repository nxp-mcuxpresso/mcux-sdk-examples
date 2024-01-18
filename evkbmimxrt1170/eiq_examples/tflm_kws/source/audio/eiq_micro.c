/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "eiq_micro.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Microphone instance. */
static EIQ_Micro_t micro;
/* Ready callback. */
static EIQ_IBufferAddrUpdater_t readyCallback = NULL;

/* Micro buffer located in noncachable memory block. */
#if !defined(__ARMCC_VERSION)
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t buffer[BUFFER_SIZE * (BUFFER_NUM + 1)], 4);
#else
AT_NONCACHEABLE_SECTION_ALIGN_INIT(static uint8_t buffer[BUFFER_SIZE * (BUFFER_NUM + 1)], 4);
#endif

typedef struct
{
    int head;
    int tail;
    uint8_t* start;
} audio_queue_t;


#ifdef DEMO_CODEC_WM8962
static wm8962_config_t wm8962Config = {
    .i2cConfig = {
        .codecI2CInstance = BOARD_CODEC_I2C_INSTANCE,
        .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ,
    },
    .route = {
        .enableLoopBack            = false,
        .leftInputPGASource        = kWM8962_InputPGASourceInput1,
        .leftInputMixerSource      = kWM8962_InputMixerSourceInputPGA,
        .rightInputPGASource       = kWM8962_InputPGASourceInput3,
        .rightInputMixerSource     = kWM8962_InputMixerSourceInputPGA,
        .leftHeadphoneMixerSource  = kWM8962_OutputMixerDisabled,
        .leftHeadphonePGASource    = kWM8962_OutputPGASourceDAC,
        .rightHeadphoneMixerSource = kWM8962_OutputMixerDisabled,
        .rightHeadphonePGASource   = kWM8962_OutputPGASourceDAC,
    },
    .slaveAddress = WM8962_I2C_ADDR,
    .bus          = kWM8962_BusI2S,
    .format       = {
        .mclk_HZ    = 12288750U,
        .sampleRate = kWM8962_AudioSampleRate16KHz,
        .bitWidth   = kWM8962_AudioBitWidth16bit
    },
    .masterSlave  = false,
};
#else
static wm8960_config_t wm8960Config = {
    .i2cConfig = {
        .codecI2CInstance = BOARD_CODEC_I2C_INSTANCE,
        .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ,
    },
    .route            = kWM8960_RoutePlaybackandRecord,
#if defined( CPU_MIMXRT1176DVMAA_cm7 ) || defined( CPU_MIMXRT1166DVM6A_cm7 ) ||\
    defined( CPU_MIMXRT1166DVM6A_cm4 ) || defined( CPU_MIMXRT1176DVMAA_cm4 )
    .leftInputSource  = kWM8960_InputDifferentialMicInput3,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
#else
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
#endif
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    .format = {
        .mclk_HZ      = 12288750,
        .sampleRate   = kWM8960_AudioSampleRate16KHz,
        .bitWidth     = kWM8960_AudioBitWidth16bit,
    },
    .master_slave = false,
};
#endif

/* Currently used buffer index. */
static audio_queue_t queue;
/* Sai configurations. */
AT_NONCACHEABLE_SECTION_INIT(static sai_edma_handle_t s_micHandle) = {0};
static codec_handle_t codecHandle;
static codec_config_t boardCodecConfig;
static edma_config_t dmaConfig = {0};
static edma_handle_t dmaRxHandle = {0};

static sai_transfer_t xfer = {NULL, 0};

/*!
 * @brief AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM)
 *                              = 24 * (32 + 77/100)
 *                              = 786.48 MHz
 */
static clock_audio_pll_config_t audioPllConfig;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Starts transfer from microphone.
 *
 * This function start transfer from microphone to buffer. Ready handler is called
 * when data are prepared.
 */
static void start(void)
{
    memset(buffer, 0, BUFFER_SIZE * (BUFFER_NUM + 1));
    SAI_RxSoftwareReset(DEMO_SAI, kSAI_ResetTypeSoftware);
    xfer.dataSize = BUFFER_SIZE;
    xfer.data = buffer;
    queue.head = 0;
    queue.tail = 0;

    if (SAI_TransferReceiveEDMA(DEMO_SAI, &s_micHandle, &xfer) != kStatus_Success)
    {
        PRINTF("SAI_TransferReceiveEDMA failed!\r\n");
    }
}

/*!
 * @brief Gets speaker data dimensions.
 *
 * This function gets dimensions of the speaker.
 * width for BUFFER_NUM and height for BUFFER_SIZE
 *
 * @return display dimensions
 */
static Dims_t getResolution(void)
{
    Dims_t dims;
    dims.width = BUFFER_NUM;
    dims.height = BUFFER_SIZE;
    return dims;
}

/*!
 * @brief Notifies microphone.
 *
 * This function notifies microphone driver that data in buffer could be overwritten.
 */
static void notify(void)
{
    if (queue.head == 0)
    {
        /* Prepend last buffer to the queue start to allow continuous processing
           of sliding window up to BUFFER_SIZE */
        memcpy(buffer + (BUFFER_SIZE * BUFFER_NUM), buffer, BUFFER_SIZE);
    }

    queue.head = (queue.head + 1) % BUFFER_NUM;
    xfer.data = queue.start + queue.head * BUFFER_SIZE;

    /* Drop oldest unprocessed buffers when overflowing */
    if (queue.head == queue.tail)
    {
        queue.tail = (queue.tail + 1) % BUFFER_NUM;
    }

    if (SAI_TransferReceiveEDMA(DEMO_SAI, &s_micHandle, &xfer) != kStatus_Success)
    {
        PRINTF("SAI_TransferReceiveEDMA failed!\r\n");
    }
}

/*!
 * @brief Sets ready callback.
 *
 * This function sets external callback which is called when
 * camera buffer is ready.
 *
 * @param updater callback
 */
static void setReadyCallback(EIQ_IBufferAddrUpdater_t updater)
{
    readyCallback = updater;
}

/*!
 * @brief RX callback
 *
 * @param pointer to I2S base address
 * @param pointer to sai edma handler
 * @param status status code
 * @param pointer to user data
 */
static void callback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_SAI_RxError == status)
    {
        PRINTF("SAI_Rx failed!\r\n");
        return;
    }

    uint32_t addr = (uint32_t)xfer.data;
    notify();

    if (readyCallback != NULL)
    {
        readyCallback((uint32_t) addr);
    }
}

/*!
 * @brief Gets ready buffer
 *
 * @return updated buffer with latest data from mic
 */
static uint8_t* getReadyBuff(void)
{
    uint8_t* p = queue.start + queue.tail * BUFFER_SIZE;
    queue.tail = (queue.tail + 1) % BUFFER_NUM;

    return p;
}

/*!
 * @brief Gets ready status
 *
 * @return True if data are ready, otherwise return false
 */
static bool isReady(void)
{
    return queue.tail != queue.head;
}

/*!
 * @brief Enables SAI output Mclk output
 *
 * @param Enables SAI Mclk output flag. Set true for output otherwise input is used
 */
static void AUDIO_EnableSaiMclkOutput(bool enable)
{
#if defined( CPU_MIMXRT1176DVMAA_cm7 ) || defined( CPU_MIMXRT1166DVM6A_cm7 ) ||\
    defined( CPU_MIMXRT1176DVMAA_cm4 ) || defined( CPU_MIMXRT1166DVM6A_cm4 )
  if(enable)
  {
    IOMUXC_GPR->GPR0 |= IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK;
  }
  else
  {
    IOMUXC_GPR->GPR0 &= (~IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK);
  }
#else
  if (enable)
  {
    IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK;
  }
  else
  {
    IOMUXC_GPR->GPR1 &= (~IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK);
  }
#endif
}

/*!
 * @brief Initialize SAI
 *
 */
static void AUDIO_InitClock(void)
{
#if defined( CPU_MIMXRT1176DVMAA_cm7 ) || defined( CPU_MIMXRT1166DVM6A_cm7 ) ||\
    defined( CPU_MIMXRT1176DVMAA_cm4 ) || defined( CPU_MIMXRT1166DVM6A_cm4 )

    /* Clock setting for LPI2C */
    CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, DEMO_LPI2C_CLOCK_SOURCE_SELECT);

    /* Clock setting for SAI1 */
    CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, DEMO_SAI1_CLOCK_SOURCE_SELECT);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, DEMO_SAI1_CLOCK_SOURCE_DIVIDER + 1U);
#else
    /* Clock setting for LPI2C */
    CLOCK_SetMux(kCLOCK_Lpi2cMux, DEMO_LPI2C_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, DEMO_LPI2C_CLOCK_SOURCE_DIVIDER);

    /* Clock setting for SAI1 */
    CLOCK_SetMux(kCLOCK_Sai1Mux, DEMO_SAI1_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Sai1PreDiv, DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER);
    CLOCK_SetDiv(kCLOCK_Sai1Div, DEMO_SAI1_CLOCK_SOURCE_DIVIDER);
#endif
}

/*!
 * @brief Initializes microphone.
 */
static void init(void)
{
    sai_transceiver_t saiConfig;

    audioPllConfig.loopDivider = 32;  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    audioPllConfig.postDivider = 1;   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    audioPllConfig.numerator = 77;    /* 30 bit numerator of fractional loop divider. */
    audioPllConfig.denominator = 100; /* 30 bit denominator of fractional loop divider */

#ifdef DEMO_CODEC_WM8962
    boardCodecConfig.codecDevType = kCODEC_WM8962;
    boardCodecConfig.codecDevConfig = &wm8962Config;
#else
    boardCodecConfig.codecDevType = kCODEC_WM8960;
    boardCodecConfig.codecDevConfig = &wm8960Config;
#endif

    /* Audio PLL clock initialization */
    CLOCK_InitAudioPll(&audioPllConfig);

    /* Enable MCLK clock */
    AUDIO_EnableSaiMclkOutput(true);

    /* Microphone clock init */
    AUDIO_InitClock();

    /* Init DMAMUX */
    DMAMUX_SetSource(DEMO_DMAMUX, DEMO_RX_EDMA_CHANNEL, (uint8_t)DEMO_SAI_RX_SOURCE);
    DMAMUX_EnableChannel(DEMO_DMAMUX, DEMO_RX_EDMA_CHANNEL);

    /* Init DMA and create handle for DMA */
    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DEMO_DMA, &dmaConfig);
    EDMA_CreateHandle(&dmaRxHandle, DEMO_DMA, DEMO_RX_EDMA_CHANNEL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(DEMO_DMA, DEMO_RX_EDMA_CHANNEL, DEMO_SAI_RX_EDMA_CHANNEL);
#endif

    /* SAI init */
    SAI_Init(DEMO_SAI);

    /* Clear RCSR interrupt flags. */
    BOARD_ClearRxInterruptFlags();
    SAI_TransferRxCreateHandleEDMA(DEMO_SAI, &s_micHandle, callback, NULL, &dmaRxHandle);

    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&saiConfig, DEMO_AUDIO_BIT_WIDTH, kSAI_MonoRight, kSAI_Channel0Mask);
    saiConfig.syncMode = kSAI_ModeSync;
    saiConfig.bitClock.bclkPolarity = kSAI_PolarityActiveLow;
    saiConfig.masterSlave = kSAI_Master;
    SAI_TransferRxSetConfigEDMA(DEMO_SAI, &s_micHandle, &saiConfig);

    /* set bit clock divider */
    SAI_RxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);

    /* master clock configurations */
#if (defined(FSL_FEATURE_SAI_HAS_MCR) && (FSL_FEATURE_SAI_HAS_MCR)) || \
    (defined(FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) && (FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER))
#if defined(FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) && (FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER)
    mclkConfig.mclkHz          = DEMO_AUDIO_MASTER_CLOCK;
    mclkConfig.mclkSourceClkHz = DEMO_SAI_CLK_FREQ;
#endif
    SAI_SetMasterClockConfig(DEMO_SAI, &mclkConfig);
#endif

    /* Use default setting to init codec */
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("Error: Could not initialize audio codec! Please, reconnect the board power supply.\r\n");
        for (;;) {}
    }
    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
                        DEMO_CODEC_VOLUME) != kStatus_Success)
    {
        PRINTF("Error: Could not set audio volume!\r\n");
        for (;;) {}
    }
}

/*!
 * @brief Initializes microphone.
 *
 * This function initializes microphone.
 *
 * @return pointer to initialized microphone instance
 */
EIQ_Micro_t* EIQ_MicroInit(void)
{
    micro.base.getResolution = getResolution;
    micro.base.notify = notify;
    micro.base.start = start;
    micro.setReadyCallback = setReadyCallback;
    micro.getReadyBuff = getReadyBuff;
    micro.isReady = isReady;

    queue.start = buffer + BUFFER_SIZE;
    init();

    return &micro;
}
