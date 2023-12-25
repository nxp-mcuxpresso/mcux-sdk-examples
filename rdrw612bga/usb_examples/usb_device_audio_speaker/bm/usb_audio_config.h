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

/*! @brief Whether USB Audio use syn mode or not, note that this case only supports async mode now  */
#define USB_DEVICE_AUDIO_USE_SYNC_MODE (0U)

/* Packet size and interval. */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if ((defined(USB_AUDIO_CHANNEL7_1) && (USB_AUDIO_CHANNEL7_1 > 0U)) || \
     (defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)) || \
     (defined(USB_AUDIO_CHANNEL3_1) && (USB_AUDIO_CHANNEL3_1 > 0U)))
#define HS_ISO_OUT_ENDP_INTERVAL (0x02)
#else
#define HS_ISO_OUT_ENDP_INTERVAL (0x01)
#endif
#else
#define HS_ISO_OUT_ENDP_INTERVAL (0x04) /*interval must be 1ms for usb audio 1.0 */
#endif
#define HS_ISO_IN_ENDP_INTERVAL (0x04)

#if ((defined(USB_AUDIO_CHANNEL7_1) && (USB_AUDIO_CHANNEL7_1 > 0U)) || \
     (defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)) || \
     (defined(USB_AUDIO_CHANNEL3_1) && (USB_AUDIO_CHANNEL3_1 > 0U)))
/* the threshold transfer count that can tolerance by frame */
#define AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD (8U)
#else
#define AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD (4U)
#endif

/* For ip3511hs in high speed mode, microframe can not be obtained and only for frame, the used feedback solution
 * requires us to have to use larger latency and buffer size to avoid buffer overflow or underflow. Sync mode can use
 * low latency (<1ms) even if on ip3511hs */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#if ((defined(USB_AUDIO_CHANNEL7_1) && (USB_AUDIO_CHANNEL7_1 > 0U)) || \
     (defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)) || \
     (defined(USB_AUDIO_CHANNEL3_1) && (USB_AUDIO_CHANNEL3_1 > 0U)))
/* 16 means 16 mico frames (16 * 250us), depends on the iso out interval */
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (16U)
/* 2 units size buffer (1 unit means the size to play during 1ms) */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT (8U)
#else
/* 6 means 6 mico frames (6 * 125us), depends on the iso out interval */
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (6U)
/* 2 units size buffer (1 unit means the size to play during 1ms) */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT         (2U)
#endif
#else
#if ((defined(USB_AUDIO_CHANNEL7_1) && (USB_AUDIO_CHANNEL7_1 > 0U)) || \
     (defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)) || \
     (defined(USB_AUDIO_CHANNEL3_1) && (USB_AUDIO_CHANNEL3_1 > 0U)))
/* 16 means 16 mico frames (16 * 250us), depends on the iso out interval */
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (16U)
/* 8 units size buffer (1 unit means the size to play during 1ms) */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT         (8U)
#else
/* 16 means 16 mico frames (16*125us, 2ms) */
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (16U)
/* 6 units size buffer (1 unit means the size to play during 1ms) */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT         (6U)
#endif
#endif /* USB_DEVICE_AUDIO_USE_SYNC_MODE */
#elif (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
/* 6 means 16 mico frames (6*125us), make sure the latency is smaller than 1ms for ehci high speed */
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (0x06U)
/* 2 units size buffer (1 unit means the size to play during 1ms) */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT         (2U)
#endif
#endif

#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL  (16 * 2)
#define AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL (16 * 2)

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
/* feedback calculate interval */
#define AUDIO_CALCULATE_Ff_INTERVAL                        (16U)
/* the threshold transfer count that can tolerance by frame */
#define USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD (4U)
/* feedback value discard times, the first feedback vaules are discarded */
#define AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT               (4U)
#endif

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
/* pll adjustment calculate interval */
#define AUDIO_PLL_ADJUST_INTERVAL (8U)
/**********************************************************************
Audio PLL contants
      AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT
      The Audio PLL clock is 12.288Mhz, the USB SOF TOGGLE frequency is 4kHz.
      so AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT = (12288000 * AUDIO_PLL_ADJUST_INTERVAL (stands for counter interval) /
1000 / 4
**********************************************************************/
#define AUDIO_PLL_FRACTIONAL_DIVIDER (5213U)
/* The USB_SOF_TOGGLE's frequency is 4kHz. */
#define AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT   (12288U * AUDIO_PLL_ADJUST_INTERVAL / 4U)
#define AUDIO_PLL_FRACTION_TICK_BASED_PRECISION (1U)
#define AUDIO_PLL_ADJUST_DATA_BASED_INTERVAL    (16U)
#define AUDIO_PLL_ADJUST_DATA_BASED_STEP        (1U)
#endif /* USB_DEVICE_AUDIO_USE_SYNC_MODE */
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
/*${prototype:end}*/

#endif /* _USB_AUDIO_CONFIG_H_ */
