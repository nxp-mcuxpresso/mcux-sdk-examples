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

#include "audio_microphone.h"
#include "streamer_pcm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define USB_AUDIO_ENTER_CRITICAL() \
                                   \
    OSA_SR_ALLOC();                \
                                   \
    OSA_ENTER_CRITICAL()

#define USB_AUDIO_EXIT_CRITICAL() OSA_EXIT_CRITICAL()

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

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
/* Audio data information */
extern usb_device_class_struct_t g_UsbDeviceAudioClass;
/* codec information */
bool g_CodecMuteUnmute = false;

/* Default value of audio microphone device struct */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_audio_microphone_struct_t s_audioMicrophone = {
    NULL,                                               /* deviceHandle */
    NULL,                                               /* audioHandle */
    0x01U,                                              /* copyProtect */
    0x00U,                                              /* curMute */
    {0x00U, 0x1FU},                                     /* curVolume */
    {0x00U, 0x00U},                                     /* minVolume */
    {0x00U, 0x43U},                                     /* maxVolume */
    {0x01U, 0x00U},                                     /* resVolume */
    0x00U,                                              /* curBass */
    0x80U,                                              /* minBass */
    0x7FU,                                              /* maxBass */
    0x01U,                                              /* resBass */
    0x00U,                                              /* curMid */
    0x80U,                                              /* minMid */
    0x7FU,                                              /* maxMid */
    0x01U,                                              /* resMid */
    0x01U,                                              /* curTreble */
    0x80U,                                              /* minTreble */
    0x7FU,                                              /* maxTreble */
    0x01U,                                              /* resTreble */
    0x01U,                                              /* curAutomaticGain */
    {0x00U, 0x40U},                                     /* curDelay */
    {0x00U, 0x00U},                                     /* minDelay */
    {0xFFU, 0xFFU},                                     /* maxDelay */
    {0x00U, 0x01U},                                     /* resDelay */
    0x01U,                                              /* curLoudness */
    {0x00U, 0x00U, 0x01U},                              /* curSamplingFrequency */
    {0x00U, 0x00U, 0x01U},                              /* minSamplingFrequency */
    {0x00U, 0x00U, 0x01U},                              /* maxSamplingFrequency */
    {0x00U, 0x00U, 0x01U},                              /* resSamplingFrequency */
    1U,                                                 /* curClockValid */
    AUDIO_SAMPLING_RATE,                                /* curSampleFrequency */
    {1U, AUDIO_SAMPLING_RATE, AUDIO_SAMPLING_RATE, 0U}, /* freqControlRange */
    {1U, 0x8001U, 0x7FFFU, 1U},                         /* volumeControlRange */
    0,                                                  /* currentConfiguration */
    {0, 0},                                             /* currentInterfaceAlternateSetting */
    USB_SPEED_FULL,                                     /* speed */
    0U,                                                 /* attach */
    0U,                                                 /* start */
    0U,                                                 /* startHalfFull */
    0U,                                                 /* usbSendTimes */
    0U,                                                 /* tdWriteNumber */
    0U,                                                 /* tdReadNumber */
    0U,                                                 /* reservedSpace */
    0U,                                                 /* codecTask */
};
/* Audio buffers for microphone. */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioMicDataBuff[AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioMicPacket[(FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE)];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)

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
    USB_DeviceLpcIp3511IsrFunction(s_audioMicrophone.deviceHandle);
}
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
void USB1_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(s_audioMicrophone.deviceHandle);
}
#endif
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
void USB_OTG_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(s_audioMicrophone.deviceHandle);
}

void USB_OTG1_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(s_audioMicrophone.deviceHandle);
}

void USB_OTG2_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(s_audioMicrophone.deviceHandle);
}
void USB1_HS_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(s_audioMicrophone.deviceHandle);
}
#endif

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

usb_status_t USB_DeviceAudioRequest(class_handle_t handle, uint32_t event, void *param)
{
    usb_device_control_request_struct_t *request = (usb_device_control_request_struct_t *)param;
    usb_status_t error                           = kStatus_USB_Success;

    switch (event)
    {
        case USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL:
            request->buffer = (uint8_t *)&s_audioMicrophone.curMute;
            request->length = sizeof(s_audioMicrophone.curMute);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL:
            request->buffer = (uint8_t *)&s_audioMicrophone.curVolume;
            request->length = sizeof(s_audioMicrophone.curVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL:
            request->buffer = &s_audioMicrophone.curBass;
            request->length = sizeof(s_audioMicrophone.curBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL:
            request->buffer = &s_audioMicrophone.curMid;
            request->length = sizeof(s_audioMicrophone.curMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL:
            request->buffer = &s_audioMicrophone.curTreble;
            request->length = sizeof(s_audioMicrophone.curTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            request->buffer = &s_audioMicrophone.curAutomaticGain;
            request->length = sizeof(s_audioMicrophone.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL:
            request->buffer = s_audioMicrophone.curDelay;
            request->length = sizeof(s_audioMicrophone.curDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL:
            request->buffer = s_audioMicrophone.minVolume;
            request->length = sizeof(s_audioMicrophone.minVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL:
            request->buffer = &s_audioMicrophone.minBass;
            request->length = sizeof(s_audioMicrophone.minBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL:
            request->buffer = &s_audioMicrophone.minMid;
            request->length = sizeof(s_audioMicrophone.minMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL:
            request->buffer = &s_audioMicrophone.minTreble;
            request->length = sizeof(s_audioMicrophone.minTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL:
            request->buffer = s_audioMicrophone.minDelay;
            request->length = sizeof(s_audioMicrophone.minDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL:
            request->buffer = s_audioMicrophone.maxVolume;
            request->length = sizeof(s_audioMicrophone.maxVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL:
            request->buffer = &s_audioMicrophone.maxBass;
            request->length = sizeof(s_audioMicrophone.maxBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL:
            request->buffer = &s_audioMicrophone.maxMid;
            request->length = sizeof(s_audioMicrophone.maxMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL:
            request->buffer = &s_audioMicrophone.maxTreble;
            request->length = sizeof(s_audioMicrophone.maxTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL:
            request->buffer = s_audioMicrophone.maxDelay;
            request->length = sizeof(s_audioMicrophone.maxDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL:
            request->buffer = s_audioMicrophone.resVolume;
            request->length = sizeof(s_audioMicrophone.resVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL:
            request->buffer = &s_audioMicrophone.resBass;
            request->length = sizeof(s_audioMicrophone.resBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL:
            request->buffer = &s_audioMicrophone.resMid;
            request->length = sizeof(s_audioMicrophone.resMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL:
            request->buffer = &s_audioMicrophone.resTreble;
            request->length = sizeof(s_audioMicrophone.resTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL:
            request->buffer = s_audioMicrophone.resDelay;
            request->length = sizeof(s_audioMicrophone.resDelay);
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&s_audioMicrophone.curSampleFrequency;
            request->length = sizeof(s_audioMicrophone.curSampleFrequency);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = (uint8_t *)&s_audioMicrophone.curSampleFrequency;
                request->length = sizeof(s_audioMicrophone.curSampleFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL:
            request->buffer = &s_audioMicrophone.curClockValid;
            request->length = sizeof(s_audioMicrophone.curClockValid);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.curClockValid;
                request->length = sizeof(s_audioMicrophone.curClockValid);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&s_audioMicrophone.freqControlRange;
            request->length = sizeof(s_audioMicrophone.freqControlRange);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL:
            request->buffer = (uint8_t *)&s_audioMicrophone.volumeControlRange;
            request->length = sizeof(s_audioMicrophone.volumeControlRange);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioMicrophone.curVolume;
                request->length = sizeof(s_audioMicrophone.curVolume);
            }
            else
            {
                s_audioMicrophone.codecTask |= VOLUME_CHANGE_TASK;
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.curMute;
                request->length = sizeof(s_audioMicrophone.curMute);
            }
            else
            {
                if (s_audioMicrophone.curMute)
                {
                    s_audioMicrophone.codecTask |= MUTE_CODEC_TASK;
                }
                else
                {
                    s_audioMicrophone.codecTask |= UNMUTE_CODEC_TASK;
                }
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.curBass;
                request->length = sizeof(s_audioMicrophone.curBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.curMid;
                request->length = sizeof(s_audioMicrophone.curMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.curTreble;
                request->length = sizeof(s_audioMicrophone.curTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.curAutomaticGain;
                request->length = sizeof(s_audioMicrophone.curAutomaticGain);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioMicrophone.curDelay;
                request->length = sizeof(s_audioMicrophone.curDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioMicrophone.minVolume;
                request->length = sizeof(s_audioMicrophone.minVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.minBass;
                request->length = sizeof(s_audioMicrophone.minBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.minMid;
                request->length = sizeof(s_audioMicrophone.minMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.minTreble;
                request->length = sizeof(s_audioMicrophone.minTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioMicrophone.minDelay;
                request->length = sizeof(s_audioMicrophone.minDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioMicrophone.maxVolume;
                request->length = sizeof(s_audioMicrophone.maxVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.maxBass;
                request->length = sizeof(s_audioMicrophone.maxBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.maxMid;
                request->length = sizeof(s_audioMicrophone.maxMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.maxTreble;
                request->length = sizeof(s_audioMicrophone.maxTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioMicrophone.maxDelay;
                request->length = sizeof(s_audioMicrophone.maxDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioMicrophone.resVolume;
                request->length = sizeof(s_audioMicrophone.resVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.resBass;
                request->length = sizeof(s_audioMicrophone.resBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.resMid;
                request->length = sizeof(s_audioMicrophone.resMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioMicrophone.resTreble;
                request->length = sizeof(s_audioMicrophone.resTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioMicrophone.resDelay;
                request->length = sizeof(s_audioMicrophone.resDelay);
            }
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}

uint32_t USB_AudioMicrophoneBufferSpaceUsed(void)
{
    if (s_audioMicrophone.tdWriteNumber > s_audioMicrophone.tdReadNumber)
    {
        s_audioMicrophone.reservedSpace = s_audioMicrophone.tdWriteNumber - s_audioMicrophone.tdReadNumber;
    }
    else
    {
        s_audioMicrophone.reservedSpace = s_audioMicrophone.tdWriteNumber +
                                          AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE -
                                          s_audioMicrophone.tdReadNumber;
    }
    return s_audioMicrophone.reservedSpace;
}

void USB_AudioMicrophoneGetBuffer(uint8_t *buffer, uint32_t size)
{
    while (size)
    {
        *buffer = audioMicDataBuff[s_audioMicrophone.tdReadNumber];
        s_audioMicrophone.tdReadNumber++;
        buffer++;
        size--;

        if (s_audioMicrophone.tdReadNumber >= AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE)
        {
            s_audioMicrophone.tdReadNumber = 0;
        }
    }
}

uint32_t USB_MicrophoneDataMatch(uint32_t reservedspace)
{
    uint32_t epPacketSize = 0;
    if (reservedspace >=
        AUDIO_BUFFER_UPPER_LIMIT(AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE))
    {
        epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_FORMAT_SIZE * AUDIO_FORMAT_CHANNELS;
    }
    else if ((reservedspace >=
              AUDIO_BUFFER_LOWER_LIMIT(AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE)) &&
             (reservedspace <
              AUDIO_BUFFER_UPPER_LIMIT(AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE)))
    {
        epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE;
    }
    else if (reservedspace <
             AUDIO_BUFFER_LOWER_LIMIT(AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE))
    {
        epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE - AUDIO_FORMAT_SIZE * AUDIO_FORMAT_CHANNELS;
    }
    else
    {
    }
    return epPacketSize;
}

usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param           = (usb_device_endpoint_callback_message_struct_t *)param;
    uint32_t epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE;

    switch (event)
    {
        case kUSB_DeviceAudioEventStreamSendResponse:
            /* endpoint callback length is USB_CANCELLED_TRANSFER_LENGTH (0xFFFFFFFFU) when transfer is canceled */
            if ((s_audioMicrophone.attach) && (ep_cb_param->length != (USB_CANCELLED_TRANSFER_LENGTH)))
            {
                if (s_audioMicrophone.start == 0)
                {
                    s_audioMicrophone.start = 1;
                }
                if ((s_audioMicrophone.tdWriteNumber >=
                     AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE / 2) &&
                    (s_audioMicrophone.startHalfFull == 0))
                {
                    s_audioMicrophone.startHalfFull = 1;
                }
                if (s_audioMicrophone.startHalfFull)
                {
                    USB_AUDIO_ENTER_CRITICAL();
                    epPacketSize = USB_MicrophoneDataMatch(USB_AudioMicrophoneBufferSpaceUsed());
                    USB_AUDIO_EXIT_CRITICAL();

                    USB_AudioMicrophoneGetBuffer(audioMicPacket, epPacketSize);

                    error = USB_DeviceAudioSend(handle, USB_AUDIO_STREAM_ENDPOINT, audioMicPacket, epPacketSize);
                    s_audioMicrophone.usbSendTimes++;
                }
                else
                {
                    error = USB_DeviceAudioSend(handle, USB_AUDIO_STREAM_ENDPOINT, &audioMicDataBuff[0],
                                                FS_ISO_IN_ENDP_PACKET_SIZE);
                }
            }
            break;
        default:
            if (param && (event > 0xFFU))
            {
                error = USB_DeviceAudioRequest(handle, event, param);
            }
            break;
    }
    return error;
}

usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t *temp8     = (uint8_t *)param;
    uint8_t count      = 0U;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            /* The device BUS reset signal detected */
            for (count = 0U; count < USB_AUDIO_MICROPHONE_INTERFACE_COUNT; count++)
            {
                s_audioMicrophone.currentInterfaceAlternateSetting[count] = 0U;
            }
            s_audioMicrophone.attach               = 0U;
            s_audioMicrophone.currentConfiguration = 0U;
            error                                  = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &s_audioMicrophone.speed))
            {
                USB_DeviceSetSpeed(handle, s_audioMicrophone.speed);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                s_audioMicrophone.attach               = 0U;
                s_audioMicrophone.currentConfiguration = 0U;
                error                                  = kStatus_USB_Success;
            }
            else if (USB_AUDIO_MICROPHONE_CONFIGURE_INDEX == (*temp8))
            {
                /* Set the configuration request */
                s_audioMicrophone.attach               = 1U;
                s_audioMicrophone.currentConfiguration = *temp8;
                error                                  = kStatus_USB_Success;
            }
            else
            {
                /* no action */
            }
            break;
        case kUSB_DeviceEventSetInterface:

            if (s_audioMicrophone.attach)
            {
                /* Set alternateSetting of the interface request */
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);

                if (USB_AUDIO_STREAM_INTERFACE_INDEX == interface)
                {
                    error = USB_DeviceAudioMicrophoneSetInterface(s_audioMicrophone.audioHandle, interface,
                                                                  alternateSetting);
                    s_audioMicrophone.currentInterfaceAlternateSetting[interface] = alternateSetting;
                }
                else if (USB_AUDIO_CONTROL_INTERFACE_INDEX == interface)
                {
                    if (alternateSetting < USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_COUNT)
                    {
                        s_audioMicrophone.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                                         = kStatus_USB_Success;
                    }
                }
                else
                {
                    /* no action */
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                /* Get the current configuration request */
                *temp8 = s_audioMicrophone.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_AUDIO_MICROPHONE_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | s_audioMicrophone.currentInterfaceAlternateSetting[interface];
                    error   = kStatus_USB_Success;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                /* Get the device descriptor request */
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                /* Get the configuration descriptor request */
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                /* Get the string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        default:
            break;
    }

    return error;
}

usb_status_t USB_DeviceAudioMicrophoneSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    usb_status_t error = kStatus_USB_Success;

    if (alternateSetting == USB_AUDIO_STREAM_INTERFACE_ALTERNATE_1)
    {
        USB_DeviceAudioMicrophoneStatusReset();

        error = USB_DeviceAudioSend(s_audioMicrophone.audioHandle, USB_AUDIO_STREAM_ENDPOINT, &audioMicDataBuff[0],
                                    FS_ISO_IN_ENDP_PACKET_SIZE);
    }
    return error;
}

void USB_DeviceAudioMicrophoneStatusReset(void)
{
    s_audioMicrophone.start         = 0;
    s_audioMicrophone.startHalfFull = 0;
    s_audioMicrophone.usbSendTimes  = 0;
    s_audioMicrophone.tdWriteNumber = 0;
    s_audioMicrophone.tdReadNumber  = 0;
    s_audioMicrophone.reservedSpace = 0;
}

void USB_DeviceApplicationInit(void)
{
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    if (kStatus_USB_Success != USB_DeviceClassInit(CONTROLLER_ID, &s_audioConfigList, &s_audioMicrophone.deviceHandle))
    {
        PRINTF("USB device failed\r\n");
        return;
    }
    else
    {
        s_audioMicrophone.audioHandle = s_audioConfigList.config->classHandle;
    }

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}

void USB_AudioCodecTask(void)
{
    if (s_audioMicrophone.codecTask & MUTE_CODEC_TASK)
    {
        PRINTF("Set Cur Mute : %x\r\n", s_audioMicrophone.curMute);
        streamer_pcm_mute(true);
        s_audioMicrophone.codecTask &= ~MUTE_CODEC_TASK;
        g_CodecMuteUnmute = true;
    }
    if (s_audioMicrophone.codecTask & UNMUTE_CODEC_TASK)
    {
        PRINTF("Set Cur Mute : %x\r\n", s_audioMicrophone.curMute);
        streamer_pcm_mute(false);
        s_audioMicrophone.codecTask &= ~UNMUTE_CODEC_TASK;
        g_CodecMuteUnmute = true;
    }
    if (s_audioMicrophone.codecTask & VOLUME_CHANGE_TASK)
    {
        PRINTF("Set Cur Volume : %x\r\n",
               (uint16_t)(s_audioMicrophone.curVolume[1] << 8U) | s_audioMicrophone.curVolume[0]);
        s_audioMicrophone.codecTask &= ~VOLUME_CHANGE_TASK;
    }
}
