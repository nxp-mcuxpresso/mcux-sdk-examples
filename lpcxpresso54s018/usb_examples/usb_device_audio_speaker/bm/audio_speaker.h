/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __USB_AUDIO_SPEAKER_H__
#define __USB_AUDIO_SPEAKER_H__ 1U

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Hs0
#endif
#endif

#define AUDIO_SAMPLING_RATE_KHZ     (48)
#define AUDIO_SAMPLING_RATE_16KHZ   (16)
#define AUDIO_SAMPLING_RATE         (AUDIO_SAMPLING_RATE_KHZ * 1000)
#define AUDIO_BUFFER_UPPER_LIMIT(x) (((x)*5) / 8)
#define AUDIO_BUFFER_LOWER_LIMIT(x) (((x)*3) / 8)

#define TSAMFREQ2BYTES(f)     (f & 0xFFU), ((f >> 8U) & 0xFFU), ((f >> 16U) & 0xFFU)
#define TSAMFREQ2BYTESHS(f)   (f & 0xFFU), ((f >> 8U) & 0xFFU), ((f >> 16U) & 0xFFU), ((f >> 24U) & 0xFFU)
#define AUDIO_ADJUST_MIN_STEP (0x10)
#if defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)
#define AUDIO_PLAY_TRANSFER_SIZE (FS_ISO_OUT_ENDP_PACKET_SIZE / 3)
#else
#define AUDIO_PLAY_TRANSFER_SIZE FS_ISO_OUT_ENDP_PACKET_SIZE
#endif

#define MUTE_CODEC_TASK    (1UL << 0U)
#define UNMUTE_CODEC_TASK  (1UL << 1U)
#define VOLUME_CHANGE_TASK (1UL << 2U)

#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
typedef struct _usb_audio_speaker_struct
{
    usb_device_handle deviceHandle; /* USB device handle.                   */
    class_handle_t audioHandle;     /* USB AUDIO GENERATOR class handle.    */
    uint32_t currentStreamOutMaxPacketSize;
    uint8_t copyProtect;
    uint8_t curMute;
    uint8_t curVolume[2]; /* need to consider the endians */
    uint8_t minVolume[2]; /* need to consider the endians */
    uint8_t maxVolume[2]; /* need to consider the endians */
    uint8_t resVolume[2]; /* need to consider the endians */
    uint8_t curBass;
    uint8_t minBass;
    uint8_t maxBass;
    uint8_t resBass;
    uint8_t curMid;
    uint8_t minMid;
    uint8_t maxMid;
    uint8_t resMid;
    uint8_t curTreble;
    uint8_t minTreble;
    uint8_t maxTreble;
    uint8_t resTreble;
    uint8_t curAutomaticGain;
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint8_t curDelay[4]; /* need to consider the endians */
    uint8_t minDelay[4]; /* need to consider the endians */
    uint8_t maxDelay[4]; /* need to consider the endians */
    uint8_t resDelay[4]; /* need to consider the endians */
#else
    uint8_t curDelay[2]; /* need to consider the endians */
    uint8_t minDelay[2]; /* need to consider the endians */
    uint8_t maxDelay[2]; /* need to consider the endians */
    uint8_t resDelay[2]; /* need to consider the endians */
#endif
    uint8_t curLoudness;
    uint8_t curSamplingFrequency[3]; /* need to consider the endians */
    uint8_t minSamplingFrequency[3]; /* need to consider the endians */
    uint8_t maxSamplingFrequency[3]; /* need to consider the endians */
    uint8_t resSamplingFrequency[3]; /* need to consider the endians */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint8_t curMute20;
    uint8_t curClockValid;
    uint8_t curVolume20[2];
    uint32_t curSampleFrequency;
    usb_device_control_range_layout3_struct_t freqControlRange;
    usb_device_control_range_layout2_struct_t volumeControlRange;
#endif
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_AUDIO_SPEAKER_INTERFACE_COUNT];
    uint8_t speed;
    uint8_t attach;
    volatile uint8_t startPlayFlag;
    volatile uint8_t startPlayHalfFull;
    volatile uint32_t tdWriteNumberPlay;
    volatile uint32_t tdReadNumberPlay;
    volatile uint32_t audioSendCount[2];
    volatile uint32_t audioSpeakerReadDataCount[2];
    volatile uint32_t audioSpeakerWriteDataCount[2];
    volatile uint32_t usbRecvCount;
    volatile uint32_t audioSendTimes;
    volatile uint32_t usbRecvTimes;
    volatile uint32_t speakerIntervalCount;
    volatile uint32_t pllAdjustIntervalCount;
    volatile uint32_t speakerReservedSpace;
    volatile uint32_t timesFeedbackCalculate;
    volatile uint32_t speakerDetachOrNoInput;
    volatile uint32_t codecTask;
    volatile uint32_t curAudioPllFrac;
    volatile uint32_t audioPllTicksPrev;
    volatile int32_t audioPllTicksDiff;
    volatile int32_t audioPllTicksEma;
    volatile int32_t audioPllTickEmaFrac;
    volatile int32_t audioPllTickBasedPrecision;
    volatile uint8_t stopDataLengthAudioSpeakerAdjust;
} usb_audio_speaker_struct_t;

#endif /* __USB_AUDIO_SPEAKER_H__ */
