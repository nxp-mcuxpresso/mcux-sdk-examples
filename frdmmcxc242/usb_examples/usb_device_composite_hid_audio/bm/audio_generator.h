/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __USB_AUDIO_H__
#define __USB_AUDIO_H__ 1

#include "usb_device_audio.h" 
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DATA_BUFF_SIZE (AUDIO_ENDPOINT_PACKET_SIZE)

typedef struct _usb_audio_generator_struct
{
    usb_device_handle deviceHandle; /* USB device handle.                   */
    class_handle_t audioHandle;     /* USB AUDIO GENERATOR class handle.    */
    uint8_t copyProtect;
    uint8_t curMute;
    uint8_t curVolume[2];
    uint8_t minVolume[2];
    uint8_t maxVolume[2];
    uint8_t resVolume[2];
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
} usb_audio_generator_struct_t;

#endif /* __USB_AUDIO_GENERATOR_H__ */
