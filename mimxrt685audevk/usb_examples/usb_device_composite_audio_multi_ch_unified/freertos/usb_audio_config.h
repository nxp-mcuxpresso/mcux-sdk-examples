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
#define USB_DEVICE_AUDIO_USE_SYNC_MODE (1U)

#define DEMO_TDM_AUDIO_OUTPUT_CHANNEL_COUNT (8U)
#define DEMO_AUIDO_OUTPUT_FORMAT_SIZE   (4U)
#define DEMO_AUDIO_INTPUT_CHANNEL_COUNT (1U)

/* Packet size and interval.
 * note: if ISO out endpoint interval is changed, please change AUDIO_UPDATE_FEEDBACK_DATA accordingly.
 */
#define HS_ISO_OUT_ENDP_INTERVAL (0x02) /* consider the capbility of CPU for some Socs, use 2 microframes interval */

/* the threshold transfer count that can tolerance by frame, this value is changed according to EP interval */
#define AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD (16U)

/* For ip3511hs in high speed mode, microframe can not be obtained and only for frame, the used feedback solution
 * requires us to have to use larger latency and buffer size to avoid buffer overflow or underflow. Sync mode can use
 * low latency (<1ms) even if on ip3511hs */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT \
    (28U) /* 6 means 16 mico frames (28*250us, the EP interval is 250us), make sure the latency is smaller than 1ms for sync mode */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT \
    (15U) /* 2 units size buffer (1 unit means the size to play during 1ms) */
#else
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (28U)
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT \
    (15U)
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
#define AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL  (16 * 2)

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
/* feedback calculate interval */
#define AUDIO_CALCULATE_Ff_INTERVAL (16U)
/* the threshold transfer count that can tolerance by frame */
#define USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD (4U)
/* feedback value discard times, the first feedback vaules are discarded */
#define AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT (4U)
#endif

/* pll adjustment and fro trim calculate interval */
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#define AUDIO_PLL_ADJUST_INTERVAL (8U)
#else
#define AUDIO_PLL_ADJUST_INTERVAL (2U)
#endif
#define AUDIO_FRO_ADJUST_INTERVAL (2U)

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
/**********************************************************************
Audio PLL contants
      AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT
      The Audio PLL clock is 24.576Mhz, and the USB SOF TOGGLE frequency is 1kHz when the full speed device is attached,
      so AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT = (24576000 * AUDIO_PLL_ADJUST_INTERVAL (stands for counter
interval) / 1000 = 49152
**********************************************************************/
#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
/* use external OSC clock source */
#define AUDIO_PLL_FRACTIONAL_DIVIDER (5040U)
/* The USB_SOF_TOGGLE's frequency is 4kHz. */
#define AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT (24576U * AUDIO_PLL_ADJUST_INTERVAL / 4U)
#endif

#define AUDIO_PLL_FRACTION_TICK_BASED_PRECISION (1U)
#define AUDIO_USB_FRO_TRIM_TICK_BASED_PRECISION (3U)
#define AUDIO_FRO_TRIM_DATA_BASED_INTERVAL      (16U)
#define AUDIO_PLL_ADJUST_DATA_BASED_INTERVAL    (16U)
#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
#define AUDIO_PLL_ADJUST_DATA_BASED_STEP (1U)
#endif
#endif /* USB_DEVICE_AUDIO_USE_SYNC_MODE */

/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
/*${prototype:end}*/

#endif /* _USB_AUDIO_CONFIG_H_ */
