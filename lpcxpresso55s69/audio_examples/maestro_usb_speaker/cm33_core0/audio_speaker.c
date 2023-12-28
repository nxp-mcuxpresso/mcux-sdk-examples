/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
/*${standard_header_anchor}*/

#include "fsl_debug_console.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif

#include "audio_speaker.h"
#include "streamer_pcm.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define AUDIO_SAMPLING_RATE_TO_10_14 (AUDIO_SAMPLING_RATE_KHZ << 10)
#define AUDIO_SAMPLING_RATE_TO_16_16 \
    ((AUDIO_SAMPLING_RATE_KHZ / (AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME / HS_ISO_OUT_ENDP_PACKET_SIZE)) << 13)

/* Audio 2.0 and high speed, use low latency, but IP3511HS controller do not have micro frame count */
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
volatile static uint8_t s_microFrameCountIp3511HS = 0;
#endif

extern volatile uint8_t feedbackValueUpdating;

#if defined(USB_DEVICE_WORKAROUND_AUDIO_20_WINDOWS) && (USB_DEVICE_WORKAROUND_AUDIO_20_WINDOWS > 0)
/* this is used for windows that supports usb audio 2.0 */
#define AUDIO_UPDATE_FEEDBACK_DATA(m, n)              \
    {                                                 \
        feedbackValueUpdating = 1;                    \
        m[0]                  = ((n << 6U) & 0xFFu);  \
        m[1]                  = ((n >> 2U) & 0xFFu);  \
        m[2]                  = ((n >> 10U) & 0xFFu); \
        m[3]                  = ((n >> 18U) & 0xFFu); \
        feedbackValueUpdating = 0;                    \
    }
#else
/* change 10.10 data to 10.14 or change 12.13 to 16.16 */
#define AUDIO_UPDATE_FEEDBACK_DATA(m, n)                  \
    feedbackValueUpdating = 1;                            \
    if (USB_SPEED_HIGH == g_UsbDeviceAudioSpeaker.speed)  \
    {                                                     \
        m[0] = (((n & 0x00001FFFu) << 3) & 0xFFu);        \
        m[1] = ((((n & 0x00001FFFu) << 3) >> 8) & 0xFFu); \
        m[2] = (((n & 0x01FFE000u) >> 13) & 0xFFu);       \
        m[3] = (((n & 0x01FFE000u) >> 21) & 0xFFu);       \
    }                                                     \
    else                                                  \
    {                                                     \
        m[0] = ((n << 4) & 0xFFU);                        \
        m[1] = (((n << 4) >> 8U) & 0xFFU);                \
        m[2] = (((n << 4) >> 16U) & 0xFFU);               \
    }                                                     \
    feedbackValueUpdating = 0;
#endif

#define USB_AUDIO_ENTER_CRITICAL() \
                                   \
    OSA_SR_ALLOC();                \
                                   \
    OSA_ENTER_CRITICAL()

#define USB_AUDIO_EXIT_CRITICAL() OSA_EXIT_CRITICAL()
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param);
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
void USB0_IRQHandler(void);
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
void USB1_IRQHandler(void);
#endif
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
void USB_OTG_IRQHandler(void);
void USB_OTG1_IRQHandler(void);
void USB_OTG2_IRQHandler(void);
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
usb_audio_speaker_struct_t g_UsbDeviceAudioSpeaker;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayDataBuff[AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayPacket[FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t usbAudioFeedBackBuffer[4];
USB_RAM_ADDRESS_ALIGNMENT(4) uint8_t audioFeedBackBuffer[4];
volatile uint8_t feedbackValueUpdating;

extern usb_device_class_struct_t g_UsbDeviceAudioClass;

/* Default value of audio generator device struct */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_audio_speaker_struct_t g_UsbDeviceAudioSpeaker = {
    .deviceHandle                  = NULL,
    .audioHandle                   = (class_handle_t)NULL,
    .currentStreamOutMaxPacketSize = (FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE),
    .currentFeedbackMaxPacketSize  = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE,
    .attach                        = 0U,
    .copyProtect                   = 0x01U,
    .curMute                       = 0x00U,
    .curVolume                     = {0x00U, 0x1fU},
    .minVolume                     = {0x00U, 0x00U},
    .maxVolume                     = {0x00U, 0x43U},
    .resVolume                     = {0x01U, 0x00U},
    .curBass                       = 0x00U,
    .minBass                       = 0x80U,
    .maxBass                       = 0x7FU,
    .resBass                       = 0x01U,
    .curMid                        = 0x00U,
    .minMid                        = 0x80U,
    .maxMid                        = 0x7FU,
    .resMid                        = 0x01U,
    .curTreble                     = 0x01U,
    .minTreble                     = 0x80U,
    .maxTreble                     = 0x7FU,
    .resTreble                     = 0x01U,
    .curAutomaticGain              = 0x01U,
    .curDelay                      = {0x00U, 0x40U},
    .minDelay                      = {0x00U, 0x00U},
    .maxDelay                      = {0xFFU, 0xFFU},
    .resDelay                      = {0x00U, 0x01U},
    .curLoudness                   = 0x01U,
    .curSamplingFrequency          = {0x00U, 0x00U, 0x01U},
    .minSamplingFrequency          = {0x00U, 0x00U, 0x01U},
    .maxSamplingFrequency          = {0x00U, 0x00U, 0x01U},
    .resSamplingFrequency          = {0x00U, 0x00U, 0x01U},
    .curClockValid                 = 1U,
    .curSampleFrequency            = 48000U, /* This should be changed to 48000 if sampling rate is 48k */
    .freqControlRange              = {1U, 48000U, 48000U, 0U},
    .volumeControlRange            = {1U, 0x8001U, 0x7FFFU, 1U},
    .speed                         = USB_SPEED_FULL,
    .tdWriteNumberPlay             = 0,
    .tdReadNumberPlay              = 0,
    .audioSendCount                = {0, 0},
    .audioSpeakerReadDataCount     = {0, 0},
    .audioSpeakerWriteDataCount    = {0, 0},
    .usbRecvCount                  = 0,
    .audioSendTimes                = 0,
    .usbRecvTimes                  = 0,
    .startPlayFlag                 = 0,
    .speakerIntervalCount          = 0,
    .speakerReservedSpace          = 0,
    .speakerDetachOrNoInput        = 0,
};

/* USB device class information */
static usb_device_class_config_struct_t s_audioConfig[1] = {{
    USB_DeviceAudioCallback,
    (class_handle_t)NULL,
    &g_UsbDeviceAudioClass,
}};

/* USB device class configuration information */
static usb_device_class_config_list_struct_t s_audioConfigList = {
    s_audioConfig,
    USB_DeviceCallback,
    1U,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief USB Interrupt service routine.
 *
 * This function serves as the USB interrupt service routine.
 *
 * @return None.
 */
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
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
void USB_OTG_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_UsbDeviceAudioSpeaker.deviceHandle);
}
void USB_OTG1_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_UsbDeviceAudioSpeaker.deviceHandle);
}
void USB_OTG2_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_UsbDeviceAudioSpeaker.deviceHandle);
}
void USB1_HS_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_UsbDeviceAudioSpeaker.deviceHandle);
}
#endif

/*!
 * @brief Enables interrupt service routines for device.
 *
 * @return None.
 */
void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
    uint8_t usbDeviceIP3511Irq[] = USB_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0];
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
    uint8_t usbDeviceIP3511Irq[] = USBHSD_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Hs0];
#endif
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber                  = usbDeviceEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
#endif

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
    USB_DeviceEhciTaskFunction(deviceHandle);
#endif
}
#endif

/*!
 * @brief Audio class specific request function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle		  The Audio class handle.
 * @param event 		  The Audio class event type.
 * @param param 		  The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioRequest(class_handle_t handle, uint32_t event, void *param)
{
    usb_device_control_request_struct_t *request = (usb_device_control_request_struct_t *)param;
    usb_status_t error                           = kStatus_USB_Success;

    switch (event)
    {
        case USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL:
            request->buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.curMute;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.curMute);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL:
            request->buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.curVolume;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.curVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.curBass;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.curBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.curMid;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.curMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.curTreble;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.curTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.curAutomaticGain;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL:
            request->buffer = g_UsbDeviceAudioSpeaker.curDelay;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.curDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL:
            request->buffer = g_UsbDeviceAudioSpeaker.minVolume;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.minVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.minBass;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.minBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.minMid;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.minMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.minTreble;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.minTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL:
            request->buffer = g_UsbDeviceAudioSpeaker.minDelay;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.minDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL:
            request->buffer = g_UsbDeviceAudioSpeaker.maxVolume;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.maxVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.maxBass;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.maxBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.maxMid;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.maxMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.maxTreble;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.maxTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL:
            request->buffer = g_UsbDeviceAudioSpeaker.maxDelay;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.maxDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL:
            request->buffer = g_UsbDeviceAudioSpeaker.resVolume;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.resVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.resBass;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.resBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.resMid;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.resMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.resTreble;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.resTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL:
            request->buffer = g_UsbDeviceAudioSpeaker.resDelay;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.resDelay);
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.curSampleFrequency;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.curSampleFrequency);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.curSampleFrequency;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.curSampleFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL:
            request->buffer = &g_UsbDeviceAudioSpeaker.curClockValid;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.curClockValid);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.curClockValid;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.curClockValid);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.freqControlRange;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.freqControlRange);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL:
            request->buffer = (uint8_t *)&g_UsbDeviceAudioSpeaker.volumeControlRange;
            request->length = sizeof(g_UsbDeviceAudioSpeaker.volumeControlRange);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbDeviceAudioSpeaker.curVolume;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.curVolume);
            }
            else
            {
                g_UsbDeviceAudioSpeaker.codecTask |= VOLUME_CHANGE_TASK;
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.curMute;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.curMute);
            }
            else
            {
                if (g_UsbDeviceAudioSpeaker.curMute)
                {
                    g_UsbDeviceAudioSpeaker.codecTask |= MUTE_CODEC_TASK;
                }
                else
                {
                    g_UsbDeviceAudioSpeaker.codecTask |= UNMUTE_CODEC_TASK;
                }
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.curBass;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.curBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.curMid;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.curMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.curTreble;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.curTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.curAutomaticGain;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.curAutomaticGain);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbDeviceAudioSpeaker.curDelay;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.curDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbDeviceAudioSpeaker.minVolume;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.minVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.minBass;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.minBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.minMid;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.minMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.minTreble;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.minTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbDeviceAudioSpeaker.minDelay;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.minDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbDeviceAudioSpeaker.maxVolume;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.maxVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.maxBass;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.maxBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.maxMid;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.maxMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.maxTreble;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.maxTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbDeviceAudioSpeaker.maxDelay;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.maxDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbDeviceAudioSpeaker.resVolume;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.resVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.resBass;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.resBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.resMid;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.resMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_UsbDeviceAudioSpeaker.resTreble;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.resTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_UsbDeviceAudioSpeaker.resDelay;
                request->length = sizeof(g_UsbDeviceAudioSpeaker.resDelay);
            }
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}

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

void USB_DeviceCalculateFeedback(void)
{
    volatile static uint64_t totalFrameValue = 0U;
    volatile static uint32_t frameDistance   = 0U;
    volatile static uint32_t feedbackValue   = 0U;

    uint32_t audioSpeakerUsedSpace = 0U;

    /* feedback interval is AUDIO_CALCULATE_Ff_INTERVAL */
    if (USB_SPEED_HIGH == g_UsbDeviceAudioSpeaker.speed)
    {
        if (g_UsbDeviceAudioSpeaker.speakerIntervalCount !=
            AUDIO_CALCULATE_Ff_INTERVAL *
                (AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME / g_UsbDeviceAudioSpeaker.audioPlayTransferSize))
        {
            g_UsbDeviceAudioSpeaker.speakerIntervalCount++;
            return;
        }
    }
    else /* full speed, feedback interval is AUDIO_CALCULATE_Ff_INTERVAL ms */
    {
        if (g_UsbDeviceAudioSpeaker.speakerIntervalCount != AUDIO_CALCULATE_Ff_INTERVAL)
        {
            g_UsbDeviceAudioSpeaker.speakerIntervalCount++;
            return;
        }
    }

    if (0U == g_UsbDeviceAudioSpeaker.firstCalculateFeedback)
    {
        g_UsbDeviceAudioSpeaker.speakerIntervalCount = 0;
        g_UsbDeviceAudioSpeaker.currentFrameCount    = 0;
        g_UsbDeviceAudioSpeaker.audioSendCount[0]    = 0;
        g_UsbDeviceAudioSpeaker.audioSendCount[1]    = 0;
        totalFrameValue                              = 0;
        frameDistance                                = 0;
        feedbackValue                                = 0;
        USB_DeviceClassGetCurrentFrameCount(CONTROLLER_ID, (uint32_t *)&g_UsbDeviceAudioSpeaker.lastFrameCount);
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
        if (USB_SPEED_HIGH == g_UsbDeviceAudioSpeaker.speed)
        {
            g_UsbDeviceAudioSpeaker.lastFrameCount += s_microFrameCountIp3511HS;
        }
#endif
        g_UsbDeviceAudioSpeaker.firstCalculateFeedback = 1U;
        return;
    }
    g_UsbDeviceAudioSpeaker.speakerIntervalCount = 0;
    USB_DeviceClassGetCurrentFrameCount(CONTROLLER_ID, (uint32_t *)&g_UsbDeviceAudioSpeaker.currentFrameCount);
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
    if (USB_SPEED_HIGH == g_UsbDeviceAudioSpeaker.speed)
    {
        g_UsbDeviceAudioSpeaker.currentFrameCount += s_microFrameCountIp3511HS;
    }
#endif
    frameDistance = ((g_UsbDeviceAudioSpeaker.currentFrameCount + USB_DEVICE_MAX_FRAME_COUNT + 1U -
                      g_UsbDeviceAudioSpeaker.lastFrameCount) &
                     USB_DEVICE_MAX_FRAME_COUNT);
    g_UsbDeviceAudioSpeaker.lastFrameCount = g_UsbDeviceAudioSpeaker.currentFrameCount;

    totalFrameValue += frameDistance;

    if (1U == g_UsbDeviceAudioSpeaker.stopFeedbackUpdate)
    {
        return;
    }
    if (1U == g_UsbDeviceAudioSpeaker.feedbackDiscardFlag)
    {
        if (0 != g_UsbDeviceAudioSpeaker.feedbackDiscardTimes)
        {
            g_UsbDeviceAudioSpeaker.feedbackDiscardTimes--;
            if (0 != g_UsbDeviceAudioSpeaker.lastFeedbackValue)
            {
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, g_UsbDeviceAudioSpeaker.lastFeedbackValue);
            }
            return;
        }
        else
        {
            g_UsbDeviceAudioSpeaker.feedbackDiscardFlag = 0;
        }
    }

    if (USB_SPEED_HIGH == g_UsbDeviceAudioSpeaker.speed)
    {
#if defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)
        feedbackValue = (uint32_t)(((((uint64_t)g_UsbDeviceAudioSpeaker.audioSendCount[1]) << 32U) |
                                    g_UsbDeviceAudioSpeaker.audioSendCount[0]) *
                                   1024UL * 8UL / totalFrameValue / ((uint64_t)(2U * AUDIO_FORMAT_SIZE)));
#else
        feedbackValue =
            (uint32_t)(((((uint64_t)g_UsbDeviceAudioSpeaker.audioSendCount[1]) << 32U) |
                        g_UsbDeviceAudioSpeaker.audioSendCount[0]) *
                       1024UL * 8UL / totalFrameValue / ((uint64_t)(AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE)));
#endif
    }
    else
    {
#if defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)
        feedbackValue = (uint32_t)(((((uint64_t)g_UsbDeviceAudioSpeaker.audioSendCount[1]) << 32U) |
                                    g_UsbDeviceAudioSpeaker.audioSendCount[0]) *
                                   1024UL / totalFrameValue / ((uint64_t)(2U * AUDIO_FORMAT_SIZE)));
#else
        feedbackValue = (uint32_t)(((((uint64_t)g_UsbDeviceAudioSpeaker.audioSendCount[1]) << 32U) |
                                    g_UsbDeviceAudioSpeaker.audioSendCount[0]) *
                                   1024UL / totalFrameValue / ((uint64_t)(AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE)));
#endif
    }

    audioSpeakerUsedSpace = USB_AudioSpeakerBufferSpaceUsed();
    if (audioSpeakerUsedSpace <=
        (g_UsbDeviceAudioSpeaker.audioPlayTransferSize * USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD))
    {
        feedbackValue += AUDIO_ADJUST_MIN_STEP;
    }

    if ((audioSpeakerUsedSpace +
         (g_UsbDeviceAudioSpeaker.audioPlayTransferSize * USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD)) >=
        g_UsbDeviceAudioSpeaker.audioPlayBufferSize)
    {
        feedbackValue -= AUDIO_ADJUST_MIN_STEP;
    }

    g_UsbDeviceAudioSpeaker.lastFeedbackValue = feedbackValue;
    AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, feedbackValue);
}

/*!
 * @brief This function fills audioRecDataBuff.
 *
 * This function fills the audioRecDataBuff with audioPlayPacket in every callback.
 *
 * @return None.
 */
void USB_AudioSpeakerPutBuffer(uint8_t *buffer, uint32_t size)
{
    uint32_t remainBufferSpace;
    uint32_t audioSpeakerPreWriteDataCount;

#if defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)
    uint8_t *buffer_temp;
    uint32_t play2ChannelLength;
    remainBufferSpace = g_UsbDeviceAudioSpeaker.audioPlayBufferSize - USB_AudioSpeakerBufferSpaceUsed();
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
#if defined(USB_AUDIO_5_1_CHANNEL_PAIR_SEL) && (0x01 == USB_AUDIO_5_1_CHANNEL_PAIR_SEL)
                audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay] = *(buffer_temp + j);
#endif
#if defined(USB_AUDIO_5_1_CHANNEL_PAIR_SEL) && (0x02 == USB_AUDIO_5_1_CHANNEL_PAIR_SEL)
                audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay] =
                    *(buffer_temp + j + AUDIO_FORMAT_SIZE * 2);
#endif
#if defined(USB_AUDIO_5_1_CHANNEL_PAIR_SEL) && (0x03 == USB_AUDIO_5_1_CHANNEL_PAIR_SEL)
                audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay] =
                    *(buffer_temp + j + AUDIO_FORMAT_SIZE * 4);
#endif
                play2ChannelLength--;
                g_UsbDeviceAudioSpeaker.tdWriteNumberPlay++;
                if (g_UsbDeviceAudioSpeaker.tdWriteNumberPlay >= g_UsbDeviceAudioSpeaker.audioPlayBufferSize)
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
    g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0] += (size / (AUDIO_FORMAT_CHANNELS / 2U));
    if (audioSpeakerPreWriteDataCount > g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0])
    {
        g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[1] += 1U;
    }
#else
    uint32_t buffer_length = 0;
    remainBufferSpace      = g_UsbDeviceAudioSpeaker.audioPlayBufferSize - USB_AudioSpeakerBufferSpaceUsed();
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
        if (buffer_length < g_UsbDeviceAudioSpeaker.audioPlayBufferSize)
        {
            memcpy((void *)(&audioPlayDataBuff[g_UsbDeviceAudioSpeaker.tdWriteNumberPlay]), (void *)(&buffer[0]), size);
            g_UsbDeviceAudioSpeaker.tdWriteNumberPlay += size;
        }
        else
        {
            uint32_t firstLength =
                g_UsbDeviceAudioSpeaker.audioPlayBufferSize - g_UsbDeviceAudioSpeaker.tdWriteNumberPlay;
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

/*!
 * @brief Audio class specific callback function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle		  The Audio class handle.
 * @param event 		  The Audio class event type.
 * @param param 		  The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */

usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param = (usb_device_endpoint_callback_message_struct_t *)param;

    switch (event)
    {
        case kUSB_DeviceAudioEventStreamSendResponse:
            /* endpoint callback length is USB_CANCELLED_TRANSFER_LENGTH (0xFFFFFFFFU) when transfer is canceled */
            if ((g_UsbDeviceAudioSpeaker.attach) && (ep_cb_param->length != (USB_CANCELLED_TRANSFER_LENGTH)))
            {
                if (ep_cb_param->length == g_UsbDeviceAudioSpeaker.currentFeedbackMaxPacketSize)
                {
                    if (!feedbackValueUpdating)
                    {
                        *((uint32_t *)&usbAudioFeedBackBuffer[0]) = *((uint32_t *)&audioFeedBackBuffer[0]);
                    }
                    error = USB_DeviceAudioSend(handle, USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT, usbAudioFeedBackBuffer,
                                                g_UsbDeviceAudioSpeaker.currentFeedbackMaxPacketSize);
                }
            }
            break;
        case kUSB_DeviceAudioEventStreamRecvResponse:
            /* endpoint callback length is USB_CANCELLED_TRANSFER_LENGTH (0xFFFFFFFFU) when transfer is canceled */
            if ((g_UsbDeviceAudioSpeaker.attach) && (ep_cb_param->length != (USB_CANCELLED_TRANSFER_LENGTH)))
            {
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
                if (USB_SPEED_HIGH ==
                    g_UsbDeviceAudioSpeaker.speed) /* high speed and audio 2.0, use low latency solution  */
                {
                    if (g_UsbDeviceAudioSpeaker.tdWriteNumberPlay >=
                        (g_UsbDeviceAudioSpeaker.audioPlayTransferSize * AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT))
                    {
                        g_UsbDeviceAudioSpeaker.startPlayFlag = 1;
                    }
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if (1U == HS_ISO_OUT_ENDP_INTERVAL)
                    if (s_microFrameCountIp3511HS < 7U)
                        s_microFrameCountIp3511HS++;
                    else
                        s_microFrameCountIp3511HS = 0U;
#elif (2U == HS_ISO_OUT_ENDP_INTERVAL)
                    if (s_microFrameCountIp3511HS < 6U)
                        s_microFrameCountIp3511HS += 2U;
                    else
                        s_microFrameCountIp3511HS = 0U;
#elif (3U == HS_ISO_OUT_ENDP_INTERVAL)
                    if (s_microFrameCountIp3511HS < 4U)
                        s_microFrameCountIp3511HS += 4U;
                    else
                        s_microFrameCountIp3511HS = 0U;
#else
                    s_microFrameCountIp3511HS = 0;
#endif
#endif
                }
                else
                {
                    if ((g_UsbDeviceAudioSpeaker.tdWriteNumberPlay >=
                         (g_UsbDeviceAudioSpeaker.audioPlayBufferSize / 2U)) &&
                        (g_UsbDeviceAudioSpeaker.startPlayFlag == 0))
                    {
                        g_UsbDeviceAudioSpeaker.startPlayFlag = 1;
                    }
                }
#else
                if ((g_UsbDeviceAudioSpeaker.tdWriteNumberPlay >= (g_UsbDeviceAudioSpeaker.audioPlayBufferSize / 2U)) &&
                    (g_UsbDeviceAudioSpeaker.startPlayFlag == 0))
                {
                    g_UsbDeviceAudioSpeaker.startPlayFlag = 1;
                }
#endif /* USB_DEVICE_CONFIG_EHCI, USB_DEVICE_CONFIG_LPCIP3511HS */
                USB_AudioSpeakerPutBuffer(audioPlayPacket, ep_cb_param->length);
                g_UsbDeviceAudioSpeaker.usbRecvCount += ep_cb_param->length;
                g_UsbDeviceAudioSpeaker.usbRecvTimes++;
                error = USB_DeviceAudioRecv(handle, USB_AUDIO_SPEAKER_STREAM_ENDPOINT, &audioPlayPacket[0],
                                            g_UsbDeviceAudioSpeaker.currentStreamOutMaxPacketSize);
            }
            break;

        default:
            if (param && (event > 0xFF))
            {
                error = USB_DeviceAudioRequest(handle, event, param);
            }
            break;
    }

    return error;
}

void USB_DeviceAudioSpeakerStatusReset(void)
{
    g_UsbDeviceAudioSpeaker.startPlayFlag                 = 0;
    g_UsbDeviceAudioSpeaker.tdWriteNumberPlay             = 0;
    g_UsbDeviceAudioSpeaker.tdReadNumberPlay              = 0;
    g_UsbDeviceAudioSpeaker.audioSendCount[0]             = 0;
    g_UsbDeviceAudioSpeaker.audioSendCount[1]             = 0;
    g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[0]  = 0;
    g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[1]  = 0;
    g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[0] = 0;
    g_UsbDeviceAudioSpeaker.audioSpeakerWriteDataCount[1] = 0;
    g_UsbDeviceAudioSpeaker.usbRecvCount                  = 0;
    g_UsbDeviceAudioSpeaker.audioSendTimes                = 0;
    g_UsbDeviceAudioSpeaker.usbRecvTimes                  = 0;
    g_UsbDeviceAudioSpeaker.speakerIntervalCount          = 0;
    g_UsbDeviceAudioSpeaker.speakerReservedSpace          = 0;
    g_UsbDeviceAudioSpeaker.speakerDetachOrNoInput        = 0;
    g_UsbDeviceAudioSpeaker.firstCalculateFeedback        = 0U;
    g_UsbDeviceAudioSpeaker.lastFrameCount                = 0U;
    g_UsbDeviceAudioSpeaker.currentFrameCount             = 0U;
    g_UsbDeviceAudioSpeaker.feedbackDiscardFlag           = 0U;
    g_UsbDeviceAudioSpeaker.feedbackDiscardTimes          = AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT;

    /* use the last saved feedback value */
    if (g_UsbDeviceAudioSpeaker.lastFeedbackValue)
    {
        AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, g_UsbDeviceAudioSpeaker.lastFeedbackValue);
    }
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
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t count      = 0U;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            for (count = 0U; count < USB_AUDIO_SPEAKER_INTERFACE_COUNT; count++)
            {
                g_UsbDeviceAudioSpeaker.currentInterfaceAlternateSetting[count] = 0U;
            }
            /* reset audio speaker status to be the initialized status */
            USB_DeviceAudioSpeakerStatusReset();
            /* reset the the last feedback value */
            g_UsbDeviceAudioSpeaker.lastFeedbackValue    = 0U;
            g_UsbDeviceAudioSpeaker.attach               = 0U;
            g_UsbDeviceAudioSpeaker.currentConfiguration = 0U;
            error                                        = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_UsbDeviceAudioSpeaker.speed))
            {
                USB_DeviceSetSpeed(handle, g_UsbDeviceAudioSpeaker.speed);
            }
            if (USB_SPEED_HIGH == g_UsbDeviceAudioSpeaker.speed)
            {
                g_UsbDeviceAudioSpeaker.currentStreamOutMaxPacketSize =
                    (HS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE);
                g_UsbDeviceAudioSpeaker.currentFeedbackMaxPacketSize = HS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_16_16);
                /* high speed and audio 2.0, audio play interval is set by HS EP packet size */
#if defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)
                g_UsbDeviceAudioSpeaker.audioPlayTransferSize =
                    HS_ISO_OUT_ENDP_PACKET_SIZE / (AUDIO_FORMAT_CHANNELS / 2U);
#else
                g_UsbDeviceAudioSpeaker.audioPlayTransferSize = HS_ISO_OUT_ENDP_PACKET_SIZE;
#endif
                /* use short play buffer size, only use two elements */
                g_UsbDeviceAudioSpeaker.audioPlayBufferSize =
                    AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME * AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT;
            }
            else
            {
                g_UsbDeviceAudioSpeaker.currentFeedbackMaxPacketSize = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
                /* full speed, audio play interval is 1 ms using this play size */
                g_UsbDeviceAudioSpeaker.audioPlayTransferSize = AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
                /* use the whole play buffer size */
                g_UsbDeviceAudioSpeaker.audioPlayBufferSize =
                    AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
            }
#else
            g_UsbDeviceAudioSpeaker.currentFeedbackMaxPacketSize = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
            AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
            /* full speed, audio play interval is 1 ms using this play size */
            g_UsbDeviceAudioSpeaker.audioPlayTransferSize = AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
            /* use the whole play buffer size */
            g_UsbDeviceAudioSpeaker.audioPlayBufferSize =
                AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
#endif /* USB_DEVICE_CONFIG_EHCI, USB_DEVICE_CONFIG_LPCIP3511HS */
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_UsbDeviceAudioSpeaker.attach               = 0U;
                g_UsbDeviceAudioSpeaker.currentConfiguration = 0U;
                error                                        = kStatus_USB_Success;
            }
            else if (USB_AUDIO_SPEAKER_CONFIGURE_INDEX == (*temp8))
            {
                g_UsbDeviceAudioSpeaker.attach               = 1U;
                g_UsbDeviceAudioSpeaker.currentConfiguration = *temp8;
                error                                        = kStatus_USB_Success;
            }
            else
            {
                /* no action, invalid request */
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (g_UsbDeviceAudioSpeaker.attach)
            {
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);

                if (USB_AUDIO_STREAM_INTERFACE_INDEX == interface)
                {
                    if (alternateSetting < USB_AUDIO_STREAM_INTERFACE_ALTERNATE_COUNT)
                    {
                        if (g_UsbDeviceAudioSpeaker.currentInterfaceAlternateSetting[interface] == alternateSetting)
                        {
                            error = kStatus_USB_Success;
                            break;
                        }
                        if (USB_AUDIO_STREAM_INTERFACE_ALTERNATE_1 == alternateSetting)
                        {
                            error = USB_DeviceAudioRecv(g_UsbDeviceAudioSpeaker.audioHandle,
                                                        USB_AUDIO_SPEAKER_STREAM_ENDPOINT, &audioPlayDataBuff[0],
                                                        g_UsbDeviceAudioSpeaker.currentStreamOutMaxPacketSize);
                            if (!feedbackValueUpdating)
                            {
                                *((uint32_t *)&usbAudioFeedBackBuffer[0]) = *((uint32_t *)&audioFeedBackBuffer[0]);
                            }
                            USB_DeviceAudioSend(g_UsbDeviceAudioSpeaker.audioHandle,
                                                USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT, usbAudioFeedBackBuffer,
                                                g_UsbDeviceAudioSpeaker.currentFeedbackMaxPacketSize);
                        }
                        else
                        {
                            error = USB_DeviceDeinitEndpoint(
                                g_UsbDeviceAudioSpeaker.deviceHandle,
                                USB_AUDIO_SPEAKER_STREAM_ENDPOINT |
                                    (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT));
                        }
                        /* usb host stops the speaker, so there is no need for feedback */
                        if ((1U == g_UsbDeviceAudioSpeaker.startPlayFlag) &&
                            (USB_AUDIO_STREAM_INTERFACE_ALTERNATE_0 == alternateSetting))
                        {
                            g_UsbDeviceAudioSpeaker.stopFeedbackUpdate = 1U;
                        }

                        /* usb host start the speaker, discard the feedback for AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT
                         * times */
                        if (USB_AUDIO_STREAM_INTERFACE_ALTERNATE_1 == alternateSetting)
                        {
                            g_UsbDeviceAudioSpeaker.stopFeedbackUpdate   = 0U;
                            g_UsbDeviceAudioSpeaker.feedbackDiscardFlag  = 1U;
                            g_UsbDeviceAudioSpeaker.feedbackDiscardTimes = AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT;
                        }
                        g_UsbDeviceAudioSpeaker.currentInterfaceAlternateSetting[interface] = alternateSetting;
                    }
                }
                else if (USB_AUDIO_CONTROL_INTERFACE_INDEX == interface)
                {
                    if (alternateSetting < USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_COUNT)
                    {
                        g_UsbDeviceAudioSpeaker.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                                               = kStatus_USB_Success;
                    }
                }
                else
                {
                    /* no action, invalid request */
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                *temp8 = g_UsbDeviceAudioSpeaker.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                /* Get current alternate setting of the interface request */
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_AUDIO_SPEAKER_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_UsbDeviceAudioSpeaker.currentInterfaceAlternateSetting[interface];
                    error   = kStatus_USB_Success;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                /* Get device descriptor request */
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                /* Get device configuration descriptor request */
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                /* Get device string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        default:
            break;
    }

    return error;
}

void USB_DeviceApplicationInit(void)
{
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);

    if (kStatus_USB_Success !=
        USB_DeviceClassInit(CONTROLLER_ID, &s_audioConfigList, &g_UsbDeviceAudioSpeaker.deviceHandle))
    {
        usb_echo("USB device failed\r\n");
        return;
    }
    else
    {
        g_UsbDeviceAudioSpeaker.audioHandle = s_audioConfigList.config->classHandle;
    }

    /* Install isr, set priority, and enable IRQ. */
    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}

void USB_AudioCodecTask(void)
{
    if (g_UsbDeviceAudioSpeaker.codecTask & MUTE_CODEC_TASK)
    {
        streamer_pcm_mute(true);
        usb_echo("Set Cur Mute : %x\r\n", g_UsbDeviceAudioSpeaker.curMute);
        g_UsbDeviceAudioSpeaker.codecTask &= ~MUTE_CODEC_TASK;
    }
    if (g_UsbDeviceAudioSpeaker.codecTask & UNMUTE_CODEC_TASK)
    {
        streamer_pcm_mute(false);
        usb_echo("Set Cur Mute : %x\r\n", g_UsbDeviceAudioSpeaker.curMute);
        g_UsbDeviceAudioSpeaker.codecTask &= ~UNMUTE_CODEC_TASK;
    }
    if (g_UsbDeviceAudioSpeaker.codecTask & VOLUME_CHANGE_TASK)
    {
        usb_echo("Set Cur Volume : %x\r\n",
                 (uint16_t)(g_UsbDeviceAudioSpeaker.curVolume[1] << 8U) | g_UsbDeviceAudioSpeaker.curVolume[0]);
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
