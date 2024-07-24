/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
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

#include "fsl_power.h"
#include "fsl_mrt.h"
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

#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if !((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
void USB_DeviceHsPhyChirpIssueWorkaround(void);
void USB_DeviceDisconnected(void);
#endif
#endif
usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param);
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
extern void USB_AudioRecorderGetBuffer(uint8_t *buffer, uint32_t size);
#if defined(AUDIO_DATA_SOURCE_DMIC) && (AUDIO_DATA_SOURCE_DMIC > 0U)
extern void Board_DMIC_DMA_Init(void);
#elif defined(AUDIO_DATA_SOURCE_PDM) && (AUDIO_DATA_SOURCE_PDM > 0U)
extern void Board_PDM_EDMA_Init(void);
#endif
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
extern void SCTIMER_CaptureInit(void);
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t hwTick;
uint32_t timerInterval;
uint32_t isConnectedToFsHost = 0U;
uint32_t isConnectedToHsHost = 0U;
/* Audio data information */
extern uint8_t s_wavBuff[];

extern usb_device_class_struct_t g_UsbDeviceAudioClass;

/* Default value of audio generator device struct */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_audio_generator_struct_t s_audioGenerator = {
    NULL,                  /* deviceHandle */
    NULL,                  /* audioHandle */
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
#if (defined(AUDIO_DATA_SOURCE_DMIC) && (AUDIO_DATA_SOURCE_DMIC > 0U)) || \
    (defined(AUDIO_DATA_SOURCE_PDM) && (AUDIO_DATA_SOURCE_PDM > 0U))
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
void USB_TimerInit(uint8_t instance, uint32_t interval)
{
    MRT_Type *instanceList[] = MRT_BASE_PTRS;
    IRQn_Type instanceIrq[]  = MRT_IRQS;
    /* Structure of initialize MRT */
    mrt_config_t mrtConfig;
    /* mrtConfig.enableMultiTask = false; */
    MRT_GetDefaultConfig(&mrtConfig);
    /* Init mrt module */
    MRT_Init(instanceList[instance], &mrtConfig);
    /* Setup Channel 0 to be repeated */
    MRT_SetupChannelMode(instanceList[instance], kMRT_Channel_0, kMRT_RepeatMode);
    /* Enable timer interrupts for channel 0 */
    MRT_EnableInterrupts(instanceList[instance], kMRT_Channel_0, kMRT_TimerInterruptEnable);
    timerInterval = interval;
    /* Enable at the NVIC */
    EnableIRQ(instanceIrq[instance]);
}
void USB_TimerInt(uint8_t instance, uint8_t enable)
{
    MRT_Type *instanceList[] = MRT_BASE_PTRS;
    uint32_t mrt_clock;
    mrt_clock = CLOCK_GetFreq(kCLOCK_BusClk);
    if (enable)
    {
        /* Start channel 0 */
        MRT_StartTimer(instanceList[instance], kMRT_Channel_0, USEC_TO_COUNT(timerInterval, mrt_clock));
    }
    else
    {
        /* Stop channel 0 */
        MRT_StopTimer(instanceList[instance], kMRT_Channel_0);
        /* Clear interrupt flag.*/
        MRT_ClearStatusFlags(instanceList[instance], kMRT_Channel_0, kMRT_TimerInterruptFlag);
    }
}
void MRT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    MRT_ClearStatusFlags(MRT0, kMRT_Channel_0, kMRT_TimerInterruptFlag);
    if (hwTick)
    {
        hwTick--;
        if (!hwTick)
        {
            USB_TimerInt(0, 0);
        }
    }
    else
    {
        USB_TimerInt(0, 0);
    }
}
void USB_DeviceDisconnected(void)
{
    isConnectedToFsHost = 0U;
}
/*
 * This is a work-around to fix the HS device Chirping issue.
 * The device (IP3511HS controller) will not work sometimes when the cable
 * is attached at the first time after a Power-on Reset.
 */
void USB_DeviceHsPhyChirpIssueWorkaround(void)
{
    uint32_t startFrame = USBHSD->INFO & USBHSD_INFO_FRAME_NR_MASK;
    uint32_t currentFrame;
    uint32_t isConnectedToFsHostFlag = 0U;
    if ((!isConnectedToHsHost) && (!isConnectedToFsHost))
    {
        if (((USBHSD->DEVCMDSTAT & USBHSD_DEVCMDSTAT_Speed_MASK) >> USBHSD_DEVCMDSTAT_Speed_SHIFT) == 0x01U)
        {
            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U | USBHSD_DEVCMDSTAT_PHY_TEST_MODE_MASK))) |
                                 USBHSD_DEVCMDSTAT_PHY_TEST_MODE(0x05U);
            hwTick = 100;
            USB_TimerInt(0, 1);
            usb_echo("The USB device PHY chirp work-around is working\r\n");
            while (hwTick)
            {
            }
            currentFrame = USBHSD->INFO & USBHSD_INFO_FRAME_NR_MASK;
            if (currentFrame != startFrame)
            {
                isConnectedToHsHost = 1U;
            }
            else
            {
                hwTick = 1;
                USB_TimerInt(0, 1);
                while (hwTick)
                {
                }
                currentFrame = USBHSD->INFO & USBHSD_INFO_FRAME_NR_MASK;
                if (currentFrame != startFrame)
                {
                    isConnectedToHsHost = 1U;
                }
                else
                {
                    isConnectedToFsHostFlag = 1U;
                }
            }
            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U | USBHSD_DEVCMDSTAT_PHY_TEST_MODE_MASK)));
            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U | USBHSD_DEVCMDSTAT_DCON_MASK)));
            hwTick             = 510;
            USB_TimerInt(0, 1);
            while (hwTick)
            {
            }
            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U))) | USB_DEVCMDSTAT_DCON_C_MASK;
            USBHSD->DEVCMDSTAT =
                (USBHSD->DEVCMDSTAT & (~(0x0F000000U))) | USBHSD_DEVCMDSTAT_DCON_MASK | USB_DEVCMDSTAT_DRES_C_MASK;
            if (isConnectedToFsHostFlag)
            {
                isConnectedToFsHost = 1U;
            }
        }
    }
}
#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
void USB0_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(s_audioGenerator.deviceHandle);
}
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
void USB1_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(s_audioGenerator.deviceHandle);
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
/*!
 * @brief Audio class specific request function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle           The Audio class handle.
 * @param event            The Audio class event type.
 * @param param            The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceAudioRequest(class_handle_t handle, uint32_t event, void *param)
{
    usb_device_control_request_struct_t *request = (usb_device_control_request_struct_t *)param;
    usb_status_t error                           = kStatus_USB_Success;

    switch (event)
    {
        case USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            request->buffer = (uint8_t *)&s_audioGenerator.curMute20;
            request->length = sizeof(s_audioGenerator.curMute20);
#else
            request->buffer = &s_audioGenerator.curMute;
            request->length = sizeof(s_audioGenerator.curMute);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            request->buffer = (uint8_t *)&s_audioGenerator.curVolume20;
            request->length = sizeof(s_audioGenerator.curVolume20);
#else
            request->buffer = s_audioGenerator.curVolume;
            request->length = sizeof(s_audioGenerator.curVolume);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL:
            request->buffer = &s_audioGenerator.curBass;
            request->length = sizeof(s_audioGenerator.curBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL:
            request->buffer = &s_audioGenerator.curMid;
            request->length = sizeof(s_audioGenerator.curMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL:
            request->buffer = &s_audioGenerator.curTreble;
            request->length = sizeof(s_audioGenerator.curTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            request->buffer = &s_audioGenerator.curAutomaticGain;
            request->length = sizeof(s_audioGenerator.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL:
            request->buffer = s_audioGenerator.curDelay;
            request->length = sizeof(s_audioGenerator.curDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL:
            request->buffer = s_audioGenerator.minVolume;
            request->length = sizeof(s_audioGenerator.minVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL:
            request->buffer = &s_audioGenerator.minBass;
            request->length = sizeof(s_audioGenerator.minBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL:
            request->buffer = &s_audioGenerator.minMid;
            request->length = sizeof(s_audioGenerator.minMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL:
            request->buffer = &s_audioGenerator.minTreble;
            request->length = sizeof(s_audioGenerator.minTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL:
            request->buffer = s_audioGenerator.minDelay;
            request->length = sizeof(s_audioGenerator.minDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL:
            request->buffer = s_audioGenerator.maxVolume;
            request->length = sizeof(s_audioGenerator.maxVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL:
            request->buffer = &s_audioGenerator.maxBass;
            request->length = sizeof(s_audioGenerator.maxBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL:
            request->buffer = &s_audioGenerator.maxMid;
            request->length = sizeof(s_audioGenerator.maxMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL:
            request->buffer = &s_audioGenerator.maxTreble;
            request->length = sizeof(s_audioGenerator.maxTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL:
            request->buffer = s_audioGenerator.maxDelay;
            request->length = sizeof(s_audioGenerator.maxDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL:
            request->buffer = s_audioGenerator.resVolume;
            request->length = sizeof(s_audioGenerator.resVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL:
            request->buffer = &s_audioGenerator.resBass;
            request->length = sizeof(s_audioGenerator.resBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL:
            request->buffer = &s_audioGenerator.resMid;
            request->length = sizeof(s_audioGenerator.resMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL:
            request->buffer = &s_audioGenerator.resTreble;
            request->length = sizeof(s_audioGenerator.resTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL:
            request->buffer = s_audioGenerator.resDelay;
            request->length = sizeof(s_audioGenerator.resDelay);
            break;
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&s_audioGenerator.curSampleFrequency;
            request->length = sizeof(s_audioGenerator.curSampleFrequency);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = (uint8_t *)&s_audioGenerator.curSampleFrequency;
                request->length = sizeof(s_audioGenerator.curSampleFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL:
            request->buffer = &s_audioGenerator.curClockValid;
            request->length = sizeof(s_audioGenerator.curClockValid);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.curClockValid;
                request->length = sizeof(s_audioGenerator.curClockValid);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&s_audioGenerator.freqControlRange;
            request->length = sizeof(s_audioGenerator.freqControlRange);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL:
            request->buffer = (uint8_t *)&s_audioGenerator.volumeControlRange;
            request->length = sizeof(s_audioGenerator.volumeControlRange);
            break;
#else
        case USB_DEVICE_AUDIO_EP_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = s_audioGenerator.curSamplingFrequency;
            request->length = sizeof(s_audioGenerator.curSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MIN_SAMPLING_FREQ_CONTROL:
            request->buffer = s_audioGenerator.minSamplingFrequency;
            request->length = sizeof(s_audioGenerator.minSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MAX_SAMPLING_FREQ_CONTROL:
            request->buffer = s_audioGenerator.maxSamplingFrequency;
            request->length = sizeof(s_audioGenerator.maxSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_RES_SAMPLING_FREQ_CONTROL:
            request->buffer = s_audioGenerator.resSamplingFrequency;
            request->length = sizeof(s_audioGenerator.resSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.curSamplingFrequency;
                request->length = sizeof(s_audioGenerator.curSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_RES_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.resSamplingFrequency;
                request->length = sizeof(s_audioGenerator.resSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_MAX_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.maxSamplingFrequency;
                request->length = sizeof(s_audioGenerator.maxSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_MIN_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.minSamplingFrequency;
                request->length = sizeof(s_audioGenerator.minSamplingFrequency);
            }
            break;
#endif
        case USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.curVolume20;
                request->length = sizeof(s_audioGenerator.curVolume20);
            }
            else
            {
                uint16_t volume = (uint16_t)((uint16_t)s_audioGenerator.curVolume20[1] << 8U);
                volume |= (uint8_t)(s_audioGenerator.curVolume20[0]);
            }
#else
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.curVolume;
                request->length = sizeof(s_audioGenerator.curVolume);
            }
            else
            {
                uint16_t volume = (uint16_t)((uint16_t)s_audioGenerator.curVolume[1] << 8U);
                volume |= (uint8_t)(s_audioGenerator.curVolume[0]);
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.curMute;
                request->length = sizeof(s_audioGenerator.curMute);
            }
            else
            {
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.curBass;
                request->length = sizeof(s_audioGenerator.curBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.curMid;
                request->length = sizeof(s_audioGenerator.curMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.curTreble;
                request->length = sizeof(s_audioGenerator.curTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.curAutomaticGain;
                request->length = sizeof(s_audioGenerator.curAutomaticGain);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.curDelay;
                request->length = sizeof(s_audioGenerator.curDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.minVolume;
                request->length = sizeof(s_audioGenerator.minVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.minBass;
                request->length = sizeof(s_audioGenerator.minBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.minMid;
                request->length = sizeof(s_audioGenerator.minMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.minTreble;
                request->length = sizeof(s_audioGenerator.minTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.minDelay;
                request->length = sizeof(s_audioGenerator.minDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.maxVolume;
                request->length = sizeof(s_audioGenerator.maxVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.maxBass;
                request->length = sizeof(s_audioGenerator.maxBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.maxMid;
                request->length = sizeof(s_audioGenerator.maxMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.maxTreble;
                request->length = sizeof(s_audioGenerator.maxTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.maxDelay;
                request->length = sizeof(s_audioGenerator.maxDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.resVolume;
                request->length = sizeof(s_audioGenerator.resVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.resBass;
                request->length = sizeof(s_audioGenerator.resBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.resMid;
                request->length = sizeof(s_audioGenerator.resMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &s_audioGenerator.resTreble;
                request->length = sizeof(s_audioGenerator.resTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = s_audioGenerator.resDelay;
                request->length = sizeof(s_audioGenerator.resDelay);
            }
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}

/*!
 * @brief Audio class specific callback function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle            The Audio class handle.
 * @param event             The Audio class event type.
 * @param param             The parameter of the class specific request.
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
            if ((0U != s_audioGenerator.attach) &&
                (ep_cb_param->length == ((USB_SPEED_HIGH == s_audioGenerator.speed) ? HS_ISO_IN_ENDP_PACKET_SIZE :
                                                                                      FS_ISO_IN_ENDP_PACKET_SIZE)))
            {
                USB_AudioRecorderGetBuffer(s_wavBuff, (USB_SPEED_HIGH == s_audioGenerator.speed) ?
                                                          HS_ISO_IN_ENDP_PACKET_SIZE :
                                                          FS_ISO_IN_ENDP_PACKET_SIZE);
                error = USB_DeviceAudioSend(handle, USB_AUDIO_STREAM_ENDPOINT, s_wavBuff,
                                            (USB_SPEED_HIGH == s_audioGenerator.speed) ? HS_ISO_IN_ENDP_PACKET_SIZE :
                                                                                         FS_ISO_IN_ENDP_PACKET_SIZE);
            }
            break;

        default:
            if ((NULL != param) && (event > 0xFFU))
            {
                error = USB_DeviceAudioRequest(handle, event, param);
            }
            break;
    }
    return error;
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
    uint16_t *temp16   = (uint16_t *)param;
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

#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if !((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
            /* The work-around is used to fix the HS device Chirping issue.
             * Please refer to the implementation for the detail information.
             */
            USB_DeviceHsPhyChirpIssueWorkaround();
#endif
#endif

#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &s_audioGenerator.speed))
            {
                USB_DeviceSetSpeed(handle, s_audioGenerator.speed);
            }
#endif
        }
        break;
#if (defined(USB_DEVICE_CONFIG_DETACH_ENABLE) && (USB_DEVICE_CONFIG_DETACH_ENABLE > 0U))
        case kUSB_DeviceEventDetach:
        {
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if !((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
            USB_DeviceDisconnected();
#endif
#endif
            error = kStatus_USB_Success;
        }
        break;
#endif
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
                /* no action */
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (0U != s_audioGenerator.attach)
            {
                /* Set alternateSetting of the interface request */
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);

                if (USB_AUDIO_CONTROL_INTERFACE_INDEX == interface)
                {
                    if (alternateSetting < USB_AUDIO_GENERATOR_CONTROL_INTERFACE_ALTERNATE_COUNT)
                    {
                        s_audioGenerator.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                                        = kStatus_USB_Success;
                    }
                }
                else if (USB_AUDIO_STREAM_INTERFACE_INDEX == interface)
                {
                    if (alternateSetting < USB_AUDIO_GENERATOR_STREAM_INTERFACE_ALTERNATE_COUNT)
                    {
                        s_audioGenerator.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                                        = kStatus_USB_Success;
                        if (USB_AUDIO_GENERATOR_STREAM_INTERFACE_ALTERNATE_1 == alternateSetting)
                        {
                            USB_AudioRecorderGetBuffer(s_wavBuff, (USB_SPEED_HIGH == s_audioGenerator.speed) ?
                                                                      HS_ISO_IN_ENDP_PACKET_SIZE :
                                                                      FS_ISO_IN_ENDP_PACKET_SIZE);
                            error = USB_DeviceAudioSend(
                                s_audioGenerator.audioHandle, USB_AUDIO_STREAM_ENDPOINT, s_wavBuff,
                                (USB_SPEED_HIGH == s_audioGenerator.speed) ? HS_ISO_IN_ENDP_PACKET_SIZE :
                                                                             FS_ISO_IN_ENDP_PACKET_SIZE);
                        }
                    }
                }
                else
                {
                    /* no action */
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (NULL != param)
            {
                /* Get the current configuration request */
                *temp8 = s_audioGenerator.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (NULL != param)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_AUDIO_GENERATOR_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | s_audioGenerator.currentInterfaceAlternateSetting[interface];
                    error   = kStatus_USB_Success;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (NULL != param)
            {
                /* Get the device descriptor request */
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (NULL != param)
            {
                /* Get the configuration descriptor request */
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (NULL != param)
            {
                /* Get the string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        default:
            /* no action */
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
static void APPInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    SCTIMER_CaptureInit();
#endif

    if (kStatus_USB_Success != USB_DeviceClassInit(CONTROLLER_ID, &s_audioConfigList, &s_audioGenerator.deviceHandle))
    {
        usb_echo("USB device failed\r\n");
        return;
    }
    else
    {
        usb_echo("USB device audio generator demo\r\n");
        s_audioGenerator.audioHandle = s_audioConfigList.config->classHandle;
    }

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(s_audioGenerator.deviceHandle);
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
    CLOCK_EnableClock(kCLOCK_InputMux);
    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);
    /* USART0 clock */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* reset USB0 and USB1 device */
    RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1H_RST_SHIFT_RSTn);

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    NVIC_ClearPendingIRQ(USB1_IRQn);
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);

    /* DMIC uses 12MHz FRO clock */
    CLOCK_AttachClk(kFRO12M_to_DMIC);
    /*12MHz divided by 5 = 2.4MHz PDM clock --> gives 48kHz sample rate */
    /*12MHz divided by 15 = 800 KHz PDM clock --> gives 16kHz sample rate */
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 15, false);
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
    USB_TimerInit(0, 1000U);
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

#if defined(AUDIO_DATA_SOURCE_DMIC) && (AUDIO_DATA_SOURCE_DMIC > 0U)
    Board_DMIC_DMA_Init();
#elif defined(AUDIO_DATA_SOURCE_PDM) && (AUDIO_DATA_SOURCE_PDM > 0U)
    Board_PDM_EDMA_Init();
#endif

    APPInit();

    while (1)
    {
#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(s_audioGenerator.deviceHandle);
#endif
    }
}
