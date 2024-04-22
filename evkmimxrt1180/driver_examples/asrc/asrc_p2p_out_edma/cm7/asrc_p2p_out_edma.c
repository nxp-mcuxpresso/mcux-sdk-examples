/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_sai_edma.h"
#include "music.h"
#include "fsl_codec_common.h"
#include "fsl_asrc_edma.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif
#include "fsl_wm8962.h"
#include "fsl_edma.h"
#include "fsl_trdc.h"
#include "fsl_ele_base_api.h"
#include "fsl_codec_adapter.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* SAI instance and clock */
// #define DEMO_CODEC_WM8960
#define DEMO_SAI         SAI1
#define DEMO_SAI_IRQ     SAI1_IRQn
#define SAI_TxIRQHandler SAI1_IRQHandler

#define DEMO_SAI_DMA               DMA3
#define DEMO_SAI_DMA_CHANNEL       kDma3RequestMuxSai1Tx
#define DEMO_ASRC_DMA              DMA4
#define DEMO_ASRC_IN_EDMA_CHANNEL  kDma4RequestMuxASRCRequest1
#define DEMO_ASRC_OUT_EDMA_CHANNEL kDma4RequestMuxASRCRequest4
/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_SELECT (2U)
/* Clock pre divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER (1U)
/* Clock divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_DIVIDER (63U)
/* Get frequency of sai1 clock */
#define DEMO_SAI_CLK_FREQ                                                        \
    (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (DEMO_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / \
     (DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))

#define DEMO_AUDIO_MASTER_CLOCK          (24576000U / 2U)
#define DEMO_ASRC_OUTPUT_SOURCE_CLOCK_HZ (16 * kSAI_SampleRate32KHz * 2)
#define DEMO_ASRC_PERIPHERAL_CLOCK       200000000U
#define DEMO_ASRC                        ASRC
#define DEMO_ASRC_CHANNEL_PAIR           kASRC_ChannelPairA

#define DEMO_AUDIO_SAMPLE_RATE_IN  (kSAI_SampleRate48KHz)
#define DEMO_AUDIO_SAMPLE_RATE_OUT (kSAI_SampleRate32KHz)

/* I2C instance and clock */
#define DEMO_I2C LPI2C1

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define DEMO_I2C_CLK_FREQ ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (DEMO_LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

/* DMA */
// #define EXAMPLE_DMA     DMA1
// #define EXAMPLE_CHANNEL       (0U)
#define DEMO_ASRC_IN_CHANNEL  (1U)
#define DEMO_ASRC_OUT_CHANNEL (4U)
#define DEMO_SAI_CHANNEL      (0U)

#define DEMO_CODEC_VOLUME              (78U)
#define DEMO_EDMA_HAS_CHANNEL_CONFIG   1
#define BOARD_SAI_EDMA_CONFIG(config)  Board_SaiEdmaConfig(config)
#define BOARD_ASRC_EDMA_CONFIG(config) Board_AsrcEdmaConfig(config)
#define ELE_TRDC_AON_ID    0x74
#define ELE_TRDC_WAKEUP_ID 0x78
#define ELE_CORE_CM33_ID   0x1
#define ELE_CORE_CM7_ID    0x2

/*
 * Set ELE_STICK_FAILED_STS to 0 when ELE status check is not required,
 * which is useful when debug reset, where the core has already get the
 * TRDC ownership at first time and ELE is not able to release TRDC
 * ownership again for the following TRDC ownership request.
 */
#define ELE_STICK_FAILED_STS 1

#if ELE_STICK_FAILED_STS
#define ELE_IS_FAILED(x) (x != kStatus_Success)
#else
#define ELE_IS_FAILED(x) false
#endif
/* demo audio data channel */
#define DEMO_AUDIO_DATA_CHANNEL (2U)
/* demo audio bit width */
#define DEMO_AUDIO_BIT_WIDTH          kSAI_WordWidth16bits
#define DEMO_ASRC_CONVERT_BUFFER_SIZE (100 * 1000U)
#ifndef DEMO_ASRC_OUTPUT_CLOCK_SOURCE
#define DEMO_ASRC_OUTPUT_CLOCK_SOURCE kASRC_ClockSourceBitClock0_SAI1_TX
#endif
#ifndef DEMO_CODEC_VOLUME
#define DEMO_CODEC_VOLUME 100U
#endif

edma_config_t dmaConfig = {0};
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void Board_SaiEdmaConfig(edma_config_t *config);
void Board_AsrcEdmaConfig(edma_config_t *config);
static void startSai(bool start);
/*******************************************************************************
 * Variables
 ******************************************************************************/
edma_channel_config_t channelConfig = {
    .enableMasterIDReplication = true,
    .securityLevel             = kEDMA_ChannelSecurityLevelSecure,
    .protectionLevel           = kEDMA_ChannelProtectionLevelPrivileged,
};

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
    .format       = {.mclk_HZ    = 12288000U,
                     .sampleRate = kWM8962_AudioSampleRate16KHz,
                     .bitWidth   = kWM8962_AudioBitWidth16bit},
    .masterSlave  = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8962, .codecDevConfig = &wm8962Config};
/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM) / (2^POST)
 *                              = 24 * (32 + 768/1000)  / 2
 *                              = 393.216MHZ
 */
const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 32,   /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,    /* Divider after the PLL, should only be 0, 1, 2, 3, 4, 5 */
    .numerator   = 768,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 1000, /* 30 bit denominator of fractional loop divider */
};
edma_handle_t asrcInDmaHandle  = {0};
edma_handle_t asrcOutDmaHandle = {0};
AT_QUICKACCESS_SECTION_DATA(asrc_edma_handle_t asrcHandle);
static volatile bool isASRCFinished = false;
extern codec_config_t boardCodecConfig;
#if (defined(FSL_FEATURE_SAI_HAS_MCR) && (FSL_FEATURE_SAI_HAS_MCR)) || \
    (defined(FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) && (FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER))
sai_master_clock_t mclkConfig = {
#if defined(FSL_FEATURE_SAI_HAS_MCR) && (FSL_FEATURE_SAI_HAS_MCR)
    .mclkOutputEnable = true,
#if !(defined(FSL_FEATURE_SAI_HAS_NO_MCR_MICS) && (FSL_FEATURE_SAI_HAS_NO_MCR_MICS))
    .mclkSource = kSAI_MclkSourceSysclk,
#endif
#endif
};
#endif
codec_handle_t codecHandle;
uint8_t asrcConvertBuffer[DEMO_ASRC_CONVERT_BUFFER_SIZE] = {0U};
asrc_channel_pair_config_t s_asrcChannelPairConfig       = {
    .audioDataChannels         = kASRC_ChannelsNumber2,
    .inClockSource             = kASRC_ClockSourceNotAvalible,
    .inSourceClock_Hz          = 0,
    .outClockSource            = DEMO_ASRC_OUTPUT_CLOCK_SOURCE,
    .outSourceClock_Hz         = DEMO_ASRC_OUTPUT_SOURCE_CLOCK_HZ,
    .sampleRateRatio           = kASRC_RatioUseIdealRatio,
    .inDataWidth               = kASRC_DataWidth16Bit,
    .inDataAlign               = kASRC_DataAlignLSB,
    .outDataWidth              = kASRC_DataWidth16Bit,
    .outDataAlign              = kASRC_DataAlignLSB,
    .outSignExtension          = kASRC_NoSignExtension,
    .outFifoThreshold          = FSL_ASRC_CHANNEL_PAIR_FIFO_DEPTH / 8,
    .inFifoThreshold           = FSL_ASRC_CHANNEL_PAIR_FIFO_DEPTH / 2,
    .bufStallWhenFifoEmptyFull = false,
};
static asrc_p2p_edma_config_t s_asrcEDMAP2PConfig = {
    .startPeripheral = startSai,
};
/*******************************************************************************
 * Code
 ******************************************************************************/
void SEI_EAR_TRDC_EDMA4_ResetPermissions()
{
    uint8_t i, j;
    /* Set the master domain access configuration for eDMA4 */
    trdc_non_processor_domain_assignment_t edma4Assignment;
    (void)memset(&edma4Assignment, 0, sizeof(edma4Assignment));
    edma4Assignment.domainId = 0x7U;
    /* Use the bus master's privileged/user attribute directly */
    edma4Assignment.privilegeAttr = kTRDC_MasterPrivilege;
    /* Use the bus master's secure/nonsecure attribute directly */
    edma4Assignment.secureAttr = kTRDC_MasterSecure;
    /* Use the DID input as the domain indentifier */
    edma4Assignment.bypassDomainId = true;
    edma4Assignment.lock           = false;
    TRDC_SetNonProcessorDomainAssignment(TRDC2, kTRDC2_MasterDMA4, &edma4Assignment);

    /* Enable all access modes for MBC and MRC. */
    trdc_hardware_config_t hwConfig;
    TRDC_GetHardwareConfig(TRDC2, &hwConfig);

    trdc_memory_access_control_config_t memAccessConfig;
    (void)memset(&memAccessConfig, 0, sizeof(memAccessConfig));

    memAccessConfig.nonsecureUsrX  = 1U;
    memAccessConfig.nonsecureUsrW  = 1U;
    memAccessConfig.nonsecureUsrR  = 1U;
    memAccessConfig.nonsecurePrivX = 1U;
    memAccessConfig.nonsecurePrivW = 1U;
    memAccessConfig.nonsecurePrivR = 1U;
    memAccessConfig.secureUsrX     = 1U;
    memAccessConfig.secureUsrW     = 1U;
    memAccessConfig.secureUsrR     = 1U;
    memAccessConfig.securePrivX    = 1U;
    memAccessConfig.securePrivW    = 1U;
    memAccessConfig.securePrivR    = 1U;
    for (i = 0U; i < hwConfig.mrcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MrcSetMemoryAccessConfig(TRDC2, &memAccessConfig, i, j);
        }
    }

    for (i = 0U; i < hwConfig.mbcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MbcSetMemoryAccessConfig(TRDC2, &memAccessConfig, i, j);
        }
    }
}

void SEI_EAR_TRDC_EDMA3_ResetPermissions()
{
    uint8_t i, j;
    /* Set the master domain access configuration for eDMA3 */
    trdc_non_processor_domain_assignment_t edma3Assignment;
    (void)memset(&edma3Assignment, 0, sizeof(edma3Assignment));
    edma3Assignment.domainId = 0x7U;
    /* Use the bus master's privileged/user attribute directly */
    edma3Assignment.privilegeAttr = kTRDC_MasterPrivilege;
    /* Use the bus master's secure/nonsecure attribute directly */
    edma3Assignment.secureAttr = kTRDC_MasterSecure;
    /* Use the DID input as the domain indentifier */
    edma3Assignment.bypassDomainId = true;
    edma3Assignment.lock           = false;
    TRDC_SetNonProcessorDomainAssignment(TRDC1, kTRDC1_MasterDMA3, &edma3Assignment);

    /* Enable all access modes for MBC and MRC. */
    trdc_hardware_config_t hwConfig;
    TRDC_GetHardwareConfig(TRDC1, &hwConfig);

    trdc_memory_access_control_config_t memAccessConfig;
    (void)memset(&memAccessConfig, 0, sizeof(memAccessConfig));

    memAccessConfig.nonsecureUsrX  = 1U;
    memAccessConfig.nonsecureUsrW  = 1U;
    memAccessConfig.nonsecureUsrR  = 1U;
    memAccessConfig.nonsecurePrivX = 1U;
    memAccessConfig.nonsecurePrivW = 1U;
    memAccessConfig.nonsecurePrivR = 1U;
    memAccessConfig.secureUsrX     = 1U;
    memAccessConfig.secureUsrW     = 1U;
    memAccessConfig.secureUsrR     = 1U;
    memAccessConfig.securePrivX    = 1U;
    memAccessConfig.securePrivW    = 1U;
    memAccessConfig.securePrivR    = 1U;
    for (i = 0U; i < hwConfig.mrcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MrcSetMemoryAccessConfig(TRDC1, &memAccessConfig, i, j);
        }
    }

    for (i = 0U; i < hwConfig.mbcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MbcSetMemoryAccessConfig(TRDC1, &memAccessConfig, i, j);
        }
    }
}

void BOARD_EnableSaiMclkOutput(bool enable)
{
    if (enable)
    {
        BLK_CTRL_NS_AONMIX->SAI1_MCLK_CTRL |= BLK_CTRL_NS_AONMIX_SAI1_MCLK_CTRL_SAI1_MCLK_DIR_MASK;
    }
    else
    {
        BLK_CTRL_NS_AONMIX->SAI1_MCLK_CTRL &= ~BLK_CTRL_NS_AONMIX_SAI1_MCLK_CTRL_SAI1_MCLK_DIR_MASK;
    }
}


void Board_SaiEdmaConfig(edma_config_t *config)
{
    config->enableMasterIdReplication       = true;
    config->channelConfig[DEMO_SAI_CHANNEL] = &channelConfig;
}

void Board_AsrcEdmaConfig(edma_config_t *config)
{
    config->enableMasterIdReplication            = true;
    config->channelConfig[DEMO_ASRC_IN_CHANNEL]  = &channelConfig;
    config->channelConfig[DEMO_ASRC_OUT_CHANNEL] = &channelConfig;
}
static void asrc_callback(ASRC_Type *base, asrc_edma_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_ASRCOutQueueIdle)
    {
        isASRCFinished = true;
    }
}

static void startSai(bool start)
{
    if (start)
    {
        SAI_TxEnable(DEMO_SAI, true);
    }
    else
    {
        SAI_TxEnable(DEMO_SAI, false);
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    sai_transceiver_t config;
    uint8_t *audioData            = (uint8_t *)music;
    uint32_t outputBufSize        = 0U;
    asrc_transfer_t asrcConvert;

    status_t sts;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /*Workaround to make SAI1 CLK Root output 12MHz*/
    CLOCK_InitAudioPll(&audioPllConfig);
    
    /* Get ELE FW status */
    do
    {
        uint32_t ele_fw_sts;
        sts = ELE_BaseAPI_GetFwStatus(MU_RT_S3MUA, &ele_fw_sts);
    } while (sts != kStatus_Success);

    /* Release TRDC A to CM7 core */
    do
    {
        sts = ELE_BaseAPI_ReleaseRDC(MU_RT_S3MUA, ELE_TRDC_AON_ID, ELE_CORE_CM7_ID);
    } while (ELE_IS_FAILED(sts));

    /* Release TRDC W to CM7 core */
    do
    {
        sts = ELE_BaseAPI_ReleaseRDC(MU_RT_S3MUA, ELE_TRDC_WAKEUP_ID, ELE_CORE_CM7_ID);
    } while (ELE_IS_FAILED(sts));

    SEI_EAR_TRDC_EDMA3_ResetPermissions();
    SEI_EAR_TRDC_EDMA4_ResetPermissions();

#if 0
#ifdef ASRC_SAI_DMA
    EDMA_SetChannelMux(DEMO_DMA_SAI, DEMO_SAI_TX_EDMA_CHANNEL, DEMO_SAI_TX_EDMA_CHANNEL);
#endif
    EDMA_SetChannelMux(EXAMPLE_DMA, DEMO_ASRC_IN_CHANNEL, DEMO_ASRC_IN_EDMA_CHANNEL);
    EDMA_SetChannelMux(EXAMPLE_DMA, DEMO_ASRC_OUT_CHANNEL, DEMO_ASRC_OUT_EDMA_CHANNEL);
#endif
    /*Clock setting for LPI2C*/
    CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c0102, 0);

    /*Clock setting for SAI1*/
    CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, 2);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, 32);
    /* 0SC400M */
    CLOCK_SetRootClockMux(kCLOCK_Root_Asrc, 2);
    /* divider 2 */
    CLOCK_SetRootClockDiv(kCLOCK_Root_Asrc, 2);

    /*Enable MCLK clock*/
    BOARD_EnableSaiMclkOutput(true);

    PRINTF("ASRC p2p edma example\n\r");

    EDMA_GetDefaultConfig(&dmaConfig);
    BOARD_ASRC_EDMA_CONFIG(&dmaConfig);
    EDMA_Init(DEMO_ASRC_DMA, &dmaConfig);
    EDMA_CreateHandle(&asrcInDmaHandle, DEMO_ASRC_DMA, DEMO_ASRC_IN_CHANNEL);
    EDMA_CreateHandle(&asrcOutDmaHandle, DEMO_ASRC_DMA, DEMO_ASRC_OUT_CHANNEL);

#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(DEMO_ASRC_DMA, DEMO_ASRC_IN_CHANNEL, DEMO_ASRC_IN_EDMA_CHANNEL);
    EDMA_SetChannelMux(DEMO_ASRC_DMA, DEMO_ASRC_OUT_CHANNEL, DEMO_ASRC_OUT_EDMA_CHANNEL);
    EDMA_SetChannelMux(DEMO_SAI_DMA, DEMO_SAI_CHANNEL, DEMO_SAI_DMA_CHANNEL);
#endif

    /* SAI init */
    SAI_Init(DEMO_SAI);
    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&config, DEMO_AUDIO_BIT_WIDTH, kSAI_Stereo, kSAI_Channel0Mask);
    SAI_TxSetConfig(DEMO_SAI, &config);
    SAI_TxEnableDMA(DEMO_SAI, kSAI_FIFORequestDMAEnable, true);

    /* set bit clock divider */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE_OUT, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);
    /* asrc init */
    ASRC_Init(DEMO_ASRC, DEMO_ASRC_PERIPHERAL_CLOCK);
    ASRC_TransferInCreateHandleEDMA(DEMO_ASRC, &asrcHandle, DEMO_ASRC_CHANNEL_PAIR, asrc_callback, &asrcInDmaHandle,
                                    NULL, NULL);
    ASRC_TransferOutCreateHandleEDMA(DEMO_ASRC, &asrcHandle, DEMO_ASRC_CHANNEL_PAIR, asrc_callback, &asrcOutDmaHandle,
                                     &s_asrcEDMAP2PConfig, NULL);
    ASRC_TransferSetChannelPairConfigEDMA(DEMO_ASRC, &asrcHandle, &s_asrcChannelPairConfig, DEMO_AUDIO_SAMPLE_RATE_IN,
                                          DEMO_AUDIO_SAMPLE_RATE_OUT);

    /* Use default setting to init codec */
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }

    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
                        DEMO_CODEC_VOLUME) != kStatus_Success)
    {
        assert(false);
    }

    PRINTF("Playback converted audio data\r\n");
    PRINTF("    sample rate : %d\r\n", DEMO_AUDIO_SAMPLE_RATE_OUT);
    PRINTF("    channel number: %d\r\n", DEMO_AUDIO_DATA_CHANNEL);
    PRINTF("    frequency: 215HZ.\r\n\r\n");

    outputBufSize = ASRC_GetOutSamplesSizeEDMA(DEMO_ASRC, &asrcHandle, DEMO_AUDIO_SAMPLE_RATE_IN,
                                               DEMO_AUDIO_SAMPLE_RATE_OUT, MUSIC_LEN);

    asrcConvert.inData      = audioData;
    asrcConvert.inDataSize  = MUSIC_LEN;
    asrcConvert.outData     = (void *)SAI_TxGetDataRegisterAddress(DEMO_SAI, DEMO_SAI_CHANNEL);
    asrcConvert.outDataSize = outputBufSize;
    ASRC_TransferEDMA(ASRC, &asrcHandle, &asrcConvert);

    while (!isASRCFinished)
    {
    }

    PRINTF("ASRC p2p edma example finished\n\r ");
    while (1)
    {
    }
}
