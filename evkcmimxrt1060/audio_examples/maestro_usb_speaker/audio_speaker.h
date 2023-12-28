/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_AUDIO_SPEAKER_H__
#define __USB_AUDIO_SPEAKER_H__ 1U

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_audio.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#ifndef CONTROLLER_ID
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Hs0
#endif
#endif

#define AUDIO_SAMPLING_RATE_KHZ                      (48)
#define AUDIO_SAMPLING_RATE                          (AUDIO_SAMPLING_RATE_KHZ * 1000)
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL (16 * 2)

/* feedback calculate interval */
#define AUDIO_CALCULATE_Ff_INTERVAL (16U)
/* the threshold transfer count that can tolerance by frame */
#define USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD (4U)
/* feedback value discard times, the first feedback vaules are discarded */
#define AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT (4U)

#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (0x08U) /* 0x08 means 8 frames (8ms) */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT \
    (16U) /* 16 units size buffer (1 unit means the size to play during 1ms) */
#elif (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (0x08U) /* 0x08 means 8 frames (8ms) */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT \
    (16U) /* 16 units size buffer (1 unit means the size to play during 1ms) */
#endif

#define TSAMFREQ2BYTES(f)     (f & 0xFFU), ((f >> 8U) & 0xFFU), ((f >> 16U) & 0xFFU)
#define TSAMFREQ2BYTESHS(f)   (f & 0xFFU), ((f >> 8U) & 0xFFU), ((f >> 16U) & 0xFFU), ((f >> 24U) & 0xFFU)
#define AUDIO_ADJUST_MIN_STEP (0x01)

#if defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)
#define AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME (AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME / 3)
#else
#define AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME
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
    uint32_t currentFeedbackMaxPacketSize;
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
    uint8_t curDelay[4];             /* need to consider the endians */
    uint8_t minDelay[4];             /* need to consider the endians */
    uint8_t maxDelay[4];             /* need to consider the endians */
    uint8_t resDelay[4];             /* need to consider the endians */
    uint8_t curLoudness;
    uint8_t curSamplingFrequency[3]; /* need to consider the endians */
    uint8_t minSamplingFrequency[3]; /* need to consider the endians */
    uint8_t maxSamplingFrequency[3]; /* need to consider the endians */
    uint8_t resSamplingFrequency[3]; /* need to consider the endians */
    uint8_t curClockValid;
    uint32_t curSampleFrequency;
    usb_device_control_range_layout3_struct_t freqControlRange;
    usb_device_control_range_layout2_struct_t volumeControlRange;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_AUDIO_SPEAKER_INTERFACE_COUNT];
    uint8_t speed;
    uint8_t attach;
    volatile uint8_t startPlayFlag;
    volatile uint32_t tdWriteNumberPlay;
    volatile uint32_t tdReadNumberPlay;
    volatile uint32_t audioSendCount[2];
    volatile uint32_t audioSpeakerReadDataCount[2];
    volatile uint32_t audioSpeakerWriteDataCount[2];
    volatile uint32_t usbRecvCount;
    volatile uint32_t audioSendTimes;
    volatile uint32_t usbRecvTimes;
    volatile uint32_t speakerIntervalCount;
    volatile uint32_t speakerReservedSpace;
    volatile uint32_t speakerDetachOrNoInput;
    volatile uint32_t codecTask;
    uint32_t audioPlayTransferSize;
    volatile uint16_t audioPlayBufferSize;
    volatile uint32_t maxFrameCount;
    volatile uint32_t lastFrameCount;
    volatile uint32_t currentFrameCount;
    volatile uint8_t firstCalculateFeedback;
    volatile uint8_t stopFeedbackUpdate;
    volatile uint32_t lastFeedbackValue;
    volatile uint8_t feedbackDiscardFlag;
    volatile uint8_t feedbackDiscardTimes;
} usb_audio_speaker_struct_t;

/*!
 * @brief Audio speaker reset task function.
 *
 * This function provides implementation of the audio speaker status reset task.
 *
 * @return None.
 */
void USB_DeviceAudioSpeakerStatusReset(void);

/*!
 * @brief Application initialization function.
 *
 * This function initializes the application.
 *
 * @return None.
 */
void USB_DeviceApplicationInit(void);

/*!
 * @brief Audio speaker reset task function.
 *
 * This function provides implementation of the audio speaker status reset task.
 *
 * @return None.
 */
void USB_AudioSpeakerResetTask(void);

/*!
 * @brief Audio speaker codec task function.
 *
 * This function provide demo implementation of the audio codec task
 *
 * @return None.
 */
void USB_AudioCodecTask(void);

/*!
 * @brief This function returns the ring buffer size.
 *
 * This function calculates and returns the used speaker ring buffer size.
 *
 * @return None.
 */
uint32_t USB_AudioSpeakerBufferSpaceUsed(void);

/*!
 * @brief This function calculates the feedback data.
 *
 * This function calculates the feedback data that are provide to the host for synchronization.
 *
 * @return None.
 */
void USB_DeviceCalculateFeedback(void);

#endif /* __USB_AUDIO_SPEAKER_H__ */
