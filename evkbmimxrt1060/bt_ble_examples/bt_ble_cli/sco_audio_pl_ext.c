
/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sco_audio_pl.h"
#include "clock_config.h"
#include "fsl_codec_common.h"
#include "fsl_adapter_audio.h"
#include "fsl_codec_adapter.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "ringtone.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Sco loop back, data from sco input to sco output */
#define SCO_SAI_LOOPBACK 			(0)
/* Codec loop back data from mic to speaker */
#define CODEC_SAI_LOOPBACK 			(0)
#define HFP_CODEC_SAI           	SAI1
#define HFP_CODEC_INSTANCE 			(1U)

#if defined (WIFI_BT_USE_M2_INTERFACE) && defined (CPU_MIMXRT1176DVMAA_cm7)
#define HFP_SCO_INSTANCE   			(3U)
#else
/*RT1060-EVKC uses I2S Lines on M2 Slot, routed to SCO2 Interface*/
#define HFP_SCO_INSTANCE   			(2U)
#endif /*defined (WIFI_BT_USE_M2_INTERFACE)*/

#define OVER_SAMPLE_RATE      		(256U)
#define BUFFER_SIZE      			(1024U)
#define BUFFER_SIZE_INBAND 			(1024U)
#define BUFFER_NUMBER    			(4U)
#define AUDIO_DUMMY_SIZE 			(64U)
/* DMA */
#define EXAMPLE_DMAMUX_INSTANCE      (0U)
#define EXAMPLE_DMA_INSTANCE         (0U)
#define EXAMPLE_MICBUF_TX_CHANNEL    (0U)
#define EXAMPLE_MICBUF_RX_CHANNEL    (1U)
#define EXAMPLE_SPKBUF_TX_CHANNEL    (2U)
#define EXAMPLE_SPKBUF_RX_CHANNEL    (3U)
/* demo audio data channel */
#define DEMO_MICBUF_TX_CHANNEL 		 (kHAL_AudioMono)
#define DEMO_MICBUF_RX_CHANNEL 		 (kHAL_AudioMonoRight)
#define DEMO_SPKBUF_TX_CHANNEL 		 (kHAL_AudioMonoLeft)
#define DEMO_SPKBUF_RX_CHANNEL 		 (kHAL_AudioMono)
#define EXAMPLE_SAI_MICBUF_RX_SOURCE (kDmaRequestMuxSai1Rx)
#define EXAMPLE_SAI_SPKBUF_TX_SOURCE (kDmaRequestMuxSai1Tx)

#if defined(WIFI_BT_USE_M2_INTERFACE) && defined (CPU_MIMXRT1176DVMAA_cm7)
/*SAI3 is used for M2-Slot SCO Connections on RB3/CA2/FC M2 Modules*/
#define EXAMPLE_SAI_SPKBUF_RX_SOURCE (kDmaRequestMuxSai3Rx)
#define EXAMPLE_SAI_MICBUF_TX_SOURCE (kDmaRequestMuxSai3Tx)
#else
/*SAI2 is used for SCO Connections on uSD-m2-adapter and rt1060-evkc-direct-m2*/
#define EXAMPLE_SAI_SPKBUF_RX_SOURCE (kDmaRequestMuxSai2Rx)
#define EXAMPLE_SAI_MICBUF_TX_SOURCE (kDmaRequestMuxSai2Tx)
#endif /*defined(WIFI_BT_USE_M2_INTERFACE)*/

#define HFP_CODEC_HP_VOLUME  		 (70U)  /* Range: 0 ~ 100 */
#if defined(CODEC_WM8962_ENABLE) || defined(CODEC_WM8904_ENABLE)
#define HFP_CODEC_DAC_VOLUME 		 (75U) /* Range: 0 ~ 100 */
#else
#define HFP_CODEC_DAC_VOLUME  		 (100U) /* Range: 0 ~ 100 */
#endif /*defined(CODEC_WM8962_ENABLE) || defined(CODEC_WM8904_ENABLE)*/

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define HFP_LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define HFP_LPI2C_CLOCK_SOURCE_DIVIDER (5U)

#if (defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))
/* Clock pre divider for sai1 clock source */
#define HFP_SAI1_CLOCK_SOURCE_PRE_DIVIDER	 (3U)
/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define HFP_SAI1_CLOCK_SOURCE_SELECT 		 (2U)
/* Clock divider for sai1 clock source */
#define HFP_SAI1_CLOCK_SOURCE_DIVIDER       (31U)
/* Get frequency of sai1 clock */
#define DEMO_SAI_CLK_FREQ                                                        \
    (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (HFP_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / \
     (HFP_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))
#elif defined(CPU_MIMXRT1176DVMAA_cm7)
/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define HFP_SAI1_CLOCK_SOURCE_SELECT 		 (4U)
/* Clock divider for sai1 clock source */
#define HFP_SAI1_CLOCK_SOURCE_DIVIDER       (16U)
/* Get frequency of sai1 clock */
#define DEMO_SAI_CLK_FREQ 	(CLOCK_GetFreq(kCLOCK_AudioPll) / HFP_SAI1_CLOCK_SOURCE_DIVIDER)
#else
#error "codec SAI clock frequency is not supported!"
#endif /*#if (defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))*/

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */


/* --------------------------------------------- Static Global Variables */
static HAL_AUDIO_HANDLE_DEFINE(tx_speaker_handle);
static HAL_AUDIO_HANDLE_DEFINE(rx_mic_handle);
static HAL_AUDIO_HANDLE_DEFINE(tx_mic_handle);
static HAL_AUDIO_HANDLE_DEFINE(rx_speaker_handle);
static OSA_MUTEX_HANDLE_DEFINE(audioInfLock);
static OSA_SEMAPHORE_HANDLE_DEFINE(xSemaphoreScoAudio);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t MicBuffer[BUFFER_NUMBER * BUFFER_SIZE], 4);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t SpeakerBuffer[BUFFER_NUMBER * BUFFER_SIZE], 4);
AT_NONCACHEABLE_SECTION(uint32_t g_AudioTxDummyBuffer[AUDIO_DUMMY_SIZE / 4U]);

static int32_t emptyMicBlock = BUFFER_NUMBER;
static int32_t emptySpeakerBlock = BUFFER_NUMBER;
static volatile uint8_t s_ringTone = 0, s_inband_ringTone= 0;
static volatile uint8_t sco_audio_setup = 0;
static codec_handle_t codec_handle;
static uint32_t txMic_index = 0U, rxMic_index = 0U;
static uint32_t txSpeaker_index = 0U, rxSpeaker_index = 0U;
static uint32_t rxSpeaker_test = 0U, rxMic_test = 0U;
static uint32_t cpy_index = 0U, tx_index = 0U;
static SCO_AUDIO_EP_INFO s_ep_info;

/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM)
 *                              = 24 * (32 + 77/100)
 *                              = 786.48 MHz
 */
const clock_audio_pll_config_t audioCodecPllConfig = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    .numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 100, /* 30 bit denominator of fractional loop divider */
};

#if defined(CODEC_WM8960_ENABLE)
static wm8960_config_t wm896xScoConfig = {
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .route     = kWM8960_RoutePlaybackandRecord,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusPCMB,
    .format           = {.mclk_HZ = 6144000U, .sampleRate = 8000, .bitWidth = kWM8960_AudioBitWidth16bit},
    .master_slave     = false,
};

static wm8960_config_t wm896xScoConfig1 = {
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .route     = kWM8960_RoutePlaybackandRecord,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    .format           = {.mclk_HZ = 6144000U, .sampleRate = 8000, .bitWidth = kWM8960_AudioBitWidth16bit},
    .master_slave     = false,
};

static codec_config_t boardCodecScoConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm896xScoConfig};
static codec_config_t boardCodecScoConfig1 = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm896xScoConfig1};

#elif defined(CODEC_WM8962_ENABLE)
static wm8962_config_t wm8962ScoConfig = {
 .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
 .route =
	 {
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
 .bus          = kWM8962_BusPCMB,
 .format       = {.mclk_HZ = 24576000U, .sampleRate = 8000, .bitWidth = kWM8962_AudioBitWidth16bit},
 .masterSlave  = false,
};
static wm8962_config_t wm8962ScoConfig1 = {
 .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
 .route =
	 {
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
 .format       = {.mclk_HZ = 24576000U, .sampleRate = 8000, .bitWidth = kWM8962_AudioBitWidth16bit},
 .masterSlave  = false,
};

static codec_config_t boardCodecScoConfig = {.codecDevType = kCODEC_WM8962, .codecDevConfig = &wm8962ScoConfig};
static codec_config_t boardCodecScoConfig1 = {.codecDevType = kCODEC_WM8962, .codecDevConfig = &wm8962ScoConfig1};
#else
#error "Supported codec is not defined!"
#endif /*defined(CODEC_WM8960_ENABLE)*/

/*1. Pre-configuring DMA/DMAMUX/HAL_AUDIO for TX-Speaker to Codec SAI*/
static hal_audio_dma_mux_config_t txSpeakerDmaMuxConfig = {
 .dmaMuxConfig.dmaMuxInstance   = EXAMPLE_DMAMUX_INSTANCE,
 .dmaMuxConfig.dmaRequestSource = EXAMPLE_SAI_SPKBUF_TX_SOURCE,
};

static hal_audio_dma_config_t txSpeakerDmaConfig = {
 .instance             = EXAMPLE_DMA_INSTANCE,
 .channel              = EXAMPLE_SPKBUF_TX_CHANNEL,
 .enablePreemption     = false,
 .enablePreemptAbility = false,
 .priority             = kHAL_AudioDmaChannelPriorityDefault,
 .dmaMuxConfig         = (void *)&txSpeakerDmaMuxConfig,
 .dmaChannelMuxConfig  = NULL,
};

static hal_audio_ip_config_t txSpeakerIpConfig = {
 .sai.lineMask = 1U << 0U,
 .sai.syncMode = kHAL_AudioSaiModeAsync,
};

static hal_audio_config_t txSpeakerConfig = {
 .dmaConfig         = &txSpeakerDmaConfig,
 .ipConfig          = (void *)&txSpeakerIpConfig,
 .srcClock_Hz       = 0,
 .sampleRate_Hz     = 0,
 .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(HFP_CODEC_SAI) / 2U,
 .msaterSlave       = kHAL_AudioMaster,
 .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
 .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
 .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
 .lineChannels      = DEMO_SPKBUF_TX_CHANNEL,
 .dataFormat        = kHAL_AudioDataFormatDspModeB,
 .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
 .instance          = HFP_CODEC_INSTANCE,
};

/*2. Pre-configuring DMA/DMAMUX/HAL_AUDIO for RX-Mic from Codec SAI*/
static hal_audio_dma_mux_config_t rxMicDmaMuxConfig = {
 .dmaMuxConfig.dmaMuxInstance   = EXAMPLE_DMAMUX_INSTANCE,
 .dmaMuxConfig.dmaRequestSource = EXAMPLE_SAI_MICBUF_RX_SOURCE,
};

static hal_audio_dma_config_t rxMicDmaConfig = {
 .instance             = EXAMPLE_DMA_INSTANCE,
 .channel              = EXAMPLE_MICBUF_RX_CHANNEL,
 .enablePreemption     = false,
 .enablePreemptAbility = false,
 .priority             = kHAL_AudioDmaChannelPriorityDefault,
 .dmaMuxConfig         = (void *)&rxMicDmaMuxConfig,
 .dmaChannelMuxConfig  = NULL,
};

static hal_audio_ip_config_t rxMicIpConfig = {
 .sai.lineMask = 1U << 0U,
 .sai.syncMode = kHAL_AudioSaiModeSync,
};

static hal_audio_config_t rxMicConfig = {
 .dmaConfig         = &rxMicDmaConfig,
 .ipConfig          = (void *)&rxMicIpConfig,
 .srcClock_Hz       = 0,
 .sampleRate_Hz     = 0,
 .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(HFP_CODEC_SAI) / 2U,
 .msaterSlave       = kHAL_AudioMaster,
 .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
 .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
 .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
 .lineChannels      = DEMO_MICBUF_RX_CHANNEL,
 .dataFormat        = kHAL_AudioDataFormatDspModeB,
 .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
 .instance          = HFP_CODEC_INSTANCE,
};

/*3. Pre-configuring DMA/DMAMUX/HAL_AUDIO for TX-Mic to SCO SAI*/
static hal_audio_dma_mux_config_t txMicDmaMuxConfig = {
 .dmaMuxConfig.dmaMuxInstance   = EXAMPLE_DMAMUX_INSTANCE,
 .dmaMuxConfig.dmaRequestSource = EXAMPLE_SAI_MICBUF_TX_SOURCE,
};

static hal_audio_dma_config_t txMicDmaConfig = {
 .instance             = EXAMPLE_DMA_INSTANCE,
 .channel              = EXAMPLE_MICBUF_TX_CHANNEL,
 .enablePreemption     = false,
 .enablePreemptAbility = false,
 .priority             = kHAL_AudioDmaChannelPriorityDefault,
 .dmaMuxConfig         = (void *)&txMicDmaMuxConfig,
 .dmaChannelMuxConfig  = NULL,
};

static hal_audio_ip_config_t txMicIpConfig = {
 .sai.lineMask = 1U << 0U,
  /*below config is made for EVKB-RT1060*/
 #if defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7)
 .sai.syncMode = kHAL_AudioSaiModeSync,
 #elif defined(CPU_MIMXRT1176DVMAA_cm7) || defined(WIFI_BT_USE_M2_INTERFACE)
  .sai.syncMode = kHAL_AudioSaiModeAsync,
 #endif /*defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7)*/
};

static hal_audio_config_t txMicConfig = {
 .dmaConfig         = &txMicDmaConfig,
 .ipConfig          = (void *)&txMicIpConfig,
 .srcClock_Hz       = 0,
 .sampleRate_Hz     = 0,
 .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(HFP_CODEC_SAI) / 2U,
 .msaterSlave       = kHAL_AudioSlave,
 .bclkPolarity      = kHAL_AudioSampleOnFallingEdge,
 .frameSyncWidth    = kHAL_AudioFrameSyncWidthOneBitClk,
 .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
 .lineChannels      = DEMO_MICBUF_TX_CHANNEL,
 .dataFormat        = kHAL_AudioDataFormatDspModeA,
 .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
 .instance          = HFP_SCO_INSTANCE,
};

/*4. Pre-configuring DMA/DMAMUX/HAL_AUDIO for RX-Speaker from SCO SAI*/
static hal_audio_dma_mux_config_t rxSpeakerDmaMuxConfig = {
 .dmaMuxConfig.dmaMuxInstance   = EXAMPLE_DMAMUX_INSTANCE,
 .dmaMuxConfig.dmaRequestSource = EXAMPLE_SAI_SPKBUF_RX_SOURCE,
};

static hal_audio_dma_config_t rxSpeakerDmaConfig = {
 .instance             = EXAMPLE_DMA_INSTANCE,
 .channel              = EXAMPLE_SPKBUF_RX_CHANNEL,
 .enablePreemption     = false,
 .enablePreemptAbility = false,
 .priority             = kHAL_AudioDmaChannelPriorityDefault,
 .dmaMuxConfig         = (void *)&rxSpeakerDmaMuxConfig,
 .dmaChannelMuxConfig  = NULL,
};

static hal_audio_ip_config_t rxSpeakerIpConfig = {
 .sai.lineMask = 1U << 0U,
 /*below config is made for EVKB-RT1060*/
 #if defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7)
 .sai.syncMode = kHAL_AudioSaiModeAsync,
 #elif defined(CPU_MIMXRT1176DVMAA_cm7) || defined(WIFI_BT_USE_M2_INTERFACE)
  .sai.syncMode = kHAL_AudioSaiModeSync,
 #endif /*defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7)*/
};

static hal_audio_config_t rxSpeakerConfig = {
 .dmaConfig         = &rxSpeakerDmaConfig,
 .ipConfig          = (void *)&rxSpeakerIpConfig,
 .srcClock_Hz       = 0,
 .sampleRate_Hz     = 0,
 .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(HFP_CODEC_SAI) / 2U,
 .msaterSlave       = kHAL_AudioSlave,
 .bclkPolarity      = kHAL_AudioSampleOnFallingEdge,
 .frameSyncWidth    = kHAL_AudioFrameSyncWidthOneBitClk,
 .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
 .lineChannels      = DEMO_SPKBUF_RX_CHANNEL,
 .dataFormat        = kHAL_AudioDataFormatDspModeA,
 .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
 .instance          = HFP_SCO_INSTANCE,
};

/* --------------------------------------------- Internal API Prototypes*/
static API_RESULT audio_setup_pl_ext(uint8_t isRing, SCO_AUDIO_EP_INFO *ep_info);
static uint32_t BOARD_SwitchAudioFreq(uint32_t sampleRate);

/* --------------------------------------------- Functions */
/* overwrite sco_audio_init_pl_ext of sco_audio_pl.c.
 * The follow functions can be overwritten too
 * if the actual example need implement them to
 * use different audio data.
 * sco_audio_init_pl_ext, sco_audio_shutdown_pl_ext,
 * sco_audio_setup_pl_ext, sco_audio_start_pl_ext,
 * sco_audio_stop_pl_ext, sco_audio_write_pl_ext.
 */
/*******************************************************************************
 * Code
 ******************************************************************************/
static uint32_t BOARD_SwitchAudioFreq(uint32_t sampleRate)
{
    CLOCK_DeinitAudioPll();
    printf("BOARD_SwitchAudioFreq(%d)\n", sampleRate);
    if (0U == sampleRate)
    {
        /* Disable MCLK output */
#if (defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))
        IOMUXC_GPR->GPR1 &= (~IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK);
#elif defined(CPU_MIMXRT1176DVMAA_cm7)
        IOMUXC_GPR->GPR0 &= (~IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK);
#endif
    }

    else
    {
        CLOCK_InitAudioPll(&audioCodecPllConfig);
#if (defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))
		/*Clock setting for LPI2C*/
		CLOCK_SetMux(kCLOCK_Lpi2cMux, HFP_LPI2C_CLOCK_SOURCE_SELECT);
		CLOCK_SetDiv(kCLOCK_Lpi2cDiv, HFP_LPI2C_CLOCK_SOURCE_DIVIDER);

		/*Clock setting for SAI1*/
		CLOCK_SetMux(kCLOCK_Sai1Mux, HFP_SAI1_CLOCK_SOURCE_SELECT);
		CLOCK_SetDiv(kCLOCK_Sai1PreDiv, HFP_SAI1_CLOCK_SOURCE_PRE_DIVIDER);
		CLOCK_SetDiv(kCLOCK_Sai1Div, HFP_SAI1_CLOCK_SOURCE_DIVIDER);
		/* Enable MCLK output */
		IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK;
#elif defined(CPU_MIMXRT1176DVMAA_cm7)
		/*Clock setting for LPI2C*/
		CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, 1);

		/*Clock setting for SAI1*/
		CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, HFP_SAI1_CLOCK_SOURCE_SELECT);
		CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, HFP_SAI1_CLOCK_SOURCE_DIVIDER);

		/* Enable MCLK output */
		IOMUXC_GPR->GPR0 |= IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK;
#else
#error "Supported RT platform is not selected!"
#endif /*(defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))*/
    }
    return DEMO_SAI_CLK_FREQ;
}

static void rxMicCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    if (kStatus_HAL_AudioError == completionStatus)
    {
        /* Handle the error. */
    }
    else
    {
        emptyMicBlock--;
        rxMic_test++;
        OSA_SemaphorePost(xSemaphoreScoAudio);
    }
}

#if SCO_SAI_LOOPBACK
static void txMicCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    if (kStatus_HAL_AudioError == completionStatus)
    {
        /* Handle the error. */
    }
    else
    {
#if SCO_SAI_LOOPBACK
        emptySpeakerBlock++;
#else
        emptyMicBlock++;
#endif
        rxMic_test++;
        OSA_SemaphorePost(xSemaphoreScoAudio);
    }
}
#else
static void txMicCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    static volatile uint8_t s_8978ConsumerActualData = 0;
    hal_audio_transfer_t xfer;
    if (s_ringTone == 1U)
    {
        s_8978ConsumerActualData = 0;
        xfer.dataSize            = AUDIO_DUMMY_SIZE;
        xfer.data                = (uint8_t *)&g_AudioTxDummyBuffer[0];
        HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer);
    }
    else
    {
        if (s_8978ConsumerActualData)
        {
            s_8978ConsumerActualData = 0;
            emptyMicBlock++;
            OSA_SemaphorePost(xSemaphoreScoAudio);

            if (emptyMicBlock < (BUFFER_NUMBER))
            {
                s_8978ConsumerActualData = 1;
                xfer.data                = MicBuffer + txMic_index * BUFFER_SIZE;
                xfer.dataSize            = BUFFER_SIZE;
                if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer))
                {
                    txMic_index++;
                }
                if (txMic_index == BUFFER_NUMBER)
                {
                    txMic_index = 0U;
                }
            }
            else
            {
                printf("mic dummy\r\n");
                s_8978ConsumerActualData = 0;
                xfer.dataSize            = AUDIO_DUMMY_SIZE;
                xfer.data                = (uint8_t *)&g_AudioTxDummyBuffer[0];
                HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer);
            }
        }
        else
        {
            if (emptyMicBlock < (BUFFER_NUMBER - 2))
            {
                s_8978ConsumerActualData = 1;
                xfer.data                = MicBuffer + txMic_index * BUFFER_SIZE;
                xfer.dataSize            = BUFFER_SIZE;
                if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer))
                {
                    txMic_index++;
                }
                if (txMic_index == BUFFER_NUMBER)
                {
                    txMic_index = 0U;
                }
            }
            else
            {
                printf("mic dummy\r\n");
                s_8978ConsumerActualData = 0;
                xfer.dataSize            = AUDIO_DUMMY_SIZE;
                xfer.data                = (uint8_t *)&g_AudioTxDummyBuffer[0];
                HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer);
            }
        }
    }
}
#endif /*SCO_SAI_LOOPBACK*/

static void txSpeakerCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    static volatile uint8_t s_consumerActualData = 0;
    hal_audio_transfer_t xfer;
    if (s_ringTone == 1U)
    {
        if ((emptySpeakerBlock > 0U) && (cpy_index < MUSIC_LEN / BUFFER_SIZE))
        {
            /* Fill in the buffers. */
            memcpy((uint8_t *)&SpeakerBuffer[BUFFER_SIZE * (cpy_index % BUFFER_NUMBER)],
                   (uint8_t *)&music[cpy_index * BUFFER_SIZE], sizeof(uint8_t) * BUFFER_SIZE);
            emptySpeakerBlock--;
            cpy_index++;
        }

        if (emptySpeakerBlock < BUFFER_NUMBER)
        {
            /*  xfer structure */
            xfer.data     = (uint8_t *)&SpeakerBuffer[BUFFER_SIZE * (tx_index % BUFFER_NUMBER)];
            xfer.dataSize = BUFFER_SIZE;
            /* Wait for available queue. */
            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer))
            {
                tx_index++;
            }
            emptySpeakerBlock++;
        }
    }
    else
    {
        if (s_consumerActualData)
        {
            s_consumerActualData = 0;
            emptySpeakerBlock++;
            OSA_SemaphorePost(xSemaphoreScoAudio);

            if (emptySpeakerBlock < (BUFFER_NUMBER))
            {
                s_consumerActualData = 1;
                xfer.data            = SpeakerBuffer + txSpeaker_index * BUFFER_SIZE;
                xfer.dataSize        = BUFFER_SIZE;
                if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer))
                {
                    txSpeaker_index++;
                }
                if (txSpeaker_index == BUFFER_NUMBER)
                {
                    txSpeaker_index = 0U;
                }
            }
            else
            {
                s_consumerActualData = 0;
                xfer.dataSize        = AUDIO_DUMMY_SIZE;
                xfer.data            = (uint8_t *)&g_AudioTxDummyBuffer[0];
                HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer);
            }
        }
        else
        {
            if (emptySpeakerBlock < (BUFFER_NUMBER - 2))
            {
                s_consumerActualData = 1;
                xfer.data            = SpeakerBuffer + txSpeaker_index * BUFFER_SIZE;
                xfer.dataSize        = BUFFER_SIZE;
                if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer))
                {
                    txSpeaker_index++;
                }
                if (txSpeaker_index == BUFFER_NUMBER)
                {
                    txSpeaker_index = 0U;
                }
            }
            else
            {
                s_consumerActualData = 0;
                xfer.dataSize        = AUDIO_DUMMY_SIZE;
                xfer.data            = (uint8_t *)&g_AudioTxDummyBuffer[0];
                HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer);
            }
        }
    }
}

static void rxSpeakerCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    if (kStatus_HAL_AudioError == completionStatus)
    {
        /* Handle the error. */
    }
    else
    {
        emptySpeakerBlock--;
        OSA_SemaphorePost(xSemaphoreScoAudio);
        rxSpeaker_test++;
    }
}

static void Deinit_Board_Audio(void)
{
    (void)OSA_MutexLock((osa_mutex_handle_t)audioInfLock, osaWaitForever_c);
    CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
    HAL_AudioTxDeinit((hal_audio_handle_t)&tx_speaker_handle[0]);
    HAL_AudioRxDeinit((hal_audio_handle_t)&rx_mic_handle[0]);
    HAL_AudioTxDeinit((hal_audio_handle_t)&tx_mic_handle[0]);
    HAL_AudioRxDeinit((hal_audio_handle_t)&rx_speaker_handle[0]);
    (void)BOARD_SwitchAudioFreq(0U);
    CODEC_Deinit (&codec_handle);
    (void)OSA_MutexUnlock((osa_mutex_handle_t)audioInfLock);
}

/*Initialize sco audio interface and codec.*/
static void Init_Board_Sco_Audio(uint32_t samplingRate, UCHAR bitWidth)
{
    uint32_t src_clk_hz;

    if (samplingRate > 0U)
    {
        PRINTF("Init Audio SCO SAI and CODEC samplingRate :%d  bitWidth:%d \r\n", samplingRate, bitWidth);
        (void)OSA_MutexLock((osa_mutex_handle_t)audioInfLock, osaWaitForever_c);
        src_clk_hz = BOARD_SwitchAudioFreq(samplingRate);

#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
    (defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
        txSpeakerConfig.srcClock_Hz = OVER_SAMPLE_RATE * samplingRate;
#else
        txSpeakerConfig.srcClock_Hz 	  = src_clk_hz;
#endif

        txSpeakerConfig.sampleRate_Hz     = samplingRate;
        txSpeakerConfig.frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
        txSpeakerConfig.frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
        txSpeakerConfig.lineChannels      = kHAL_AudioMonoLeft;
        txSpeakerConfig.dataFormat        = kHAL_AudioDataFormatDspModeB;

#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
    (defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
        rxMicConfig.srcClock_Hz = OVER_SAMPLE_RATE * samplingRate;
#else
        rxMicConfig.srcClock_Hz   		= src_clk_hz;
#endif
        txMicConfig.srcClock_Hz   		= src_clk_hz;
        rxSpeakerConfig.srcClock_Hz 	= src_clk_hz;
        rxMicConfig.sampleRate_Hz 		= samplingRate;
        txMicConfig.sampleRate_Hz 		= samplingRate;
        rxSpeakerConfig.sampleRate_Hz 	= samplingRate;

        HAL_AudioTxInit((hal_audio_handle_t)&tx_speaker_handle[0], &txSpeakerConfig);
        HAL_AudioRxInit((hal_audio_handle_t)&rx_mic_handle[0], &rxMicConfig);
        HAL_AudioTxInit((hal_audio_handle_t)&tx_mic_handle[0], &txMicConfig);
        HAL_AudioRxInit((hal_audio_handle_t)&rx_speaker_handle[0], &rxSpeakerConfig);

        HAL_AudioTxInstallCallback((hal_audio_handle_t)&tx_speaker_handle[0], txSpeakerCallback, NULL);
        HAL_AudioRxInstallCallback((hal_audio_handle_t)&rx_mic_handle[0], rxMicCallback, NULL);
        HAL_AudioTxInstallCallback((hal_audio_handle_t)&tx_mic_handle[0], txMicCallback, NULL);
        HAL_AudioRxInstallCallback((hal_audio_handle_t)&rx_speaker_handle[0], rxSpeakerCallback, NULL);


        PRINTF("Initialization of codec for SCO\n");
        /* Codec Init*/
        if (CODEC_Init(&codec_handle, &boardCodecScoConfig) != kStatus_Success)
        {
            PRINTF("codec init failed!\r\n");
            assert(0);
        }
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
        CODEC_SetFormat(&codec_handle, txSpeakerConfig.srcClock_Hz, txSpeakerConfig.sampleRate_Hz, txSpeakerConfig.bitWidth);
        CODEC_SetVolume(&codec_handle, kCODEC_VolumeDAC, HFP_CODEC_DAC_VOLUME);
        CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, HFP_CODEC_HP_VOLUME);
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, false);
        (void)OSA_MutexUnlock((osa_mutex_handle_t)audioInfLock);
    }
}

static void Init_Board_RingTone_Audio(uint32_t samplingRate, UCHAR bitWidth)
{
    uint32_t src_clk_hz;

    if (samplingRate > 0U)
    {
        PRINTF("Init Audio CODEC for RingTone\r\n");

        (void)OSA_MutexLock((osa_mutex_handle_t)audioInfLock, osaWaitForever_c);
        src_clk_hz = BOARD_SwitchAudioFreq(samplingRate);

        /* Audio streamer */
        txSpeakerConfig.sampleRate_Hz     = samplingRate;
        txSpeakerConfig.frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
        txSpeakerConfig.frameSyncPolarity = kHAL_AudioBeginAtFallingEdge,
        txSpeakerConfig.lineChannels      = kHAL_AudioStereo;
        txSpeakerConfig.dataFormat        = kHAL_AudioDataFormatI2sClassic;
#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
    (defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
        txSpeakerConfig.srcClock_Hz = OVER_SAMPLE_RATE * samplingRate;
#else
        txSpeakerConfig.srcClock_Hz = src_clk_hz;
#endif
        HAL_AudioTxInit((hal_audio_handle_t)&tx_speaker_handle[0], &txSpeakerConfig);
        HAL_AudioTxInstallCallback((hal_audio_handle_t)&tx_speaker_handle[0], txSpeakerCallback, NULL);

        CODEC_Init(&codec_handle, &boardCodecScoConfig1);
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
        CODEC_SetFormat(&codec_handle, txSpeakerConfig.srcClock_Hz, txSpeakerConfig.sampleRate_Hz, txSpeakerConfig.bitWidth);
        CODEC_SetVolume(&codec_handle, kCODEC_VolumeDAC, HFP_CODEC_DAC_VOLUME);
        CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, HFP_CODEC_HP_VOLUME);
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, false);
        (void)OSA_MutexUnlock((osa_mutex_handle_t)audioInfLock);
    }
}

static API_RESULT audio_setup_pl_ext(uint8_t isRing, SCO_AUDIO_EP_INFO *ep_info)
{
    static uint32_t isLockCreated = 0;

    txMic_index     = 0U;
    rxMic_index     = 0U;
    txSpeaker_index = 0U;
    rxSpeaker_index = 0U;
    emptyMicBlock   = BUFFER_NUMBER;
    emptySpeakerBlock = BUFFER_NUMBER;

#if (defined(CPU_MIMXRT1062DVL6A_cm7))
    BOARD_InitScoPins();
#elif (defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))
    BOARD_InitM2ScoPins();
#endif

	if (isLockCreated == 0)
	{
        OSA_MutexCreate((osa_mutex_handle_t)audioInfLock);
        isLockCreated = 1U;
	}

    if (isRing)
    {
        Init_Board_RingTone_Audio(ep_info->sampl_freq, ep_info->sample_len);
    }
    else
    {
        Init_Board_Sco_Audio(ep_info->sampl_freq, ep_info->sample_len);
    }
    return API_SUCCESS;
}

void SCO_Edma_Task(void *handle)
{
    hal_audio_transfer_t xfer;
    static uint32_t count = 0;

    while (1)
    {
        OSA_SemaphoreWait(xSemaphoreScoAudio, osaWaitForever_c);
        (void)OSA_MutexLock((osa_mutex_handle_t)audioInfLock, osaWaitForever_c);
        if ((s_ringTone == 0) && (sco_audio_setup == 0))
        {
            (void)OSA_MutexUnlock((osa_mutex_handle_t)audioInfLock);
            continue;
        }
        count++;
#ifdef SCO_DEBUG_MSG
        if (count % 300 == 0)
        {
            PRINTF("@(%d  %d)", emptyMicBlock, emptySpeakerBlock);
            PRINTF("#( %d %d)", rxSpeaker_test, rxMic_test);
        }
#endif
#if SCO_SAI_LOOPBACK
        if (emptySpeakerBlock > 0)
        {
            xfer.data     = SpeakerBuffer + rxSpeaker_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;
            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&rx_speaker_handle[0], &xfer))
            {
                rxSpeaker_index++;
            }
            if (rxSpeaker_index == BUFFER_NUMBER)
            {
                rxSpeaker_index = 0U;
            }
        }
        if (emptySpeakerBlock < BUFFER_NUMBER)
        {
            xfer.data     = SpeakerBuffer + txSpeaker_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;
            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer))
            {
                txSpeaker_index++;
            }
            if (txSpeaker_index == BUFFER_NUMBER)
            {
                txSpeaker_index = 0U;
            }
        }
#else
        if (emptyMicBlock > 0U)
        {
            xfer.data     = MicBuffer + rxMic_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;
            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&rx_mic_handle[0], &xfer))
            {
                rxMic_index++;
            }

            if (rxMic_index == BUFFER_NUMBER)
            {
                rxMic_index = 0U;
            }
        }
#if CODEC_SAI_LOOPBACK
        if (emptyMicBlock < BUFFER_NUMBER)
        {
            xfer.data     = MicBuffer + txMic_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;
            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer))
            {
                txMic_index++;
            }
            if (txMic_index == BUFFER_NUMBER)
            {
                txMic_index = 0U;
            }
        }
#else
        if (emptySpeakerBlock > 0U)
        {
            xfer.data     = SpeakerBuffer + rxSpeaker_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;
            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&rx_speaker_handle[0], &xfer))
            {
                rxSpeaker_index++;
            }
            if (rxSpeaker_index == BUFFER_NUMBER)
            {
                rxSpeaker_index = 0U;
            }
        }

#endif /*CODEC_SAI_LOOPBACK*/
#endif /*SCO_SAI_LOOPBACK*/
        (void)OSA_MutexUnlock((osa_mutex_handle_t)audioInfLock);
    }
}

void sco_audio_init_pl_ext(void)
{
    return;
}

void sco_audio_shutdown_pl_ext(void)
{
    return;
}

API_RESULT sco_audio_setup_pl_ext(SCO_AUDIO_EP_INFO *ep_info)
{
    if (sco_audio_setup == 0)
    {
        if (s_ringTone == 0U)
        {
            audio_setup_pl_ext(false, ep_info);
        }
        memcpy(&s_ep_info, ep_info, sizeof(SCO_AUDIO_EP_INFO));
        sco_audio_setup = 1U;
    }

    return API_SUCCESS;
}

API_RESULT sco_audio_start_pl_ext(void)
{
    static uint32_t taskCreated = 0;
    hal_audio_transfer_t xfer;
    BaseType_t result = 0;

    PRINTF("sco_audio_start_pl_ext\r\n");

    if (s_ringTone == 1U)
    {
        return API_SUCCESS;
    }
    if (taskCreated == 0)
    {
        OSA_SemaphoreCreate(xSemaphoreScoAudio, 0);
        result = xTaskCreate(SCO_Edma_Task, "SCO_Edma", 1024, NULL, 5U, NULL);
        assert(pdPASS == result);
        taskCreated = 1U;
        (void)result;
    }

	(void)OSA_MutexLock((osa_mutex_handle_t)audioInfLock, osaWaitForever_c);

    for (uint8_t index = 0; index < BUFFER_NUMBER; ++index)
    {
        xfer.data     = MicBuffer + rxMic_index * BUFFER_SIZE;
        xfer.dataSize = BUFFER_SIZE;
        if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&rx_mic_handle[0], &xfer))
        {
            rxMic_index++;
        }
        if (rxMic_index == BUFFER_NUMBER)
        {
            rxMic_index = 0U;
        }
    }
    emptyMicBlock = BUFFER_NUMBER;

    for (uint8_t index = 0; index < BUFFER_NUMBER; ++index)
    {
        xfer.data     = SpeakerBuffer + rxSpeaker_index * BUFFER_SIZE;
        xfer.dataSize = BUFFER_SIZE;
        if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&rx_speaker_handle[0], &xfer))
        {
            rxSpeaker_index++;
        }
        if (rxSpeaker_index == BUFFER_NUMBER)
        {
            rxSpeaker_index = 0U;
        }
    }
    emptySpeakerBlock = BUFFER_NUMBER;

    /* play dummy data to codec */
    xfer.dataSize = AUDIO_DUMMY_SIZE;
    xfer.data     = (uint8_t *)&g_AudioTxDummyBuffer[0];
    HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer);

    /* play dummy data to 8978 */
    xfer.dataSize = AUDIO_DUMMY_SIZE;
    xfer.data     = (uint8_t *)&g_AudioTxDummyBuffer[0];
    HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer);

    (void)OSA_MutexUnlock((osa_mutex_handle_t)audioInfLock);

    return API_SUCCESS;
}

API_RESULT sco_audio_stop_pl_ext(void)
{
    if (sco_audio_setup == 1)
    {
        Deinit_Board_Audio();
        sco_audio_setup = 0;
    }
    return API_SUCCESS;
}

#ifdef HCI_SCO
void sco_audio_spkr_play_pl_ext(UCHAR *m_data, UINT16 m_datalen)
{
    /* Write to Codec */
}
#endif /* HCI_SCO */

API_RESULT platform_audio_play_inband_ringtone()
{
    cpy_index  = 0;
    tx_index = 0U;
    hal_audio_transfer_t xfer;
    emptyMicBlock = BUFFER_NUMBER;
    if (s_ringTone == 0)
    {
        printf("platform_audio_play_inband_ringtone() %d\n", wavSize);
        if (sco_audio_setup == 1)
        {
            Deinit_Board_Audio();
        }
        audio_setup_pl_ext(false, &s_ep_info);
        s_ringTone = 1U;
        s_inband_ringTone = 1U;
    }

    memset(MicBuffer, 0x0, BUFFER_NUMBER * BUFFER_SIZE);
	if ((emptyMicBlock > 0U) && (cpy_index < wavSize / BUFFER_SIZE_INBAND))
	{
		/* Fill in the buffers. */
		memcpy((uint8_t *)&MicBuffer[BUFFER_SIZE_INBAND * (cpy_index % BUFFER_NUMBER)],
			 (uint8_t *)&wavData[cpy_index * BUFFER_SIZE_INBAND], sizeof(uint8_t) * BUFFER_SIZE_INBAND);
        emptyMicBlock--;
        cpy_index++;
    }
    if (emptyMicBlock < BUFFER_NUMBER)
    {
        (void)OSA_MutexLock((osa_mutex_handle_t)audioInfLock, osaWaitForever_c);
        /*  xfer structure */
        xfer.data     = (uint8_t *)&MicBuffer[BUFFER_SIZE_INBAND * (tx_index % BUFFER_NUMBER)];
        xfer.dataSize = BUFFER_SIZE_INBAND;
        /* Wait for available queue. */
        if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer))
        {
            tx_index++;
        }
        (void)OSA_MutexUnlock((osa_mutex_handle_t)audioInfLock);
        emptyMicBlock++;
    }
    return API_SUCCESS;
}

API_RESULT platform_audio_play_ringtone()
{
    cpy_index = 0;
    tx_index  = 0U;
    hal_audio_transfer_t xfer;
    emptySpeakerBlock = BUFFER_NUMBER;
    if (s_ringTone == 0)
    {
        if (sco_audio_setup == 1)
        {
            Deinit_Board_Audio();
        }

        SCO_AUDIO_EP_INFO ep_info;
        ep_info.sampl_freq = 16000U;
        ep_info.sample_len = 16U;
        audio_setup_pl_ext(true, &ep_info);
        s_ringTone = 1;
    }

    memset(SpeakerBuffer, 0x0, BUFFER_NUMBER * BUFFER_SIZE);
    if ((emptySpeakerBlock > 0U) && (cpy_index < MUSIC_LEN / BUFFER_SIZE))
    {
        /* Fill in the buffers. */
        memcpy((uint8_t *)&SpeakerBuffer[BUFFER_SIZE * (cpy_index % BUFFER_NUMBER)],
               (uint8_t *)&music[cpy_index * BUFFER_SIZE], sizeof(uint8_t) * BUFFER_SIZE);
        emptySpeakerBlock--;
        cpy_index++;
    }
    if (emptySpeakerBlock < BUFFER_NUMBER)
    {
        (void)OSA_MutexLock((osa_mutex_handle_t)audioInfLock, osaWaitForever_c);
        /*  xfer structure */
        xfer.data     = (uint8_t *)&SpeakerBuffer[BUFFER_SIZE * (tx_index % BUFFER_NUMBER)];
        xfer.dataSize = BUFFER_SIZE;
        /* Wait for available queue. */
        if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer))
        {
            tx_index++;
        }
        emptySpeakerBlock++;
        (void)OSA_MutexUnlock((osa_mutex_handle_t)audioInfLock);

    }
    return API_SUCCESS;
}

API_RESULT sco_audio_set_speaker_volume(UCHAR volume)
{
    if (sco_audio_setup == 0)
    {
        return API_FAILURE;
    }
    /* HFP support 0- 15, codec support 0-100*/
    if (kStatus_Success == CODEC_SetVolume(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, ((volume * 6U) + 9U)))
    {
        return API_SUCCESS;
    }
    
    return API_FAILURE;
}

void sco_audio_play_ringtone_pl_ext (void)
{
  platform_audio_play_ringtone();
}

void sco_audio_play_inband_ringtone_pl_ext (void)
{
  platform_audio_play_inband_ringtone();
}

void sco_audio_play_ringtone_exit_pl_ext(void)
{
    if (s_ringTone == 1)
    {
        Deinit_Board_Audio();
        memset(SpeakerBuffer, 0x0, BUFFER_NUMBER * BUFFER_SIZE);
        memset(MicBuffer, 0x0, BUFFER_NUMBER * BUFFER_SIZE);

        s_ringTone = 0U;
        s_inband_ringTone = 0U;
        if (sco_audio_setup == 1)
        {
            printf("***STOP Ring Tone***\n");
            audio_setup_pl_ext(false, &s_ep_info);
            sco_audio_start_pl_ext();
        }
    }
}
