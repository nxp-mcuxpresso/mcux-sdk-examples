/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_AUDIO_CONFIG_H__
#define __USB_AUDIO_CONFIG_H__ 1U

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
/*! @brief Whether USB Audio use syn mode or not, note that some socs may not support sync mode */
#define USB_DEVICE_AUDIO_USE_SYNC_MODE (0U)

/* Packet size and interval.
 * note: if ISO out endpoint interval is changed, please change AUDIO_UPDATE_FEEDBACK_DATA accordingly.
 */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#define HS_ISO_OUT_ENDP_INTERVAL (0x01)
#else
#define HS_ISO_OUT_ENDP_INTERVAL (0x02) /* consider the capbility of CPU for some Socs, use 2 microframes interval */
#endif
#elif (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
#if (defined(USB_DEVICE_AUDIO_SPEAKER_DEDICATED_INTERVAL) && (USB_DEVICE_AUDIO_SPEAKER_DEDICATED_INTERVAL > 0U))
#define HS_ISO_OUT_ENDP_INTERVAL (0x02)
#else
#define HS_ISO_OUT_ENDP_INTERVAL (0x01)
#endif
#else
#define HS_ISO_OUT_ENDP_INTERVAL (0x04)
#endif
#else
#define HS_ISO_OUT_ENDP_INTERVAL (0x04) /*interval must be 1ms for usb audio 1.0 */
#endif

/* For ip3511hs in high speed mode, microframe can not be obtained and only for frame, the used feedback solution
 * requires us to have to use larger latency and buffer size to avoid buffer overflow or underflow. Sync mode can use
 * low latency (<1ms) even if on ip3511hs */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT \
    (0x06U) /* 6 means 16 mico frames (6*125us), make sure the latency is smaller than 1ms for sync mode */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT \
    (2U) /* 2 units size buffer (1 unit means the size to play during 1ms) */
#else
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (0x08U) /* 0x08 means 8 transfer count */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT \
    (0x06U) /* 6 units size buffer (1 unit means the size to play during 1ms) */
#endif
#elif (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
#if (defined(USB_DEVICE_AUDIO_SPEAKER_DEDICATED_INTERVAL) && (USB_DEVICE_AUDIO_SPEAKER_DEDICATED_INTERVAL > 0U))
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (0x03U) /* 0x03 means 3 transfer count */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT \
    (2U) /* 2 units size buffer (1 unit means the size to play during 1ms) */
#else
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT \
    (0x06U) /* 6 means 16 mico frames (6*125us), make sure the latency is smaller than 1ms for ehci high speed */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT \
    (2U) /* 2 units size buffer (1 unit means the size to play during 1ms) */
#endif
#endif
#endif

#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL  (16 * 2)
#define AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL (16 * 2)

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
/* feedback calculate interval */
#define AUDIO_CALCULATE_Ff_INTERVAL (16U)
/* the threshold transfer count that can tolerance by frame */
#define USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD (4U)
/* feedback value discard times, the first feedback vaules are discarded */
#define AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT (4U)
#endif


/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
/*${prototype:end}*/

#endif /* _USB_AUDIO_CONFIG_H_ */
