/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_COMPOSITE_H__
#define __USB_DEVICE_COMPOSITE_H__ 1

#include "audio_unified.h"
#include "hid_keyboard.h"

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

#define USB_DEVICE_INTERRUPT_PRIORITY (3U)

#define AUDIO_SAMPLING_RATE_TO_10_14 (AUDIO_OUT_SAMPLING_RATE_KHZ << 10)
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#define AUDIO_SAMPLING_RATE_TO_16_16_SPECIFIC \
    ((AUDIO_OUT_SAMPLING_RATE_KHZ / (AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME / HS_ISO_OUT_ENDP_PACKET_SIZE)) << 12)
#elif defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#define AUDIO_SAMPLING_RATE_TO_16_16 \
    ((AUDIO_OUT_SAMPLING_RATE_KHZ / (AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME / HS_ISO_OUT_ENDP_PACKET_SIZE)) << 13)
#endif

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
extern volatile uint8_t feedbackValueUpdating;
#endif

#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if defined(USB_DEVICE_WORKAROUND_AUDIO_20_WINDOWS) && (USB_DEVICE_WORKAROUND_AUDIO_20_WINDOWS > 0)
/* change 10.10 data to 16.16 */
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
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
/* change 10.10 data to 10.14, 10.12 to 16.16 */
#define AUDIO_UPDATE_FEEDBACK_DATA(m, n)                  \
    feedbackValueUpdating = 1;                            \
    if (USB_SPEED_HIGH == g_composite.speed)              \
    {                                                     \
        m[0] = ((n << 4) & 0xFFU);                        \
        m[1] = (((n << 4) >> 8U) & 0xFFU);                \
        m[2] = (((n << 4) >> 16U) & 0xFFU);               \
        m[3] = (((n << 4) >> 24U) & 0xFFU);               \
    }                                                     \
    else                                                  \
    {                                                     \
        m[0] = ((n << 4) & 0xFFU);                        \
        m[1] = (((n << 4) >> 8U) & 0xFFU);                \
        m[2] = (((n << 4) >> 16U) & 0xFFU);               \
    }                                                     \
    feedbackValueUpdating = 0;
#elif defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#if defined(USB_DEVICE_AUDIO_SPEAKER_DEDICATED_INTERVAL) && (USB_DEVICE_AUDIO_SPEAKER_DEDICATED_INTERVAL > 0U)
/* change 10.10 data to 10.14, 10.12 to 16.16 */
#define AUDIO_UPDATE_FEEDBACK_DATA(m, n)                  \
    feedbackValueUpdating = 1;                            \
    if (USB_SPEED_HIGH == g_composite.speed)              \
    {                                                     \
        m[0] = ((n << 4) & 0xFFU);                        \
        m[1] = (((n << 4) >> 8U) & 0xFFU);                \
        m[2] = (((n << 4) >> 16U) & 0xFFU);               \
        m[3] = (((n << 4) >> 24U) & 0xFFU);               \
    }                                                     \
    else                                                  \
    {                                                     \
        m[0] = ((n << 4) & 0xFFU);                        \
        m[1] = (((n << 4) >> 8U) & 0xFFU);                \
        m[2] = (((n << 4) >> 16U) & 0xFFU);               \
    }                                                     \
    feedbackValueUpdating = 0;
#else
/* change 10.10 data to 10.14, 12.13 to 16.16 */
#define AUDIO_UPDATE_FEEDBACK_DATA(m, n)                  \
    feedbackValueUpdating = 1;                            \
    if (USB_SPEED_HIGH == g_composite.speed)              \
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
#endif /* USB_DEVICE_AUDIO_SPEAKER_DEDICATED_INTERVAL */
#else
/* change 10.10 data to 10.14 */
#define AUDIO_UPDATE_FEEDBACK_DATA(m, n)                     \
    {                                                        \
        feedbackValueUpdating = 1;                           \
        m[0]                  = ((n << 4) & 0xFFU);          \
        m[1]                  = (((n << 4) >> 8U) & 0xFFU);  \
        m[2]                  = (((n << 4) >> 16U) & 0xFFU); \
        feedbackValueUpdating = 0;                           \
    }
#endif /* USB_DEVICE_CONFIG_LPCIP3511HS */
#endif /* USB_DEVICE_WORKAROUND_AUDIO_20_WINDOWS */
#else
/* change 10.10 data to 10.14 */
#define AUDIO_UPDATE_FEEDBACK_DATA(m, n)                     \
    {                                                        \
        feedbackValueUpdating = 1;                           \
        m[0]                  = ((n << 4) & 0xFFU);          \
        m[1]                  = (((n << 4) >> 8U) & 0xFFU);  \
        m[2]                  = (((n << 4) >> 16U) & 0xFFU); \
        feedbackValueUpdating = 0;                           \
    }
#endif /* USB_DEVICE_CONFIG_AUDIO_CLASS_2_0 */

typedef struct _usb_device_composite_struct
{
    usb_device_handle deviceHandle;
    usb_audio_composite_struct_t audioUnified;
    usb_device_hid_keyboard_struct_t hidKeyboard;
    uint8_t speed;
    uint8_t attach;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_DEVICE_INTERFACE_COUNT];
} usb_device_composite_struct_t;

extern void audio_generator_task(void);

/*******************************************************************************
 * API
 ******************************************************************************/
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
extern usb_status_t USB_DeviceAudioCompositeCallback(class_handle_t handle, uint32_t event, void *param);
/*!
 * @brief Audio device set configuration function.
 *
 * This function sets configuration for Audio class.
 *
 * @param handle The Audio class handle.
 * @param configure The Audio class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceAudioCompositeSetConfigure(class_handle_t handle, uint8_t configure);
/*!
 * @brief Audio device initialization function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param device_composite The pointer to the composite device structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceAudioCompositeInit(usb_device_composite_struct_t *device_composite);
extern usb_status_t USB_DeviceAudioRecorderSetInterface(class_handle_t handle,
                                                        uint8_t interface,
                                                        uint8_t alternateSetting);
extern usb_status_t USB_DeviceAudioSpeakerSetInterface(class_handle_t handle,
                                                       uint8_t interface,
                                                       uint8_t alternateSetting);

/*!
 * @brief HID class specific callback function.
 *
 * This function handles the HID class specific requests.
 *
 * @param handle		  The HID class handle.
 * @param event 		  The HID class event type.
 * @param param 		  The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceHidKeyboardCallback(class_handle_t handle, uint32_t event, void *param);
/*!
 * @brief HID device set configuration function.
 *
 * This function sets configuration for HID class.
 *
 * @param handle The HID class handle.
 * @param configure The HID class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceHidKeyboardSetConfigure(class_handle_t handle, uint8_t configure);
extern usb_status_t USB_DeviceHidKeyboardSetInterface(class_handle_t handle,
                                                      uint8_t interface,
                                                      uint8_t alternateSetting);
/*!
 * @brief HID device initialization function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param device_composite The pointer to the composite device structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceHidKeyboardInit(usb_device_composite_struct_t *device_composite);

#endif
