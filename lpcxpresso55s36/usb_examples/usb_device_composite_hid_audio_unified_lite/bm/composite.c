/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
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

#include "usb_device_hid.h"
#include "usb_device_audio.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "fsl_adapter_audio.h"
#include "composite.h"

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

#include "usb_audio_config.h"
#include "fsl_i2c.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"
#include "fsl_wm8904.h"
#include "fsl_power.h"
#include "fsl_codec_adapter.h"
#include "fsl_codec_common.h"
#include "fsl_gint.h"
#include "fsl_sysctl.h"
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#include "fsl_ctimer.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_I2S_MASTER_CLOCK_FREQUENCY (24576000)
#define DEMO_DMA_INSTANCE_INDEX         (0U)
#define DEMO_I2S_TX_INSTANCE_INDEX      (0U)
#define DEMO_I2S_TX_CHANNEL             (5U)
#define DEMO_I2S_TX_MODE                kHAL_AudioMaster
#define DEMO_I2S_RX_INSTANCE_INDEX      (5U)
#define DEMO_I2S_RX_CHANNEL             (14U)
#define DEMO_I2S_RX_MODE                kHAL_AudioSlave

#define DEMO_GINT0_PORT kGINT_Port0
#define DEMO_GINT1_PORT kGINT_Port1

/* Select one input, active low for GINT0 */
#define DEMO_GINT0_POL_MASK ~(1U << BOARD_SW3_GPIO_PIN)
#define DEMO_GINT0_ENA_MASK (1U << BOARD_SW3_GPIO_PIN)

#define DEMO_GINT1_POL_MASK ~(1U << BOARD_SW1_GPIO_PIN)
#define DEMO_GINT1_ENA_MASK (1U << BOARD_SW1_GPIO_PIN)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern uint32_t USB_AudioSpeakerBufferSpaceUsed(void);
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
void CTIMER_SOF_TOGGLE_HANDLER_FRO(uint32_t i);
#endif
void CTIMER_SOF_TOGGLE_HANDLER_PLL(uint32_t i);
#else
extern void USB_DeviceCalculateFeedback(void);
#endif
extern void WM8904_USB_Audio_Init(void *I2CBase);
void WM8904_Config_Audio_Formats(uint32_t samplingRate);
void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
extern void AUDIO_DMA_EDMA_Start();
extern void BOARD_Codec_Init();
extern usb_status_t USB_DeviceHidKeyboardAction(void);
extern char *SW_GetName(void);
extern void USB_AudioCodecTask(void);
extern void USB_DeviceAudioSpeakerStatusReset(void);
extern void USB_DeviceAudioRecorderStatusReset(void);
extern usb_status_t USB_DeviceAudioRecorderSetInterface(usb_device_handle handle,
                                                        uint8_t interface,
                                                        uint8_t alternateSetting);
extern usb_status_t USB_DeviceAudioSpeakerSetInterface(usb_device_handle handle,
                                                       uint8_t interface,
                                                       uint8_t alternateSetting);
extern void USB_AudioSpeakerResetTask(void);
extern usb_status_t USB_DeviceAudioProcessTerminalRequest(uint32_t audioCommand,
                                                          uint32_t *length,
                                                          uint8_t **buffer,
                                                          uint8_t entityOrEndpoint);
/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_device_composite_struct_t g_composite;
extern uint8_t audioPlayDataBuff[AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME];
extern uint8_t audioRecDataBuff[AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE];

volatile bool g_ButtonPress = false;
static hal_audio_transfer_t s_TxTransfer;
static hal_audio_transfer_t s_RxTransfer;
HAL_AUDIO_HANDLE_DEFINE(audioTxHandle);
HAL_AUDIO_HANDLE_DEFINE(audioRxHandle);
hal_audio_config_t audioTxConfig;
hal_audio_config_t audioRxConfig;
hal_audio_dma_config_t dmaTxConfig;
hal_audio_dma_config_t dmaRxConfig;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t audioPlayDMATempBuff[AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t audioRecDMATempBuff[FS_ISO_IN_ENDP_PACKET_SIZE];
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
    .mclk_HZ            = DEMO_I2S_MASTER_CLOCK_FREQUENCY,
    .master             = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8904, .codecDevConfig = &wm8904Config};

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
static ctimer_config_t ctimerInfoFro;
ctimer_callback_t *cb_func_fro[] = {(ctimer_callback_t *)CTIMER_SOF_TOGGLE_HANDLER_FRO};
#endif
ctimer_callback_t *cb_func_pll[] = {(ctimer_callback_t *)CTIMER_SOF_TOGGLE_HANDLER_PLL};
static ctimer_config_t ctimerInfoPll;
#endif
/* Composite device structure. */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_device_composite_struct_t g_composite;
extern hal_audio_config_t audioTxConfig;
extern hal_audio_config_t audioRxConfig;
extern HAL_AUDIO_HANDLE_DEFINE(audioTxHandle);
extern HAL_AUDIO_HANDLE_DEFINE(audioRxHandle);
extern volatile bool g_ButtonPress;
extern usb_device_composite_struct_t *g_UsbDeviceComposite;
extern usb_device_composite_struct_t *g_deviceComposite;
extern uint8_t g_UsbDeviceInterface[USB_COMPOSITE_INTERFACE_COUNT];
extern uint32_t totalCount;
extern uint8_t audioFeedBackBuffer[4];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_SetupOutBuffer[8];
/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
void audio_fro_trim_up(void)
{
    static uint8_t usbClkAdj = 0;
    if (0U == usbClkAdj)
    {
        /* USBCLKADJ is turned off, start using software adjustment */
        ANACTRL->FRO192M_CTRL = (ANACTRL->FRO192M_CTRL & ~(ANACTRL_FRO192M_CTRL_USBCLKADJ_MASK));
        usbClkAdj             = 1U;
    }
    uint32_t val          = ANACTRL->FRO192M_CTRL;
    val                   = (val & ~(0xff << 16)) | ((((val >> 16) & 0xFF) + 1) << 16) | (1UL << 31);
    ANACTRL->FRO192M_CTRL = val;
}

void audio_fro_trim_down(void)
{
    static uint8_t usbClkAdj = 0;
    if (0U == usbClkAdj)
    {
        /* USBCLKADJ is turned off, start using software adjustment */
        ANACTRL->FRO192M_CTRL = (ANACTRL->FRO192M_CTRL & ~(ANACTRL_FRO192M_CTRL_USBCLKADJ_MASK));
        usbClkAdj             = 1U;
    }
    uint32_t val          = ANACTRL->FRO192M_CTRL;
    ANACTRL->FRO192M_CTRL = (val & ~(0xff << 16)) | ((((val >> 16) & 0xFF) - 1) << 16) | (1UL << 31);
}
#endif

void BOARD_DMA_EDMA_Set_AudioFormat(void)
{
    return;
}

void BOARD_DMA_EDMA_Enable_Audio_Interrupts(void)
{
    return;
}

void gint1_callback(void)
{
    g_ButtonPress = true;
}

void BOARD_USB_AUDIO_KEYBOARD_Init(void)
{
    GINT_Init(GINT1);
    GINT_SetCtrl(GINT1, kGINT_CombineOr, kGINT_TrigEdge, gint1_callback);
    GINT_ConfigPins(GINT1, DEMO_GINT1_PORT, DEMO_GINT1_POL_MASK, DEMO_GINT1_ENA_MASK);
    GINT_EnableCallback(GINT1);
}

char *SW_GetName(void)
{
    return BOARD_SW1_NAME;
}

void APP_InitDebugConsole(void)
{
    /* attach 12 MHz clock to FLEXCOMM6 */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM6);

    RESET_ClearPeripheralReset(kFC6_RST_SHIFT_RSTn);

    uint32_t uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;

    DbgConsole_Init(6U, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

void BOARD_InitSysctrl(void)
{
    SYSCTL_Init(SYSCTL);
    /* select signal source for share set */
    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet0, kSYSCTL_SharedCtrlSignalSCK, kSYSCTL_Flexcomm0);
    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet0, kSYSCTL_SharedCtrlSignalWS, kSYSCTL_Flexcomm0);
    /* select share set for special flexcomm signal */
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm0, kSYSCTL_FlexcommSignalSCK, kSYSCTL_ShareSet0);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm0, kSYSCTL_FlexcommSignalWS, kSYSCTL_ShareSet0);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm5, kSYSCTL_FlexcommSignalSCK, kSYSCTL_ShareSet0);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm5, kSYSCTL_FlexcommSignalWS, kSYSCTL_ShareSet0);
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

void BOARD_SetCodecMuteUnmute(bool mute)
{
    if (CODEC_SetMute(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, mute) !=
        kStatus_Success)
    {
        assert(false);
    }
}

static void TxCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    uint32_t audioSpeakerPreReadDataCount = 0U;
    uint32_t preAudioSendCount            = 0U;

    if ((USB_AudioSpeakerBufferSpaceUsed() < (g_composite.audioUnified.audioPlayTransferSize)) &&
        (g_composite.audioUnified.startPlayFlag == 1U))
    {
        g_composite.audioUnified.startPlayFlag          = 0;
        g_composite.audioUnified.speakerDetachOrNoInput = 1;
    }
    if (0U != g_composite.audioUnified.startPlayFlag)
    {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
        USB_DeviceCalculateFeedback();
#endif
        s_TxTransfer.dataSize = g_composite.audioUnified.audioPlayTransferSize;
        s_TxTransfer.data     = audioPlayDataBuff + g_composite.audioUnified.tdReadNumberPlay;
        preAudioSendCount     = g_composite.audioUnified.audioSendCount[0];
        g_composite.audioUnified.audioSendCount[0] += g_composite.audioUnified.audioPlayTransferSize;
        if (preAudioSendCount > g_composite.audioUnified.audioSendCount[0])
        {
            g_composite.audioUnified.audioSendCount[1] += 1U;
        }
        g_composite.audioUnified.audioSendTimes++;
        g_composite.audioUnified.tdReadNumberPlay += g_composite.audioUnified.audioPlayTransferSize;
        if (g_composite.audioUnified.tdReadNumberPlay >= g_composite.audioUnified.audioPlayBufferSize)
        {
            g_composite.audioUnified.tdReadNumberPlay = 0;
        }
        audioSpeakerPreReadDataCount = g_composite.audioUnified.audioSpeakerReadDataCount[0];
        g_composite.audioUnified.audioSpeakerReadDataCount[0] += g_composite.audioUnified.audioPlayTransferSize;
        if (audioSpeakerPreReadDataCount > g_composite.audioUnified.audioSpeakerReadDataCount[0])
        {
            g_composite.audioUnified.audioSpeakerReadDataCount[1] += 1U;
        }
    }
    else
    {
        if (0U != g_composite.audioUnified.audioPlayTransferSize)
        {
            s_TxTransfer.dataSize = g_composite.audioUnified.audioPlayTransferSize;
        }
        else
        {
            s_TxTransfer.dataSize = HS_ISO_OUT_ENDP_PACKET_SIZE;
        }
        s_TxTransfer.data = audioPlayDMATempBuff;
    }
    HAL_AudioTransferSendNonBlocking(handle, &s_TxTransfer);
}

static void RxCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    if (g_composite.audioUnified.startRec)
    {
        s_RxTransfer.dataSize = FS_ISO_IN_ENDP_PACKET_SIZE;
        s_RxTransfer.data     = audioRecDataBuff + g_composite.audioUnified.tdWriteNumberRec;
        g_composite.audioUnified.tdWriteNumberRec += FS_ISO_IN_ENDP_PACKET_SIZE;
        if (g_composite.audioUnified.tdWriteNumberRec >=
            AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE)
        {
            g_composite.audioUnified.tdWriteNumberRec = 0;
        }
    }
    else
    {
        s_RxTransfer.dataSize = FS_ISO_IN_ENDP_PACKET_SIZE;
        s_RxTransfer.data     = audioRecDMATempBuff;
    }
    HAL_AudioTransferReceiveNonBlocking(handle, &s_RxTransfer);
}

void AUDIO_DMA_EDMA_Start()
{
    usb_echo("Init Audio I2S and CODEC\r\n");
    s_TxTransfer.dataSize = HS_ISO_OUT_ENDP_PACKET_SIZE;
    s_TxTransfer.data     = audioPlayDMATempBuff;
    s_RxTransfer.dataSize = FS_ISO_IN_ENDP_PACKET_SIZE;
    s_RxTransfer.data     = audioRecDMATempBuff;

    HAL_AudioTxInstallCallback((hal_audio_handle_t)&audioTxHandle[0], TxCallback, (void *)&s_TxTransfer);
    HAL_AudioRxInstallCallback((hal_audio_handle_t)&audioRxHandle[0], RxCallback, (void *)&s_RxTransfer);

    /* need to queue two receive buffers so when the first one is filled,
     * the other is immediatelly starts to be filled */
    HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&audioRxHandle[0], &s_RxTransfer);
    HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&audioRxHandle[0], &s_RxTransfer);

    HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&audioTxHandle[0], &s_TxTransfer);
}

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
void USB_AudioPllChange(void)
{
    SYSCON->PLL0SSCG0 = g_composite.audioUnified.curAudioPllFrac;
}

void CTIMER_CaptureInit(void)
{
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    INPUTMUX->TIMER0CAP[0] = 0x14U; /* 0x15U for USB1 and 0x14U for USB0. */
#endif
    CTIMER_GetDefaultConfig(&ctimerInfoPll);

    /* Initialize CTimer module */
    CTIMER_Init(CTIMER0, &ctimerInfoPll);

    CTIMER_SetupCapture(CTIMER0, kCTIMER_Capture_0, kCTIMER_Capture_RiseEdge, true);

    CTIMER_RegisterCallBack(CTIMER0, (ctimer_callback_t *)&cb_func_pll[0], kCTIMER_SingleCallback);

    /* Start the L counter */
    CTIMER_StartTimer(CTIMER0);

    /* if full speed controller, use another ctimer */
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    INPUTMUX->TIMER1CAP[0] = 0x14U; /* 0x15U for USB1 and 0x14U for USB0. */

    CTIMER_GetDefaultConfig(&ctimerInfoFro);

    CTIMER_Init(CTIMER1, &ctimerInfoFro);

    CTIMER_SetupCapture(CTIMER1, kCTIMER_Capture_0, kCTIMER_Capture_RiseEdge, true);

    CTIMER_RegisterCallBack(CTIMER1, (ctimer_callback_t *)&cb_func_fro[0], kCTIMER_SingleCallback);

    /* Start the L counter */
    CTIMER_StartTimer(CTIMER1);
#endif
}
#endif

#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
void USB0_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_composite.deviceHandle);
}
#endif
void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbfsSrcFro, CLOCK_GetFroHfFreq());
    ANACTRL->FRO192M_CTRL = (ANACTRL->FRO192M_CTRL & ~(ANACTRL_FRO192M_CTRL_USBCLKADJ_MASK));
#endif
}
void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USB_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0];
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
}
#endif

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
            /* USB bus reset signal detected */
            /* Initialize the control IN and OUT pipes */
            for (count = 0U; count < USB_DEVICE_INTERFACE_COUNT; count++)
            {
                g_composite.currentInterfaceAlternateSetting[count] = 0U;
            }
            /* reset audio speaker status to be the initialized status */
            USB_DeviceAudioSpeakerStatusReset();
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
            /* reset the the last feedback value */
            g_composite.audioUnified.lastFeedbackValue             = 0U;
#endif
            USB_DeviceControlPipeInit(handle);
            g_composite.attach               = 0U;
            g_composite.currentConfiguration = 0U;
            error                            = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success ==
                USB_DeviceGetStatus(g_composite.deviceHandle, kUSB_DeviceStatusSpeed, &g_composite.speed))
            {
                USB_DeviceSetSpeed(g_composite.speed);
            }
            if (USB_SPEED_HIGH == g_composite.speed)
            {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
                g_composite.audioUnified.currentStreamOutMaxPacketSize = (HS_ISO_OUT_ENDP_PACKET_SIZE);
#else
                g_composite.audioUnified.currentStreamOutMaxPacketSize =
                    (HS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE);
                g_composite.audioUnified.currentFeedbackMaxPacketSize = HS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
#endif /* USB_DEVICE_AUDIO_USE_SYNC_MODE */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
                /* high speed and audio 2.0, audio play interval is set by HS EP packet size */
                g_composite.audioUnified.audioPlayTransferSize = HS_ISO_OUT_ENDP_PACKET_SIZE;
                /* use short play buffer size, only use two elements */
                g_composite.audioUnified.audioPlayBufferSize =
                    AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME * AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_16_16_SPECIFIC);
#else
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_16_16);
#endif
#endif
#else
                /* high speed and audio 1.0, audio play interval is 1 ms using this play size */
                g_composite.audioUnified.audioPlayTransferSize = AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
                /* use the whole play buffer size */
                g_composite.audioUnified.audioPlayBufferSize =
                    AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
#endif
#endif /* USB_DEVICE_CONFIG_AUDIO_CLASS_2_0 */
                g_composite.audioUnified.speed = USB_SPEED_HIGH;
            }
            else
            {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
                g_composite.audioUnified.currentStreamOutMaxPacketSize = (FS_ISO_OUT_ENDP_PACKET_SIZE);
#else
                g_composite.audioUnified.currentStreamOutMaxPacketSize =
                    (FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE);
                g_composite.audioUnified.currentFeedbackMaxPacketSize = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
#endif
                /* full speed, audio play interval is 1 ms using this play size */
                g_composite.audioUnified.audioPlayTransferSize = AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
                /* use the whole play buffer size */
                g_composite.audioUnified.audioPlayBufferSize =
                    AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
            }
#else
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
            g_composite.audioUnified.currentStreamOutMaxPacketSize = (FS_ISO_OUT_ENDP_PACKET_SIZE);
#else
            g_composite.audioUnified.currentStreamOutMaxPacketSize =
                (FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE);
            g_composite.audioUnified.currentFeedbackMaxPacketSize = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
            AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
#endif
            /* full speed, audio play interval is 1 ms using this play size */
            g_composite.audioUnified.audioPlayTransferSize = AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
            /* use the whole play buffer size */
            g_composite.audioUnified.audioPlayBufferSize =
                AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME;
#endif /* USB_DEVICE_CONFIG_EHCI, USB_DEVICE_CONFIG_LPCIP3511HS */
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_composite.attach               = 0U;
                g_composite.currentConfiguration = 0U;
                error                            = kStatus_USB_Success;
            }
            else if (USB_COMPOSITE_CONFIGURE_INDEX == (*temp8))
            {
                g_composite.attach               = 1U;
                g_composite.currentConfiguration = *temp8;
                USB_DeviceAudioUnifiedSetConfigure(handle, *temp8);
                USB_DeviceHidKeyboardSetConfigure(handle, *temp8);
                error = kStatus_USB_Success;
            }
            else
            {
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (g_composite.attach)
            {
                uint8_t interface        = (uint8_t)(*temp8);
                uint8_t alternateSetting = (uint8_t)g_UsbDeviceInterface[interface];
                if (g_composite.audioUnified.currentInterfaceAlternateSetting[interface] != alternateSetting)
                {
                    if (USB_AUDIO_RECORDER_STREAM_INTERFACE_INDEX == interface)
                    {
                        if (alternateSetting < USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_COUNT)
                        {
                            if (USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_0 == alternateSetting)
                            {
                                error = USB_DeviceDeinitEndpoint(
                                    g_composite.deviceHandle,
                                    USB_AUDIO_RECORDER_STREAM_ENDPOINT |
                                        (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT));
                            }
                            else
                            {
                                USB_DeviceAudioRecorderStatusReset();
                                error = USB_DeviceAudioRecorderSetInterface(handle, interface, alternateSetting);
                            }
                            g_composite.audioUnified.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        }
                    }
                    else if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_INDEX == interface)
                    {
                        if (alternateSetting < USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_COUNT)
                        {
                            if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_0 == alternateSetting)
                            {
                                error = USB_DeviceDeinitEndpoint(
                                    g_composite.deviceHandle,
                                    USB_AUDIO_SPEAKER_STREAM_ENDPOINT |
                                        (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT));
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
                                g_composite.audioUnified.stopDataLengthAudioAdjust = 1U;
#endif
                            }
                            else
                            {
                                error = USB_DeviceAudioSpeakerSetInterface(handle, interface, alternateSetting);
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
                                g_composite.audioUnified.stopDataLengthAudioAdjust = 0U;
#endif
                            }
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
                            /* usb host stops the speaker, so there is no need for feedback */
                            if ((1U == g_composite.audioUnified.startPlayFlag) &&
                                (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_0 == alternateSetting))
                            {
                                g_composite.audioUnified.stopFeedbackUpdate = 1U;
                            }

                            /* usb host start the speaker, discard the feedback for AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT
                             * times */
                            if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_1 == alternateSetting)
                            {
                                g_composite.audioUnified.stopFeedbackUpdate   = 0U;
                                g_composite.audioUnified.feedbackDiscardFlag  = 1U;
                                g_composite.audioUnified.feedbackDiscardTimes = AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT;
                            }
#endif
                            g_composite.audioUnified.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        }
                    }
                    else if (USB_AUDIO_CONTROL_INTERFACE_INDEX == interface)
                    {
                        if (alternateSetting < USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_COUNT)
                        {
                            g_composite.audioUnified.currentInterfaceAlternateSetting[interface] = alternateSetting;
                            error                                                                = kStatus_USB_Success;
                        }
                    }
                    else
                    {
                        /* no action, invalid request */
                    }
                }
                else
                {
                    error = kStatus_USB_Success;
                }
            }
            break;
        default:
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
    static uint32_t compositeSetup[2];
    if (NULL == setupBuffer)
    {
        return kStatus_USB_InvalidParameter;
    }
    *setupBuffer = (usb_setup_struct_t *)&compositeSetup;
    return kStatus_USB_Success;
}

/*!
 * @brief Get the vendor request data buffer.
 *
 * This function gets the data buffer for vendor request.
 *
 * @param handle The USB device handle.
 * @param setup The pointer to the setup packet.
 * @param length The pointer to the length of the data buffer.
 * @param buffer The pointer to the address of setup packet data buffer.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetVendorReceiveBuffer(usb_device_handle handle,
                                              usb_setup_struct_t *setup,
                                              uint32_t *length,
                                              uint8_t **buffer)
{
    return kStatus_USB_Error;
}

/*!
 * @brief Audio vendor specific callback function.
 *
 * This function handles the CDC vendor specific requests.
 *
 * @param handle The USB device handle.
 * @param setup The pointer to the setup packet.
 * @param length The pointer to the length of the data buffer.
 * @param buffer The pointer to the address of setup packet data buffer.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceProcessVendorRequest(usb_device_handle handle,
                                            usb_setup_struct_t *setup,
                                            uint32_t *length,
                                            uint8_t **buffer)
{
    return kStatus_USB_InvalidRequest;
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
    usb_status_t error = kStatus_USB_InvalidRequest;
    error              = USB_DeviceAudioUnifiedConfigureEndpointStatus(handle, ep, status);
    error              = USB_DeviceHidConfigureEndpointStatus(handle, ep, status);

    return error;
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
 * @brief Audio class specific callback function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle The USB device handle.
 * @param setup The pointer to the setup packet.
 * @param length The pointer to the length of the data buffer.
 * @param buffer The pointer to the address of setup packet data buffer.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceProcessClassRequest(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    if ((((setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) != USB_REQUEST_TYPE_RECIPIENT_INTERFACE)) ||
        ((((setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) == USB_REQUEST_TYPE_RECIPIENT_INTERFACE)) &&
         (USB_AUDIO_CONTROL_INTERFACE_INDEX == (setup->wIndex & 0xFFU))))
    {
        return USB_DeviceAudioUnifiedClassRequest(handle, setup, length, buffer);
    }
    else if (USB_HID_KEYBOARD_INTERFACE_INDEX == (setup->wIndex & 0xFFU))
    {
        return USB_DeviceHidKeyboardClassRequest(handle, setup, buffer, length);
    }
    else
    {
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

    g_composite.speed        = USB_SPEED_FULL;
    g_composite.attach       = 0U;
    g_composite.deviceHandle = NULL;

    if (kStatus_USB_Success != USB_DeviceInit(CONTROLLER_ID, USB_DeviceCallback, &g_composite.deviceHandle))
    {
        usb_echo("USB device composite demo init failed\r\n");
        return;
    }
    else
    {
        usb_echo("USB device composite demo\r\n");
        usb_echo("Please Press switch(%s) to mute/unmute device audio speaker.\r\n", SW_GetName());

        USB_DeviceAudioUnifiedInit(&g_composite);
        USB_DeviceHidKeyboardInit(&g_composite);
    }

    /*Initialize audio interface and codec.*/
    HAL_AudioTxInit((hal_audio_handle_t)audioTxHandle, &audioTxConfig);
    HAL_AudioRxInit((hal_audio_handle_t)audioRxHandle, &audioRxConfig);
    BOARD_Codec_Init();
    AUDIO_DMA_EDMA_Start();

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_composite.deviceHandle);
}

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
    /* USART clock */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom6Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom6Clk, 1u, true);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM6);
    /* I2C clock */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom7Clk, 0U, true);  /*!< Reset FCCLKDIV7 divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom7Clk, 1U, false); /*!< Set FCCLKDIV7 divider to value 1 */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM7);
    /*!< Configure XTAL32M */
    POWER_DisablePD(kPDRUNCFG_PD_XTALHF);                   /* Ensure XTAL32M is powered */
    POWER_DisablePD(kPDRUNCFG_PD_LDOXTALHF);                /* Ensure XTAL32M is powered */
    CLOCK_SetupExtClocking(16000000U);                      /* Enable clk_in clock */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK; /*!< Ensure CLK_IN is on  */
    ANACTRL->XO32M_CTRL |= ANACTRL_XO32M_CTRL_ENABLE_SYSTEM_CLK_OUT_MASK;

    /*!< Set up PLL, use FRO for full speed */
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    CLOCK_AttachClk(kFRO12M_to_PLL0);   /*!< Switch PLL0CLKSEL to FRO12M */
    POWER_DisablePD(kPDRUNCFG_PD_PLL0); /* Ensure PLL is on  */
    POWER_DisablePD(kPDRUNCFG_PD_PLL0_SSCG);
    const pll_setup_t pll0Setup = {
        .pllctrl = SYSCON_PLL0CTRL_LIMUPOFF_MASK | SYSCON_PLL0CTRL_CLKEN_MASK | SYSCON_PLL0CTRL_SELI(4U) |
                   SYSCON_PLL0CTRL_SELP(3U),
        .pllndec = SYSCON_PLL0NDEC_NDIV(3U),
        .pllpdec = SYSCON_PLL0PDEC_PDIV(8U),
        .pllsscg = {(SYSCON_PLL0SSCG0_MD_LBS(3298534883U)), (SYSCON_PLL0SSCG1_MD_MBS(0U) | (uint32_t)(kSS_MF_512) |
                                                             (uint32_t)(kSS_MR_K0) | (uint32_t)(kSS_MC_NOC))},
        .pllRate = 24576000U,
        .flags   = PLL_SETUPFLAG_POWERUP};
#endif

    /*!< Configure PLL to the desired values */
    CLOCK_SetPLL0Freq(&pll0Setup);

    CLOCK_SetClkDiv(kCLOCK_DivPllClk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivPllClk, 1U, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg0, 0U, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg5, 0U, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1U, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom5Clk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom5Clk, 1U, false);

    /* SYSCTL clocks */
    CLOCK_EnableClock(kCLOCK_Sysctl);

    /* I2S clocks */
    CLOCK_AttachClk(kPLL0_to_PLLCLKDIV);
    CLOCK_AttachClk(kPLL_CLK_DIV_FRG0_to_FLEXCOMM0);
    CLOCK_AttachClk(kPLL_CLK_DIV_FRG5_to_FLEXCOMM5);

    /* Attach PLL clock to MCLK for I2S, no divider */
    CLOCK_SetClkDiv(kCLOCK_DivMclk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivMclk, 1U, false);
    CLOCK_AttachClk(kPLL0_to_MCLK);
    SYSCON->MCLKIO = 1U;

    /* reset FLEXCOMM for I2C */
    RESET_PeripheralReset(kFC7_RST_SHIFT_RSTn);

    /* reset FLEXCOMM for DMA0 */
    RESET_PeripheralReset(kDMA0_RST_SHIFT_RSTn);

    /* reset FLEXCOMM for I2S */
    RESET_PeripheralReset(kFC0_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kFC5_RST_SHIFT_RSTn);

    /* reset NVIC for FLEXCOMM0 */
    NVIC_ClearPendingIRQ(FLEXCOMM0_IRQn);
    NVIC_ClearPendingIRQ(FLEXCOMM5_IRQn);

    /* Enable interrupts for I2S */
    EnableIRQ(FLEXCOMM0_IRQn);
    EnableIRQ(FLEXCOMM5_IRQn);

    /* Initialize the rest */
    BOARD_InitBootPins();
    BOARD_BootClockFROHF96M();
    APP_InitDebugConsole();
    BOARD_InitSysctrl();

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    POWER_DisablePD(kPDRUNCFG_PD_USBFSPHY); /*< Turn on USB0 Phy */

    /* reset the IP to make sure it's in reset state. */
    RESET_PeripheralReset(kUSB0_DEV_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);

#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    POWER_DisablePD(kPDRUNCFG_PD_USBFSPHY); /*< Turn on USB Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
#endif
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 0U, true);  /*!< Reset CTIMER0CLKDIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1U, false); /*!< Set CTIMER0CLKDIV divider to value 1 */
    CLOCK_AttachClk(kPLL0_to_CTIMER0);
    CLOCK_SetClkDiv(kCLOCK_DivCtimer1Clk, 0U, true);  /*!< Reset CTIMER1CLKDIV divider counter and halt it */
    CLOCK_SetClkDiv(kCLOCK_DivCtimer1Clk, 1U, false); /*!< Set CTIMER1CLKDIV divider to value 1 */
    CLOCK_AttachClk(kFRO_HF_to_CTIMER1);
#endif
    BOARD_USB_AUDIO_KEYBOARD_Init();

    dmaTxConfig.instance            = DEMO_DMA_INSTANCE_INDEX;
    dmaTxConfig.channel             = DEMO_I2S_TX_CHANNEL;
    dmaTxConfig.priority            = kHAL_AudioDmaChannelPriority3;
    audioTxConfig.dmaConfig         = &dmaTxConfig;
    audioTxConfig.instance          = DEMO_I2S_TX_INSTANCE_INDEX;
    audioTxConfig.srcClock_Hz       = CLOCK_GetPll0OutFreq();
    audioTxConfig.sampleRate_Hz     = 48000;
    audioTxConfig.msaterSlave       = DEMO_I2S_TX_MODE;
    audioTxConfig.bclkPolarity      = kHAL_AudioSampleOnRisingEdge;
    audioTxConfig.frameSyncPolarity = kHAL_AudioBeginAtFallingEdge;
    audioTxConfig.frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame;
    audioTxConfig.dataFormat        = kHAL_AudioDataFormatI2sClassic;
    audioTxConfig.bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits;
    audioTxConfig.lineChannels      = kHAL_AudioStereo;

    dmaRxConfig.instance            = DEMO_DMA_INSTANCE_INDEX;
    dmaRxConfig.channel             = DEMO_I2S_RX_CHANNEL;
    dmaRxConfig.priority            = kHAL_AudioDmaChannelPriority2;
    audioRxConfig.dmaConfig         = &dmaRxConfig;
    audioRxConfig.instance          = DEMO_I2S_RX_INSTANCE_INDEX;
    audioRxConfig.srcClock_Hz       = CLOCK_GetPll0OutFreq();
    audioRxConfig.sampleRate_Hz     = 48000;
    audioRxConfig.msaterSlave       = DEMO_I2S_RX_MODE;
    audioRxConfig.bclkPolarity      = kHAL_AudioSampleOnRisingEdge;
    audioRxConfig.frameSyncPolarity = kHAL_AudioBeginAtFallingEdge;
    audioRxConfig.frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame;
    audioRxConfig.dataFormat        = kHAL_AudioDataFormatI2sClassic;
    audioRxConfig.bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits;
    audioRxConfig.lineChannels      = kHAL_AudioStereo;

    APPInit();

    while (1)
    {
        USB_DeviceHidKeyboardAction();

        USB_AudioCodecTask();

        USB_AudioSpeakerResetTask();

#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_composite.deviceHandle);
#endif
    }
}
