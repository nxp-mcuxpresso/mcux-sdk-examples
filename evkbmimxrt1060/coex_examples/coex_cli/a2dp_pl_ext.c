/*
 * Copyright 2020-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "a2dp_pl.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_sai.h"
#include "fsl_dmamux.h"
#include "fsl_sai_edma.h"
#include "fsl_codec_common.h"
#include "fsl_adapter_audio.h"
#include "fsl_codec_adapter.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define A2DP_CODEC_SAI      	 	SAI1
#define AUDIO_CODEC_INSTANCE 		(1U)
/* demo audio data channel */
#define A2DP_AUDIO_DATA_CHANNEL 	(kHAL_AudioStereo)
#define A2DP_AUDIO_BIT_WIDTH 		(kHAL_AudioWordWidth16bits)
#define A2DP_AUDIO_SAMPLING_RATE 	(kHAL_AudioSampleRate44100Hz)
#define A2DP_CODEC_DAC_VOLUME 		(100U) /* Range: 0 ~ 100 */
#define A2DP_CODEC_HP_VOLUME  		(70U)  /* Range: 0 ~ 100 */
#define OVER_SAMPLE_RATE     		(256U)

/* DMA */
#define AUDIO_CODEC_DMAMUX_INSTANCE 	(0U)
#define AUDIO_CODEC_DMA_INSTANCE    	(0U)
#define AUDIO_CODEC_TX_CHANNEL      	(0U)
#define AUDIO_CODEC_SAI_TX_SOURCE   	(kDmaRequestMuxSai1Tx)

#if (defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))
/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define A2DP_SAI1_CLOCK_SOURCE_SELECT 		(2U)
#define A2DP_SAI1_CLOCK_SOURCE_PRE_DIVIDER 	(0U)
#define A2DP_SAI1_CLOCK_SOURCE_DIVIDER 		(63U)
/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define A2DP_LPI2C_CLOCK_SOURCE_SELECT 		(0U)
/* Clock divider for master lpi2c clock source */
#define A2DP_LPI2C_CLOCK_SOURCE_DIVIDER 	(5U)
/* Get frequency of lpi2c clock */
#define AUDIO_CODEC_I2C_FREQ ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (A2DP_LPI2C_CLOCK_SOURCE_DIVIDER + 1U))
#define A2DP_AUDIO_SAI_CLK_FREQ                                                        \
    (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (A2DP_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / \
     (A2DP_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))
#elif defined(CPU_MIMXRT1176DVMAA_cm7)
/* Select Audio/Video PLL (393.24 MHz) as sai1 clock source */
#define A2DP_SAI1_CLOCK_SOURCE_SELECT 		(4U)
#define A2DP_SAI1_CLOCK_SOURCE_DIVIDER 		(32U)
#define A2DP_AUDIO_SAI_CLK_FREQ (CLOCK_GetFreq(kCLOCK_AudioPll) / A2DP_SAI1_CLOCK_SOURCE_DIVIDER)
#endif /*(defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))*/

/* --------------------------------------------- Exported Global Variables */
#ifdef A2DP_SOURCE
extern UINT32 a2dp_src_sf;
extern UCHAR a2dp_src_bps;
extern UCHAR a2dp_src_nc;
extern UINT16 a2dp_src_size;
extern void (* a2dp_src_cb)(const UCHAR *data, UINT16 datalen);
extern UCHAR a2dp_src_playback;
extern BT_timer_handle a2dp_src_timer;
void a2dp_pl_start_playback_timer(void);
void a2dp_init_default_src_pl(void);
#endif /* A2DP_SOURCE */

#ifdef A2DP_SINK
extern UINT32 a2dp_snk_sf;
extern UCHAR  a2dp_snk_bps;
extern UCHAR  a2dp_snk_nc;
extern UINT16 a2dp_snk_size;
extern void (* a2dp_snk_cb)(const UCHAR *data, UINT16 datalen);
extern osa_semaphore_handle_t xSemaphoreAudio;
#endif /* A2DP_SINK */
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void txCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam);
/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_ALIGN(static HAL_AUDIO_HANDLE_DEFINE(a2dp_audio_tx_handle), 4);
static codec_handle_t codec_handle;
static uint32_t audioStart;
static uint32_t audiosetup;

#if defined(CODEC_WM8960_ENABLE)
wm8960_config_t wm8960Config = {
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .route     = kWM8960_RoutePlaybackandRecord,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    .format           = {.mclk_HZ    = 11289750U,
                         .sampleRate = kWM8960_AudioSampleRate44100Hz,
                         .bitWidth   = kWM8960_AudioBitWidth16bit},
    .master_slave     = false,
};
static codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm8960Config};
#elif defined(CODEC_WM8962_ENABLE)
wm8962_config_t wm8962Config = {
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
    .format       = {.mclk_HZ    = 11289600U,
               .sampleRate = kWM8962_AudioSampleRate44100Hz,
               .bitWidth   = kWM8962_AudioBitWidth16bit},
    .masterSlave  = false,
};
static codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8962, .codecDevConfig = &wm8962Config};
#endif /*defined(CODEC_WM8960_ENABLE)*/

/*setting for 44.1Khz*/
/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM) / (2^POST)
 *                              = 24 * (30 + 1056/10000)  / 2
 *                              = 361.267MHZ
 */
const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 30,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    .numerator   = 1056,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 10000, /* 30 bit denominator of fractional loop divider */
};

/*setting for multiple of 8Khz,such as 48Khz/16Khz/32KHz*/
const clock_audio_pll_config_t audioPllConfig1 = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    .numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 100, /* 30 bit denominator of fractional loop divider */
};

static hal_audio_dma_mux_config_t audioTxDmaMuxConfig = {
    .dmaMuxConfig.dmaMuxInstance   = AUDIO_CODEC_DMAMUX_INSTANCE,
    .dmaMuxConfig.dmaRequestSource = AUDIO_CODEC_SAI_TX_SOURCE,
};

static hal_audio_dma_config_t audioTxDmaConfig = {
    .instance             = AUDIO_CODEC_DMA_INSTANCE,
    .channel              = AUDIO_CODEC_TX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = (void *)&audioTxDmaMuxConfig,
    .dmaChannelMuxConfig  = NULL,
};

static hal_audio_ip_config_t audioTxIpConfig = {
    .sai.lineMask = 1U << 0U,
    .sai.syncMode = kHAL_AudioSaiModeAsync,
};

static hal_audio_config_t audioTxConfig = {
    .dmaConfig         = &audioTxDmaConfig,
    .ipConfig          = (void *)&audioTxIpConfig,
    .srcClock_Hz       = 11289600U,
    .sampleRate_Hz     = (uint32_t)A2DP_AUDIO_SAMPLING_RATE,
    .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(A2DP_CODEC_SAI) / 2U,
    .masterSlave       = kHAL_AudioMaster,
    .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    .frameSyncPolarity = kHAL_AudioBeginAtFallingEdge,
    .lineChannels      = A2DP_AUDIO_DATA_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatI2sClassic,
    .bitWidth          = (uint8_t)A2DP_AUDIO_BIT_WIDTH,
    .instance          = AUDIO_CODEC_INSTANCE,
};
/*******************************************************************************
 * Code
 ******************************************************************************/

/* overwrite a2dp_init_pl_ext of a2dp_pl.c.
 * The follow functions can be overwritten too
 * if the actual example need implement them to
 * use different audio data.
 * a2dp_init_pl_ext, a2dp_shutdown_pl_ext,
 * a2dp_setup_pl_ext, a2dp_start_pl_ext,
 * a2dp_stop_pl_ext, a2dp_write_pl_ext.
 */
static uint32_t BOARD_SwitchAudioFreq(uint32_t sampleRate)
{
    CLOCK_DeinitAudioPll();

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
        if (44100 == sampleRate)
        {
            CLOCK_InitAudioPll(&audioPllConfig);
        }
        else if (0U == sampleRate % 8000)
        {
            CLOCK_InitAudioPll(&audioPllConfig1);
        }
        else
        {
            PRINTF("BOARD_SwitchAudioFreq failed!(%d)\r\n", sampleRate);
        }

#if (defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))
		 /*Clock setting for LPI2C*/
		 CLOCK_SetMux(kCLOCK_Lpi2cMux, A2DP_LPI2C_CLOCK_SOURCE_SELECT);
		 CLOCK_SetDiv(kCLOCK_Lpi2cDiv, A2DP_LPI2C_CLOCK_SOURCE_DIVIDER);

		 /*Clock setting for SAI1*/
		 CLOCK_SetMux(kCLOCK_Sai1Mux, A2DP_SAI1_CLOCK_SOURCE_SELECT);
		 CLOCK_SetDiv(kCLOCK_Sai1PreDiv, A2DP_SAI1_CLOCK_SOURCE_PRE_DIVIDER);
		 CLOCK_SetDiv(kCLOCK_Sai1Div, A2DP_SAI1_CLOCK_SOURCE_DIVIDER);
		 /* Enable MCLK output */
		 IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK;
#elif defined(CPU_MIMXRT1176DVMAA_cm7)
		 /*Clock setting for LPI2C*/
		 CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, 1);

		 /*Clock setting for SAI1*/
		 CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, A2DP_SAI1_CLOCK_SOURCE_SELECT);
		 CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, A2DP_SAI1_CLOCK_SOURCE_DIVIDER);

		 wm8962Config.format.sampleRate             = sampleRate;
		 wm8962Config.format.mclk_HZ                = A2DP_AUDIO_SAI_CLK_FREQ;
		 /* Enable MCLK output */
		 IOMUXC_GPR->GPR0 |= IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK;
#endif
    }

    return A2DP_AUDIO_SAI_CLK_FREQ;
}

static void txCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
#ifdef A2DP_SINK
    OSA_SemaphorePost(xSemaphoreAudio);
#endif
}

static void SAI_Codec_Start(UCHAR ep)
{
    int ret = 0;
#ifdef A2DP_SINK
    audioTxConfig.sampleRate_Hz  = a2dp_snk_sf;
#endif
    audioTxConfig.lineChannels 	 = A2DP_AUDIO_DATA_CHANNEL;
    audioTxConfig.srcClock_Hz    = BOARD_SwitchAudioFreq(audioTxConfig.sampleRate_Hz);
#if (defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))
    PRINTF("audio-init, Fs:%dHz,MCLK:%dHz, PLL-CLK:%dHz\r\n", audioTxConfig.sampleRate_Hz, audioTxConfig.srcClock_Hz,CLOCK_GetFreq(kCLOCK_AudioPllClk));
#elif defined(CPU_MIMXRT1176DVMAA_cm7)
    PRINTF("audio-init, Fs:%dHz,MCLK:%dHz, PLL-CLK:%dHz\r\n", audioTxConfig.sampleRate_Hz, audioTxConfig.srcClock_Hz,CLOCK_GetFreq(kCLOCK_AudioPll));
#endif

    if ((ret == 0) && (HAL_AudioTxInit((hal_audio_handle_t)&a2dp_audio_tx_handle[0], &audioTxConfig) != kStatus_HAL_AudioSuccess))
    {
        PRINTF("HAL_AudioTxInit failed\r\n");
        ret = -1;
    }

    if ((ret == 0) && (HAL_AudioTxInstallCallback((hal_audio_handle_t)&a2dp_audio_tx_handle[0], txCallback, NULL) != kStatus_HAL_AudioSuccess))
    {
        PRINTF("HAL_AudioTxInstallCallback failed\r\n");
        ret = -1;
    }

    if ((ret == 0) && (CODEC_Init(&codec_handle, &boardCodecConfig) != kStatus_Success))
    {
        PRINTF("CODEC_Init failed\r\n");
        ret = -1;
    }

    if ((ret == 0) && (CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true) != kStatus_Success))
    {
        PRINTF("CODEC_SetMute(true) failed\r\n");
        ret = -1;
    }

    if ((ret == 0) && (CODEC_SetFormat(&codec_handle, audioTxConfig.srcClock_Hz, audioTxConfig.sampleRate_Hz, audioTxConfig.bitWidth) != kStatus_Success))
    {
        PRINTF("CODEC_SetFormat failed\r\n");
        ret = -1;
    }

    if ((ret == 0) && (CODEC_SetVolume(&codec_handle, kCODEC_VolumeDAC, A2DP_CODEC_DAC_VOLUME) != kStatus_Success))
    {
        PRINTF("CODEC_SetVolume(DAC) failed\r\n");
        ret = -1;
    }

    if ((ret == 0) && (CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, A2DP_CODEC_HP_VOLUME) != kStatus_Success))
    {
        PRINTF("CODEC_SetVolume(L|R) failed\r\n");
        ret = -1;
    }

    if ((ret == 0) && (CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, false) != kStatus_Success))
    {
        PRINTF("CODEC_SetMute(false) failed\r\n");
        ret = -1;
    }

    if (ret == -1)
    {
        /*codec or sai has failed, stop here*/
        assert(0);
    }
}

void a2dp_init_pl_ext (UCHAR role)
{
#ifdef A2DP_SOURCE
    if (A2DP_EP_SOURCE == role)
    {
        a2dp_init_default_src_pl();
    }
#endif /* A2DP_SOURCE */
}

void a2dp_shutdown_pl_ext ()
{
#ifdef A2DP_SINK
    if(audiosetup == 1)
    {
    	audiosetup = 0;
	    audioStart = 0;
    	/* Stop Audio Player */
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
        HAL_AudioTxDeinit((hal_audio_handle_t)&a2dp_audio_tx_handle[0]);
        (void)BOARD_SwitchAudioFreq(0U);
    }
#endif /* A2DP_SINK */
}

API_RESULT a2dp_setup_pl_ext
     (
         UCHAR ep,
         void (* ep_cb)(const UCHAR *data, UINT16 datalen),
         UINT16 sf,
         UCHAR bps,
         UCHAR nc,
         UINT16 size
     )
{
#ifdef A2DP_SOURCE
    if (A2DP_EP_SOURCE == ep)
    {
        a2dp_src_cb = ep_cb;
        a2dp_src_sf = sf;
        a2dp_src_bps = bps;
        a2dp_src_nc = nc;
        a2dp_src_size = size;
    }
#endif /* A2DP_SOURCE */

#ifdef A2DP_SINK
    if (A2DP_EP_SINK == ep)
    {
        a2dp_snk_cb = ep_cb;
        a2dp_snk_sf = sf;
        a2dp_snk_bps = bps;
        a2dp_snk_nc = nc;
        a2dp_snk_size = size;
        /* Configuring CODEC when received configuration request*/
        SAI_Codec_Start(ep);
        audiosetup = 1;
    }
#endif /* A2DP_SINK */

    return API_SUCCESS;
}

API_RESULT a2dp_start_pl_ext (UCHAR ep)
{
#ifdef A2DP_SOURCE
    if (A2DP_EP_SOURCE == ep)
    {
        /* Start Audio Source */
        a2dp_src_timer = BT_TIMER_HANDLE_INIT_VAL;
        a2dp_pl_start_playback_timer();
        a2dp_src_playback = BT_TRUE;
    }
#endif /* A2DP_SOURCE */

#ifdef A2DP_SINK
    if (A2DP_EP_SINK == ep)
    {
        /* Start Audio Player */
        audioStart = 1;
    }
#endif /* A2DP_SINK */

    return API_SUCCESS;
}

API_RESULT a2dp_stop_pl_ext (UCHAR ep)
{
#ifdef A2DP_SOURCE
    if (A2DP_EP_SOURCE == ep)
    {
        /* Stop Audio Source */
        a2dp_src_playback = BT_FALSE;
        BT_stop_timer(a2dp_src_timer);
        a2dp_src_timer = BT_TIMER_HANDLE_INIT_VAL;
    }
#endif /* A2DP_SOURCE */

#ifdef A2DP_SINK
    if (A2DP_EP_SINK == ep)
    {
        if (audioStart == 1)
        {
            audioStart = 0;
            /* Stop Audio Player */
            HAL_AudioTransferAbortSend((hal_audio_handle_t)&a2dp_audio_tx_handle[0]);
        }
    }
#endif /* A2DP_SINK */

    return API_SUCCESS;
}

void a2dp_write_pl_ext (UCHAR ep, UCHAR * m_data, UINT16 m_datalen)
{
#ifdef A2DP_SINK
	hal_audio_status_t ret = kStatus_HAL_AudioSuccess;
    hal_audio_transfer_t xfer;

    if ((A2DP_EP_SINK != ep) || (0 == audioStart) || (m_data == NULL) || (m_datalen == 0))
    {
        return;
    }

    xfer.dataSize = m_datalen;
    xfer.data     = m_data;

    if (kStatus_HAL_AudioSuccess != (ret = HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&a2dp_audio_tx_handle[0], &xfer)))
    {
        PRINTF("audio-sai-tx fail, %d\r\n",ret);
    }

    if (ret == kStatus_HAL_AudioQueueFull)
    {
        /*reinit audio interface*/
        PRINTF("reinitializing audio-interface\r\n");
        HAL_AudioTxDeinit ((hal_audio_handle_t)&a2dp_audio_tx_handle[0]);
        HAL_AudioTxInit((hal_audio_handle_t)&a2dp_audio_tx_handle[0], &audioTxConfig);
        HAL_AudioTxInstallCallback((hal_audio_handle_t)&a2dp_audio_tx_handle[0], txCallback, NULL);
        OSA_SemaphorePost(xSemaphoreAudio);
    }
#endif /* A2DP_SINK */
}
