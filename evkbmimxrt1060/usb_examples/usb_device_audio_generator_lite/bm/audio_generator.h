/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __USB_AUDIO_GENERATOR_H__
#define __USB_AUDIO_GENERATOR_H__ 1
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

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
/**********************************************************************
Audio PLL contants
      AUDIO_PLL_USB1_SOF_INTERVAL_COUNT
      The Audio PLL clock is 24.576Mhz, and the USB1_SOF_TOGGLE frequency is 4kHz when the device is attached,
      so AUDIO_PLL_USB1_SOF_INTERVAL_COUNT = (24576000 * 100 (stands for counter interval)) /4000 = 614400
      AUDIO_PLL_FRACTIONAL_CHANGE_STEP
      The Audio input clock is 24Mhz, and denominator is 4500, divider is 15 and PFD is 26.
      so AUDIO_PLL_FRACTIONAL_CHANGE_STEP = (24000000 * 100 (stands for counter interval) * 18) / (27000 * 26 * 15
*4000) + 1
**********************************************************************/
#define AUDIO_PLL_USB1_SOF_INTERVAL_COUNT (614400)  /* The USB1_SOF_TOGGLE's frequency is 4kHz. */
#define AUDIO_PLL_USB1_SOF_INTERVAL_COUNT1 (491520) /* The USB1_SOF_TOGGLE's frequency is 4kHz. */
#define AUDIO_PLL_FRACTIONAL_CHANGE_STEP (2)
#endif
#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
#define DATA_BUFF_SIZE (AUDIO_ENDPOINT_PACKET_SIZE)

typedef struct _usb_audio_generator_struct
{
    usb_device_handle deviceHandle;
    uint8_t currentStreamInterfaceAlternateSetting;
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
    uint8_t currentInterfaceAlternateSetting[USB_AUDIO_GENERATOR_INTERFACE_COUNT];
    uint8_t speed;
    uint8_t attach;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    volatile uint32_t generatorIntervalCount;
    volatile uint32_t curAudioPllFrac;
    volatile uint32_t audioPllTicksPrev;
    volatile int32_t audioPllTicksDiff;
    volatile int32_t audioPllTicksEma;
    volatile int32_t audioPllTickEmaFrac;
    volatile int32_t audioPllStep;
#endif
} usb_audio_generator_struct_t;
#endif /* __USB_AUDIO_GENERATOR_H__ */
