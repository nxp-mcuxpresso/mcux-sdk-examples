/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017, 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
/*${standard_header_anchor}*/
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_audio.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "fsl_adapter_audio.h"
#include "audio_speaker.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */


#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif

#include "fsl_sctimer.h"
#include "usb_audio_config.h"
#include "fsl_i2c.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"
#include "fsl_wm8904.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_I2S_TX_INSTANCE_INDEX (0U)
#define DEMO_DMA_INSTANCE_INDEX    (0U)
#define DEMO_I2S_TX_CHANNEL        (13)

#define USB_AUDIO_ENTER_CRITICAL() \
                                   \
    OSA_SR_ALLOC();                \
                                   \
    OSA_ENTER_CRITICAL()

#define USB_AUDIO_EXIT_CRITICAL() OSA_EXIT_CRITICAL()
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APPInit(void);
usb_status_t USB_DeviceAudioProcessTerminalRequest(uint32_t audioCommand,
                                                   uint32_t *length,
                                                   uint8_t **buffer,
                                                   uint8_t entityOrEndpoint);
void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

extern void Init_Board_Audio(void);
extern void audio_fro_trim_up(void);
extern void audio_fro_trim_down(void);
extern void AUDIO_DMA_EDMA_Start();
extern void BOARD_Codec_Init();
/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_audio_speaker_struct_t g_UsbDeviceAudioSpeaker;
extern uint8_t audioPlayDataBuff[AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE];
hal_audio_transfer_t s_TxTransfer;
HAL_AUDIO_HANDLE_DEFINE(audioTxHandle);
hal_audio_config_t audioTxConfig;
hal_audio_dma_config_t dmaTxConfig;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t audioPlayDMATempBuff[AUDIO_PLAY_TRANSFER_SIZE];
codec_handle_t codecHandle;

wm8904_config_t wm8904Config = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .recordSource = kWM8904_RecordSourceLineInput,
    .recordChannelLeft  = kWM8904_RecordChannelLeft2,
    .recordChannelRight = kWM8904_RecordChannelRight2,
    .playSource         = kWM8904_PlaySourceDAC,
    .slaveAddress       = WM8904_I2C_ADDRESS,
    .protocol           = kWM8904_ProtocolI2S,
    .format             = {.sampleRate = kWM8904_SampleRate48kHz, .bitWidth = kWM8904_BitWidth16},
    .mclk_HZ            = 24576000U,
    .master             = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8904, .codecDevConfig = &wm8904Config};
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayDataBuff[AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayPacket[(FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE)];

static uint32_t eventCounterU = 0;
static uint32_t captureRegisterNumber;
static sctimer_config_t sctimerInfo;
extern hal_audio_config_t audioTxConfig;
extern HAL_AUDIO_HANDLE_DEFINE(audioTxHandle);
extern uint8_t
    g_UsbDeviceInterface[USB_AUDIO_SPEAKER_INTERFACE_COUNT]; /* Default value of audio speaker device struct */

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_SetupOutBuffer[8];
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_audio_speaker_struct_t g_UsbDeviceAudioSpeaker = {
    .deviceHandle                           = NULL,
    .currentStreamOutMaxPacketSize          = FS_ISO_OUT_ENDP_PACKET_SIZE,
    .attach                                 = 0U,
    .currentStreamInterfaceAlternateSetting = 0U,
    .copyProtect                            = 0x01U,
    .curMute                                = 0x00U,
    .curVolume                              = {0x00U, 0x1fU},
    .minVolume                              = {0x00U, 0x00U},
    .maxVolume                              = {0x00U, 0x43U},
    .resVolume                              = {0x01U, 0x00U},
    .curBass                                = 0x00U,
    .minBass                                = 0x80U,
    .maxBass                                = 0x7FU,
    .resBass                                = 0x01U,
    .curMid                                 = 0x00U,
    .minMid                                 = 0x80U,
    .maxMid                                 = 0x7FU,
    .resMid                                 = 0x01U,
    .curTreble                              = 0x01U,
    .minTreble                              = 0x80U,
    .maxTreble                              = 0x7FU,
    .resTreble                              = 0x01U,
    .curAutomaticGain                       = 0x01U,
    .curDelay                               = {0x00U, 0x40U},
    .minDelay                               = {0x00U, 0x00U},
    .maxDelay                               = {0xFFU, 0xFFU},
    .resDelay                               = {0x00U, 0x01U},
    .curLoudness                            = 0x01U,
    .curSamplingFrequency                   = {0x00U, 0x00U, 0x01U},
    .minSamplingFrequency                   = {0x00U, 0x00U, 0x01U},
    .maxSamplingFrequency                   = {0x00U, 0x00U, 0x01U},
    .resSamplingFrequency                   = {0x00U, 0x00U, 0x01U},
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    .curMute20          = 0U,
    .curClockValid      = 1U,
    .curVolume20        = {0x00U, 0x1FU},
    .curSampleFrequency = 48000U, /* This should be changed to 48000 if sampling rate is 48k */
    .freqControlRange   = {1U, 48000U, 48000U, 0U},
    .volumeControlRange = {1U, 0x8001U, 0x7FFFU, 1U},
#endif
    .speed                      = USB_SPEED_FULL,
    .tdWriteNumberPlay          = 0,
    .tdReadNumberPlay           = 0,
    .audioSendCount             = {0, 0},
    .audioSpeakerReadDataCount  = {0, 0},
    .audioSpeakerWriteDataCount = {0, 0},
    .usbRecvCount               = 0,
    .audioSendTimes             = 0,
    .usbRecvTimes               = 0,
    .startPlayFlag              = 0,
    .speakerIntervalCount       = 0,
    .pllAdjustIntervalCount     = 0,
    .speakerReservedSpace       = 0,
    .timesFeedbackCalculate     = 0,
    .speakerDetachOrNoInput     = 0,
    .curAudioPllFrac            = AUDIO_PLL_FRACTIONAL_DIVIDER,
    .audioPllTicksPrev          = 0,
    .audioPllTicksDiff          = 0,
    .audioPllTicksEma           = AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT,
    .audioPllTickEmaFrac        = 0,
    .audioPllTickBasedPrecision = AUDIO_PLL_FRACTIONAL_CHANGE_STEP,
};
/*******************************************************************************
 * Code
 ******************************************************************************/
extern void WM8904_USB_Audio_Init(void *I2CBase);
void WM8904_Config_Audio_Formats(uint32_t samplingRate);

void audio_fro_trim_up(void)
{
    static uint8_t usbClkAdj = 0;
    if (0U == usbClkAdj)
    {
        /* USBCLKADJ is turned off, start using software adjustment */
        SYSCON->FROCTRL = (SYSCON->FROCTRL & ~(SYSCON_FROCTRL_USBCLKADJ_MASK));
        usbClkAdj       = 1U;
    }
    uint32_t val    = SYSCON->FROCTRL;
    val             = (val & ~(0xff << 16)) | ((((val >> 16) & 0xFF) + 1) << 16) | (1UL << 31);
    SYSCON->FROCTRL = val;
}

void audio_fro_trim_down(void)
{
    static uint8_t usbClkAdj = 0;
    if (0U == usbClkAdj)
    {
        /* USBCLKADJ is turned off, start using software adjustment */
        SYSCON->FROCTRL = (SYSCON->FROCTRL & ~(SYSCON_FROCTRL_USBCLKADJ_MASK));
        usbClkAdj       = 1U;
    }
    uint32_t val    = SYSCON->FROCTRL;
    SYSCON->FROCTRL = (val & ~(0xff << 16)) | ((((val >> 16) & 0xFF) - 1) << 16) | (1UL << 31);
}


void BOARD_Codec_Init()
{
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }
    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, 0x0020) !=
        kStatus_Success)
    {
        assert(false);
    }
}

static void TxCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    uint32_t audioSpeakerPreReadDataCount = 0U;
    uint32_t preAudioSendCount            = 0U;

    if ((g_UsbDeviceAudioSpeaker.audioSendTimes >= g_UsbDeviceAudioSpeaker.usbRecvTimes) &&
        (g_UsbDeviceAudioSpeaker.startPlayFlag == 1))
    {
        g_UsbDeviceAudioSpeaker.startPlayFlag          = 0;
        g_UsbDeviceAudioSpeaker.speakerDetachOrNoInput = 1;
    }
    if (g_UsbDeviceAudioSpeaker.startPlayFlag)
    {
        s_TxTransfer.dataSize = AUDIO_PLAY_TRANSFER_SIZE;
        s_TxTransfer.data     = audioPlayDataBuff + g_UsbDeviceAudioSpeaker.tdReadNumberPlay;
        preAudioSendCount     = g_UsbDeviceAudioSpeaker.audioSendCount[0];
        g_UsbDeviceAudioSpeaker.audioSendCount[0] += AUDIO_PLAY_TRANSFER_SIZE;
        if (preAudioSendCount > g_UsbDeviceAudioSpeaker.audioSendCount[0])
        {
            g_UsbDeviceAudioSpeaker.audioSendCount[1] += 1U;
        }
        g_UsbDeviceAudioSpeaker.audioSendTimes++;
        g_UsbDeviceAudioSpeaker.tdReadNumberPlay += AUDIO_PLAY_TRANSFER_SIZE;
        if (g_UsbDeviceAudioSpeaker.tdReadNumberPlay >=
            AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE)
        {
            g_UsbDeviceAudioSpeaker.tdReadNumberPlay = 0;
        }
        audioSpeakerPreReadDataCount = g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[0];
        g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[0] += AUDIO_PLAY_TRANSFER_SIZE;
        if (audioSpeakerPreReadDataCount > g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[0])
        {
            g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[1] += 1U;
        }
    }
    else
    {
        s_TxTransfer.dataSize = AUDIO_PLAY_TRANSFER_SIZE;
        s_TxTransfer.data     = audioPlayDMATempBuff;
    }
    HAL_AudioTransferSendNonBlocking(handle, &s_TxTransfer);
}

void AUDIO_DMA_EDMA_Start()
{
    usb_echo("Init Audio I2S and CODEC\r\n");
    s_TxTransfer.dataSize = AUDIO_PLAY_TRANSFER_SIZE;
    s_TxTransfer.data     = audioPlayDMATempBuff;
    HAL_AudioTxInstallCallback(&audioTxHandle[0], TxCallback, (void *)&s_TxTransfer);
    HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&audioTxHandle[0], &s_TxTransfer);
    /* need to queue two transmit buffers so when the first one
     * finishes transfer, the other immediatelly starts */
    HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&audioTxHandle[0], &s_TxTransfer);
}

#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
void USB0_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_UsbDeviceAudioSpeaker.deviceHandle);
}
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
void USB1_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_UsbDeviceAudioSpeaker.deviceHandle);
}
#endif
void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbSrcFro, CLOCK_GetFroHfFreq());
#if defined(FSL_FEATURE_USB_USB_RAM) && (FSL_FEATURE_USB_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USB_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USB_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif

#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbhs0DeviceClock(kCLOCK_UsbSrcUsbPll, 0U);
#if defined(FSL_FEATURE_USBHSD_USB_RAM) && (FSL_FEATURE_USBHSD_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USBHSD_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USBHSD_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif
#endif
}
void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USB_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0];
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USBHSD_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Hs0];
#endif
    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
}
#endif

usb_status_t USB_DeviceAudioGetControlTerminal(
    usb_device_handle handle, usb_setup_struct_t *setup, uint32_t *length, uint8_t **buffer, uint8_t entityId)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint32_t audioCommand   = 0U;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;

    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_TE_COPY_PROTECT_CONTROL:
                    if (USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID == entityId)
                    {
                        audioCommand = USB_DEVICE_AUDIO_TE_GET_CUR_COPY_PROTECT_CONTROL;
                    }
                    else
                    {
                        /* Input Terminals only support the Get Terminal Copy Protect Control request */
                    }
                    break;
                case USB_DEVICE_AUDIO_TE_CONNECTOR_CONTROL:
                    audioCommand = USB_DEVICE_AUDIO_TE_GET_CUR_CONNECTOR_CONTROL;
                    break;
                case USB_DEVICE_AUDIO_TE_OVERLOAD_CONTROL:
                    audioCommand = USB_DEVICE_AUDIO_TE_GET_CUR_OVERLOAD_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
#else
        case USB_DEVICE_AUDIO_GET_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_TE_COPY_PROTECT_CONTROL:
                    if (USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID == entityId)
                    {
                        audioCommand = USB_DEVICE_AUDIO_TE_GET_CUR_COPY_PROTECT_CONTROL;
                    }
                    else
                    {
                        /* Input Terminals only support the Get Terminal Copy Protect Control request */
                    }
                    break;
                default:
                    /*no action*/
                    break;
            }
#endif
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioSetControlTerminal(
    usb_device_handle handle, usb_setup_struct_t *setup, uint32_t *length, uint8_t **buffer, uint8_t entityId)
{
    uint32_t audioCommand   = 0U;
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;

    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_TE_COPY_PROTECT_CONTROL:
                    if (USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID == entityId)
                    {
                        audioCommand = USB_DEVICE_AUDIO_TE_SET_CUR_COPY_PROTECT_CONTROL;
                    }
                    else
                    {
                        return kStatus_USB_InvalidRequest; /* Output Terminals only support the Set Terminal Copy
                                                     Protect Control request */
                    }
                    break;
                default:
                    /*no action*/
                    break;
            }
#else
        case USB_DEVICE_AUDIO_GET_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_TE_COPY_PROTECT_CONTROL:
                    if (USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID == entityId)
                    {
                        audioCommand = USB_DEVICE_AUDIO_TE_SET_CUR_COPY_PROTECT_CONTROL;
                    }
                    else
                    {
                        return kStatus_USB_InvalidRequest; /* Output Terminals only support the Set Terminal Copy
                                                     Protect Control request */
                    }
                    break;
                default:
                    /*no action*/
                    break;
            }
#endif
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioGetCurAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_MUTE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_AUTOMATIC_GAIN_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_BOOST_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_BASS_BOOST_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_LOUDNESS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_LOUDNESS_CONTROL;
            break;
        default:
            break;
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
usb_status_t USB_DeviceAudioGetRangeAudioFeatureUnit(usb_device_handle handle,
                                                     usb_setup_struct_t *setup,
                                                     uint32_t *length,
                                                     uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select GET RANGE request Control Feature Unit Module */
    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            break;
        default:
            /*no action*/
            break;
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}
#endif

usb_status_t USB_DeviceAudioGetMinAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioGetMaxAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioGetResAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);

    return error;
}

usb_status_t USB_DeviceAudioSetCurAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_MUTE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_AUTOMATIC_GAIN_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_BOOST_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_BASS_BOOST_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_LOUDNESS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_LOUDNESS_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioSetMinAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);

    return error;
}

usb_status_t USB_DeviceAudioSetMaxAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);

    return error;
}

usb_status_t USB_DeviceAudioSetResAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioGetFeatureUnit(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    /* Select SET request Control Feature Unit Module */
    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            error = USB_DeviceAudioGetCurAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_RANGE_REQUEST:
            error = USB_DeviceAudioGetRangeAudioFeatureUnit(handle, setup, length, buffer);
            break;
        default:
            /*no action*/
            break;
#else
        case USB_DEVICE_AUDIO_GET_CUR_REQUEST:
            error = USB_DeviceAudioGetCurAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_GET_MIN_REQUEST:
            error = USB_DeviceAudioGetMinAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_GET_MAX_REQUEST:
            error = USB_DeviceAudioGetMaxAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_GET_RES_REQUEST:
            error = USB_DeviceAudioGetResAudioFeatureUnit(handle, setup, length, buffer);
            break;
        default:
            break;
#endif
    }
    return error;
}

usb_status_t USB_DeviceAudioSetFeatureUnit(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    /* Select SET request Control Feature Unit Module */
    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            error = USB_DeviceAudioSetCurAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_RANGE_REQUEST:
            break;
        default:
            /*no action*/
            break;
#else
        case USB_DEVICE_AUDIO_SET_CUR_REQUEST:
            error = USB_DeviceAudioSetCurAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_SET_MIN_REQUEST:
            error = USB_DeviceAudioSetMinAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_SET_MAX_REQUEST:
            error = USB_DeviceAudioSetMaxAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_SET_RES_REQUEST:
            error = USB_DeviceAudioSetResAudioFeatureUnit(handle, setup, length, buffer);
            break;
        default:
            break;
#endif
    }
    return error;
}

#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
usb_status_t USB_DeviceAudioSetClockSource(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (setup->bRequest)
    {
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_CS_SAM_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL;
                    break;
                case USB_DEVICE_AUDIO_CS_CLOCK_VALID_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
        case USB_DEVICE_AUDIO_RANGE_REQUEST:
            break;
        default:
            /*no action*/
            break;
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioGetClockSource(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (setup->bRequest)
    {
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_CS_SAM_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL;
                    break;
                case USB_DEVICE_AUDIO_CS_CLOCK_VALID_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_RANGE_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_CS_SAM_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        default:
            /*no action*/
            break;
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}
#endif

usb_status_t USB_DeviceAudioSetRequestInterface(usb_device_handle handle,
                                                usb_setup_struct_t *setup,
                                                uint32_t *length,
                                                uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint8_t entityId   = (uint8_t)(setup->wIndex >> 0x08);

    if ((USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID == entityId) ||
        (USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID == entityId))
    {
        error = USB_DeviceAudioSetControlTerminal(handle, setup, length, buffer, entityId);
    }
    else if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityId)
    {
        error = USB_DeviceAudioSetFeatureUnit(handle, setup, length, buffer);
    }
    else
    {
    }
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    if (USB_AUDIO_SPEAKER_CONTROL_CLOCK_SOURCE_ENTITY_ID == entityId)
    {
        error = USB_DeviceAudioSetClockSource(handle, setup, length, buffer);
    }
#endif

    return error;
}

usb_status_t USB_DeviceAudioGetRequestInterface(usb_device_handle handle,
                                                usb_setup_struct_t *setup,
                                                uint32_t *length,
                                                uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint8_t entityId   = (uint8_t)(setup->wIndex >> 0x08);

    if ((USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID == entityId) ||
        (USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID == entityId))
    {
        error = USB_DeviceAudioGetControlTerminal(handle, setup, length, buffer, entityId);
    }
    else if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityId)
    {
        error = USB_DeviceAudioGetFeatureUnit(handle, setup, length, buffer);
    }
    else
    {
    }
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    if (USB_AUDIO_SPEAKER_CONTROL_CLOCK_SOURCE_ENTITY_ID == entityId)
    {
        error = USB_DeviceAudioGetClockSource(handle, setup, length, buffer);
    }
#endif
    return error;
}

usb_status_t USB_DeviceAudioSetRequestEndpoint(usb_device_handle handle,
                                               usb_setup_struct_t *setup,
                                               uint32_t *length,
                                               uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t endpoint        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_PITCH_CONTROL_SELECTOR_AUDIO20:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_CUR_PITCH_CONTROL_AUDIO20;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        default:
            /*no action*/
            break;
#else
        case USB_DEVICE_AUDIO_SET_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_CUR_SAMPLING_FREQ_CONTROL;
                    break;
                case USB_DEVICE_AUDIO_EP_PITCH_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_CUR_PITCH_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MIN_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_MIN_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MAX_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_MAX_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_SET_RES_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_RES_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;

        default:
            break;
#endif
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, endpoint); /* endpoint is not used */
    return error;
}

usb_status_t USB_DeviceAudioGetRequestEndpoint(usb_device_handle handle,
                                               usb_setup_struct_t *setup,
                                               uint32_t *length,
                                               uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t endpoint        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_DATA_OVERRUN_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_GET_CUR_DATA_OVERRUN_CONTROL;
                    break;
                case USB_DEVICE_AUDIO_EP_DATA_UNDERRUN_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_GET_CUR_DATA_UNDERRUN_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
        default:
            /*no action*/
            break;
#else
        case USB_DEVICE_AUDIO_GET_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_GET_CUR_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_GET_MIN_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_GET_MIN_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_GET_MAX_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:

                    audioCommand = USB_DEVICE_AUDIO_EP_GET_MAX_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_GET_RES_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_GET_RES_SAMPLING_FREQ_CONTROL;

                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        default:
            /*no action*/
            break;
#endif
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, endpoint); /* endpoint is not used */
    return error;
}

usb_status_t USB_DeviceProcessClassRequest(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error          = kStatus_USB_InvalidRequest;
    uint8_t interfaceOrEndpoint = (uint8_t)setup->wIndex;

    if ((setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) == USB_REQUEST_TYPE_RECIPIENT_ENDPOINT)
    {
        if ((interfaceOrEndpoint == USB_AUDIO_SPEAKER_STREAM_ENDPOINT) ||
            (interfaceOrEndpoint == USB_AUDIO_CONTROL_ENDPOINT))
        {
            switch (setup->bmRequestType)
            {
                case USB_DEVICE_AUDIO_SET_REQUEST_ENDPOINT:
                    error = USB_DeviceAudioSetRequestEndpoint(handle, setup, length, buffer);
                    break;
                case USB_DEVICE_AUDIO_GET_REQUEST_ENDPOINT:
                    error = USB_DeviceAudioGetRequestEndpoint(handle, setup, length, buffer);
                    break;
                default:
                    break;
            }
        }
    }
    else if ((setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) == USB_REQUEST_TYPE_RECIPIENT_INTERFACE)
    {
        if (USB_AUDIO_CONTROL_INTERFACE_INDEX == interfaceOrEndpoint)
        {
            switch (setup->bmRequestType)
            {
                case USB_DEVICE_AUDIO_SET_REQUEST_INTERFACE:
                    error = USB_DeviceAudioSetRequestInterface(handle, setup, length, buffer);
                    break;
                case USB_DEVICE_AUDIO_GET_REQUEST_INTERFACE:
                    error = USB_DeviceAudioGetRequestInterface(handle, setup, length, buffer);
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        /* no action */
    }

    return error;
}

usb_status_t USB_DeviceAudioProcessTerminalRequest(uint32_t audioCommand,
                                                   uint32_t *length,
                                                   uint8_t **buffer,
                                                   uint8_t entityOrEndpoint)
{
    usb_status_t error = kStatus_USB_Success;
    uint8_t *volBuffAddr;
#if (!USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint16_t volume;
#endif

    switch (audioCommand)
    {
        case USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            *buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.curMute20;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curMute20);
#else
            *buffer = &g_UsbDeviceAudioSpeaker.curMute;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curMute);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            *buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.curVolume20;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curVolume20);
#else
            *buffer = g_UsbDeviceAudioSpeaker.curVolume;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curVolume);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.curBass;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.curMid;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.curTreble;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.curAutomaticGain;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL:
            *buffer = g_UsbDeviceAudioSpeaker.curDelay;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL:
            *buffer = g_UsbDeviceAudioSpeaker.minVolume;
            *length = sizeof(g_UsbDeviceAudioSpeaker.minVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.minBass;
            *length = sizeof(g_UsbDeviceAudioSpeaker.minBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.minMid;
            *length = sizeof(g_UsbDeviceAudioSpeaker.minMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.minTreble;
            *length = sizeof(g_UsbDeviceAudioSpeaker.minTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL:
            *buffer = g_UsbDeviceAudioSpeaker.minDelay;
            *length = sizeof(g_UsbDeviceAudioSpeaker.minDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL:
            *buffer = g_UsbDeviceAudioSpeaker.maxVolume;
            *length = sizeof(g_UsbDeviceAudioSpeaker.maxVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.maxBass;
            *length = sizeof(g_UsbDeviceAudioSpeaker.maxBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.maxMid;
            *length = sizeof(g_UsbDeviceAudioSpeaker.maxMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.maxTreble;
            *length = sizeof(g_UsbDeviceAudioSpeaker.maxTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL:
            *buffer = g_UsbDeviceAudioSpeaker.maxDelay;
            *length = sizeof(g_UsbDeviceAudioSpeaker.maxDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL:
            *buffer = g_UsbDeviceAudioSpeaker.resVolume;
            *length = sizeof(g_UsbDeviceAudioSpeaker.resVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.resBass;
            *length = sizeof(g_UsbDeviceAudioSpeaker.resBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.resMid;
            *length = sizeof(g_UsbDeviceAudioSpeaker.resMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL:
            *buffer = &g_UsbDeviceAudioSpeaker.resTreble;
            *length = sizeof(g_UsbDeviceAudioSpeaker.resTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL:
            *buffer = g_UsbDeviceAudioSpeaker.resDelay;
            *length = sizeof(g_UsbDeviceAudioSpeaker.resDelay);
            break;
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL:
            *buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.curSampleFrequency;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curSampleFrequency);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL:
            g_UsbDeviceAudioSpeaker.curSampleFrequency = *(uint32_t *)(*buffer);
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL:
            *buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.curClockValid;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curClockValid);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL:
            g_UsbDeviceAudioSpeaker.curClockValid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL:
            *buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.volumeControlRange;
            *length = sizeof(g_UsbDeviceAudioSpeaker.volumeControlRange);
            break;
        case USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL:
            *buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.freqControlRange;
            *length = sizeof(g_UsbDeviceAudioSpeaker.freqControlRange);
            break;
#else
        case USB_DEVICE_AUDIO_EP_GET_CUR_SAMPLING_FREQ_CONTROL:
            *buffer = g_UsbDeviceAudioSpeaker.curSamplingFrequency;
            *length = sizeof(g_UsbDeviceAudioSpeaker.curSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MIN_SAMPLING_FREQ_CONTROL:
            *buffer = g_UsbDeviceAudioSpeaker.minSamplingFrequency;
            *length = sizeof(g_UsbDeviceAudioSpeaker.minSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MAX_SAMPLING_FREQ_CONTROL:
            *buffer = g_UsbDeviceAudioSpeaker.maxSamplingFrequency;
            *length = sizeof(g_UsbDeviceAudioSpeaker.maxSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_RES_SAMPLING_FREQ_CONTROL:
            *buffer = g_UsbDeviceAudioSpeaker.resSamplingFrequency;
            *length = sizeof(g_UsbDeviceAudioSpeaker.resSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_SET_CUR_SAMPLING_FREQ_CONTROL:
            g_UsbDeviceAudioSpeaker.curSamplingFrequency[0] = **(buffer);
            g_UsbDeviceAudioSpeaker.curSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_MIN_SAMPLING_FREQ_CONTROL:
            g_UsbDeviceAudioSpeaker.minSamplingFrequency[0] = **(buffer);
            g_UsbDeviceAudioSpeaker.minSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_MAX_SAMPLING_FREQ_CONTROL:
            g_UsbDeviceAudioSpeaker.maxSamplingFrequency[0] = **(buffer);
            g_UsbDeviceAudioSpeaker.maxSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_RES_SAMPLING_FREQ_CONTROL:
            g_UsbDeviceAudioSpeaker.resSamplingFrequency[0] = **(buffer);
            g_UsbDeviceAudioSpeaker.resSamplingFrequency[1] = *((*buffer) + 1);
            break;
#endif
        case USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            volBuffAddr                            = *buffer;
            g_UsbDeviceAudioSpeaker.curVolume20[0] = *(volBuffAddr);
            g_UsbDeviceAudioSpeaker.curVolume20[1] = *(volBuffAddr + 1);
            g_UsbDeviceAudioSpeaker.codecTask |= VOLUME_CHANGE_TASK;
#else
            volBuffAddr                          = *buffer;
            g_UsbDeviceAudioSpeaker.curVolume[0] = *volBuffAddr;
            g_UsbDeviceAudioSpeaker.curVolume[1] = *(volBuffAddr + 1);
            volume                               = (uint16_t)((uint16_t)g_UsbDeviceAudioSpeaker.curVolume[1] << 8U);
            volume |= (uint8_t)(g_UsbDeviceAudioSpeaker.curVolume[0]);
            g_UsbDeviceAudioSpeaker.codecTask |= VOLUME_CHANGE_TASK;
            /* If needs print information while adjusting the volume, please enable the following sentence. */
            /* usb_echo("Set Cur Volume : %x\r\n", volume); */
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_UsbDeviceAudioSpeaker.curMute20 = **(buffer);
            if (g_UsbDeviceAudioSpeaker.curMute20)
            {
                g_UsbDeviceAudioSpeaker.codecTask |= MUTE_CODEC_TASK;
            }
            else
            {
                g_UsbDeviceAudioSpeaker.codecTask |= UNMUTE_CODEC_TASK;
            }
#else
            g_UsbDeviceAudioSpeaker.curMute = **(buffer);
            if (g_UsbDeviceAudioSpeaker.curMute)
            {
                g_UsbDeviceAudioSpeaker.codecTask |= MUTE_CODEC_TASK;
            }
            else
            {
                g_UsbDeviceAudioSpeaker.codecTask |= UNMUTE_CODEC_TASK;
            }
            /* If needs print information while adjusting the volume, please enable the following sentence. */
            /* usb_echo("Set Cur Mute : %x\r\n", g_UsbDeviceAudioSpeaker.curMute); */
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL:
            g_UsbDeviceAudioSpeaker.curBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL:
            g_UsbDeviceAudioSpeaker.curMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL:
            g_UsbDeviceAudioSpeaker.curTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            g_UsbDeviceAudioSpeaker.curAutomaticGain = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL:
            g_UsbDeviceAudioSpeaker.curDelay[0] = **(buffer);
            g_UsbDeviceAudioSpeaker.curDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_UsbDeviceAudioSpeaker.curDelay[2] = *((*buffer) + 2);
            g_UsbDeviceAudioSpeaker.curDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL:
            g_UsbDeviceAudioSpeaker.minVolume[0] = **(buffer);
            g_UsbDeviceAudioSpeaker.minVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL:
            g_UsbDeviceAudioSpeaker.minBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL:
            g_UsbDeviceAudioSpeaker.minMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL:
            g_UsbDeviceAudioSpeaker.minTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL:
            g_UsbDeviceAudioSpeaker.minDelay[0] = **(buffer);
            g_UsbDeviceAudioSpeaker.minDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_UsbDeviceAudioSpeaker.minDelay[2] = *((*buffer) + 2);
            g_UsbDeviceAudioSpeaker.minDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL:
            g_UsbDeviceAudioSpeaker.maxVolume[0] = **(buffer);
            g_UsbDeviceAudioSpeaker.maxVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL:
            g_UsbDeviceAudioSpeaker.maxBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL:
            g_UsbDeviceAudioSpeaker.maxMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL:
            g_UsbDeviceAudioSpeaker.maxTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL:
            g_UsbDeviceAudioSpeaker.maxDelay[0] = **(buffer);
            g_UsbDeviceAudioSpeaker.maxDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_UsbDeviceAudioSpeaker.maxDelay[2] = *((*buffer) + 2);
            g_UsbDeviceAudioSpeaker.maxDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL:
            g_UsbDeviceAudioSpeaker.resVolume[0] = **(buffer);
            g_UsbDeviceAudioSpeaker.resVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL:
            g_UsbDeviceAudioSpeaker.resBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL:
            g_UsbDeviceAudioSpeaker.resMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL:
            g_UsbDeviceAudioSpeaker.resTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL:
            g_UsbDeviceAudioSpeaker.resDelay[0] = **(buffer);
            g_UsbDeviceAudioSpeaker.resDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_UsbDeviceAudioSpeaker.resDelay[2] = *((*buffer) + 2);
            g_UsbDeviceAudioSpeaker.resDelay[3] = *((*buffer) + 3);
#endif
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}

/* The USB_AudioSpeakerBufferSpaceUsed() function gets the used speaker ringbuffer size */
uint32_t USB_AudioSpeakerBufferSpaceUsed(void)
{
    uint64_t write_count = 0U;
    uint64_t read_count  = 0U;

    write_count = (uint64_t)((((uint64_t)g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[1]) << 32U) |
                             g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0]);
    read_count  = (uint64_t)((((uint64_t)g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[1]) << 32U) |
                            g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[0]);

    if (write_count >= read_count)
    {
        return (uint32_t)(write_count - read_count);
    }
    else
    {
        return 0;
    }
}

#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
static int32_t audioSpeakerUsedDiff = 0x0, audioSpeakerDiffThres = 0x0;
static uint32_t audioSpeakerUsedSpace = 0x0, audioSpeakerLastUsedSpace = 0x0;
void USB_AudioFSSync()
{
    if (1U == g_UsbDeviceAudioSpeaker.stopDataLengthAudioSpeakerAdjust)
    {
        g_UsbDeviceAudioSpeaker.speakerIntervalCount = 1U;
        return;
    }

    if (g_UsbDeviceAudioSpeaker.speakerIntervalCount != AUDIO_FRO_ADJUST_INTERVAL)
    {
        g_UsbDeviceAudioSpeaker.speakerIntervalCount++;
        return;
    }
    g_UsbDeviceAudioSpeaker.speakerIntervalCount = 1;
    g_UsbDeviceAudioSpeaker.timesFeedbackCalculate++;
    if (g_UsbDeviceAudioSpeaker.timesFeedbackCalculate == 2)
    {
        audioSpeakerUsedSpace     = USB_AudioSpeakerBufferSpaceUsed();
        audioSpeakerLastUsedSpace = audioSpeakerUsedSpace;
    }

    if (g_UsbDeviceAudioSpeaker.timesFeedbackCalculate > 2)
    {
        audioSpeakerUsedSpace = USB_AudioSpeakerBufferSpaceUsed();
        audioSpeakerUsedDiff += (audioSpeakerUsedSpace - audioSpeakerLastUsedSpace);
        audioSpeakerLastUsedSpace = audioSpeakerUsedSpace;

        if ((audioSpeakerUsedDiff > -AUDIO_SAMPLING_RATE_KHZ) && (audioSpeakerUsedDiff < AUDIO_SAMPLING_RATE_KHZ))
        {
            audioSpeakerDiffThres = 4 * AUDIO_SAMPLING_RATE_KHZ;
        }
        if (audioSpeakerUsedDiff <= -audioSpeakerDiffThres)
        {
            audioSpeakerDiffThres += 4 * AUDIO_SAMPLING_RATE_KHZ;
            audio_fro_trim_down();
        }
        if (audioSpeakerUsedDiff >= audioSpeakerDiffThres)
        {
            audioSpeakerDiffThres += 4 * AUDIO_SAMPLING_RATE_KHZ;
            audio_fro_trim_up();
        }
    }
}
#endif

/* The USB_AudioSpeakerPutBuffer() function fills the audioRecDataBuff with audioPlayPacket in every callback*/
void USB_AudioSpeakerPutBuffer(uint8_t *buffer, uint32_t size)
{
    uint32_t remainBufferSpace;
    uint32_t audioSpeakerPreWriteDataCount;

#if ((defined(USB_AUDIO_CHANNEL7_1) && (USB_AUDIO_CHANNEL7_1 > 0U)) || \
     (defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)) || \
     (defined(USB_AUDIO_CHANNEL3_1) && (USB_AUDIO_CHANNEL3_1 > 0U)))
#if (defined(AUDIO_SPEAKER_MULTI_CHANNEL_PLAY) && (AUDIO_SPEAKER_MULTI_CHANNEL_PLAY > 0U))
    uint8_t empty_channel = DEMO_TDM_AUDIO_OUTPUT_CHANNEL_COUNT - AUDIO_FORMAT_CHANNELS;
    uint8_t *buffer_temp;
    uint32_t buffer_length = 0;
    uint8_t fill_zero      = 1U;
    uint8_t channel_data   = 0U;

    remainBufferSpace =
        (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE) - USB_AudioSpeakerBufferSpaceUsed();
    if (size > remainBufferSpace) /* discard the overflow data */
    {
        if (remainBufferSpace > (AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE))
        {
            size = (remainBufferSpace - (AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE));
        }
        else
        {
            size = 0;
        }
    }
    for (uint32_t i = 0; i < size; i += (AUDIO_FORMAT_SIZE * AUDIO_FORMAT_CHANNELS))
    {
        buffer_temp  = buffer + i;
        fill_zero    = 1U;
        channel_data = 0U;
        for (uint32_t j = 0; j < (AUDIO_FORMAT_SIZE * AUDIO_FORMAT_CHANNELS); j++)
        {
            /* make up 32 bits */
            if (1U == fill_zero)
            {
                for (uint8_t k = 0U; k < (4U - AUDIO_FORMAT_SIZE); k++)
                {
                    audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay] = 0;
                    g_UsbDeviceAudioSpeaker.tdWriteNumberPlay++;
                    if (g_UsbDeviceAudioSpeaker.tdWriteNumberPlay >=
                        (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE))
                    {
                        g_UsbDeviceAudioSpeaker.tdWriteNumberPlay = 0;
                    }
                }
                fill_zero = 0;
            }
            audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay] = *(buffer_temp + j);
            g_UsbDeviceAudioSpeaker.tdWriteNumberPlay++;
            if (g_UsbDeviceAudioSpeaker.tdWriteNumberPlay >=
                (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE))
            {
                g_UsbDeviceAudioSpeaker.tdWriteNumberPlay = 0;
            }

            if (4U != AUDIO_FORMAT_SIZE)
            {
                channel_data++;
                if (channel_data == AUDIO_FORMAT_SIZE)
                {
                    fill_zero    = 1U;
                    channel_data = 0U;
                }
            }
        }
        /* if totoal channel is not 8, fill 0 to meet 8 channels for TDM */
        if (empty_channel)
        {
            buffer_length = g_UsbDeviceAudioSpeaker.tdWriteNumberPlay + (4U * empty_channel);
            if (buffer_length < (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE))
            {
                memset((void *)(&audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay]), 0U, 4U * empty_channel);
                g_UsbDeviceAudioSpeaker.tdWriteNumberPlay += (4U * empty_channel);
            }
            else
            {
                uint32_t firstLength = (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE) -
                                       g_UsbDeviceAudioSpeaker.tdWriteNumberPlay;
                memset((void *)(&audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay]), 0U, firstLength);
                buffer_length = (4U * empty_channel) - firstLength; /* the remain data length */
                if ((buffer_length) > 0U)
                {
                    memset((void *)(&audioPlayDataBuff[0]), 0U, buffer_length);
                    g_UsbDeviceAudioSpeaker.tdWriteNumberPlay = buffer_length;
                }
                else
                {
                    g_UsbDeviceAudioSpeaker.tdWriteNumberPlay = 0;
                }
            }
        }
    }
    audioSpeakerPreWriteDataCount = g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0];
    g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0] += (DEMO_TDM_AUDIO_OUTPUT_CHANNEL_COUNT * 4U);
    if (audioSpeakerPreWriteDataCount > g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0])
    {
        g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[1] += 1U;
    }
#else
    uint8_t *buffer_temp;
    uint32_t play2ChannelLength;
    remainBufferSpace =
        (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE) - USB_AudioSpeakerBufferSpaceUsed();
    if ((size % (AUDIO_FORMAT_SIZE * AUDIO_FORMAT_CHANNELS)) != 0U)
    {
        size = (size / (AUDIO_FORMAT_SIZE * AUDIO_FORMAT_CHANNELS)) * (AUDIO_FORMAT_SIZE * AUDIO_FORMAT_CHANNELS);
    }
    play2ChannelLength = size / (AUDIO_FORMAT_CHANNELS / 2U); /* only play the selected two channels */
    if (play2ChannelLength > remainBufferSpace)               /* discard the overflow data */
    {
        play2ChannelLength = remainBufferSpace;
    }
    for (uint32_t i = 0; i < size; i += AUDIO_FORMAT_SIZE * AUDIO_FORMAT_CHANNELS)
    {
        buffer_temp = buffer + i;
        if (play2ChannelLength <= 0U)
        {
            break;
        }
        for (uint32_t j = 0; j < AUDIO_FORMAT_SIZE * 2; j++)
        {
            if (play2ChannelLength)
            {
#if ((defined(USB_AUDIO_7_1_CHANNEL_PAIR_SEL) && (0x01 == USB_AUDIO_7_1_CHANNEL_PAIR_SEL)) || \
     (defined(USB_AUDIO_5_1_CHANNEL_PAIR_SEL) && (0x01 == USB_AUDIO_5_1_CHANNEL_PAIR_SEL)) || \
     (defined(USB_AUDIO_3_1_CHANNEL_PAIR_SEL) && (0x01 == USB_AUDIO_3_1_CHANNEL_PAIR_SEL)))
                audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay] = *(buffer_temp + j);
#endif
#if ((defined(USB_AUDIO_7_1_CHANNEL_PAIR_SEL) && (0x02 == USB_AUDIO_7_1_CHANNEL_PAIR_SEL)) || \
     (defined(USB_AUDIO_5_1_CHANNEL_PAIR_SEL) && (0x02 == USB_AUDIO_5_1_CHANNEL_PAIR_SEL)) || \
     (defined(USB_AUDIO_3_1_CHANNEL_PAIR_SEL) && (0x02 == USB_AUDIO_3_1_CHANNEL_PAIR_SEL)))
                audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay] =
                    *(buffer_temp + j + AUDIO_FORMAT_SIZE * 2);
#endif
#if ((defined(USB_AUDIO_7_1_CHANNEL_PAIR_SEL) && (0x03 == USB_AUDIO_7_1_CHANNEL_PAIR_SEL)) || \
     (defined(USB_AUDIO_5_1_CHANNEL_PAIR_SEL) && (0x03 == USB_AUDIO_5_1_CHANNEL_PAIR_SEL)))
                audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay] =
                    *(buffer_temp + j + AUDIO_FORMAT_SIZE * 4);
#elif (defined(USB_AUDIO_7_1_CHANNEL_PAIR_SEL) && (0x04 == USB_AUDIO_7_1_CHANNEL_PAIR_SEL))
                audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay] =
                    *(buffer_temp + j + AUDIO_FORMAT_SIZE * 6);
#endif
                play2ChannelLength--;
                g_UsbDeviceAudioSpeaker.tdWriteNumberPlay++;
                if (g_UsbDeviceAudioSpeaker.tdWriteNumberPlay >=
                    (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE))
                {
                    g_UsbDeviceAudioSpeaker.tdWriteNumberPlay = 0;
                }
            }
            else
            {
                break;
            }
        }
    }
    audioSpeakerPreWriteDataCount = g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0];
    g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0] += play2ChannelLength;
    if (audioSpeakerPreWriteDataCount > g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0])
    {
        g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[1] += 1U;
    }
#endif /* AUDIO_SPEAKER_MULTI_CHANNEL_PLAY */
#else
    uint32_t buffer_length = 0;
    remainBufferSpace =
        (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE) - USB_AudioSpeakerBufferSpaceUsed();
    if (size > remainBufferSpace) /* discard the overflow data */
    {
        if (remainBufferSpace > (AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE))
        {
            size = (remainBufferSpace - (AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE));
        }
        else
        {
            size = 0;
        }
    }
    if (size > 0)
    {
        buffer_length = g_UsbDeviceAudioSpeaker.tdWriteNumberPlay + size;
        if (buffer_length < (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE))
        {
            memcpy((void *)(&audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay]), (void *)(&buffer[0]), size);
            g_UsbDeviceAudioSpeaker.tdWriteNumberPlay += size;
        }
        else
        {
            uint32_t firstLength = (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE) -
                                   g_UsbDeviceAudioSpeaker.tdWriteNumberPlay;
            memcpy((void *)(&audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay]), (void *)(&buffer[0]),
                   firstLength);
            buffer_length = size - firstLength; /* the remain data length */
            if ((buffer_length) > 0U)
            {
                memcpy((void *)(&audioPlayDataBuff[0]), (void *)((uint8_t *)(&buffer[0]) + firstLength), buffer_length);
                g_UsbDeviceAudioSpeaker.tdWriteNumberPlay = buffer_length;
            }
            else
            {
                g_UsbDeviceAudioSpeaker.tdWriteNumberPlay = 0;
            }
        }
    }
    audioSpeakerPreWriteDataCount = g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0];
    g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0] += size;
    if (audioSpeakerPreWriteDataCount > g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0])
    {
        g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[1] += 1U;
    }
#endif
}

/* USB device audio ISO IN endpoint callback */
usb_status_t USB_DeviceAudioIsoOut(usb_device_handle deviceHandle,
                                   usb_device_endpoint_callback_message_struct_t *event,
                                   void *arg)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param = (usb_device_endpoint_callback_message_struct_t *)event;

    /* endpoint callback length is USB_CANCELLED_TRANSFER_LENGTH (0xFFFFFFFFU) when transfer is canceled */
    if ((g_UsbDeviceAudioSpeaker.attach) && (ep_cb_param->length != (USB_CANCELLED_TRANSFER_LENGTH)))
    {
        if ((g_UsbDeviceAudioSpeaker.tdWriteNumberPlay >=
             AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE / 2) &&
            (g_UsbDeviceAudioSpeaker.startPlayFlag == 0))
        {
            g_UsbDeviceAudioSpeaker.startPlayFlag = 1;
        }
        USB_AudioSpeakerPutBuffer(audioPlayPacket, ep_cb_param->length);
        g_UsbDeviceAudioSpeaker.usbRecvCount += ep_cb_param->length;
        g_UsbDeviceAudioSpeaker.usbRecvTimes++;
#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
        USB_AUDIO_ENTER_CRITICAL();
        USB_AudioFSSync();
        USB_AUDIO_EXIT_CRITICAL();
#endif
        error = USB_DeviceRecvRequest(deviceHandle, USB_AUDIO_SPEAKER_STREAM_ENDPOINT, &audioPlayPacket[0],
                                      g_UsbDeviceAudioSpeaker.currentStreamOutMaxPacketSize);
    }
    return error;
}

/*!
 * @brief Get the setup packet buffer.
 *
 * This function provides the buffer for setup packet.
 *
 * @param handle The USB device handle.
 * @param setupBuffer The pointer to the address of setup packet buffer.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetSetupBuffer(usb_device_handle handle, usb_setup_struct_t **setupBuffer)
{
    static uint32_t audioSpeakerSetup[2];
    if (NULL == setupBuffer)
    {
        return kStatus_USB_InvalidParameter;
    }
    *setupBuffer = (usb_setup_struct_t *)&audioSpeakerSetup;
    return kStatus_USB_Success;
}

/*!
 * @brief Get the setup packet data buffer.
 *
 * This function gets the data buffer for setup packet.
 *
 * @param handle The USB device handle.
 * @param setup The pointer to the setup packet.
 * @param length The pointer to the length of the data buffer.
 * @param buffer The pointer to the address of setup packet data buffer.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetClassReceiveBuffer(usb_device_handle handle,
                                             usb_setup_struct_t *setup,
                                             uint32_t *length,
                                             uint8_t **buffer)
{
    if ((NULL == buffer) || ((*length) > sizeof(s_SetupOutBuffer)))
    {
        return kStatus_USB_InvalidRequest;
    }
    *buffer = s_SetupOutBuffer;
    return kStatus_USB_Success;
}

/*!
 * @brief Configure remote wakeup feature.
 *
 * This function configures the remote wakeup feature.
 *
 * @param handle The USB device handle.
 * @param enable 1: enable, 0: disable.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceConfigureRemoteWakeup(usb_device_handle handle, uint8_t enable)
{
    return kStatus_USB_InvalidRequest;
}

/*!
 * @brief USB configure endpoint function.
 *
 * This function configure endpoint status.
 *
 * @param handle The USB device handle.
 * @param ep Endpoint address.
 * @param status A flag to indicate whether to stall the endpoint. 1: stall, 0: unstall.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceConfigureEndpointStatus(usb_device_handle handle, uint8_t ep, uint8_t status)
{
    if (status)
    {
        if ((USB_AUDIO_SPEAKER_STREAM_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
        else if ((USB_AUDIO_CONTROL_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
        else
        {
        }
    }
    else
    {
        if ((USB_AUDIO_SPEAKER_STREAM_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
        else if ((USB_AUDIO_CONTROL_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
        else
        {
        }
    }
    return kStatus_USB_InvalidRequest;
}

/* The USB_DeviceAudioSpeakerStatusReset() function resets the audio speaker status to the initialized status */
void USB_DeviceAudioSpeakerStatusReset(void)
{
    g_UsbDeviceAudioSpeaker.startPlayFlag                    = 0;
    g_UsbDeviceAudioSpeaker.tdWriteNumberPlay                = 0;
    g_UsbDeviceAudioSpeaker.tdReadNumberPlay                 = 0;
    g_UsbDeviceAudioSpeaker.audioSendCount[0]                = 0;
    g_UsbDeviceAudioSpeaker.audioSendCount[1]                = 0;
    g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[0]     = 0;
    g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[1]     = 0;
    g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0]    = 0;
    g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[1]    = 0;
    g_UsbDeviceAudioSpeaker.usbRecvCount                     = 0;
    g_UsbDeviceAudioSpeaker.audioSendTimes                   = 0;
    g_UsbDeviceAudioSpeaker.usbRecvTimes                     = 0;
    g_UsbDeviceAudioSpeaker.speakerIntervalCount             = 0;
    g_UsbDeviceAudioSpeaker.pllAdjustIntervalCount           = 0;
    g_UsbDeviceAudioSpeaker.speakerReservedSpace             = 0;
    g_UsbDeviceAudioSpeaker.timesFeedbackCalculate           = 0;
    g_UsbDeviceAudioSpeaker.speakerDetachOrNoInput           = 0;
    g_UsbDeviceAudioSpeaker.curAudioPllFrac                  = AUDIO_PLL_FRACTIONAL_DIVIDER;
    g_UsbDeviceAudioSpeaker.audioPllTicksPrev                = 0;
    g_UsbDeviceAudioSpeaker.audioPllTicksDiff                = 0;
    g_UsbDeviceAudioSpeaker.audioPllTicksEma                 = AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT;
    g_UsbDeviceAudioSpeaker.audioPllTickEmaFrac              = 0;
    g_UsbDeviceAudioSpeaker.audioPllTickBasedPrecision       = AUDIO_PLL_FRACTIONAL_CHANGE_STEP;
    g_UsbDeviceAudioSpeaker.stopDataLengthAudioSpeakerAdjust = 0;
}

/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle		  The USB device handle.
 * @param event 		  The USB device event type.
 * @param param 		  The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint8_t *temp8     = (uint8_t *)param;
    uint8_t count      = 0U;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            for (count = 0U; count < USB_AUDIO_SPEAKER_INTERFACE_COUNT; count++)
            {
                g_UsbDeviceAudioSpeaker.currentInterfaceAlternateSetting[count] = 0U;
            }
            g_UsbDeviceAudioSpeaker.attach                                 = 0U;
            g_UsbDeviceAudioSpeaker.currentConfiguration                   = 0U;
            g_UsbDeviceAudioSpeaker.currentStreamInterfaceAlternateSetting = 0U;
            error                                                          = kStatus_USB_Success;
            USB_DeviceControlPipeInit(g_UsbDeviceAudioSpeaker.deviceHandle);
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceGetStatus(g_UsbDeviceAudioSpeaker.deviceHandle, kUSB_DeviceStatusSpeed,
                                                           &g_UsbDeviceAudioSpeaker.speed))
            {
                USB_DeviceSetSpeed(g_UsbDeviceAudioSpeaker.speed);
            }

            if (USB_SPEED_HIGH == g_UsbDeviceAudioSpeaker.speed)
            {
                g_UsbDeviceAudioSpeaker.currentStreamOutMaxPacketSize =
                    (HS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_UsbDeviceAudioSpeaker.attach                                 = 0U;
                g_UsbDeviceAudioSpeaker.currentConfiguration                   = 0U;
                g_UsbDeviceAudioSpeaker.currentStreamInterfaceAlternateSetting = 0U;
                error                                                          = kStatus_USB_Success;
            }
            else if (USB_AUDIO_SPEAKER_CONFIGURE_INDEX == (*temp8))
            {
                g_UsbDeviceAudioSpeaker.attach                                 = 1U;
                g_UsbDeviceAudioSpeaker.currentConfiguration                   = *temp8;
                g_UsbDeviceAudioSpeaker.currentStreamInterfaceAlternateSetting = 0U;
                error                                                          = kStatus_USB_Success;
            }
            else
            {
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (g_UsbDeviceAudioSpeaker.attach)
            {
                uint8_t interface        = (*temp8) & 0xFF;
                uint8_t alternateSetting = g_UsbDeviceInterface[interface];
                error                    = kStatus_USB_Success;
                if (g_UsbDeviceAudioSpeaker.currentStreamInterfaceAlternateSetting != alternateSetting)
                {
                    if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_0 == alternateSetting)
                    {
                        g_UsbDeviceAudioSpeaker.stopDataLengthAudioSpeakerAdjust = 1U;
                        USB_DeviceDeinitEndpoint(g_UsbDeviceAudioSpeaker.deviceHandle,
                                                 USB_AUDIO_SPEAKER_STREAM_ENDPOINT | (USB_OUT << 7U));
                    }
                    if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_1 == alternateSetting)
                    {
                        usb_device_endpoint_init_struct_t epInitStruct;
                        usb_device_endpoint_callback_struct_t epCallback;

                        epCallback.callbackFn    = USB_DeviceAudioIsoOut;
                        epCallback.callbackParam = handle;

                        epInitStruct.zlt             = 0U;
                        epInitStruct.transferType    = USB_ENDPOINT_ISOCHRONOUS;
                        epInitStruct.endpointAddress = USB_AUDIO_SPEAKER_STREAM_ENDPOINT |
                                                       (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
                        if (USB_SPEED_HIGH == g_UsbDeviceAudioSpeaker.speed)
                        {
                            epInitStruct.maxPacketSize =
                                (HS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE);
                            epInitStruct.interval = HS_ISO_OUT_ENDP_INTERVAL;
                        }
                        else
                        {
                            epInitStruct.maxPacketSize =
                                (FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE);
                            epInitStruct.interval = FS_ISO_OUT_ENDP_INTERVAL;
                        }

                        USB_DeviceInitEndpoint(g_UsbDeviceAudioSpeaker.deviceHandle, &epInitStruct, &epCallback);

                        error = USB_DeviceRecvRequest(g_UsbDeviceAudioSpeaker.deviceHandle,
                                                      USB_AUDIO_SPEAKER_STREAM_ENDPOINT, &audioPlayPacket[0],
                                                      g_UsbDeviceAudioSpeaker.currentStreamOutMaxPacketSize);
                    }
                    g_UsbDeviceAudioSpeaker.currentStreamInterfaceAlternateSetting = alternateSetting;
                    USB_DeviceAudioSpeakerStatusReset();
                }
            }
            break;
        default:
            break;
    }

    return error;
}

void SCTIMER_SOF_TOGGLE_HANDLER_PLL()
{
    uint32_t currentSctCap = 0, pllCountPeriod = 0, pll_change = 0;
    uint32_t usedSpace      = 0;
    static int32_t pllCount = 0, pllDiff = 0;
    static int32_t err, abs_err;
    static uint32_t delay_adj_up       = 0;
    static uint32_t delay_adj_down     = 0;
    static uint32_t PllPreUsbRecvCount = 0U;

    if (SCTIMER_GetStatusFlags(SCT0) & (1 << eventCounterU))
    {
        /* Clear interrupt flag.*/
        SCTIMER_ClearStatusFlags(SCT0, (1 << eventCounterU));
    }

    if (g_UsbDeviceAudioSpeaker.pllAdjustIntervalCount != AUDIO_PLL_ADJUST_INTERVAL)
    {
        g_UsbDeviceAudioSpeaker.pllAdjustIntervalCount++;
        return;
    }
    g_UsbDeviceAudioSpeaker.pllAdjustIntervalCount = 1;
    currentSctCap                                  = SCT0->CAP[0];
    pllCountPeriod                                 = currentSctCap - g_UsbDeviceAudioSpeaker.audioPllTicksPrev;
    g_UsbDeviceAudioSpeaker.audioPllTicksPrev      = currentSctCap;
    pllCount                                       = pllCountPeriod;
    if (g_UsbDeviceAudioSpeaker.attach)
    {
        if (abs(pllCount - AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT) < (AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT >> 7))
        {
            pllDiff = pllCount - g_UsbDeviceAudioSpeaker.audioPllTicksEma;
            g_UsbDeviceAudioSpeaker.audioPllTickEmaFrac += (pllDiff % 8);
            g_UsbDeviceAudioSpeaker.audioPllTicksEma += (pllDiff / 8) + g_UsbDeviceAudioSpeaker.audioPllTickEmaFrac / 8;
            g_UsbDeviceAudioSpeaker.audioPllTickEmaFrac = (g_UsbDeviceAudioSpeaker.audioPllTickEmaFrac % 8);

            err     = g_UsbDeviceAudioSpeaker.audioPllTicksEma - AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT;
            abs_err = abs(err);
            if (abs_err > g_UsbDeviceAudioSpeaker.audioPllTickBasedPrecision)
            {
                if (err > 0)
                {
                    g_UsbDeviceAudioSpeaker.curAudioPllFrac -=
                        abs_err / g_UsbDeviceAudioSpeaker.audioPllTickBasedPrecision;
                }
                else
                {
                    g_UsbDeviceAudioSpeaker.curAudioPllFrac +=
                        abs_err / g_UsbDeviceAudioSpeaker.audioPllTickBasedPrecision;
                }
                pll_change = 1;
            }

            if (0U != g_UsbDeviceAudioSpeaker.startPlayFlag)
            {
                /* if USB transfer stops, can not use data length to do adjustment */
                if (0U == g_UsbDeviceAudioSpeaker.stopDataLengthAudioSpeakerAdjust)
                {
                    /* USB is transferring */
                    if (PllPreUsbRecvCount != g_UsbDeviceAudioSpeaker.usbRecvCount)
                    {
                        PllPreUsbRecvCount = g_UsbDeviceAudioSpeaker.usbRecvCount;
                        usedSpace          = USB_AudioSpeakerBufferSpaceUsed();
                        if (usedSpace <= (AUDIO_PLAY_TRANSFER_SIZE * AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD))
                        {
                            if (delay_adj_down == 0)
                            {
                                delay_adj_up   = 0;
                                delay_adj_down = AUDIO_PLL_ADJUST_DATA_BASED_INTERVAL;
                                g_UsbDeviceAudioSpeaker.curAudioPllFrac -= AUDIO_PLL_ADJUST_DATA_BASED_STEP;
                                pll_change = 1;
                            }
                            else
                            {
                                delay_adj_down--;
                            }
                        }
                        else if ((usedSpace + (AUDIO_PLAY_TRANSFER_SIZE * AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD)) >=
                                 (AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_TRANSFER_SIZE))
                        {
                            if (delay_adj_up == 0)
                            {
                                delay_adj_down = 0;
                                delay_adj_up   = AUDIO_PLL_ADJUST_DATA_BASED_INTERVAL;
                                g_UsbDeviceAudioSpeaker.curAudioPllFrac += AUDIO_PLL_ADJUST_DATA_BASED_STEP;
                                pll_change = 1;
                            }
                            else
                            {
                                delay_adj_up--;
                            }
                        }
                    }
                }
            }
        }

        if (pll_change)
        {
            SYSCON->AUDPLLFRAC = g_UsbDeviceAudioSpeaker.curAudioPllFrac;
            SYSCON->AUDPLLFRAC = g_UsbDeviceAudioSpeaker.curAudioPllFrac | (1U << SYSCON_AUDPLLFRAC_REQ_SHIFT);
        }
    }
}

void SCTIMER_CaptureInit(void)
{
#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
    INPUTMUX->SCT0_INMUX[eventCounterU] = 0x0FU; /* 0x10U for USB1 and 0x0FU for USB0. */
#elif (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
    INPUTMUX->SCT0_INMUX[eventCounterU] = 0x10U; /* 0x10U for USB1 and 0x0FU for USB0. */
#endif
    SCTIMER_GetDefaultConfig(&sctimerInfo);

    /* Switch to 16-bit mode */
    sctimerInfo.clockMode   = kSCTIMER_Input_ClockMode;
    sctimerInfo.clockSelect = kSCTIMER_Clock_On_Rise_Input_7;

    /* Initialize SCTimer module */
    SCTIMER_Init(SCT0, &sctimerInfo);

    if (SCTIMER_SetupCaptureAction(SCT0, kSCTIMER_Counter_U, &captureRegisterNumber, eventCounterU) == kStatus_Fail)
    {
        usb_echo("SCT Setup Capture failed!\r\n");
    }
    SCT0->EV[0].STATE = 0x1;
    SCT0->EV[0].CTRL  = (0x01 << 10) | (0x2 << 12);

    /* Enable interrupt flag for event associated with out 4, we use the interrupt to update dutycycle */
    SCTIMER_EnableInterrupts(SCT0, (1 << eventCounterU));

    /* Receive notification when event is triggered */
    SCTIMER_SetCallback(SCT0, SCTIMER_SOF_TOGGLE_HANDLER_PLL, eventCounterU);

    /* Enable at the NVIC */
    EnableIRQ(SCT0_IRQn);

    /* Start the L counter */
    SCTIMER_StartTimer(SCT0, kSCTIMER_Counter_U);
}

/*!
 * @brief Application initialization function.
 *
 * This function initializes the application.
 *
 * @return None.
 */

void APPInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
    /*Initialize audio interface and codec.*/
    HAL_AudioTxInit((hal_audio_handle_t)audioTxHandle, &audioTxConfig);
    BOARD_Codec_Init();
    AUDIO_DMA_EDMA_Start();

    if (kStatus_USB_Success != USB_DeviceInit(CONTROLLER_ID, USB_DeviceCallback, &g_UsbDeviceAudioSpeaker.deviceHandle))
    {
        usb_echo("USB device failed\r\n");
        return;
    }
    else
    {
        usb_echo("USB device audio speaker demo\r\n");
    }

    /* Install isr, set priority, and enable IRQ. */
    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_UsbDeviceAudioSpeaker.deviceHandle);

    SCTIMER_CaptureInit();
}

void USB_AudioCodecTask(void)
{
    if (g_UsbDeviceAudioSpeaker.codecTask & MUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Cur Mute : %x\r\n", g_UsbDeviceAudioSpeaker.curMute20);
#else
        usb_echo("Set Cur Mute : %x\r\n", g_UsbDeviceAudioSpeaker.curMute);
#endif
        g_UsbDeviceAudioSpeaker.codecTask &= ~MUTE_CODEC_TASK;
    }
    if (g_UsbDeviceAudioSpeaker.codecTask & UNMUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Cur Mute : %x\r\n", g_UsbDeviceAudioSpeaker.curMute20);
#else
        usb_echo("Set Cur Mute : %x\r\n", g_UsbDeviceAudioSpeaker.curMute);
#endif
        g_UsbDeviceAudioSpeaker.codecTask &= ~UNMUTE_CODEC_TASK;
    }
    if (g_UsbDeviceAudioSpeaker.codecTask & VOLUME_CHANGE_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Cur Volume : %x\r\n",
                 (uint16_t)(g_UsbDeviceAudioSpeaker.curVolume20[1] << 8U) | g_UsbDeviceAudioSpeaker.curVolume20[0]);
#else
        usb_echo("Set Cur Volume : %x\r\n",
                 (uint16_t)(g_UsbDeviceAudioSpeaker.curVolume[1] << 8U) | g_UsbDeviceAudioSpeaker.curVolume[0]);
#endif
        g_UsbDeviceAudioSpeaker.codecTask &= ~VOLUME_CHANGE_TASK;
    }
}

void USB_AudioSpeakerResetTask(void)
{
    if (g_UsbDeviceAudioSpeaker.speakerDetachOrNoInput)
    {
        USB_DeviceAudioSpeakerStatusReset();
    }
}
/*!
 * @brief Application task function.
 *
 * This function runs the task for application.
 *
 * @return None.
 */
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    pll_setup_t audio_pll_setup = {
        .pllctrl    = 0U,
        .pllndec    = 514U,
        .pllpdec    = 29U,
        .pllmdec    = 0U,
        .audpllfrac = AUDIO_PLL_FRACTIONAL_DIVIDER,
    };

    CLOCK_EnableClock(kCLOCK_InputMux);
    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);
    /* USART0 clock */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    /* I2C clock */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);
#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
    SYSCON->AUDPLLCLKSEL = SYSCON_AUDPLLCLKSEL_SEL(0x00U); /* Select FRO_12M for FS*/
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
    SYSCON->AUDPLLCLKSEL =
        SYSCON_AUDPLLCLKSEL_SEL(0x01U); /* Select the external Crystal CLK_IN instead of FRO_12M for HS*/
#endif
    audio_pll_setup.flags = PLL_SETUPFLAG_POWERUP | PLL_SETUPFLAG_WAITLOCK;
    CLOCK_SetupAudioPLLPrecFract(&audio_pll_setup, audio_pll_setup.flags);
    SYSCON->SCTCLKSEL = SYSCON_AUDPLLCLKSEL_SEL(0x03U); /* Select the AUDIO PLL OUT for SCTimer source */
    CLOCK_SetClkDiv(kCLOCK_DivSctClk, 1, false);

    /* I2S clocks */
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM6);
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM7);
    /* Attach PLL clock to MCLK for I2S, no divider */
    CLOCK_AttachClk(kAUDIO_PLL_to_MCLK);
    SYSCON->MCLKDIV = SYSCON_MCLKDIV_DIV(0U);
    SYSCON->MCLKIO  = 1U;
    /* reset FLEXCOMM for I2C */
    RESET_PeripheralReset(kFC2_RST_SHIFT_RSTn);
    /* reset FLEXCOMM for I2S */
    RESET_PeripheralReset(kFC6_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kFC7_RST_SHIFT_RSTn);
    /* Enable interrupts for I2S */
    EnableIRQ(FLEXCOMM6_IRQn);
    EnableIRQ(FLEXCOMM7_IRQn);
    /* Initialize the rest */
    BOARD_InitBootPins();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();
#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY);
    /* enable usb1 host clock */
    CLOCK_EnableClock(kCLOCK_Usbh1);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBHSH_BASE + 0x50)) |= USBHSH_PORTMODE_DEV_ENABLE_MASK;
    /* enable usb1 host clock */
    CLOCK_DisableClock(kCLOCK_Usbh1);
#endif
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
#endif
    dmaTxConfig.instance            = DEMO_DMA_INSTANCE_INDEX;
    dmaTxConfig.channel             = DEMO_I2S_TX_CHANNEL;
    dmaTxConfig.priority            = kHAL_AudioDmaChannelPriority3;
    audioTxConfig.dmaConfig         = &dmaTxConfig;
    audioTxConfig.instance          = DEMO_I2S_TX_INSTANCE_INDEX;
    audioTxConfig.srcClock_Hz       = 24576000;
    audioTxConfig.sampleRate_Hz     = 48000;
    audioTxConfig.msaterSlave       = kHAL_AudioMaster;
    audioTxConfig.bclkPolarity      = kHAL_AudioSampleOnRisingEdge;
    audioTxConfig.frameSyncPolarity = kHAL_AudioBeginAtFallingEdge;
    audioTxConfig.frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame;
    audioTxConfig.dataFormat        = kHAL_AudioDataFormatI2sClassic;
    audioTxConfig.bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits;
    audioTxConfig.lineChannels      = kHAL_AudioStereo;

    APPInit();

    while (1)
    {
        USB_AudioCodecTask();

        USB_AudioSpeakerResetTask();

#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_UsbDeviceAudioSpeaker.deviceHandle);
#endif
    }
}
