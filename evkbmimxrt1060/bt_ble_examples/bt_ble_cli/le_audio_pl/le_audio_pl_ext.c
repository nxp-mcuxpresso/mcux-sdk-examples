/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if (LE_AUDIO_SRC_SYNC_ENABLE == 1U) && (LE_AUDIO_SINK_SYNC_ENABLE == 1U)

#include "audio_pl.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_sai.h"
#include "fsl_dmamux.h"
#include "fsl_sai_edma.h"
#include "fsl_codec_common.h"
#include "fsl_adapter_audio.h"
#include "fsl_codec_adapter.h"
#include "fsl_adapter_gpio.h"
#if defined (LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)
#include "fsl_gpt.h"
#endif /*defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)*/
#include "fsl_iomuxc.h"
#include "BT_common.h"
#include "BT_status.h"
#include "leaudio_pl.h"
#include "BT_fops.h"
#include "wav.h"
#include "appl_ga_utils.h"
#include "appl_ga_utils_audio_snk.h"
#include "appl_ga_utils_audio_src.h"
#include "le_audio_pl_sync.h"
#include "le_audio_pl_i2s.h"
#include "pin_mux.h"
#include "BT_hci_api.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define LE_AUDIO_SRC_MEDIA_FILE_ROOT_DIR \
     BT_FOPS_PATH_JOIN \
     ( \
         BT_FOPS_BASE, BT_FOPS_PATH_SEP "ga" \
     )

/*max audio configuration supported for le-audio source and sink*/
#define LE_AUDIO_MAX_SAMPLE_RATE			48000U
#define LE_AUDIO_MAX_CHAN_COUNT				2U
#define LE_AUDIO_MAX_BPS 					2U
#define LE_AUDIO_MAX_SAMPLES_PER_FRAME		(LE_AUDIO_MAX_SAMPLE_RATE / 100U * LE_AUDIO_MAX_BPS * LE_AUDIO_MAX_CHAN_COUNT)

/*sink and source task stack and prio*/
#define LE_AUDIO_SRC_TX_TASK_PRIO	 		(BT_TASK_PRIORITY - 2U)
#define LE_AUDIO_SRC_TX_TASK_STACK	 		BT_TASK_STACK_DEPTH
#define LE_AUDIO_SNK_TASK_PRIO				(BT_TASK_PRIORITY - 2U)
#define LE_AUDIO_SNK_TASK_STACK				BT_TASK_STACK_DEPTH


/* For source, allowable transmission window between 4 to 10 */
#define LE_AUDIO_SRC_ISO_QUEUE_MIN_WIN		3U
#define LE_AUDIO_SRC_ISO_QUEUE_MAX_WIN		7U
#define LE_AUDIO_SRC_TX_QUEUE_FLOW_ON		1U
#define LE_AUDIO_SRC_TX_QUEUE_FLOW_OFF		0U

/*buffer length calculated for source and sink*/
#define LE_AUDIO_SRC_BUF_LEN				(LE_AUDIO_MAX_SAMPLES_PER_FRAME * 1U)
#define LE_AUDIO_SNK_PCM_QUEUE_SIZE			(LE_AUDIO_MAX_SAMPLES_PER_FRAME * 30U)
#define LE_AUDIO_SNK_SKIP_FRAME_CNT			4U

/*SAI buffers to tx/rx data over SAI through EDMA*/
#define LE_AUDIO_SNK_MAX_SAI_QUEUE_CNT		1U
#define LE_AUDIO_MAX_SAI_QUEUE_LEN			1//(LE_AUDIO_MAX_SAMPLES_PER_FRAME * LE_AUDIO_SNK_MAX_SAI_QUEUE_CNT)


/*HW CODEC Config*/
#define LE_AUDIO_CODEC_SAI      	 		SAI1
#define LE_AUDIO_CODEC_INSTANCE 			(1U)
#define LE_AUDIO_DATA_CHANNEL 				(kHAL_AudioMonoLeft)
#define LE_AUDIO_RXDATA_CHANNEL 			(kHAL_AudioMonoRight)
#define LE_AUDIO_BIT_WIDTH 					(kHAL_AudioWordWidth16bits)
#define LE_AUDIO_SAMPLING_RATE 				(kHAL_AudioSampleRate48KHz)
#define LE_AUDIO_CODEC_DAC_VOLUME 			(80) /* Range: 0 ~ 100 */
//#define LE_AUDIO_CODEC_HP_VOLUME  			(70U)  /* Range: 0 ~ 100 */
#define LE_AUDIO_CODEC_HP_VOLUME  			(80)  /* Range: 0 ~ 100 */

/* SAI DMA/DMAMUX Config */
#define LE_AUDIO_CODEC_DMAMUX_INSTANCE 		(0U)
#define LE_AUDIO_CODEC_DMA_INSTANCE    		(0U)
#define LE_AUDIO_CODEC_TX_CHANNEL      		(0U)
#define LE_AUDIO_CODEC_SAI_TX_SOURCE   		(kDmaRequestMuxSai1Tx)
#define LE_AUDIO_CODEC_RX_CHANNEL      		(1U)
#define LE_AUDIO_CODEC_SAI_RX_SOURCE   		(kDmaRequestMuxSai1Rx)

#if (defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))
/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define LE_AUDIO_SAI1_CLOCK_SOURCE_SELECT 		(2U)
#define LE_AUDIO_SAI1_CLOCK_SOURCE_PRE_DIVIDER 	(0U)
#define LE_AUDIO_SAI1_CLOCK_SOURCE_DIVIDER 		(63U)
/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LE_AUDIO_LPI2C_CLOCK_SOURCE_SELECT 		(0U)
/* Clock divider for master lpi2c clock source */
#define LE_AUDIO_LPI2C_CLOCK_SOURCE_DIVIDER 	(5U)
/* Get frequency of lpi2c clock */
#define LE_AUDIO_CODEC_I2C_FREQ ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LE_AUDIO_LPI2C_CLOCK_SOURCE_DIVIDER + 1U))
#define LE_AUDIO_SAI_CLK_FREQ                                                        \
    (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (LE_AUDIO_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / \
     (LE_AUDIO_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))
#elif defined(CPU_MIMXRT1176DVMAA_cm7)
/* Select Audio/Video PLL (393.24 MHz) as sai1 clock source */
#define LE_AUDIO_SAI1_CLOCK_SOURCE_SELECT 		(kCLOCK_SAI1_ClockRoot_MuxAudioPllOut)
#define LE_AUDIO_SAI1_CLOCK_SOURCE_DIVIDER 		(16U)
#define LE_AUDIO_SAI_CLK_FREQ (CLOCK_GetFreq(kCLOCK_AudioPll) / LE_AUDIO_SAI1_CLOCK_SOURCE_DIVIDER)
#endif /*(defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))*/

/* GPT - SyncTimer */

#if defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)
#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2)
#define SyncTimer_GPT                (GPT2)
#define SyncTimer_GPT_Irq            (GPT2_IRQn)
#define SyncTimer_GPT_ClockRoot      (kCLOCK_Root_Gpt2)
#define SyncTimer_GPT_ClockRoot_Mux  (kCLOCK_GPT2_ClockRoot_MuxAudioPllOut)
#define SyncTimer_GPT_ClockRoot_Div  (16U)
#elif defined(WIFI_IW612_BOARD_RD_USD)
#define SyncTimer_GPT                (GPT3)
#define SyncTimer_GPT_Irq            (GPT3_IRQn)
#define SyncTimer_GPT_ClockRoot      (kCLOCK_Root_Gpt3)
#define SyncTimer_GPT_ClockRoot_Mux  (kCLOCK_GPT3_ClockRoot_MuxAudioPllOut)
#define SyncTimer_GPT_ClockRoot_Div  (16U)
#endif /*defined(WIFI_IW612_BOARD_MURATA_2EL_M2)*/
#endif /*defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)*/
/* --------------------------------------------- typedef declarations */
typedef enum {
	LE_AUDIO_SRC_ROLE_MEDIA,
	LE_AUDIO_SRC_ROLE_VOICE,
	LE_AUDIO_SRC_ROLE_UNKNOWN
}le_audio_src_role_e;

typedef struct {
	OSA_SEMAPHORE_HANDLE_DEFINE(le_audio_src_voice_data_ready);
	UINT8 *le_audio_src_voice_data_ptr;
	UINT8  le_audio_src_voice_oneTimeInit;
	UINT32 le_audio_src_voice_data_rd_ptr;
	UINT32 le_audio_src_voice_data_wr_ptr;
	INT32  le_audio_src_voice_available_buf_cnt;
	INT32  le_audio_src_voice_start_render;
}le_audio_src_voice_obj_t;

typedef struct
{
	void (*sink_cb)(uint32_t sync_index, uint64_t bclk_count);
	UINT32 sink_capture1_val;
	UINT32 sink_capture2_val;
	UINT64 sink_precapture2_val;
	UINT32 sink_bclk_divisior;
	UINT64 sink_total_cnt_val;
	UINT64 sink_bclk_val;
	UINT32 sink_sync_counter;
	UINT32 sink_sync_started;
} le_audio_sink_sync_info_t;

typedef struct
{
	UINT8 le_audio_snk_pcm_data_queue[LE_AUDIO_SNK_PCM_QUEUE_SIZE];
	UINT8 *le_audio_snk_pcm_buf_ptr;
	UINT8 *le_audio_snk_dummy_buf_ptr;
	OSA_SEMAPHORE_HANDLE_DEFINE(le_audio_snk_start_rx);
	OSA_SEMAPHORE_HANDLE_DEFINE(le_audio_snk_data_available);
	BT_DEFINE_MUTEX (le_audio_snk_rx_lock)
	BT_thread_type le_audio_snk_task_handle;
	UINT32 le_audio_snk_fd_in_us;
	UINT32 le_audio_snk_sf;
	UCHAR le_audio_snk_bps;
	UCHAR le_audio_snk_nc;
	INT32 le_audio_snk_rx_stop;
	INT32 le_audio_snk_available_buf_cnt;
	UINT32 le_audio_snk_expected_frame_len;
	UINT32 le_audio_snk_mute_frame_len;
	INT32 le_audio_snk_pcm_q_rd_ptr;
	INT32 le_audio_snk_pcm_q_wr_ptr;
	INT32 le_audio_snk_sai_index;
	UINT8 le_audio_snk_skip_frame;
	UINT8 le_audio_snk_enqueue_start;
#if defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)
	le_audio_sink_sync_info_t sink_sync_obj;
#endif /*defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)*/
} le_audio_snk_pl_data_t;

typedef struct
{
	UINT32 iso_interval_us;
	UINT32 big_cig_sync_delay_us;
	UINT32 timestamp;
	UINT32 timestamp_start;
	UINT32 sync_counter;
	UINT8 sdu_tx_started;
} le_audio_src_host_controller_sync_info_t;

typedef struct
{
	BT_thread_mutex_type le_audio_src_tx_lock;
	OSA_SEMAPHORE_HANDLE_DEFINE(le_audio_src_start_tx);
#if defined (LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)
	OSA_SEMAPHORE_HANDLE_DEFINE(le_audio_sync_signal);
	GPIO_HANDLE_DEFINE(le_audio_src_sync_handle);
	UINT8 le_audio_src_sync_configure_init;
#endif /*defined (LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)*/
	BT_thread_type le_audio_rx_task_handle;
	UINT8 le_audio_src_buf[LE_AUDIO_SRC_BUF_LEN];
	UINT32 le_audio_src_frame_len;
	le_audio_src_role_e le_audio_src_role;
	le_audio_src_voice_obj_t voice_data_obj;
	FIL le_audio_src_file_fd;
	WAV le_audio_src_wav_hdr;
	FSIZE_t le_audio_src_data_pos;
	UINT32 le_audio_file_remain;
	UINT32 le_audio_file_len;
	UINT32 le_audio_src_fd_in_us;
	UINT32 le_audio_src_sf;
	UCHAR le_audio_src_bps;
	UCHAR le_audio_src_nc;
	UINT8 le_audio_src_tx_on;
	UINT8 le_audio_src_tx_stop;
	le_audio_src_host_controller_sync_info_t le_audio_src_sync_info;
} le_audio_src_pl_data_t;
/* --------------------------------------------- externs */
void (* leaudio_snk_cb)(const UCHAR *data, UINT16 datlen);
void (* leaudio_src_cb)(const UCHAR *data, UINT16 datalen);
/*voice task related globals*/
extern UINT8 isroleCT;
extern UINT8 isroleCG;
/* --------------------------------------------- Global Variables */
DECL_STATIC UINT8 le_audio_snk_pcm_buf[LE_AUDIO_MAX_SAI_QUEUE_LEN];
DECL_STATIC UINT8 le_audio_snk_dummy_buf[LE_AUDIO_MAX_SAMPLES_PER_FRAME];
#if !defined(LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE) && (LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE == 0)
AT_NONCACHEABLE_SECTION_ALIGN(UINT8 le_audio_mic_pcm_buf[LE_AUDIO_MAX_SAI_QUEUE_LEN], 4);
#else
DECL_STATIC UINT8 le_audio_mic_pcm_buf[LE_AUDIO_MAX_SAI_QUEUE_LEN];
#endif /*!defined(LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE) && (LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE == 0)*/

AT_NONCACHEABLE_SECTION_ALIGN(HAL_AUDIO_HANDLE_DEFINE(le_audio_sink_handle), 4);
AT_NONCACHEABLE_SECTION_ALIGN(HAL_AUDIO_HANDLE_DEFINE(le_audio_sai_src_handle), 4);

DECL_STATIC le_audio_snk_pl_data_t le_audio_snk_pl_data;
DECL_STATIC le_audio_src_pl_data_t le_audio_src_pl_data;
DECL_STATIC UINT32 glbl_abs_volume=100;
DECL_STATIC bool glbl_volume_setting=false;

/*hw-codec handle, and flags*/
DECL_STATIC codec_handle_t hwCodecHandle;
DECL_STATIC UINT8 isCodecConfigured = BT_FALSE;;
DECL_STATIC UINT8 is_sink_setup_done = BT_FALSE;
DECL_STATIC UINT8 is_src_setup_done = BT_FALSE;

#if defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)
DECL_STATIC UINT8 isGtpStarted = BT_FALSE;
#endif /*defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)*/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
DECL_STATIC BT_THREAD_RETURN_TYPE le_audio_src_task (BT_THREAD_ARGS args);
DECL_STATIC BT_THREAD_RETURN_TYPE le_audio_sink_task (BT_THREAD_ARGS args);
DECL_STATIC API_RESULT le_audio_init_audio_tx_data ( void );
DECL_STATIC API_RESULT le_audio_init_voice_tx_data ( void );
DECL_STATIC void le_audio_deinit_audio_path(UCHAR ep);
DECL_STATIC API_RESULT le_audio_src_pl_prepare_file_name(UCHAR *file_name);
DECL_STATIC API_RESULT le_audio_open_audio_file(UCHAR *fn, FIL *fp);
 UINT32 le_audio_pl_switch_audio_pll(UINT32 sampleRate);
DECL_STATIC void le_audio_pl_start_tx(void);
DECL_STATIC void le_audio_pl_stop_tx(void);
DECL_STATIC void le_audio_pl_stop_audio_rx(void);
DECL_STATIC void le_audio_pl_start_audio_rx(void);
DECL_STATIC void le_audio_iso_sdu_flow_control_state_update ( void );
DECL_STATIC void le_audio_pl_transmit_media_frame(void);
DECL_STATIC void le_audio_pl_transmit_voice_frame(void);
DECL_STATIC void le_audio_sink_sai_tx_cb(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam);
DECL_STATIC void le_audio_src_voice_sai_tx_cb(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam);
/*******************************************************************************
 * Variables
 ******************************************************************************/
#if defined(CODEC_WM8960_ENABLE)
wm8960_config_t wm896xConfig = {
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .route     = kWM8960_RoutePlaybackandRecord,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    .format           = {.mclk_HZ    = 24576000U,
                         .sampleRate = kWM8960_AudioSampleRate48KHz,
                         .bitWidth   = kWM8960_AudioBitWidth16bit},
    .master_slave     = true,
};
DECL_STATIC codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm896xConfig};
#elif defined(CODEC_WM8962_ENABLE)
wm8962_config_t wm896xConfig = {
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
    .format       = {.mclk_HZ    = 24576000U,
               .sampleRate = kWM8962_AudioSampleRate48KHz,
               .bitWidth   = kWM8962_AudioBitWidth16bit},
    .masterSlave  = false,
};
DECL_STATIC codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8962, .codecDevConfig = &wm896xConfig};
#endif /*defined(CODEC_WM8960_ENABLE)*/

/*setting for 44.1Khz*/
DECL_STATIC const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 30,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    .numerator   = 1056,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 10000, /* 30 bit denominator of fractional loop divider */
};

/*setting for multiple of 8Khz,such as 48Khz/16Khz/32KHz*/
DECL_STATIC const clock_audio_pll_config_t audioPllConfig1 = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    .numerator   = 7680,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 10000, /* 30 bit denominator of fractional loop divider */
};

DECL_STATIC hal_audio_dma_mux_config_t audioTxDmaMuxConfig = {
    .dmaMuxConfig.dmaMuxInstance   = LE_AUDIO_CODEC_DMAMUX_INSTANCE,
    .dmaMuxConfig.dmaRequestSource = LE_AUDIO_CODEC_SAI_TX_SOURCE,
};

DECL_STATIC hal_audio_dma_config_t audioTxDmaConfig = {
    .instance             = LE_AUDIO_CODEC_DMA_INSTANCE,
    .channel              = LE_AUDIO_CODEC_TX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = (void *)&audioTxDmaMuxConfig,
    .dmaChannelMuxConfig  = NULL,
};

DECL_STATIC hal_audio_ip_config_t audioTxIpConfig = {
    .sai.lineMask = 1U << 0U,
    .sai.syncMode = kHAL_AudioSaiModeAsync,
};

hal_audio_config_t le_audio_sai_sink_config = {
    .dmaConfig         = &audioTxDmaConfig,
    .ipConfig          = (void *)&audioTxIpConfig,
    .srcClock_Hz       = 11289600U,
    .sampleRate_Hz     = (uint32_t)LE_AUDIO_SAMPLING_RATE,
    .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(LE_AUDIO_CODEC_SAI) - 1U, //2: 4bytes aligned, 1: 2 byte aligned
    .msaterSlave       = kHAL_AudioMaster,
    .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    .frameSyncPolarity = kHAL_AudioBeginAtFallingEdge,
    .lineChannels      = LE_AUDIO_DATA_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatI2sClassic,
    .bitWidth          = (uint8_t)LE_AUDIO_BIT_WIDTH,
    .instance          = LE_AUDIO_CODEC_INSTANCE,
};

DECL_STATIC hal_audio_dma_mux_config_t audioRxDmaMuxConfig = {
    .dmaMuxConfig.dmaMuxInstance   = LE_AUDIO_CODEC_DMAMUX_INSTANCE,
    .dmaMuxConfig.dmaRequestSource = LE_AUDIO_CODEC_SAI_RX_SOURCE,
};

DECL_STATIC hal_audio_dma_config_t audioRxDmaConfig = {
    .instance             = LE_AUDIO_CODEC_DMA_INSTANCE,
    .channel              = LE_AUDIO_CODEC_RX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = (void *)&audioRxDmaMuxConfig,
    .dmaChannelMuxConfig  = NULL,
};


DECL_STATIC hal_audio_ip_config_t audioRxIpConfig = {
    .sai.lineMask = 1U << 0U,
    .sai.syncMode = kHAL_AudioSaiModeSync,
};

DECL_STATIC hal_audio_config_t le_audio_sai_src_config = {
    .dmaConfig         = &audioRxDmaConfig,
    .ipConfig          = (void *)&audioRxIpConfig,
    .srcClock_Hz       = 0,
    .sampleRate_Hz     = 0,
    .fifoWatermark     = 2,/* Support both 7.5ms/10ms FD */
    .msaterSlave       = kHAL_AudioMaster,
    .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    .frameSyncPolarity = kHAL_AudioBeginAtFallingEdge,
    .lineChannels      = LE_AUDIO_RXDATA_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatI2sClassic,
    .bitWidth          = (uint8_t)LE_AUDIO_BIT_WIDTH,
    .instance          = LE_AUDIO_CODEC_INSTANCE,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
UINT32 le_audio_pl_switch_audio_pll(UINT32 sampleRate)
{
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
        	CLOCK_DeinitAudioPll();
            CLOCK_InitAudioPll(&audioPllConfig);
        }
        else if (0U == sampleRate % 8000)
        {
        	/*this is already initialized at init*/
            //CLOCK_InitAudioPll(&audioPllConfig1);
        }
        else
        {
            PRINTF("le_audio_pl_switch_audio_pll failed!(%d)\r\n", sampleRate);
        }

#if (defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))
        /*Clock setting for LPI2C*/
	CLOCK_SetMux(kCLOCK_Lpi2cMux, LE_AUDIO_LPI2C_CLOCK_SOURCE_SELECT);
	CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LE_AUDIO_LPI2C_CLOCK_SOURCE_DIVIDER);

	/*Clock setting for SAI1*/
	CLOCK_SetMux(kCLOCK_Sai1Mux, LE_AUDIO_SAI1_CLOCK_SOURCE_SELECT);
	CLOCK_SetDiv(kCLOCK_Sai1PreDiv, LE_AUDIO_SAI1_CLOCK_SOURCE_PRE_DIVIDER);
	CLOCK_SetDiv(kCLOCK_Sai1Div, LE_AUDIO_SAI1_CLOCK_SOURCE_DIVIDER);
	/* Enable MCLK output */
	IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK;
#elif defined(CPU_MIMXRT1176DVMAA_cm7)
	/*Clock setting for LPI2C*/
	CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, 1);

	/*Clock setting for SAI1*/
	CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, LE_AUDIO_SAI1_CLOCK_SOURCE_SELECT);
	CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, LE_AUDIO_SAI1_CLOCK_SOURCE_DIVIDER);

	wm896xConfig.format.sampleRate             = sampleRate;
	wm896xConfig.format.mclk_HZ                = LE_AUDIO_SAI_CLK_FREQ;
	/* Enable MCLK output */
	IOMUXC_GPR->GPR0 |= IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK;
#endif
    }

    return LE_AUDIO_SAI_CLK_FREQ;
}

DECL_STATIC void le_audio_sink_sai_tx_cb(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    if (kStatus_HAL_AudioError == completionStatus)
    {
        /* Handle the error. */
        PRINTF("sink-cbk %x\n",completionStatus);
    }
    else
    {
    	OSA_SemaphorePost(le_audio_snk_pl_data.le_audio_snk_data_available);
    }
}

DECL_STATIC void le_audio_src_voice_sai_tx_cb(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    if (kStatus_HAL_AudioError != completionStatus)
    {
    	hal_audio_transfer_t xfer = {0};
    	le_audio_src_voice_obj_t *voice_data_obj = (le_audio_src_voice_obj_t*)&le_audio_src_pl_data.voice_data_obj;
    	voice_data_obj->le_audio_src_voice_available_buf_cnt++;
    
    	xfer.data  = voice_data_obj->le_audio_src_voice_data_ptr + le_audio_src_pl_data.voice_data_obj.le_audio_src_voice_data_wr_ptr;
    	xfer.dataSize = le_audio_src_pl_data.le_audio_src_frame_len;
        hal_audio_status_t err = HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&le_audio_sai_src_handle[0], &xfer);

    	if ( (err != kStatus_HAL_AudioQueueFull) && (err != kStatus_HAL_AudioError) )
    	{
            if (le_audio_src_pl_data.le_audio_src_frame_len > (LE_AUDIO_MAX_SAI_QUEUE_LEN - voice_data_obj->le_audio_src_voice_data_wr_ptr))
			{
				voice_data_obj->le_audio_src_voice_data_wr_ptr = 0U;
			}
			else
			{
				voice_data_obj->le_audio_src_voice_data_wr_ptr += le_audio_src_pl_data.le_audio_src_frame_len;
			}
    	}
    	else
    	{
            PRINTF("voice-sai-failed,%d\r\n",err);
    	}
    
    	OSA_SemaphorePost(voice_data_obj->le_audio_src_voice_data_ready);
    }
}

DECL_STATIC void le_audio_hw_codec_start(UCHAR ep)
{
    int ret = 0;

    if (CODEC_SetMute(&hwCodecHandle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true) != kStatus_Success)
    {
        PRINTF("CODEC_SetMute(true) failed\r\n");
        ret = -1;
    }

    if (ret == 0)
    {
        if ((AUDIO_EP_SINK == ep) && (CODEC_SetFormat(&hwCodecHandle, le_audio_sai_sink_config.srcClock_Hz, le_audio_sai_sink_config.sampleRate_Hz, le_audio_sai_sink_config.bitWidth) != kStatus_Success))
        {
            PRINTF("CODEC_SetFormat(SINK) failed\r\n");
            ret = -1;
        }

        if ((AUDIO_EP_SOURCE == ep) && (CODEC_SetFormat(&hwCodecHandle, le_audio_sai_src_config.srcClock_Hz, le_audio_sai_src_config.sampleRate_Hz, le_audio_sai_src_config.bitWidth) != kStatus_Success))
        {
            PRINTF("CODEC_SetFormat(SRC) failed\r\n");
            ret = -1;
        }
    }

    if ((ret == 0) && (CODEC_SetVolume(&hwCodecHandle, kCODEC_VolumeDAC, LE_AUDIO_CODEC_DAC_VOLUME) != kStatus_Success))
    {
        PRINTF("CODEC_SetVolume(DAC) failed\r\n");
        ret = -1;
    }

    if ((ret == 0) && (CODEC_SetVolume(&hwCodecHandle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, LE_AUDIO_CODEC_HP_VOLUME) != kStatus_Success))
    {
        PRINTF("CODEC_SetVolume(L|R) failed\r\n");
        ret = -1;
    }

    if ((ret == 0) && (CODEC_SetMute(&hwCodecHandle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, false) != kStatus_Success))
    {
        PRINTF("CODEC_SetMute(false) failed\r\n");
        ret = -1;
    }

    if (ret == -1)
    {
        /*HW Codec or SAI has failed, stop here?*/
        assert(0);
    }
}

DECL_STATIC void le_audio_snk_pcm_enqueue
          (
              /* IN */  UCHAR * data,
              /* IN */  uint16_t  datalen
          )
{
    int32_t  n_free;
    uint32_t count = 0;

    BT_thread_mutex_lock(&le_audio_snk_pl_data.le_audio_snk_rx_lock);
	if (le_audio_snk_pl_data.le_audio_snk_rx_stop == BT_TRUE)
    {
        BT_thread_mutex_unlock(&le_audio_snk_pl_data.le_audio_snk_rx_lock);
        return;
    }

	if (le_audio_snk_pl_data.le_audio_snk_expected_frame_len != datalen)
	{
		/* Indicate the Bad frame reception*/
		PRINTF ("@");
		return;
	}

    n_free = le_audio_snk_pl_data.le_audio_snk_pcm_q_rd_ptr - le_audio_snk_pl_data.le_audio_snk_pcm_q_wr_ptr;

    if (le_audio_snk_pl_data.le_audio_snk_pcm_q_wr_ptr >= le_audio_snk_pl_data.le_audio_snk_pcm_q_rd_ptr)
    {
        n_free = (LE_AUDIO_SNK_PCM_QUEUE_SIZE - le_audio_snk_pl_data.le_audio_snk_pcm_q_wr_ptr) + le_audio_snk_pl_data.le_audio_snk_pcm_q_rd_ptr;
    }

    if( (le_audio_snk_pl_data.le_audio_snk_available_buf_cnt > 0) && (n_free == LE_AUDIO_SNK_PCM_QUEUE_SIZE) )
	{
    	/* q is full,currently overwriting the latest write pointer index */
    	n_free = 0;
	}

    if(n_free != 0)
	{
	    INT32 temp_wr_ptr = le_audio_snk_pl_data.le_audio_snk_pcm_q_wr_ptr;
	    for (count = 0U; count < datalen; count++)
	    {
		    le_audio_snk_pl_data.le_audio_snk_pcm_data_queue[temp_wr_ptr] = data[count];
		    temp_wr_ptr++;

		    if (temp_wr_ptr == LE_AUDIO_SNK_PCM_QUEUE_SIZE)
		    {
			    temp_wr_ptr = 0;
		    }
	    }
	    le_audio_snk_pl_data.le_audio_snk_pcm_q_wr_ptr = temp_wr_ptr;
	    le_audio_snk_pl_data.le_audio_snk_available_buf_cnt++;
    }

    BT_thread_mutex_unlock(&le_audio_snk_pl_data.le_audio_snk_rx_lock);
    if (le_audio_snk_pl_data.le_audio_snk_enqueue_start)
    {
		le_audio_snk_pl_data.le_audio_snk_enqueue_start = BT_FALSE;
		/*start audio-sink*/
		OSA_SemaphorePost(le_audio_snk_pl_data.le_audio_snk_start_rx);
    }
}

void leaudio_init_pl_ext (UCHAR role)
{
    DECL_STATIC UCHAR le_audio_src_task_init = 0x00U;
    DECL_STATIC UCHAR le_audio_snk_task_init = 0x00U;

    BT_thread_attr_type audio_task_attr;

    if ((le_audio_src_task_init == 0) && (le_audio_snk_task_init == 0))
    {
    	/*default PLL is set for 8/16/24/32/48 sampling-freq*/
        CLOCK_DeinitAudioPll();
        CLOCK_InitAudioPll(&audioPllConfig1);

        if (CODEC_Init(&hwCodecHandle, &boardCodecConfig) != kStatus_Success)
        {
            PRINTF("CODEC_Init failed!\r\n");
            assert(0);
        }

#if defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)
    le_audio_sync_init();
#endif /*defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)*/
    }

    if (AUDIO_EP_SOURCE == role)
    {
        if (0x00U != le_audio_src_task_init)
        {
            return;
        }

        BT_hci_set_sdu_tx_on_nocp (BT_TRUE);
        memset (&le_audio_src_pl_data, 0, sizeof (le_audio_src_pl_data));
        le_audio_src_pl_data.le_audio_src_tx_stop = BT_TRUE;
        (void) BT_thread_mutex_init (&le_audio_src_pl_data.le_audio_src_tx_lock, NULL);
        OSA_SemaphoreCreateBinary(le_audio_src_pl_data.le_audio_src_start_tx);
        /* Initialize the Write Task Attributes */
        audio_task_attr.thread_name       = (DECL_CONST CHAR  *)"audio-tx";
        audio_task_attr.thread_stack_size = LE_AUDIO_SRC_TX_TASK_STACK;
        audio_task_attr.thread_priority   = LE_AUDIO_SRC_TX_TASK_PRIO;

        if (0 != BT_thread_create(&le_audio_src_pl_data.le_audio_rx_task_handle, &audio_task_attr, le_audio_src_task, NULL))
        {
            PRINTF("Failed to create leaudio Media Task\n");
            return;
        }

        PRINTF ("le audio src task initialized!\n");
        le_audio_src_task_init = 1U;
     }

    if (AUDIO_EP_SINK == role)
    {
        if (0x00U != le_audio_snk_task_init)
        {
            return;
        }

        (void) BT_thread_mutex_init (&le_audio_snk_pl_data.le_audio_snk_rx_lock, NULL);

#if !defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) || (LE_AUDIO_ENABLE_SYNC_SIG_SUPP == 0)
    	memset (&le_audio_snk_pl_data, 0, sizeof(le_audio_snk_pl_data));
    	memset (le_audio_snk_dummy_buf, 0, sizeof(le_audio_snk_dummy_buf));
    	memset (le_audio_snk_pcm_buf, 0, sizeof(le_audio_snk_pcm_buf));

        OSA_SemaphoreCreateBinary(le_audio_snk_pl_data.le_audio_snk_start_rx);
        OSA_SemaphoreCreateBinary(le_audio_snk_pl_data.le_audio_snk_data_available);

        /* Initialize the sink task Attributes */
        audio_task_attr.thread_name       = (DECL_CONST CHAR  *)"audio-rx";
        audio_task_attr.thread_stack_size = LE_AUDIO_SNK_TASK_STACK;
        audio_task_attr.thread_priority   = LE_AUDIO_SNK_TASK_PRIO;
        if (0 != BT_thread_create(&le_audio_snk_pl_data.le_audio_snk_task_handle, &audio_task_attr, le_audio_sink_task, NULL))
        {
            PRINTF("audio sink task create failed!\n");
            assert(0);
        }

        PRINTF ("le audio sink tasks initialized!\n");
        le_audio_snk_task_init = 1U;
#endif /*defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)*/

        PRINTF ("le audio sink tasks initialized!\n");
        le_audio_snk_task_init = 1U;
    }
}

void leaudio_shutdown_pl_ext (void)
{
	le_audio_deinit_audio_path (AUDIO_EP_SINK);
	le_audio_deinit_audio_path (AUDIO_EP_SOURCE);
}

API_RESULT leaudio_setup_pl_ext
     (
         UCHAR ep,
         void (* ep_cb)(const UCHAR *data, UINT16 datalen),
         UINT16 sf,
         UCHAR bps,
         UCHAR nc,
         UINT16 size
     )
{
    uint32_t result = API_SUCCESS;

    if (AUDIO_EP_SOURCE == ep)
    {
    	UINT16 temp_sf = sf;
        if(temp_sf == 44100)
        {
    	    temp_sf = 48000;
        }

    	le_audio_src_pl_data.le_audio_src_sf = sf;
    	le_audio_src_pl_data.le_audio_src_bps = bps;
    	le_audio_src_pl_data.le_audio_src_nc = nc;
#if defined (LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)
    	le_audio_src_pl_data.le_audio_src_frame_len = ( (temp_sf / 100U) * appl_ga_utils_audio_src_get_fd() * (bps / 8U) * nc ) / 100U;
#endif /*defined (LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)*/
        leaudio_src_cb = ep_cb;
#ifndef LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE
        le_audio_sai_src_config.lineChannels = kHAL_AudioMonoRight;

        if (nc == 2U)
        {
            /*enable stereo support if 2 cc*/
            le_audio_sai_src_config.lineChannels = kHAL_AudioStereo;
        }

        le_audio_sai_src_config.sampleRate_Hz  = sf;
        le_audio_sai_src_config.srcClock_Hz    = le_audio_pl_switch_audio_pll(le_audio_sai_src_config.sampleRate_Hz);
        le_audio_sai_src_config.bitWidth 	   = bps;
        HAL_AudioRxInit((hal_audio_handle_t)&le_audio_sai_src_handle[0], &le_audio_sai_src_config);
        HAL_AudioRxInstallCallback((hal_audio_handle_t)&le_audio_sai_src_handle[0], le_audio_src_voice_sai_tx_cb, NULL);
        le_audio_hw_codec_start(ep);
        PRINTF ("audio-src:%dHz,mclk:%dHz,cc:%d,bps:%d\n", sf, le_audio_sai_src_config.srcClock_Hz, nc, bps);
#endif /*LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE*/
        is_src_setup_done = BT_TRUE;
    }

    if (AUDIO_EP_SINK == ep)
    {
    	UINT16 temp_sf = sf;
        if(temp_sf == 44100)
        {
    	    temp_sf = 48000;
        }

        //location: 0 for left, 1 for right.
		le_audio_pl_set_config_values (LE_AUDIO_BITS_PER_SAMPLE, bps);
		le_audio_pl_set_config_values (LE_AUDIO_SAMPLE_RATE, sf);
		le_audio_pl_set_config_values (LE_AUDIO_NO_OF_CHANNELS, nc);
		le_audio_pl_set_config_values (LE_AUDIO_SAMPLE_PER_FRAME, (( ( temp_sf / 100 ) * appl_ga_utils_audio_snk_get_fd() ) / 100U));
    	leaudio_snk_cb = ep_cb;
	    le_audio_sai_sink_config.lineChannels = kHAL_AudioMonoLeft;

        if (nc == 2U)
    	{
            /*enable stereo support if 2 cc*/
            le_audio_sai_sink_config.lineChannels = kHAL_AudioStereo;
    	}

        le_audio_sai_sink_config.sampleRate_Hz  = sf;
        le_audio_sai_sink_config.bitWidth  = bps;
        audio_i2s_init(sf, le_audio_sai_sink_config.lineChannels, bps, 1);
        le_audio_hw_codec_start(ep);

#if defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)
        le_audio_sync_set_config
                            (
                                le_audio_pl_get_config_values(LE_AUDIO_ISO_INTERVAL),
                                le_audio_pl_get_config_values (LE_AUDIO_BIG_CIG_DELAY),
                                le_audio_pl_get_config_values (LE_AUDIO_SAMPLE_RATE),
                                le_audio_pl_get_config_values (LE_AUDIO_SAMPLE_PER_FRAME),
                                le_audio_pl_get_config_values (LE_AUDIO_PRESENTATION_DELAY),
                                le_audio_pl_get_config_values (LE_AUDIO_LOCATION),
                                le_audio_pl_get_config_values (LE_AUDIO_BITS_PER_SAMPLE),
								le_audio_pl_get_config_values (LE_AUDIO_NO_OF_CHANNELS)
                            );
#endif /*defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)*/

#if defined(LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING) && (LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING == 1)
        PRINTF ("audio-sink:%dHz,mclk:%dHz,cc:%d,bps:%d\n", sf, le_audio_sai_sink_config.srcClock_Hz, nc, bps);
#endif
        is_sink_setup_done = BT_TRUE;
    }

    if ((is_src_setup_done == BT_TRUE ) || ((is_sink_setup_done == BT_TRUE )))
    {
		isCodecConfigured = BT_TRUE;
	}
    else
    {
    	isCodecConfigured = BT_FALSE;
    }


    return result;
}

API_RESULT leaudio_start_pl_ext (UCHAR ep)
{
    if (AUDIO_EP_SOURCE == ep)
    {
    	le_audio_src_pl_data.le_audio_src_role = LE_AUDIO_SRC_ROLE_MEDIA;

#ifdef BT_GAM
        if ( (isroleCG == 1) || (isroleCT == 1U) )
#endif
        {
            le_audio_src_pl_data.le_audio_src_role = LE_AUDIO_SRC_ROLE_VOICE;
        }

    	le_audio_pl_start_tx ();
    }

    if (AUDIO_EP_SINK == ep)
    {
    	le_audio_pl_start_audio_rx ();
    }

    return API_SUCCESS;
}

API_RESULT leaudio_stop_pl_ext (UCHAR ep)
{
    if (AUDIO_EP_SOURCE == ep)
    {
    	le_audio_pl_stop_tx ();
    }

    if (AUDIO_EP_SINK == ep)
    {
		audio_i2s_stop();
		audio_i2s_deinit();
    	le_audio_pl_stop_audio_rx ();
    }

    return API_SUCCESS;
}

DECL_STATIC void le_audio_deinit_audio_path(UCHAR ep)
{
    if(isCodecConfigured == BT_TRUE)
    {
        /* Stop Audio Player */
        CODEC_SetMute(&hwCodecHandle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);

        if((ep == AUDIO_EP_SINK) && (is_sink_setup_done == BT_TRUE))
        {
            HAL_AudioTxDeinit((hal_audio_handle_t)&le_audio_sink_handle[0]);
            is_sink_setup_done = BT_FALSE;
        }

        if((ep == AUDIO_EP_SOURCE) && (is_src_setup_done == BT_TRUE))
        {
            HAL_AudioRxDeinit((hal_audio_handle_t)&le_audio_sai_src_handle[0]);
            is_src_setup_done = BT_FALSE;
        }

        if ((is_src_setup_done == BT_FALSE) && ((is_sink_setup_done == BT_FALSE)))
        {
            isCodecConfigured = BT_FALSE;
        }

        CODEC_SetMute(&hwCodecHandle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, false);
    }
}

DECL_STATIC API_RESULT le_audio_init_voice_tx_data ( void )
{
	le_audio_src_voice_obj_t *voice_data_obj;
	API_RESULT retval = API_SUCCESS;

	voice_data_obj = (le_audio_src_voice_obj_t*)&le_audio_src_pl_data.voice_data_obj;
	memset (le_audio_mic_pcm_buf, 0, sizeof(le_audio_mic_pcm_buf));

	if(voice_data_obj->le_audio_src_voice_oneTimeInit == BT_FALSE)
	{
	    OSA_SemaphoreCreateBinary(voice_data_obj->le_audio_src_voice_data_ready);
	    voice_data_obj->le_audio_src_voice_oneTimeInit = BT_TRUE;
	}

	voice_data_obj->le_audio_src_voice_data_ptr = le_audio_mic_pcm_buf;
	voice_data_obj->le_audio_src_voice_data_rd_ptr = 0;
	voice_data_obj->le_audio_src_voice_data_wr_ptr = 0;
	voice_data_obj->le_audio_src_voice_available_buf_cnt = 0;
	voice_data_obj->le_audio_src_voice_start_render = BT_FALSE;

	return retval;
}

DECL_STATIC API_RESULT le_audio_init_audio_tx_data ( void )
{
    UCHAR audio_file_name[100] = { 0 };
    API_RESULT retval;

    UINT16 sf = le_audio_src_pl_data.le_audio_src_sf;
    UINT8 cc = le_audio_src_pl_data.le_audio_src_nc;
    UINT8 bps = le_audio_src_pl_data.le_audio_src_bps;

    memset (audio_file_name, 0, sizeof(audio_file_name));

    retval = le_audio_src_pl_prepare_file_name(audio_file_name);

    if (API_SUCCESS == retval)
    {
    	if (le_audio_open_audio_file (audio_file_name, &le_audio_src_pl_data.le_audio_src_file_fd) != 0)
    	{
    		retval = API_FAILURE;
    	}
    }
    else
    {
        PRINTF("failed to open audio-file (sf %d, cc %d bps %d)\n", sf, cc, bps);
    }

    if (retval == API_SUCCESS)
    {
     	FRESULT f_res;
        f_res = f_lseek(&le_audio_src_pl_data.le_audio_src_file_fd, le_audio_src_pl_data.le_audio_src_data_pos);
        if (f_res != FR_OK)
        {
            PRINTF("audio file re-pos failed!\n");
            retval = API_FAILURE;
        }
        else
        {
            memset (le_audio_src_pl_data.le_audio_src_buf, 0, sizeof(le_audio_src_pl_data.le_audio_src_buf));
            le_audio_src_pl_data.le_audio_file_remain = le_audio_src_pl_data.le_audio_file_len;
        }
    }
    return retval;
}

DECL_STATIC void le_audio_pl_start_audio_rx(void)
{
    UINT32 audio_frame_length;

    /* start audio-sink */
    BT_thread_mutex_lock(&le_audio_snk_pl_data.le_audio_snk_rx_lock);
    le_audio_snk_pl_data.le_audio_snk_skip_frame = LE_AUDIO_SNK_SKIP_FRAME_CNT;
    le_audio_snk_pl_data.le_audio_snk_pcm_q_rd_ptr = 0;
    le_audio_snk_pl_data.le_audio_snk_pcm_q_wr_ptr = 0;
    le_audio_snk_pl_data.le_audio_snk_dummy_buf_ptr = (UINT8*)le_audio_snk_dummy_buf;
    le_audio_snk_pl_data.le_audio_snk_pcm_buf_ptr = (UINT8*)le_audio_snk_pcm_buf;
    le_audio_snk_pl_data.le_audio_snk_available_buf_cnt = 0;
    le_audio_snk_pl_data.le_audio_snk_nc = le_audio_pl_get_config_values (LE_AUDIO_NO_OF_CHANNELS);
    le_audio_snk_pl_data.le_audio_snk_bps = le_audio_pl_get_config_values (LE_AUDIO_BITS_PER_SAMPLE);
    le_audio_snk_pl_data.le_audio_snk_sf = le_audio_pl_get_config_values (LE_AUDIO_SAMPLE_RATE);
    le_audio_snk_pl_data.le_audio_snk_fd_in_us =  appl_ga_utils_audio_snk_get_fd();
    audio_frame_length = le_audio_pl_get_config_values (LE_AUDIO_SAMPLE_PER_FRAME) * le_audio_pl_get_config_values (LE_AUDIO_BITS_PER_SAMPLE) / 8U;
    le_audio_snk_pl_data.le_audio_snk_expected_frame_len = audio_frame_length * le_audio_snk_pl_data.le_audio_snk_nc;;
    le_audio_snk_pl_data.le_audio_snk_mute_frame_len = le_audio_snk_pl_data.le_audio_snk_expected_frame_len / 4;
    le_audio_snk_pl_data.le_audio_snk_rx_stop = BT_FALSE;
    le_audio_snk_pl_data.le_audio_snk_enqueue_start = BT_TRUE;
    le_audio_snk_pl_data.le_audio_snk_sai_index = 0;
    BT_thread_mutex_unlock(&le_audio_snk_pl_data.le_audio_snk_rx_lock);
#if defined(LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING) && (LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING == 1)
    PRINTF("expected audio-frame-len %d\n",audio_frame_length);
#endif
}

DECL_STATIC void le_audio_pl_stop_audio_rx(void)
{
    /* Stop audio-sink */
    BT_thread_mutex_lock(&le_audio_snk_pl_data.le_audio_snk_rx_lock);
    le_audio_snk_pl_data.le_audio_snk_rx_stop = BT_TRUE;
    BT_thread_mutex_unlock(&le_audio_snk_pl_data.le_audio_snk_rx_lock);
}

DECL_STATIC void le_audio_pl_start_tx(void)
{
	API_RESULT retval = API_FAILURE;
	/* start audio/voice-tx */
	BT_thread_mutex_lock(&le_audio_src_pl_data.le_audio_src_tx_lock);
	le_audio_src_pl_data.le_audio_src_tx_on = LE_AUDIO_SRC_TX_QUEUE_FLOW_ON;
	le_audio_src_pl_data.le_audio_src_tx_stop = BT_FALSE;

	if (le_audio_src_pl_data.le_audio_src_role == LE_AUDIO_SRC_ROLE_MEDIA)
	{
		retval = le_audio_init_audio_tx_data();
	}
	else if (le_audio_src_pl_data.le_audio_src_role == LE_AUDIO_SRC_ROLE_VOICE)
	{
#ifdef LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE
		retval = le_audio_init_audio_tx_data();
#else
		retval = le_audio_init_voice_tx_data();
#endif /*LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE*/
	}
	else
	{
		PRINTF ("unknown tmap-role\n");
	}

	BT_thread_mutex_unlock(&le_audio_src_pl_data.le_audio_src_tx_lock);

	if (retval == API_SUCCESS)
	{
		OSA_SemaphorePost(le_audio_src_pl_data.le_audio_src_start_tx);
#if defined(LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING) && (LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING == 1)
		PRINTF ("frame-len %d, frame-dur %d\n", le_audio_src_pl_data.le_audio_src_frame_len, appl_ga_utils_audio_src_get_fd());
#endif
	}
}

DECL_STATIC void le_audio_pl_stop_tx(void)
{
    /* stop audio-tx */
	BT_thread_mutex_lock(&le_audio_src_pl_data.le_audio_src_tx_lock);
	le_audio_src_pl_data.le_audio_src_tx_stop = BT_TRUE;

	if (le_audio_src_pl_data.le_audio_src_role == LE_AUDIO_SRC_ROLE_VOICE)
	{
#ifndef LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE
		 HAL_AudioTransferAbortSend((hal_audio_handle_t)&le_audio_sai_src_handle[0]);
		 le_audio_src_pl_data.voice_data_obj.le_audio_src_voice_start_render = BT_FALSE;
		 OSA_SemaphorePost(le_audio_src_pl_data.voice_data_obj.le_audio_src_voice_data_ready);
#endif /*LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE*/
	}
	BT_thread_mutex_unlock(&le_audio_src_pl_data.le_audio_src_tx_lock);
}

DECL_STATIC API_RESULT le_audio_open_audio_file(UCHAR *fn, FIL *fp)
{
    API_RESULT ret = API_FAILURE;
	WAV wav;

#if defined(LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING) && (LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING == 1)
	PRINTF("opening audio file: %s\n", fn);
#endif
	FRESULT f_res = f_open(fp, (const TCHAR*)fn , FA_READ);
	if(f_res != FR_OK)
	{
		PRINTF("failed, %d\n", f_res);
		ret = API_FAILURE;
	}
	else
	{
		if(0 != readWavInfo(&wav, fp))
		{
			PRINTF("invalid audio-wav file!\n");
	        ret = API_FAILURE;
		}
		else
		{
			if(0 != memcmp(wav.Subchunk2ID, "data", 4))
			{
				PRINTF("audio-wav file contain unrecognized chunks\n");
		        ret = API_FAILURE;
			}
			else
			{
#if defined(LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING) && (LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING == 1)
				PRINTF("audio-file-info:\n");
				PRINTF("\tsample rate: %d\n", wav.SampleRate);
				PRINTF("\tchannels: %d\n", wav.NumChannels);
				PRINTF("\tbits per sample: %d\n", wav.BitsPerSample);
				PRINTF("\tsamples: %d\n", wav.Subchunk2Size / wav.BlockAlign);
#endif
				/* verify audio wav configuration with required audio config */
				if ( (le_audio_src_pl_data.le_audio_src_sf == wav.SampleRate) &&
					 (le_audio_src_pl_data.le_audio_src_nc == wav.NumChannels) &&
					 (le_audio_src_pl_data.le_audio_src_bps == wav.BitsPerSample)
					)
				{
					/* fetch audio data length and its initial position */
					le_audio_src_pl_data.le_audio_file_remain = wav.Subchunk2Size;
					le_audio_src_pl_data.le_audio_file_len = wav.Subchunk2Size;
					le_audio_src_pl_data.le_audio_src_data_pos = f_tell(fp);
					ret = API_SUCCESS;
				}
				else
				{
					PRINTF ("audio wav file configurations are different(require %d %d %d)\n", le_audio_src_pl_data.le_audio_src_sf,
							le_audio_src_pl_data.le_audio_src_nc, le_audio_src_pl_data.le_audio_src_bps);
			        ret = API_FAILURE;
				}
			}
		}
	}

	return ret;
}

DECL_STATIC void le_audio_pl_transmit_media_frame(void)
{
	UINT re;
	FRESULT f_res;

	/*TODO: Check BN count, and generate SDUs accordingly*/
	if(le_audio_src_pl_data.le_audio_file_remain < le_audio_src_pl_data.le_audio_src_frame_len)
	{
		f_res = f_lseek(&le_audio_src_pl_data.le_audio_src_file_fd, le_audio_src_pl_data.le_audio_src_data_pos);
		if(f_res != FR_OK)
		{
			PRINTF("audio file re-pos failed!, %d\n", f_res);
			return ;
		}

		le_audio_src_pl_data.le_audio_file_remain = le_audio_src_pl_data.le_audio_file_len;
	}

	f_res = f_read(&le_audio_src_pl_data.le_audio_src_file_fd, le_audio_src_pl_data.le_audio_src_buf, le_audio_src_pl_data.le_audio_src_frame_len, &re);

	if(f_res != FR_OK)
	{
		PRINTF("file read failed!, %d\n", f_res);
		return;
	}

	le_audio_src_pl_data.le_audio_file_remain -= re;

	/*send this frame to encode and prepare for sdu*/
	leaudio_src_cb
		(
		 le_audio_src_pl_data.le_audio_src_buf,
		 le_audio_src_pl_data.le_audio_src_frame_len
		);
}

DECL_STATIC void le_audio_iso_sdu_flow_control_state_update ( void )
{
	UINT16 pcm_queue_count = 0;

#if defined (LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)
	/*1. check current pcm-sdu-queue used count*/
	if (appl_ga_utils_audio_src_get_pcm_queue_info (&pcm_queue_count) == API_SUCCESS)
#endif /* defined (LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)*/
	{
		/* Switch OFF audio-transmission if iso-queue occupied >= MAX */
		if ((pcm_queue_count >= LE_AUDIO_SRC_ISO_QUEUE_MAX_WIN) &&
				(LE_AUDIO_SRC_TX_QUEUE_FLOW_ON == le_audio_src_pl_data.le_audio_src_tx_on))
		{
			le_audio_src_pl_data.le_audio_src_tx_on = LE_AUDIO_SRC_TX_QUEUE_FLOW_OFF;
		}
		else if ((pcm_queue_count <= LE_AUDIO_SRC_ISO_QUEUE_MIN_WIN) &&
				(LE_AUDIO_SRC_TX_QUEUE_FLOW_OFF == le_audio_src_pl_data.le_audio_src_tx_on))
		{
			/* Switch ON audio-transmission if iso-queue occupied <= MIN */
			le_audio_src_pl_data.le_audio_src_tx_on = LE_AUDIO_SRC_TX_QUEUE_FLOW_ON;
		}
		else
		{
			//ignore
		}
	}
}

DECL_STATIC API_RESULT le_audio_src_pl_prepare_file_name(UCHAR *file_name)
{
    API_RESULT retval;

    retval = API_FAILURE;

    if (NULL != file_name)
    {
        /* Copy audio files path in audio file name */
        BT_str_copy(file_name, LE_AUDIO_SRC_MEDIA_FILE_ROOT_DIR);
        BT_str_cat(file_name, BT_FOPS_PATH_SEP);
        BT_str_cat(file_name, "all_");
        retval = API_SUCCESS;

        /* Append sampling freq in audio file name */
        switch (le_audio_src_pl_data.le_audio_src_sf)
        {
        case 8000:
            BT_str_cat(file_name, "8000hz_");
            break;

        case 16000:
            BT_str_cat(file_name, "16000hz_");
            break;

        case 24000:
            BT_str_cat(file_name, "24000hz_");
            break;

        case 32000:
            BT_str_cat(file_name, "32000hz_");
            break;

        case 44100:
            BT_str_cat(file_name, "44100hz_");
            break;

        case 48000:
        		BT_str_cat(file_name, "48000hz_");
            break;

        default:
            retval = API_FAILURE;
            break;
        }

        if (API_SUCCESS == retval)
        {
            switch (le_audio_src_pl_data.le_audio_src_nc)
            {
            case 1:
                BT_str_cat(file_name, "1ch_");
                break;

            case 2:
                BT_str_cat(file_name, "2ch_");
                break;

            default:
                BT_str_cat(file_name, "1ch_");
                break;
            }
        }

        if (API_SUCCESS == retval)
        {
            switch (le_audio_src_pl_data.le_audio_src_bps)
            {
            case 16:
                BT_str_cat(file_name, "16b");
                break;

            case 24:
                BT_str_cat(file_name, "24b");
                break;

            case 32:
                BT_str_cat(file_name, "32b");
                break;

            default:
                retval = API_FAILURE;
                break;
            }
        }
    }

    if (API_SUCCESS == retval)
    {
        /*
         * Append file extension "wav" in audio file name with
         * NULL(\0) at the end.
         */
        BT_str_cat(file_name, ".wav\0");
    }

    return retval;
}

void leaudio_write_pl_ext (UCHAR ep, UCHAR * m_data, UINT16 m_datalen)
{
    if (AUDIO_EP_SINK == ep)
    {
#if defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)
        le_audio_snk_i2s_enqueue(m_data, m_datalen);
#else
        le_audio_snk_pcm_enqueue(m_data, m_datalen);
#endif /*defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)*/
    }
}

void le_audio_codec_setabsvol_pl_ext(UCHAR volume)
{
    glbl_abs_volume = ( ( volume * 100U ) / 255U );
    if(isCodecConfigured == BT_TRUE)
    {
        CODEC_SetVolume(&hwCodecHandle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, glbl_abs_volume);
    }
}

void le_audio_codec_setmute_pl_ext()
{
    glbl_volume_setting = true;
    if(isCodecConfigured == BT_TRUE)
    {
        CODEC_SetMute(&hwCodecHandle, kCODEC_PlayChannelHeadphoneRight |kCODEC_PlayChannelHeadphoneLeft, glbl_volume_setting);
    }
}

void le_audio_codec_setunmute_pl_ext()
{
    glbl_volume_setting = false;
    if(isCodecConfigured == BT_TRUE)
    {
        CODEC_SetMute(&hwCodecHandle, kCODEC_PlayChannelHeadphoneRight |kCODEC_PlayChannelHeadphoneLeft, glbl_volume_setting);
    }
}

/*******************************************************************************
 * Code
 ******************************************************************************/
DECL_STATIC void le_audio_pl_transmit_voice_frame (void)
{
	hal_audio_status_t err;
	UINT32 temp_rd_ptr;
	hal_audio_transfer_t xfer = {0};
	le_audio_src_voice_obj_t *voice_data_obj = (le_audio_src_voice_obj_t*)&le_audio_src_pl_data.voice_data_obj;

	if (voice_data_obj->le_audio_src_voice_start_render == BT_FALSE)
	{
		xfer.dataSize = le_audio_src_pl_data.le_audio_src_frame_len;
		xfer.data = voice_data_obj->le_audio_src_voice_data_ptr + voice_data_obj->le_audio_src_voice_data_wr_ptr;
		err = HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&le_audio_sai_src_handle[0], &xfer);
		if ( (err != kStatus_HAL_AudioQueueFull) && (err != kStatus_HAL_AudioError) )
		{
			voice_data_obj->le_audio_src_voice_data_wr_ptr += le_audio_src_pl_data.le_audio_src_frame_len;
            voice_data_obj->le_audio_src_voice_start_render = BT_TRUE;
		}
		else
		{
			PRINTF("first-voice-sai-failed,%d\r\n",err);
		}
		/*send first frame with zero data*/
		memset (le_audio_src_pl_data.le_audio_src_buf, 0, xfer.dataSize);
	}
	else
	{
		OSA_SemaphoreWait(le_audio_src_pl_data.voice_data_obj.le_audio_src_voice_data_ready, osaWaitForever_c);

		if (voice_data_obj->le_audio_src_voice_start_render == BT_FALSE)
		{
			return;
		}

		if (voice_data_obj->le_audio_src_voice_available_buf_cnt > 0)
		{
			temp_rd_ptr = voice_data_obj->le_audio_src_voice_data_rd_ptr;
			if (le_audio_src_pl_data.le_audio_src_frame_len > (LE_AUDIO_MAX_SAI_QUEUE_LEN - temp_rd_ptr))
			{
				temp_rd_ptr = 0U;
			}

			for (UINT32 index = 0; index < le_audio_src_pl_data.le_audio_src_frame_len; index++)
			{
				le_audio_src_pl_data.le_audio_src_buf[index] = voice_data_obj->le_audio_src_voice_data_ptr[temp_rd_ptr];
				temp_rd_ptr = temp_rd_ptr + 1;
			}
			/* Update the read pointer */
			voice_data_obj->le_audio_src_voice_available_buf_cnt--;
			voice_data_obj->le_audio_src_voice_data_rd_ptr = temp_rd_ptr;
		}
		else
		{
			memset (le_audio_src_pl_data.le_audio_src_buf, 0, xfer.dataSize);
		}
	}

	leaudio_src_cb(le_audio_src_pl_data.le_audio_src_buf, le_audio_src_pl_data.le_audio_src_frame_len);
}

DECL_STATIC BT_THREAD_RETURN_TYPE le_audio_src_task (BT_THREAD_ARGS args)
{
	uint32_t timeout = 0;
	PRINTF ("audio-transmission task created!\n");

	BT_LOOP_FOREVER()
	{
		OSA_SemaphoreWait(le_audio_src_pl_data.le_audio_src_start_tx, osaWaitForever_c);
#if !defined (LE_AUDIO_SRC_SYNC_ENABLE) || (LE_AUDIO_SRC_SYNC_ENABLE == 0)
        /*wait at least for one frame-duration to complete data-path activities*/
          EM_usleep(100U * appl_ga_utils_audio_src_get_fd());
#endif /*!defined (LE_AUDIO_SRC_SYNC_ENABLE) || (LE_AUDIO_SRC_SYNC_ENABLE == 0)*/
#if defined(LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING) && (LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING == 1)
		PRINTF ("audio-transmission is started now!\n");
#endif

		BT_LOOP_FOREVER()
		{
			BT_thread_mutex_lock(&le_audio_src_pl_data.le_audio_src_tx_lock);
			if (le_audio_src_pl_data.le_audio_src_tx_stop == BT_TRUE)
			{
				/*audio-transmission stopped!*/
				BT_thread_mutex_unlock(&le_audio_src_pl_data.le_audio_src_tx_lock);
				break;
			}

			/*control audio transmission as per iso-sdu queue occupancy*/
			le_audio_iso_sdu_flow_control_state_update ();

			/*allow audio-tx if there is a space*/
			if (le_audio_src_pl_data.le_audio_src_tx_on == LE_AUDIO_SRC_TX_QUEUE_FLOW_ON)
			{
				if (le_audio_src_pl_data.le_audio_src_role == LE_AUDIO_SRC_ROLE_MEDIA)
				{
					le_audio_pl_transmit_media_frame();
					timeout = 256U;
				}
				else if (le_audio_src_pl_data.le_audio_src_role == LE_AUDIO_SRC_ROLE_VOICE)
				{
#ifdef LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE
					le_audio_pl_transmit_media_frame();
					timeout = 256U;
#else
					le_audio_pl_transmit_voice_frame();
#endif /*LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE*/
				}
				else
				{
					//UNKNOWN
				}
			}
			else
			{
				/*suspend audio-tx with 2 iso-sdu-frame-duration*/
				timeout = 1000U * (appl_ga_utils_audio_src_get_fd() / 10U ) * 2U;
			}

            BT_thread_mutex_unlock(&le_audio_src_pl_data.le_audio_src_tx_lock);

#ifndef LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE
			if (le_audio_src_pl_data.le_audio_src_role == LE_AUDIO_SRC_ROLE_MEDIA)
#endif /*LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE*/
			{
				EM_usleep(timeout);
			}
		}
		PRINTF ("audio-transmission is stopped now!\n");
#ifndef LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE
		 if (le_audio_src_pl_data.le_audio_src_role == LE_AUDIO_SRC_ROLE_MEDIA)
#endif /*LE_AUDIO_DISABLE_MIC_FOR_CT_CG_ROLE*/
		{
			f_close(&le_audio_src_pl_data.le_audio_src_file_fd);
		}
	}
}

DECL_STATIC BT_THREAD_RETURN_TYPE le_audio_sink_task (BT_THREAD_ARGS args)
{
	INT32  temp_rd_ptr, index;
	hal_audio_status_t ret;
	hal_audio_transfer_t xfer = {0};
	UINT32 remaining = 0;
	UINT32 bytes_to_read = 0;

	BT_LOOP_FOREVER()
	{
		/*wait for starting sink*/
		OSA_SemaphoreWait(le_audio_snk_pl_data.le_audio_snk_start_rx, osaWaitForever_c);
		PRINTF ("audio-sink is started now!\n");

		BT_LOOP_FOREVER()
		{
			BT_thread_mutex_lock(&le_audio_snk_pl_data.le_audio_snk_rx_lock);

			if (le_audio_snk_pl_data.le_audio_snk_rx_stop == BT_TRUE)
			{
				/*audio-sink stopped!*/
			    /*if ((isCodecConfigured == BT_TRUE) && CODEC_SetMute(&hwCodecHandle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true) != kStatus_Success)
			    {
			        PRINTF("CODEC_SetMute(true) failed\r\n");
			    }*/

				BT_thread_mutex_unlock(&le_audio_snk_pl_data.le_audio_snk_rx_lock);
				break;
			}

			if (le_audio_snk_pl_data.le_audio_snk_pcm_q_wr_ptr > le_audio_snk_pl_data.le_audio_snk_pcm_q_rd_ptr)
			{
				remaining = le_audio_snk_pl_data.le_audio_snk_pcm_q_wr_ptr - le_audio_snk_pl_data.le_audio_snk_pcm_q_rd_ptr;
			}
			else
			{
				remaining = LE_AUDIO_SNK_PCM_QUEUE_SIZE -
					(le_audio_snk_pl_data.le_audio_snk_pcm_q_rd_ptr - le_audio_snk_pl_data.le_audio_snk_pcm_q_wr_ptr);
			}

			if( (le_audio_snk_pl_data.le_audio_snk_available_buf_cnt == 0) && (remaining == LE_AUDIO_SNK_PCM_QUEUE_SIZE) )//q empty
			{
				remaining = 0;
			}

			bytes_to_read = le_audio_snk_pl_data.le_audio_snk_expected_frame_len;

			/*data frame received here*/
			if( (remaining != 0) && (remaining >= bytes_to_read) )
			{
				temp_rd_ptr = le_audio_snk_pl_data.le_audio_snk_pcm_q_rd_ptr;
				for (index = 0; index < bytes_to_read; index++)
				{
					le_audio_snk_pl_data.le_audio_snk_pcm_buf_ptr[index] = le_audio_snk_pl_data.le_audio_snk_pcm_data_queue[temp_rd_ptr];

					temp_rd_ptr = temp_rd_ptr + 1;
					if (temp_rd_ptr == LE_AUDIO_SNK_PCM_QUEUE_SIZE)
					{
						temp_rd_ptr = 0;
					}
				}

				/* Update the read pointer */
				le_audio_snk_pl_data.le_audio_snk_pcm_q_rd_ptr = temp_rd_ptr;
				le_audio_snk_pl_data.le_audio_snk_available_buf_cnt--;
				xfer.dataSize = bytes_to_read;
				xfer.data = &le_audio_snk_pl_data.le_audio_snk_pcm_buf_ptr[0];
			}
			else
			{
				/*no enough data receive to play-off one silent-frame*/
				PRINTF("!");
				xfer.dataSize = le_audio_snk_pl_data.le_audio_snk_mute_frame_len;
				xfer.dataSize -= (xfer.dataSize % 4);
				xfer.data = le_audio_snk_pl_data.le_audio_snk_dummy_buf_ptr;
			}

			ret = HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&le_audio_sink_handle[0], &xfer);
			if ((ret == kStatus_HAL_AudioQueueFull) || (ret == kStatus_HAL_AudioError))
			{
				PRINTF("sink-tx-fail %d\r\n", ret); // may come due to datasize, data mismatch
			}

			BT_thread_mutex_unlock(&le_audio_snk_pl_data.le_audio_snk_rx_lock);
			OSA_SemaphoreWait(le_audio_snk_pl_data.le_audio_snk_data_available, osaWaitForever_c);
		}

		/* stop audio sink*/
		ret = HAL_AudioTransferAbortSend((hal_audio_handle_t)&le_audio_sink_handle[0]);
		if ((ret == kStatus_HAL_AudioQueueFull) || (ret == kStatus_HAL_AudioError))
		{
			PRINTF("sink-abort-fail %d\r\n", ret); // may come due to datasize, data mismatch
		}
		PRINTF ("audio-sink is stopped!\n");
	}
}

#if defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)
void le_audio_pl_set_aca(uint32_t aca)
{
    if (GA_AUDIO_LOCATION_FL == (aca & GA_AUDIO_LOCATION_FL))
    {
        le_audio_pl_set_config_values (LE_AUDIO_LOCATION, AUDIO_SINK_ROLE_LEFT);
    }
    else if (GA_AUDIO_LOCATION_FR == (aca & GA_AUDIO_LOCATION_FR))
    {
        le_audio_pl_set_config_values (LE_AUDIO_LOCATION, AUDIO_SINK_ROLE_RIGHT);
    }
    else if ((GA_AUDIO_LOCATION_FR | GA_AUDIO_LOCATION_FL) == (aca & (GA_AUDIO_LOCATION_FR | GA_AUDIO_LOCATION_FL)))
    {
        le_audio_pl_set_config_values (LE_AUDIO_LOCATION, AUDIO_SINK_ROLE_LEFT);
    }
    else
    {
    	//
    }
}

void le_audio_set_sync_info_pl_ext (UINT8 ep, UINT8 evt, void *sync_data)
{
    if (sync_data != NULL)
    {
    	if (evt == AUDIO_SYNC_CIG_ESTABLISHED)
    	{
    		unicast_sink_data_t *cis_data = (unicast_sink_data_t *)sync_data;
            le_audio_pl_set_config_values (LE_AUDIO_IS_BIG_ESTABLISHED, AUDIO_SYNC_CIG_ESTABLISHED);
            le_audio_pl_set_config_values (LE_AUDIO_ISO_INTERVAL, cis_data->iso_interval);
            le_audio_pl_set_config_values (LE_AUDIO_BIG_CIG_DELAY, cis_data->cig_sync_delay);
            //update src sync object
			le_audio_src_pl_data.le_audio_src_sync_info.big_cig_sync_delay_us = cis_data->cig_sync_delay;
			le_audio_src_pl_data.le_audio_src_sync_info.iso_interval_us = cis_data->iso_interval;

			/*TODO*/
            //audio location
            qos_config_data_t *qos = appl_ga_ucs_fetch_qos_config_data_buf ();

            if (qos->qos_config_state == BT_TRUE)
            {
                if (qos->qos_config.sdu_interval != 0)
                {
                    le_audio_pl_set_config_values (LE_AUDIO_SDU_INTERVAL, qos->qos_config.sdu_interval);
                    le_audio_pl_set_config_values (LE_AUDIO_FRAMED_UNFRAMED, qos->qos_config.framing);
                    le_audio_pl_set_config_values (LE_AUDIO_PRESENTATION_DELAY, qos->qos_config.presentation_delay);
                }
				memset(qos, 0, sizeof(qos_config_data_t));
            }
            else
            {
                qos_config_data_t *qos = appl_ga_ucc_fetch_qos_config_data_buf ();

            	if (qos->qos_config_state == BT_TRUE)
				{
					if (qos->qos_config.sdu_interval != 0)
					{
						le_audio_pl_set_config_values (LE_AUDIO_SDU_INTERVAL, qos->qos_config.sdu_interval);
						le_audio_pl_set_config_values (LE_AUDIO_FRAMED_UNFRAMED, qos->qos_config.framing);
						le_audio_pl_set_config_values (LE_AUDIO_PRESENTATION_DELAY, qos->qos_config.presentation_delay);
					}
					memset(qos, 0, sizeof(qos_config_data_t));
				}
            }
    	}
    	else if (evt == AUDIO_SYNC_BIG_ESTABLISHED)
    	{
    		//broadcast sink
    		if (ep == AUDIO_EP_SINK)
    		{
        		//Sink Sync
                broadcast_sink_data_t *bis_data = (broadcast_sink_data_t *)sync_data;
                le_audio_pl_set_config_values (LE_AUDIO_IS_BIG_ESTABLISHED, AUDIO_SYNC_BIG_ESTABLISHED);
                le_audio_pl_set_config_values (LE_AUDIO_ISO_INTERVAL, bis_data->iso_interval);
                le_audio_pl_set_config_values (LE_AUDIO_TRANSPORT_LATENCY, bis_data->transport_latency);
                le_audio_pl_set_config_values (LE_AUDIO_PRESENTATION_DELAY, bis_data->presentation_delay);
                le_audio_pl_set_config_values (LE_AUDIO_SDU_INTERVAL, bis_data->sdu_interval);
                le_audio_pl_set_config_values (LE_AUDIO_BC_BN, bis_data->BN);
                le_audio_pl_set_config_values (LE_AUDIO_BC_NSE, bis_data->NSE);
                le_audio_pl_set_config_values (LE_AUDIO_BC_PTO, bis_data->PTO);
                le_audio_pl_set_config_values (LE_AUDIO_BC_IRC, bis_data->IRC);
                le_audio_pl_set_config_values (LE_AUDIO_FRAMED_UNFRAMED, bis_data->framing);
    		}
    	}
    	else if (evt == AUDIO_SYNC_BIG_CREATE)
    	{
    		// broadcast source
    		if (ep == AUDIO_EP_SOURCE)
    		{
    			src_sync_data_t *src_sync_data = (src_sync_data_t *)sync_data;
    			le_audio_src_pl_data.le_audio_src_sync_info.big_cig_sync_delay_us = src_sync_data->big_sync_delay;
    			le_audio_src_pl_data.le_audio_src_sync_info.iso_interval_us = src_sync_data->iso_interval;
    		}
    	}
    	else
    	{
    		//
    	}
    }
}
#endif /*defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)*/

#if defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)
/*Host Controller Synchronization*/
#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2) || defined(WIFI_IW612_BOARD_RD_USD)
void le_audio_pl_ext_sync_timer_init(void (*sync_cb)(uint32_t sync_index, uint64_t bclk_count))
{
    gpt_config_t config;

    /* Set SyncTimer clock root to Audio_PLL. */
    /* 393216000 / 16 = 24576000 */
    CLOCK_SetRootClockMux(SyncTimer_GPT_ClockRoot, SyncTimer_GPT_ClockRoot_Mux);
    CLOCK_SetRootClockDiv(SyncTimer_GPT_ClockRoot, SyncTimer_GPT_ClockRoot_Div);

    GPT_GetDefaultConfig(&config);
    config.clockSource = kGPT_ClockSource_HighFreq; /* GPT_CLK come from AUDIO PLL. */
    config.divider = 1U;
    config.enableRunInStop = true;
    config.enableRunInWait = true;
    config.enableRunInDoze = false;
    config.enableRunInDbg = false;
    config.enableFreeRun = true;
    config.enableMode  = true;
    GPT_Init(SyncTimer_GPT, &config);

    /* Input Capture 2 capture signal come from Sync INT signal, in order to measure CLK drift. */
    GPT_SetInputOperationMode(SyncTimer_GPT, kGPT_InputCapture_Channel2, kGPT_InputOperation_RiseEdge);
#if defined (LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)
    /* Input Capture 1 capture signal come from SAI FS signal, in order to capture offset between BCLK and FS. */
    GPT_SetInputOperationMode(SyncTimer_GPT, kGPT_InputCapture_Channel1, kGPT_InputOperation_RiseEdge);
    le_audio_snk_pl_data.sink_sync_obj.sink_cb = sync_cb;
#endif /*defined (LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)*/
}
#endif /*defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)*/

#if defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)
void le_audio_pl_ext_sync_timer_start(UINT8 ep, UINT32 sink_cnter)
{
	if ((le_audio_snk_pl_data.sink_sync_obj.sink_sync_started == BT_FALSE) &&	\
									( (ep == AUDIO_EP_SINK) || (ep == 0x02) ) )
	{
#if defined (LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)
		le_audio_snk_pl_data.sink_sync_obj.sink_sync_counter = sink_cnter;
		le_audio_snk_pl_data.sink_sync_obj.sink_capture1_val = 0;
		le_audio_snk_pl_data.sink_sync_obj.sink_capture2_val = 0;
		le_audio_snk_pl_data.sink_sync_obj.sink_bclk_divisior = 1U;
		le_audio_snk_pl_data.sink_sync_obj.sink_precapture2_val = 0;
		le_audio_snk_pl_data.sink_sync_obj.sink_bclk_val = 0;
		le_audio_snk_pl_data.sink_sync_obj.sink_total_cnt_val = 0;
		le_audio_snk_pl_data.sink_sync_obj.sink_sync_started = BT_TRUE;
		/* Reset all parameter values. */
#endif /*defined (LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)*/
	}

	if ((le_audio_src_pl_data.le_audio_src_sync_configure_init == BT_FALSE) &&	\
										( (ep == AUDIO_EP_SOURCE) || (ep == 0x02) ) )
	{
#if defined (LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE == 1U)
		le_audio_src_pl_data.le_audio_src_sync_configure_init = BT_TRUE;
		le_audio_src_pl_data.le_audio_src_sync_info.sync_counter = 0;
		le_audio_src_pl_data.le_audio_src_sync_info.timestamp = 0;
		le_audio_src_pl_data.le_audio_src_sync_info.sdu_tx_started = 0;
		le_audio_src_pl_data.le_audio_src_sync_info.iso_interval_us = 0;
		le_audio_src_pl_data.le_audio_src_sync_info.big_cig_sync_delay_us = 0;
#endif /*defined (LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE == 1U)*/
	}

	if (isGtpStarted == BT_FALSE)
	{
#if defined (LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)
		if (le_audio_snk_pl_data.sink_sync_obj.sink_sync_started == BT_TRUE)
		{
		    GPT_ClearStatusFlags(SyncTimer_GPT, kGPT_InputCapture1Flag);
		    GPT_EnableInterrupts(SyncTimer_GPT, kGPT_InputCapture1InterruptEnable);
		}
#endif /*defined (LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)*/
	    GPT_ClearStatusFlags(SyncTimer_GPT, kGPT_InputCapture2Flag);
	    GPT_EnableInterrupts(SyncTimer_GPT, kGPT_InputCapture2InterruptEnable);
	    EnableIRQWithPriority(SyncTimer_GPT_Irq, 3);
	    GPT_StartTimer(SyncTimer_GPT);
	    isGtpStarted = BT_TRUE;
	}
}
#endif /*defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)*/

#if defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)
void le_audio_pl_ext_sync_timer_stop(void)
{
    if (isGtpStarted == BT_TRUE)
	{
		isGtpStarted = BT_FALSE;
		GPT_StopTimer(SyncTimer_GPT);
	    le_audio_snk_pl_data.sink_sync_obj.sink_sync_started = BT_FALSE;
	}
}
#endif /*defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)*/

#if defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)
#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2)
void GPT2_IRQHandler(void)
#elif defined(WIFI_IW612_BOARD_RD_USD)
void GPT3_IRQHandler(void)
#endif
{
    uint32_t flag;
    uint32_t capture2_value_delta;
    le_audio_sink_sync_info_t* sink_sync = &le_audio_snk_pl_data.sink_sync_obj;
    flag  = GPT_GetStatusFlags(SyncTimer_GPT, (gpt_status_flag_t)(kGPT_InputCapture1Flag | kGPT_InputCapture2Flag));
    flag &= GPT_GetEnabledInterrupts(SyncTimer_GPT);

    if(flag & kGPT_InputCapture1Flag)
    {
        /* We only need capture the first SAI FS signal to calculate offset. */
        GPT_DisableInterrupts(SyncTimer_GPT, kGPT_InputCapture1InterruptEnable);
        GPT_ClearStatusFlags(SyncTimer_GPT, kGPT_InputCapture1Flag);
        /* SAI FS trigged BCLK offset. */
        sink_sync->sink_capture1_val = GPT_GetInputCaptureValue(SyncTimer_GPT, kGPT_InputCapture_Channel1);
        sink_sync->sink_bclk_divisior = (CLOCK_GetRootClockFreq(SyncTimer_GPT_ClockRoot) / (le_audio_pl_get_config_values(LE_AUDIO_SAMPLE_RATE) * le_audio_pl_get_config_values(LE_AUDIO_BITS_PER_SAMPLE)) ); //1ch:32, 2ch:16
        PRINTF("codec i/f starts now at %d(%08u),divisor %d\n",le_audio_src_pl_data.le_audio_src_sync_info.sync_counter, sink_sync->sink_capture1_val,sink_sync->sink_bclk_divisior);
    }
    if(flag & kGPT_InputCapture2Flag)
    {
        GPT_ClearStatusFlags(SyncTimer_GPT, kGPT_InputCapture2Flag);

        if (le_audio_src_pl_data.le_audio_src_sync_configure_init == BT_TRUE)
        {
            /*increment src sync counter*/
        	le_audio_src_pl_data.le_audio_src_sync_info.sync_counter++;

            if (le_audio_src_pl_data.le_audio_src_tx_stop == BT_FALSE)
            {
    			/*if framed mode, check if timestamp+sdu_interval is within iso-interval else don't send sdu!*/
    			if (le_audio_pl_validate_next_timestamp() == API_SUCCESS)
    			{
    					/*wake-up encoder task to send sdu*/
    					appl_ga_utils_audio_src_signal_encoder_task ();
    			}
            }
        }


    	if (le_audio_snk_pl_data.sink_sync_obj.sink_sync_started == BT_TRUE)
    	{
            /* Sync INT trigged BCLK count capture. */
            sink_sync->sink_capture2_val = GPT_GetInputCaptureValue(SyncTimer_GPT, kGPT_InputCapture_Channel2);

            /* Fix GPT overflow issue. */
            if (sink_sync->sink_capture2_val < sink_sync->sink_precapture2_val)
            {
                capture2_value_delta = (uint64_t)0x100000000 + sink_sync->sink_capture2_val - sink_sync->sink_precapture2_val;
            }
            else
            {
                capture2_value_delta = sink_sync->sink_capture2_val - sink_sync->sink_precapture2_val;
            }

            sink_sync->sink_precapture2_val = sink_sync->sink_capture2_val;
            sink_sync->sink_total_cnt_val += capture2_value_delta;


            /* Calculate BCLK. */
            /* BCLK should be 0 before the SAI_SW start, because we use AUDIO_PLL as clk source. */
            if(sink_sync->sink_capture1_val > 0U)
            {
            	sink_sync->sink_bclk_val = (sink_sync->sink_total_cnt_val - sink_sync->sink_capture1_val) / sink_sync->sink_bclk_divisior;
            }
            else
            {
            	sink_sync->sink_bclk_val = 0;
            }

            /* invoke callback. */
            if(sink_sync->sink_cb != NULL)
            {
            	sink_sync->sink_cb (sink_sync->sink_sync_counter, sink_sync->sink_bclk_val);
            }

            /* SyncTimer trigger signal count. */
            sink_sync->sink_sync_counter++;
    	}
    }

    SDK_ISR_EXIT_BARRIER;
}
#endif /* WIFI_IW612_BOARD_MURATA_2EL_M2 || WIFI_IW612_BOARD_RD_USD */
#endif /*defined(LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)*/

#if defined(LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)
void le_audio_pl_sync_stop(void)
{
	if(le_audio_src_pl_data.le_audio_src_sync_configure_init == BT_TRUE)
	{
	    le_audio_src_pl_data.le_audio_src_sync_configure_init = BT_FALSE;
	}

	if(le_audio_snk_pl_data.sink_sync_obj.sink_sync_started == BT_TRUE)
	{
		le_audio_snk_pl_data.sink_sync_obj.sink_sync_started = BT_FALSE;
	}

	if (isGtpStarted == BT_TRUE)
	{
	    le_audio_sync_stop();
	}

}
#endif /*defined(LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)*/

#if defined(LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)
UINT32 le_audio_pl_get_sdu_timestamp (UINT32 seq_num)
{
	static UINT64 tx_samples = 0;
	UINT32 ts = 0;

	/*for framed mode only*/
	if (le_audio_src_pl_data.le_audio_src_sf == 44100U)
	{
		if(seq_num == 0)
		{
			tx_samples = 0;
			ts = (le_audio_src_pl_data.le_audio_src_sync_info.sync_counter * le_audio_src_pl_data.le_audio_src_sync_info.iso_interval_us) +
					(le_audio_src_pl_data.le_audio_src_sync_info.iso_interval_us);
			le_audio_src_pl_data.le_audio_src_sync_info.sdu_tx_started = BT_TRUE;
			le_audio_src_pl_data.le_audio_src_sync_info.timestamp_start = ts;
			le_audio_src_pl_data.le_audio_src_sync_info.timestamp = ts;
		}
		else
		{
			UINT16 spf =  le_audio_src_pl_data.le_audio_src_frame_len / ( (le_audio_src_pl_data.le_audio_src_bps / 8U) * (le_audio_src_pl_data.le_audio_src_nc) );
			tx_samples += spf;
			ts = (UINT32)((double)le_audio_src_pl_data.le_audio_src_sync_info.timestamp_start + (double)tx_samples * 1000000.0 / (double)le_audio_src_pl_data.le_audio_src_sf);
			le_audio_src_pl_data.le_audio_src_sync_info.timestamp = ts;
		}
	}


	return ts;
}
#endif /*defined(LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)*/

#if defined(LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)
API_RESULT le_audio_pl_validate_next_timestamp (void)
{
	API_RESULT retval = API_SUCCESS;
	static int count = 0;
	/*for framed mode only*/
	if (le_audio_src_pl_data.le_audio_src_sf == 44100U)
	{
		UINT32 sync_cnt  = le_audio_src_pl_data.le_audio_src_sync_info.sync_counter;
		UINT32 iso_interval = le_audio_src_pl_data.le_audio_src_sync_info.iso_interval_us;
		UINT32 ts = le_audio_src_pl_data.le_audio_src_sync_info.timestamp;
		retval = API_FAILURE;

		if (le_audio_src_pl_data.le_audio_src_sync_info.sdu_tx_started == BT_TRUE)
		{
			if ((ts - (iso_interval * sync_cnt)) < iso_interval)
			{
				retval = API_SUCCESS;
				count++;
			}
			else
			{
				count = 0;
			}
		}
		else
		{
			retval = API_SUCCESS;
			count = 0;
		}

		PRINTF ("[sdu %d] sync_pulse %d, prev_ts %d, iso_event %d, delta(ts-iso) %d, ret %d\n",count, sync_cnt, ts, (sync_cnt*iso_interval), (ts - (iso_interval * sync_cnt)), retval);
	}

	return retval;
}

#endif /*defined(LE_AUDIO_SRC_SYNC_ENABLE) && (LE_AUDIO_SRC_SYNC_ENABLE > 0)*/

#endif /*#if (LE_AUDIO_SRC_SYNC_ENABLE == 1U) && (LE_AUDIO_SINK_SYNC_ENABLE == 1U)*/
