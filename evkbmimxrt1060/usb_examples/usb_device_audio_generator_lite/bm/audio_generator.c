/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017,2019 NXP
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
#include "usb_audio_config.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "audio_generator.h"

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

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

usb_status_t USB_DeviceAudioProcessTerminalRequest(uint32_t audioCommand,
                                                   uint32_t *length,
                                                   uint8_t **buffer,
                                                   uint8_t entityOrEndpoint);
extern void USB_AudioRecorderGetBuffer(uint8_t *buffer, uint32_t size);
#if defined(AUDIO_DATA_SOURCE_DMIC) && (AUDIO_DATA_SOURCE_DMIC > 0U)
extern void Board_DMIC_DMA_Init(void);
#endif
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
extern void SCTIMER_CaptureInit(void);
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Audio data information */
extern uint8_t s_wavBuff[];

extern uint8_t g_UsbDeviceInterface[USB_AUDIO_GENERATOR_INTERFACE_COUNT];

extern usb_status_t USB_DeviceSetSpeed(uint8_t speed);

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_SetupOutBuffer[8];
/* Default value of audio generator device struct */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_audio_generator_struct_t s_audioGenerator = {
    NULL,                  /* deviceHandle */
    0x0U,                  /* currentStreamInterfaceAlternateSetting */
    0x01U,                 /* copyProtect */
    0x01U,                 /* curMute */
    {0x00U, 0x80U},        /* curVolume */
    {0x00U, 0x80U},        /* minVolume */
    {0xFFU, 0x7FU},        /* maxVolume */
    {0x01U, 0x00U},        /* resVolume */
    0x00U,                 /* curBass */
    0x80U,                 /* minBass */
    0x7FU,                 /* maxBass */
    0x01U,                 /* resBass */
    0x00U,                 /* curMid */
    0x80U,                 /* minMid */
    0x7FU,                 /* maxMid */
    0x01U,                 /* resMid */
    0x01U,                 /* curTreble */
    0x80U,                 /* minTreble */
    0x7FU,                 /* maxTreble */
    0x01U,                 /* resTreble */
    0x01U,                 /* curAutomaticGain */
    {0x00U, 0x40U},        /* curDelay */
    {0x00U, 0x00U},        /* minDelay */
    {0xFFU, 0xFFU},        /* maxDelay */
    {0x00U, 0x01U},        /* resDelay */
    0x01U,                 /* curLoudness */
    {0x00U, 0x00U, 0x01U}, /* curSamplingFrequency */
    {0x00U, 0x00U, 0x01U}, /* minSamplingFrequency */
    {0x00U, 0x00U, 0x01U}, /* maxSamplingFrequency */
    {0x00U, 0x00U, 0x01U}, /* resSamplingFrequency */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    0U,             /* curMute20 */
    1U,             /* curClockValid */
    {0x00U, 0x1FU}, /* curVolume20 */
#if defined(AUDIO_DATA_SOURCE_DMIC) && (AUDIO_DATA_SOURCE_DMIC > 0U)
    16000U,                   /* curSampleFrequency, This should be changed to 16000 if sampling rate is 16k */
    {1U, 16000U, 16000U, 0U}, /* freqControlRange */
#else
    8000U,                  /* curSampleFrequency, This should be changed to 8000 if sampling rate is 8k */
    {1U, 8000U, 8000U, 0U}, /* freqControlRange */
#endif
    {1U, 0x8001U, 0x7FFFU, 1U}, /* volumeControlRange */
#endif
    0,              /* currentConfiguration */
    {0, 0},         /* currentInterfaceAlternateSetting */
    USB_SPEED_FULL, /* speed */
    0U,             /* attach */
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    0,                                 /* generatorIntervalCount */
    0,                                 /* curAudioPllFrac */
    0,                                 /* audioPllTicksPrev */
    0,                                 /* audioPllTicksDiff */
    AUDIO_PLL_USB1_SOF_INTERVAL_COUNT, /* audioPllTicksEma */
    0,                                 /* audioPllTickEmaFrac */
    AUDIO_PLL_FRACTIONAL_CHANGE_STEP,  /* audioPllStep */
#endif
};

/*******************************************************************************
 * Code
 ******************************************************************************/

void USB_OTG1_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(s_audioGenerator.deviceHandle);
}

void USB_OTG2_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(s_audioGenerator.deviceHandle);
}

void USB_DeviceClockInit(void)
{
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    if (CONTROLLER_ID == kUSB_ControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, 480000000U);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, 480000000U);
    }
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
}
void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber                  = usbDeviceEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
    USB_DeviceEhciTaskFunction(deviceHandle);
}
#endif

/* USB device audio ISO OUT endpoint callback */
usb_status_t USB_DeviceAudioIsoOut(usb_device_handle deviceHandle,
                                   usb_device_endpoint_callback_message_struct_t *event,
                                   void *arg)
{
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param = (usb_device_endpoint_callback_message_struct_t *)event;
    if ((0U != s_audioGenerator.attach) &&
        (ep_cb_param->length ==
         ((USB_SPEED_HIGH == s_audioGenerator.speed) ? HS_ISO_IN_ENDP_PACKET_SIZE : FS_ISO_IN_ENDP_PACKET_SIZE)))
    {
        USB_AudioRecorderGetBuffer(s_wavBuff, (USB_SPEED_HIGH == s_audioGenerator.speed) ? HS_ISO_IN_ENDP_PACKET_SIZE :
                                                                                           FS_ISO_IN_ENDP_PACKET_SIZE);
        return USB_DeviceSendRequest(
            deviceHandle, USB_AUDIO_STREAM_ENDPOINT, s_wavBuff,
            (USB_SPEED_HIGH == s_audioGenerator.speed) ? HS_ISO_IN_ENDP_PACKET_SIZE : FS_ISO_IN_ENDP_PACKET_SIZE);
    }

    return kStatus_USB_Error;
}

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
                    if (USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID == entityId)
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
                    if (USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID == entityId)
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
                    if (USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId)
                    {
                        audioCommand = USB_DEVICE_AUDIO_TE_SET_CUR_COPY_PROTECT_CONTROL;
                    }
                    else
                    {
                        /* Output Terminals only support the Set Terminal Copy Protect Control request */
                    }
                    break;
                default:
                    /*no action*/
                    break;
            }
#else
        case USB_DEVICE_AUDIO_SET_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_TE_COPY_PROTECT_CONTROL:
                    if (USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId)
                    {
                        audioCommand = USB_DEVICE_AUDIO_TE_SET_CUR_COPY_PROTECT_CONTROL;
                    }
                    else
                    {
                        /* Output Terminals only support the Set Terminal Copy Protect Control request */
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

    if ((USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID == entityId) ||
        (USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId))
    {
        error = USB_DeviceAudioSetControlTerminal(handle, setup, length, buffer, entityId);
    }
    else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityId)
    {
        error = USB_DeviceAudioSetFeatureUnit(handle, setup, length, buffer);
    }
    else
    {
    }
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    if (USB_AUDIO_RECORDER_CONTROL_CLOCK_SOURCE_ENTITY_ID == entityId)
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

    if ((USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID == entityId) ||
        (USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId))
    {
        error = USB_DeviceAudioGetControlTerminal(handle, setup, length, buffer, entityId);
    }
    else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityId)
    {
        error = USB_DeviceAudioGetFeatureUnit(handle, setup, length, buffer);
    }
    else
    {
    }
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    if (USB_AUDIO_RECORDER_CONTROL_CLOCK_SOURCE_ENTITY_ID == entityId)
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
        if ((interfaceOrEndpoint == USB_AUDIO_STREAM_ENDPOINT) || (interfaceOrEndpoint == USB_AUDIO_CONTROL_ENDPOINT))
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
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint8_t *volBuffAddr;
#else
    uint16_t volume;
#endif

    switch (audioCommand)
    {
        case USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            *buffer = (uint8_t *)&s_audioGenerator.curMute20;
            *length = sizeof(s_audioGenerator.curMute20);
#else
            *buffer = &s_audioGenerator.curMute;
            *length = sizeof(s_audioGenerator.curMute);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            *buffer = (uint8_t *)&s_audioGenerator.curVolume20;
            *length = sizeof(s_audioGenerator.curVolume20);
#else
            *buffer = s_audioGenerator.curVolume;
            *length = sizeof(s_audioGenerator.curVolume);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL:
            *buffer = &s_audioGenerator.curBass;
            *length = sizeof(s_audioGenerator.curBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL:
            *buffer = &s_audioGenerator.curMid;
            *length = sizeof(s_audioGenerator.curMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL:
            *buffer = &s_audioGenerator.curTreble;
            *length = sizeof(s_audioGenerator.curTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            *buffer = &s_audioGenerator.curAutomaticGain;
            *length = sizeof(s_audioGenerator.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL:
            *buffer = s_audioGenerator.curDelay;
            *length = sizeof(s_audioGenerator.curDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL:
            *buffer = s_audioGenerator.minVolume;
            *length = sizeof(s_audioGenerator.minVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL:
            *buffer = &s_audioGenerator.minBass;
            *length = sizeof(s_audioGenerator.minBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL:
            *buffer = &s_audioGenerator.minMid;
            *length = sizeof(s_audioGenerator.minMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL:
            *buffer = &s_audioGenerator.minTreble;
            *length = sizeof(s_audioGenerator.minTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL:
            *buffer = s_audioGenerator.minDelay;
            *length = sizeof(s_audioGenerator.minDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL:
            *buffer = s_audioGenerator.maxVolume;
            *length = sizeof(s_audioGenerator.maxVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL:
            *buffer = &s_audioGenerator.maxBass;
            *length = sizeof(s_audioGenerator.maxBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL:
            *buffer = &s_audioGenerator.maxMid;
            *length = sizeof(s_audioGenerator.maxMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL:
            *buffer = &s_audioGenerator.maxTreble;
            *length = sizeof(s_audioGenerator.maxTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL:
            *buffer = s_audioGenerator.maxDelay;
            *length = sizeof(s_audioGenerator.maxDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL:
            *buffer = s_audioGenerator.resVolume;
            *length = sizeof(s_audioGenerator.resVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL:
            *buffer = &s_audioGenerator.resBass;
            *length = sizeof(s_audioGenerator.resBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL:
            *buffer = &s_audioGenerator.resMid;
            *length = sizeof(s_audioGenerator.resMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL:
            *buffer = &s_audioGenerator.resTreble;
            *length = sizeof(s_audioGenerator.resTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL:
            *buffer = s_audioGenerator.resDelay;
            *length = sizeof(s_audioGenerator.resDelay);
            break;
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL:
            *buffer = (uint8_t *)&s_audioGenerator.curSampleFrequency;
            *length = sizeof(s_audioGenerator.curSampleFrequency);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL:
            s_audioGenerator.curSampleFrequency = *(uint32_t *)(*buffer);
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL:
            *buffer = (uint8_t *)&s_audioGenerator.curClockValid;
            *length = sizeof(s_audioGenerator.curClockValid);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL:
            s_audioGenerator.curClockValid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL:
            *buffer = (uint8_t *)&s_audioGenerator.volumeControlRange;
            *length = sizeof(s_audioGenerator.volumeControlRange);
            break;
        case USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL:
            *buffer = (uint8_t *)&s_audioGenerator.freqControlRange;
            *length = sizeof(s_audioGenerator.freqControlRange);
            break;
#else
        case USB_DEVICE_AUDIO_EP_GET_CUR_SAMPLING_FREQ_CONTROL:
            *buffer = s_audioGenerator.curSamplingFrequency;
            *length = sizeof(s_audioGenerator.curSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MIN_SAMPLING_FREQ_CONTROL:
            *buffer = s_audioGenerator.minSamplingFrequency;
            *length = sizeof(s_audioGenerator.minSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MAX_SAMPLING_FREQ_CONTROL:
            *buffer = s_audioGenerator.maxSamplingFrequency;
            *length = sizeof(s_audioGenerator.maxSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_RES_SAMPLING_FREQ_CONTROL:
            *buffer = s_audioGenerator.resSamplingFrequency;
            *length = sizeof(s_audioGenerator.resSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_SET_CUR_SAMPLING_FREQ_CONTROL:
            s_audioGenerator.curSamplingFrequency[0] = **(buffer);
            s_audioGenerator.curSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_MIN_SAMPLING_FREQ_CONTROL:
            s_audioGenerator.minSamplingFrequency[0] = **(buffer);
            s_audioGenerator.minSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_MAX_SAMPLING_FREQ_CONTROL:
            s_audioGenerator.maxSamplingFrequency[0] = **(buffer);
            s_audioGenerator.maxSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_RES_SAMPLING_FREQ_CONTROL:
            s_audioGenerator.resSamplingFrequency[0] = **(buffer);
            s_audioGenerator.resSamplingFrequency[1] = *((*buffer) + 1);
            break;
#endif
        case USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            volBuffAddr                     = *buffer;
            s_audioGenerator.curVolume20[0] = *(volBuffAddr);
            s_audioGenerator.curVolume20[1] = *(volBuffAddr + 1);
#else
            s_audioGenerator.curVolume[0] = **(buffer);
            s_audioGenerator.curVolume[1] = *((*buffer) + 1);
            volume = (uint16_t)((uint16_t)s_audioGenerator.curVolume[1] << 8U);
            volume |= (uint8_t)(s_audioGenerator.curVolume[0]);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            s_audioGenerator.curMute20 = **(buffer);
#else
            s_audioGenerator.curMute = **(buffer);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL:
            s_audioGenerator.curBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL:
            s_audioGenerator.curMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL:
            s_audioGenerator.curTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            s_audioGenerator.curAutomaticGain = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL:
            s_audioGenerator.curDelay[0] = **(buffer);
            s_audioGenerator.curDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            s_audioGenerator.curDelay[2] = *((*buffer) + 2);
            s_audioGenerator.curDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL:
            s_audioGenerator.minVolume[0] = **(buffer);
            s_audioGenerator.minVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL:
            s_audioGenerator.minBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL:
            s_audioGenerator.minMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL:
            s_audioGenerator.minTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL:
            s_audioGenerator.minDelay[0] = **(buffer);
            s_audioGenerator.minDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            s_audioGenerator.minDelay[2] = *((*buffer) + 2);
            s_audioGenerator.minDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL:
            s_audioGenerator.maxVolume[0] = **(buffer);
            s_audioGenerator.maxVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL:
            s_audioGenerator.maxBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL:
            s_audioGenerator.maxMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL:
            s_audioGenerator.maxTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL:
            s_audioGenerator.maxDelay[0] = **(buffer);
            s_audioGenerator.maxDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            s_audioGenerator.maxDelay[2] = *((*buffer) + 2);
            s_audioGenerator.maxDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL:
            s_audioGenerator.resVolume[0] = **(buffer);
            s_audioGenerator.resVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL:
            s_audioGenerator.resBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL:
            s_audioGenerator.resMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL:
            s_audioGenerator.resTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL:
            s_audioGenerator.resDelay[0] = **(buffer);
            s_audioGenerator.resDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            s_audioGenerator.resDelay[2] = *((*buffer) + 2);
            s_audioGenerator.resDelay[3] = *((*buffer) + 3);
#endif
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
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
    static uint32_t audioGeneratorSetup[2];
    if (NULL == setupBuffer)
    {
        return kStatus_USB_InvalidParameter;
    }
    *setupBuffer = (usb_setup_struct_t *)&audioGeneratorSetup;
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
        if ((USB_AUDIO_STREAM_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
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
        if ((USB_AUDIO_STREAM_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
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

/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle           The USB device handle.
 * @param event            The USB device event type.
 * @param param            The parameter of the device specific request.
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
            /* The device BUS reset signal detected */
            for (count = 0U; count < USB_AUDIO_GENERATOR_INTERFACE_COUNT; count++)
            {
                s_audioGenerator.currentInterfaceAlternateSetting[count] = 0U;
            }
            s_audioGenerator.attach               = 0U;
            s_audioGenerator.currentConfiguration = 0U;
            error                                 = kStatus_USB_Success;
            USB_DeviceControlPipeInit(s_audioGenerator.deviceHandle);
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success ==
                USB_DeviceGetStatus(s_audioGenerator.deviceHandle, kUSB_DeviceStatusSpeed, &s_audioGenerator.speed))
            {
                USB_DeviceSetSpeed(s_audioGenerator.speed);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                s_audioGenerator.attach               = 0U;
                s_audioGenerator.currentConfiguration = 0U;
                error                                 = kStatus_USB_Success;
            }
            else if (USB_AUDIO_GENERATOR_CONFIGURE_INDEX == (*temp8))
            {
                /* Set the configuration request */
                s_audioGenerator.attach               = 1U;
                s_audioGenerator.currentConfiguration = *temp8;
                error                                 = kStatus_USB_Success;
            }
            else
            {
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (0U != s_audioGenerator.attach)
            {
                uint8_t interface        = (*temp8) & 0xFFU;
                uint8_t alternateSetting = g_UsbDeviceInterface[interface];

                error = kStatus_USB_Success;
                if (s_audioGenerator.currentStreamInterfaceAlternateSetting != alternateSetting)
                {
                    if (s_audioGenerator.currentStreamInterfaceAlternateSetting)
                    {
                        USB_DeviceDeinitEndpoint(
                            s_audioGenerator.deviceHandle,
                            USB_AUDIO_STREAM_ENDPOINT | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT));
                    }
                    else
                    {
                        usb_device_endpoint_init_struct_t epInitStruct;
                        usb_device_endpoint_callback_struct_t epCallback;

                        epCallback.callbackFn    = USB_DeviceAudioIsoOut;
                        epCallback.callbackParam = handle;

                        epInitStruct.zlt          = 0U;
                        epInitStruct.transferType = USB_ENDPOINT_ISOCHRONOUS;
                        epInitStruct.endpointAddress =
                            USB_AUDIO_STREAM_ENDPOINT | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
                        if (USB_SPEED_HIGH == s_audioGenerator.speed)
                        {
                            epInitStruct.maxPacketSize = HS_ISO_IN_ENDP_PACKET_SIZE;
                            epInitStruct.interval      = HS_ISO_IN_ENDP_INTERVAL;
                        }
                        else
                        {
                            epInitStruct.maxPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE;
                            epInitStruct.interval      = FS_ISO_IN_ENDP_INTERVAL;
                        }

                        USB_DeviceInitEndpoint(s_audioGenerator.deviceHandle, &epInitStruct, &epCallback);
                        USB_AudioRecorderGetBuffer(s_wavBuff, (USB_SPEED_HIGH == s_audioGenerator.speed) ?
                                                                  HS_ISO_IN_ENDP_PACKET_SIZE :
                                                                  FS_ISO_IN_ENDP_PACKET_SIZE);
                        error = USB_DeviceSendRequest(
                            s_audioGenerator.deviceHandle, USB_AUDIO_STREAM_ENDPOINT, s_wavBuff,
                            (USB_SPEED_HIGH == s_audioGenerator.speed) ? HS_ISO_IN_ENDP_PACKET_SIZE :
                                                                         FS_ISO_IN_ENDP_PACKET_SIZE);
                    }
                    s_audioGenerator.currentStreamInterfaceAlternateSetting = alternateSetting;
                }
            }
            break;

        default:
            break;
    }

    return error;
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

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    SCTIMER_CaptureInit();
#endif

    if (kStatus_USB_Success != USB_DeviceInit(CONTROLLER_ID, USB_DeviceCallback, &s_audioGenerator.deviceHandle))
    {
        usb_echo("USB device failed\r\n");
        return;
    }
    else
    {
        usb_echo("USB device audio generator demo\r\n");
    }

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(s_audioGenerator.deviceHandle);
}

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

#if defined(AUDIO_DATA_SOURCE_DMIC) && (AUDIO_DATA_SOURCE_DMIC > 0U)
    Board_DMIC_DMA_Init();
#endif

    APPInit();

    while (1)
    {
#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(s_audioGenerator.deviceHandle);
#endif
    }
}
