/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_pl.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_sai.h"
#include "fsl_dmamux.h"
#include "fsl_sai_edma.h"
#include "fsl_codec_common.h"
#include "fsl_adapter_audio.h"
#include "fsl_codec_adapter.h"
#include "BT_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define os_thread_sleep(ticks) vTaskDelay(ticks)
#define os_msec_to_ticks(msecs) ((msecs) / (portTICK_PERIOD_MS))

#define LEAUDIO_CODEC_SAI      	 	SAI1
#define LEAUDIO_CODEC_INSTANCE 		(1U)
/* demo audio data channel */
#define LEAUDIO_DATA_CHANNEL 	(kHAL_AudioMonoLeft)
#define LEAUDIO_RXDATA_CHANNEL 	(kHAL_AudioMonoRight)

#define LEAUDIO_BIT_WIDTH 		(kHAL_AudioWordWidth16bits)
#define LEAUDIO_SAMPLING_RATE 	(kHAL_AudioSampleRate48KHz)
#define LEAUDIO_CODEC_DAC_VOLUME 		(100U) /* Range: 0 ~ 100 */
#define LEAUDIO_CODEC_HP_VOLUME  		(70U)  /* Range: 0 ~ 100 */

/* DMA */
#define LEAUDIO_CODEC_DMAMUX_INSTANCE 	(0U)
#define LEAUDIO_CODEC_DMA_INSTANCE    	(0U)
#define LEAUDIO_CODEC_TX_CHANNEL      	(0U)
#define LEAUDIO_CODEC_SAI_TX_SOURCE   	(kDmaRequestMuxSai1Tx)
#define LEAUDIO_CODEC_RX_CHANNEL      	(1U)
#define LEAUDIO_CODEC_SAI_RX_SOURCE   	(kDmaRequestMuxSai1Rx)

#if (defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))
/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define LEAUDIO_SAI1_CLOCK_SOURCE_SELECT 		(2U)
#define LEAUDIO_SAI1_CLOCK_SOURCE_PRE_DIVIDER 	(0U)
#define LEAUDIO_SAI1_CLOCK_SOURCE_DIVIDER 		(63U)
/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LEAUDIO_LPI2C_CLOCK_SOURCE_SELECT 		(0U)
/* Clock divider for master lpi2c clock source */
#define LEAUDIO_LPI2C_CLOCK_SOURCE_DIVIDER 	(5U)
/* Get frequency of lpi2c clock */
#define LEAUDIO_CODEC_I2C_FREQ ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LEAUDIO_LPI2C_CLOCK_SOURCE_DIVIDER + 1U))
#define LEAUDIO_SAI_CLK_FREQ                                                        \
    (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (LEAUDIO_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / \
     (LEAUDIO_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))
#elif defined(CPU_MIMXRT1176DVMAA_cm7)
/* Select Audio/Video PLL (393.24 MHz) as sai1 clock source */
#define LEAUDIO_SAI1_CLOCK_SOURCE_SELECT 		(4U)
#define LEAUDIO_SAI1_CLOCK_SOURCE_DIVIDER 		(16U)
#define LEAUDIO_SAI_CLK_FREQ (CLOCK_GetFreq(kCLOCK_AudioPll) / LEAUDIO_SAI1_CLOCK_SOURCE_DIVIDER)
#endif /*(defined(CPU_MIMXRT1062DVMAA_cm7) || defined(CPU_MIMXRT1062DVL6A_cm7) || defined(CPU_MIMXRT1062DVL6B_cm7))*/

#define BUFFER_SIZE   (960U)
#define BUFFER_NUMBER (4U)

/* --------------------------------------------- Exported Global Variables */
extern UINT32 leaudio_src_sf;
extern UCHAR leaudio_src_bps;
extern UCHAR leaudio_src_nc;
extern UINT16 leaudio_src_size;
extern UCHAR leaudio_src_playback;
extern BT_timer_handle leaudio_src_timer;
OSA_SEMAPHORE_HANDLE_DEFINE(xSemaphoreLEvoiceAudio);

extern UINT32 leaudio_snk_sf;
extern UCHAR  leaudio_snk_bps;
extern UCHAR  leaudio_snk_nc;
extern UINT16 leaudio_snk_size;
OSA_SEMAPHORE_HANDLE_DEFINE(xSemaphoreLEAudio);
OSA_SEMAPHORE_HANDLE_DEFINE(xSemaphoreLEsinkAudio);

#ifdef BT_GAM
extern UINT8 isroleCT;
extern UINT8 isroleCG;
#endif

TaskHandle_t leaudio_task_handler;
TaskHandle_t levoice_task_handler;

AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t sinkdummydata[BUFFER_SIZE], 4);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t Buffer[BUFFER_NUMBER * BUFFER_SIZE], 4);
static uint32_t tx_index = 0U, rx_index = 0U;
volatile static int emptyBlock = BUFFER_NUMBER;

uint32_t glbl_abs_volume=100;
bool glbl_volume_setting=false;

static INT32 rd_ptr = 0;
static INT32 wr_ptr = 0;
static INT32 buffer_size = 30*960;
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t src_buffer[30 * BUFFER_SIZE], 4);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t src_pcm_to_send[BUFFER_SIZE], 4);

BT_DEFINE_MUTEX(snk_th_mutex)
BT_DEFINE_COND(snk_th_cond)

#ifdef LE_AUDIO_CT_CG_TIME_DBG
#define TICKS_TO_MSEC(tick) ((uint32_t)((uint64_t)(tick)*1000uL / (uint64_t)configTICK_RATE_HZ))

static uint32_t diff;
static uint32_t startTime;
static uint32_t endTime;
static uint8_t firstTime = 1;
static uint32_t startTime1;
static uint32_t endTime1;
static uint8_t firstTime1 = 1;

static uint32_t startTime2;
static uint32_t endTime2;
static uint8_t firstTime2 = 1;
#endif


AT_NONCACHEABLE_SECTION_ALIGN( HAL_AUDIO_HANDLE_DEFINE(audio_tx_handle), 4);
AT_NONCACHEABLE_SECTION_ALIGN(HAL_AUDIO_HANDLE_DEFINE(audio_rx_handle), 4);

static codec_handle_t codec_handle;
static uint32_t audiosetup;

void LEAudiosinkTask(void *handle);
void le_voice_start_pl_ext(void);
static void LevoiceTask(void *handle);
void leaudio_pl_start_playback_timer(void);
void leaudio_init_default_src_pl(void);
extern void (* leaudio_src_cb)(const UCHAR *data, UINT16 datalen);
extern void (* leaudio_snk_cb)(const UCHAR *data, UINT16 datalen);

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void txCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam);
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
    .format           = {.mclk_HZ    = 24576000U/2,
                         .sampleRate = kWM8960_AudioSampleRate48KHz,
                         .bitWidth   = kWM8960_AudioBitWidth16bit},
    .master_slave     = true,
};
static codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm896xConfig};
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
static codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8962, .codecDevConfig = &wm896xConfig};
#endif /*defined(CODEC_WM8960_ENABLE)*/

/*setting for 44.1Khz*/
static const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 30,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    .numerator   = 106,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 1000, /* 30 bit denominator of fractional loop divider */
};
/*setting for multiple of 8Khz,such as 48Khz/16Khz/32KHz*/
static const clock_audio_pll_config_t audioPllConfig1 = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    .numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 100, /* 30 bit denominator of fractional loop divider */
};

static hal_audio_dma_mux_config_t audioTxDmaMuxConfig = {
    .dmaMuxConfig.dmaMuxInstance   = LEAUDIO_CODEC_DMAMUX_INSTANCE,
    .dmaMuxConfig.dmaRequestSource = LEAUDIO_CODEC_SAI_TX_SOURCE,
};

static hal_audio_dma_config_t audioTxDmaConfig = {
    .instance             = LEAUDIO_CODEC_DMA_INSTANCE,
    .channel              = LEAUDIO_CODEC_TX_CHANNEL,
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
    .srcClock_Hz       = 11289750U,
    .sampleRate_Hz     = (uint32_t)LEAUDIO_SAMPLING_RATE,
    .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(LEAUDIO_CODEC_SAI) / 2U,
    .msaterSlave       = kHAL_AudioMaster,
    .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    .frameSyncPolarity = kHAL_AudioBeginAtFallingEdge,
    .lineChannels      = LEAUDIO_DATA_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatI2sClassic,
    .bitWidth          = (uint8_t)LEAUDIO_BIT_WIDTH,
    .instance          = LEAUDIO_CODEC_INSTANCE,
};

static hal_audio_dma_mux_config_t audioRxDmaMuxConfig = {
    .dmaMuxConfig.dmaMuxInstance   = LEAUDIO_CODEC_DMAMUX_INSTANCE,
    .dmaMuxConfig.dmaRequestSource = LEAUDIO_CODEC_SAI_RX_SOURCE,
};

static hal_audio_dma_config_t audioRxDmaConfig = {
    .instance             = LEAUDIO_CODEC_DMA_INSTANCE,
    .channel              = LEAUDIO_CODEC_RX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = (void *)&audioRxDmaMuxConfig,
    .dmaChannelMuxConfig  = NULL,
};


static hal_audio_ip_config_t audioRxIpConfig = {
    .sai.lineMask = 1U << 0U,
    .sai.syncMode = kHAL_AudioSaiModeSync,
};

static hal_audio_config_t audioRxConfig = {
    .dmaConfig         = &audioRxDmaConfig,
    .ipConfig          = (void *)&audioRxIpConfig,
    .srcClock_Hz       = 0,
    .sampleRate_Hz     = 0,
    .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(LEAUDIO_CODEC_SAI) / 2U,
    .msaterSlave       = kHAL_AudioMaster,
    .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    .frameSyncPolarity = kHAL_AudioBeginAtFallingEdge,
    .lineChannels      = LEAUDIO_RXDATA_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatI2sClassic,
    .bitWidth          = (uint8_t)LEAUDIO_BIT_WIDTH,
    .instance          = LEAUDIO_CODEC_INSTANCE,
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
		 CLOCK_SetMux(kCLOCK_Lpi2cMux, LEAUDIO_LPI2C_CLOCK_SOURCE_SELECT);
		 CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LEAUDIO_LPI2C_CLOCK_SOURCE_DIVIDER);

		 /*Clock setting for SAI1*/
		 CLOCK_SetMux(kCLOCK_Sai1Mux, LEAUDIO_SAI1_CLOCK_SOURCE_SELECT);
		 CLOCK_SetDiv(kCLOCK_Sai1PreDiv, LEAUDIO_SAI1_CLOCK_SOURCE_PRE_DIVIDER);
		 CLOCK_SetDiv(kCLOCK_Sai1Div, LEAUDIO_SAI1_CLOCK_SOURCE_DIVIDER);
		 /* Enable MCLK output */
		 IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK;
#elif defined(CPU_MIMXRT1176DVMAA_cm7)
		 /*Clock setting for LPI2C*/
		 CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, 1);

		 /*Clock setting for SAI1*/
		 CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, LEAUDIO_SAI1_CLOCK_SOURCE_SELECT);
		 CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, LEAUDIO_SAI1_CLOCK_SOURCE_DIVIDER);

		 /* Enable MCLK output */
		 IOMUXC_GPR->GPR0 |= IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK;
#endif
    }

    return LEAUDIO_SAI_CLK_FREQ;
}

static void txCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
#ifdef LE_AUDIO_CT_CG_TIME_DBG
    if (!firstTime)
    {
        endTime = OSA_TimeGetMsec();
        diff = endTime - startTime;
    	printf ("sai txd in %dms\n", TICKS_TO_MSEC(diff));
    }
#endif

    if (kStatus_HAL_AudioError == completionStatus)
	{
		/* Handle the error. */
		printf("txcall backerr %x\n",completionStatus);
	}
    else
    {
    	OSA_SemaphorePost(xSemaphoreLEAudio);
    }
}

static void rxCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
#ifdef LE_AUDIO_CT_CG_TIME_DBG
    /* Get the current time */
	static int src_sent_ms1 = 0;
    int now_ms = BT_get_time_ms();

    if ((0U < src_sent_ms1) && ((now_ms - src_sent_ms1) > 10))
    {
		printf("mic-cb %d\n",(now_ms - src_sent_ms1));
    }
    src_sent_ms1 = now_ms;
#endif

    if (kStatus_HAL_AudioError == completionStatus)
    {
        /* Handle the error. */
    	printf("rxcall backerr %x\n",completionStatus);
    }
    else
    {
        emptyBlock--;
        OSA_SemaphorePost(xSemaphoreLEvoiceAudio);
    }
}

static void SAI_Codec_Start(UCHAR ep)
{
    CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
    if (AUDIO_EP_SINK == ep)
    {
    	CODEC_SetFormat(&codec_handle, audioTxConfig.srcClock_Hz, audioTxConfig.sampleRate_Hz, audioTxConfig.bitWidth);
    }
    if (AUDIO_EP_SOURCE == ep)
    {
        CODEC_SetFormat(&codec_handle, audioRxConfig.srcClock_Hz, audioRxConfig.sampleRate_Hz, audioRxConfig.bitWidth);
    }
    CODEC_SetVolume(&codec_handle, kCODEC_VolumeDAC, LEAUDIO_CODEC_DAC_VOLUME);
    CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, LEAUDIO_CODEC_HP_VOLUME);
    CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, false);
}

void init_leaudio()
{    
    OSA_SemaphoreCreateBinary(xSemaphoreLEvoiceAudio);
    OSA_SemaphoreCreateBinary(xSemaphoreLEAudio);
    OSA_SemaphoreCreateBinary(xSemaphoreLEsinkAudio);
    
    (void)BT_thread_mutex_init(&snk_th_mutex, NULL);
    (void)BT_thread_cond_init(&snk_th_cond, NULL);
    
    xTaskCreate(LevoiceTask, "SCO_Edma", 1024, NULL, BT_TASK_PRIORITY + 1, &levoice_task_handler);
    xTaskCreate(LEAudiosinkTask, "leaudio", 1024, NULL, BT_TASK_PRIORITY + 1, &leaudio_task_handler);

    if (CODEC_Init(&codec_handle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("codec init failed!\r\n");
    }
}

int snk_enqueue
          (
              /* IN */  UCHAR * data,
              /* IN */  uint16_t  datalen
          )
{
    int32_t  n_free;
    uint32_t count;

    //printf("datalen %d\n",datalen);

    BT_thread_mutex_lock(&snk_th_mutex);

    /*
     *  Get amount of free buffer space.
     */
    if (wr_ptr >= rd_ptr)
    {
        /*
         *    ----------DDDDDDDDDDDDDDD--------------X
         *    ^         ^              ^             ^
         *    |         |              |             |
         *    0       rd_ptr         wr_ptr         max
         *
         *  Free Space => '-----------'
         */
        n_free = (buffer_size - wr_ptr) + rd_ptr;
    }
    else
    {
        /*
         *    DDDDDDDDDD---------------DDDDDDDDDDDDDDX
         *    ^         ^              ^             ^
         *    |         |              |             |
         *    0       wr_ptr         rd_ptr         max
         *
         *  Free Space => '-----------'
         */
        n_free = rd_ptr - wr_ptr;
    }

    /*
     *  Do we have enough space to accomodate new data ?
     *  Buffer Empty Condition: when rd_ptr == wr_ptr
     *  Buffer Full  Condition: when diff(rd_ptr, wr_ptr) == 1
     */
    if ((n_free < 1) || (n_free < datalen))
    {
        /* Overflow ! */
        printf ("+ %d %d + ", n_free, datalen);
		BT_thread_mutex_unlock(&snk_th_mutex);
        return kStatus_Fail;
    }

    /* Store new data into Buffer */
    for (count = 0U; count < datalen; count++)
    {
        src_buffer[wr_ptr] = data[count];
        wr_ptr++;

        if (wr_ptr == buffer_size)
        {
            wr_ptr = 0;
        }
    }

    //printf("wr_ptr %d rd_ptr %d\n", wr_ptr, rd_ptr);

    //BT_thread_cond_signal(&snk_th_cond);

	BT_thread_mutex_unlock(&snk_th_mutex);

	return kStatus_Success;
}

int snk_dequeue(hal_audio_transfer_t *xfer, uint16_t expected_len)
{
    INT32  temp_rd_ptr, index, remaining;
    UINT16 bytes_to_send;

    //printf("snk_dequeue\n");
    BT_thread_mutex_lock(&snk_th_mutex);

    //BT_thread_cond_wait(&snk_th_cond, &snk_th_mutex);

    if (wr_ptr >= rd_ptr)
	{
		/*
			*    ----------DDDDDDDDDDDDDDD--------------X
			*    ^         ^              ^             ^
			*    |         |              |             |
			*    0       rd_ptr         wr_ptr         max
			*
			*  Free Space => '-----------'
			*/
		remaining = wr_ptr - rd_ptr;
	}
	else
	{
		/*
			*    DDDDDDDDDD---------------DDDDDDDDDDDDDDX
			*    ^         ^              ^             ^
			*    |         |              |             |
			*    0       wr_ptr         rd_ptr         max
			*
			*  Free Space => '-----------'
			*/
		remaining = buffer_size - (rd_ptr - wr_ptr);
	}

    /* Do we really have anything to read ? */
    if (remaining < expected_len)
    {
        BT_thread_mutex_unlock(&snk_th_mutex);
        return -1;
    }

    //printf("remaining %d wr_ptr %d rd_ptr %d\n", remaining, wr_ptr, rd_ptr);

    //bytes_to_send = (remaining >= expected_len) ? expected_len : (UINT16)remaining;
    bytes_to_send = expected_len;

    xfer->dataSize = bytes_to_send;
    //printf("bytes_to_send %d\n",bytes_to_send);

	temp_rd_ptr = rd_ptr;

	for (index = 0; index < bytes_to_send; index++)
	{
		src_pcm_to_send[index] = src_buffer[temp_rd_ptr];

		temp_rd_ptr = temp_rd_ptr + 1;
		if (temp_rd_ptr == buffer_size)
		{
			temp_rd_ptr = 0;
		}
	}

	xfer->data = &src_pcm_to_send[0];

	/* Update the read pointer */
	rd_ptr = temp_rd_ptr;

	BT_thread_mutex_unlock(&snk_th_mutex);

    return 0;
}

void leaudio_init_pl_ext (UCHAR role)
{
    if (AUDIO_EP_SOURCE == role)
    {
        leaudio_init_default_src_pl();
    }
}

void leaudio_shutdown_pl_ext ()
{
    if(audiosetup == 1)
    {
    	audiosetup = 0;
    	/* Stop Audio Player */
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
        HAL_AudioTxDeinit((hal_audio_handle_t)&audio_tx_handle[0]);
        HAL_AudioRxDeinit((hal_audio_handle_t)&audio_rx_handle[0]);
        (void)BOARD_SwitchAudioFreq(0U);
    }
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
        leaudio_src_cb = ep_cb;
        leaudio_src_sf = sf;
        leaudio_src_bps = bps;
        leaudio_src_nc = nc;
        leaudio_src_size = size;
    }

    if (AUDIO_EP_SINK == ep)
    {
    	leaudio_snk_cb = ep_cb;
    	leaudio_snk_sf = sf;
    	leaudio_snk_bps = bps;
    	leaudio_snk_nc = nc;
    	leaudio_snk_size = size;
    }

    /* Configuring CODEC when received configuration request*/
    if(audiosetup == 0)
	{
		audioTxConfig.sampleRate_Hz  = sf;
		audioTxConfig.srcClock_Hz    = BOARD_SwitchAudioFreq(audioTxConfig.sampleRate_Hz);

		HAL_AudioTxInit((hal_audio_handle_t)&audio_tx_handle[0], &audioTxConfig);//sink spk
		HAL_AudioTxInstallCallback((hal_audio_handle_t)&audio_tx_handle[0], txCallback, NULL);//sink spk

		audioRxConfig.sampleRate_Hz  = sf;
		audioRxConfig.srcClock_Hz    = BOARD_SwitchAudioFreq(audioRxConfig.sampleRate_Hz);

		HAL_AudioRxInit((hal_audio_handle_t)&audio_rx_handle[0], &audioRxConfig);//src mic
		HAL_AudioRxInstallCallback((hal_audio_handle_t)&audio_rx_handle[0], rxCallback, NULL);//src mic

		SAI_Codec_Start(ep);

		audiosetup = 1;
	}

    return result;
}

API_RESULT leaudio_start_pl_ext (UCHAR ep)
{
    if (AUDIO_EP_SOURCE == ep)
    {
        /* Start Audio Source */
        leaudio_src_timer = BT_TIMER_HANDLE_INIT_VAL;
#ifdef BT_GAM
        if( (isroleCG == 1) || ((isroleCT == 1)) )
        {
        	if(levoice_task_handler)
        	{
        		vTaskResume(levoice_task_handler);
        	}

        	le_voice_start_pl_ext();
        }
        else
#endif
	    {
            leaudio_pl_start_playback_timer();
    	}
        leaudio_src_playback = BT_TRUE;
    }

    if (AUDIO_EP_SINK == ep)
    {
        /* Start Audio Player */
		OSA_SemaphorePost(xSemaphoreLEsinkAudio);
		OSA_SemaphorePost(xSemaphoreLEAudio);
		if(leaudio_task_handler)
		{
			vTaskResume(leaudio_task_handler);
		}
    }

    return API_SUCCESS;
}

API_RESULT leaudio_stop_pl_ext (UCHAR ep)
{
    if (AUDIO_EP_SOURCE == ep)
    {
        /* Stop Audio Source */
        leaudio_src_playback = BT_FALSE;
#ifdef BT_GAM
        if( (isroleCG == 1) || ((isroleCT == 1)) )
        {
        	HAL_AudioTransferAbortSend((hal_audio_handle_t)&audio_rx_handle[0]);
        	if(levoice_task_handler)
        	{
        		vTaskSuspend(levoice_task_handler);
        	}
        }
        else
#endif
        {
            BT_stop_timer(leaudio_src_timer);
            leaudio_src_timer = BT_TIMER_HANDLE_INIT_VAL;
        }
    }

    if (AUDIO_EP_SINK == ep)
    {
        /* Stop Audio Player */
        HAL_AudioTransferAbortSend((hal_audio_handle_t)&audio_tx_handle[0]);
        if(leaudio_task_handler)
        {
        	vTaskSuspend(leaudio_task_handler);
        }
    }

    return API_SUCCESS;
}

void leaudio_write_pl_ext (UCHAR ep, UCHAR * m_data, UINT16 m_datalen)
{
    if (AUDIO_EP_SINK != ep)
    {
        return;
    }

#ifdef LE_AUDIO_CT_CG_TIME_DBG
    if (firstTime1){
		startTime1 = OSA_TimeGetMsec();
		firstTime1 = 0;
	}
	else
	{
        endTime1 = OSA_TimeGetMsec();
		//if (TICKS_TO_MSEC(endTime1 - startTime1) > 10)
		{
			printf("rx=%dms\n", TICKS_TO_MSEC(endTime1 - startTime1));
		}
        startTime1 = endTime1;
	}
#endif

    snk_enqueue(m_data, m_datalen);
}

void audio_codec_setabsvol_pl_ext(UCHAR volume)
{
    glbl_abs_volume = ( ( volume * 100U ) / 255U );
    if(audiosetup == 1)
    {
        CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, glbl_abs_volume);
    }
}

void audio_codec_setmute_pl_ext()
{
    glbl_volume_setting = true;
    if(audiosetup == 1)
    {
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight |kCODEC_PlayChannelHeadphoneLeft, glbl_volume_setting);
    }
}

void audio_codec_setunmute_pl_ext()
{
    glbl_volume_setting = false;
    if(audiosetup == 1)
    {
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight |kCODEC_PlayChannelHeadphoneLeft, glbl_volume_setting);
    }
}

/*******************************************************************************
 * Code
 ******************************************************************************/
static void LevoiceTask(void *handle)
{
	hal_audio_status_t err;
	hal_audio_transfer_t xfer = {0};

#ifdef LE_AUDIO_CT_CG_TIME_DBG
    INT32 now_ms, period_ms, now_ms1;
    static INT32 src_sent_ms;
    static UINT16 src_num_samples;
    static UINT32 src_missed_count;
#endif

    while (1)
    {
        OSA_SemaphoreWait(xSemaphoreLEvoiceAudio, osaWaitForever_c);

#ifdef LE_AUDIO_CT_CG_TIME_DBG
        /* Get the current time */
        now_ms = BT_get_time_ms();
        //if ((0U < src_sent_ms) && ((now_ms - src_sent_ms) > 10))
        {
			printf("tx=%d\n", (now_ms - src_sent_ms));
        }
        src_sent_ms = now_ms;
#endif

        if(leaudio_src_sf == 48000)
    		xfer.dataSize = 960;
    	else if(leaudio_src_sf == 44100)
    		xfer.dataSize = 960;
    	else if(leaudio_src_sf == 32000)
    		xfer.dataSize = 640;
    	else if(leaudio_src_sf == 24000)
    		xfer.dataSize = 480;
    	else if(leaudio_src_sf == 16000)
    		xfer.dataSize = 320;
    	else if(leaudio_src_sf == 8000)
    		xfer.dataSize = 160;

        if(leaudio_src_playback == BT_TRUE)
        {
            /* consuming index */
			if (emptyBlock < BUFFER_NUMBER)
			{
				leaudio_src_cb((const UCHAR *)&Buffer[tx_index * xfer.dataSize], xfer.dataSize);
				tx_index++;
				if (tx_index == BUFFER_NUMBER)
				{
					tx_index = 0U;
				}
				emptyBlock++;
			}
			/* producing index*/
			if (emptyBlock > 0)
			{
				xfer.data     = Buffer + rx_index * xfer.dataSize;

				err = HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&audio_rx_handle[0], &xfer);
				if(err == kStatus_HAL_AudioQueueFull)
				{
					//printf("audio_rx_handle[0].xferDmaHandle.queueUser\r\n");
				}
				if (err == kStatus_HAL_AudioSuccess)
				{
				   rx_index++;
				}
				if (rx_index == BUFFER_NUMBER)
				{
					rx_index = 0U;
				}
			}
        }
    }
}

void LEAudiosinkTask(void *handle)
{
	int ret;
	hal_audio_transfer_t xfer = {0};

#ifdef LE_AUDIO_CT_CG_TIME_DBG
    TickType_t current_ticks = 0;
#endif

	while (1U)
	{
		OSA_SemaphoreWait(xSemaphoreLEsinkAudio, osaWaitForever_c);

		if(leaudio_snk_sf == 48000)
			xfer.dataSize = 960;
		else if(leaudio_snk_sf == 44100)
			xfer.dataSize = 960;
		else if(leaudio_snk_sf == 32000)
			xfer.dataSize = 640;
		else if(leaudio_snk_sf == 24000)
			xfer.dataSize = 480;
		else if(leaudio_snk_sf == 16000)
			xfer.dataSize = 320;
		else if(leaudio_snk_sf == 8000)
			xfer.dataSize = 160;

        xfer.data = sinkdummydata;

		OSA_SemaphoreWait(xSemaphoreLEAudio, osaWaitForever_c);

        ret = snk_dequeue(&xfer, xfer.dataSize);
	    if (ret == -1)
		{
		    xfer.data = sinkdummydata;
		}

#ifdef LE_AUDIO_CT_CG_TIME_DBG
        if (firstTime){
			firstTime =0;
		}
		startTime = OSA_TimeGetMsec();
#endif

		ret = HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&audio_tx_handle[0], &xfer);
		if (ret != kStatus_HAL_AudioSuccess)
		{
			PRINTF("prime fail %d\r\n", ret); // may come due to datasize, data mismatch
		}

		OSA_SemaphorePost(xSemaphoreLEsinkAudio);
	}
}

/*!
 * @brief le_voice_start_pl_ext function
 */
void le_voice_start_pl_ext(void)
{
	hal_audio_transfer_t xfer = {0};
	hal_audio_status_t err;

	if(leaudio_src_sf == 48000)
		xfer.dataSize = 960;
	else if(leaudio_src_sf == 44100)
		xfer.dataSize = 960;
	else if(leaudio_src_sf == 32000)
		xfer.dataSize = 640;
	else if(leaudio_src_sf == 24000)
		xfer.dataSize = 480;
	else if(leaudio_src_sf == 16000)
		xfer.dataSize = 320;
	else if(leaudio_src_sf == 8000)
		xfer.dataSize = 160;

	for (uint8_t index = 0; index < BUFFER_NUMBER; ++index)
	{
		xfer.data = Buffer + rx_index * xfer.dataSize;

		err = HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&audio_rx_handle[0], &xfer);
		if (err == kStatus_HAL_AudioSuccess)
		{
			rx_index++;
		}
		if (rx_index == BUFFER_NUMBER)
		{
			rx_index = 0U;
		}
	}
}

// manage buffer size to 640 in mic case using a2dp_src_sf
// For global data buffer access, use mutex apis
// Manage JPL kind on layer for LE audio sink side from core stack itself
// Add recovery mechanism when tx callback is stuck and tx queue full
// calculate transmit delay currently 10ms for both mic and media
/* calculate time task delay time based on codec configuration for Rx and Tx sai edma */

