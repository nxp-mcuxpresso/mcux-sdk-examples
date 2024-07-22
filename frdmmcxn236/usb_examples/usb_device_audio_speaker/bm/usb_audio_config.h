/*
 * Copyright 2023 NXP
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

/* Packet size and interval.
 * note: if ISO out endpoint interval is changed, please change AUDIO_UPDATE_FEEDBACK_DATA accordingly.
 */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
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

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
/* the threshold transfer count that can tolerance by frame */
#define AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD (5U)
#else
/* the threshold transfer count that can tolerance by frame */
#define AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD (4U)
#endif
#endif

/* For ip3511hs in high speed mode, microframe can not be obtained and only for frame, the used feedback solution
 * requires us to have to use larger latency and buffer size to avoid buffer overflow or underflow. Sync mode can use
 * low latency (<1ms) even if on ip3511hs */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
/* 6 means 16 mico frames (6*125us)*/
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (7U)
/* 7 units size buffer (1 unit means the size to play during 1ms) */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT (2U)
#else
/* 0x10 means 16 mico frames (32*125us, 4ms) */
#define AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT (32U)
/* 6 units size buffer (1 unit means the size to play during 1ms) */
#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT         (8U)
#endif
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

/* pll adjustment and fro trim calculate interval */
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
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
      interval) / 1000 = 49152. If high speed, the calculation is similar.
**********************************************************************/
#if (defined USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI)
/* use FRO clock source */
#define AUDIO_PLL_FRACTIONAL_DIVIDER (3435973837U)
#elif (defined USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI)
/* use external OSC clock source */
#define AUDIO_PLL_FRACTIONAL_DIVIDER (2886218023U)
#endif

#if (defined USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI)
/* The USB_SOF_TOGGLE's frequency is 1kHz. */
#define AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT (24576U * AUDIO_PLL_ADJUST_INTERVAL)
#define AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT (144000U * AUDIO_FRO_ADJUST_INTERVAL)
#elif (defined USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI)
/* The USB_SOF_TOGGLE's frequency is 4kHz. */
#define AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT (24576U * AUDIO_PLL_ADJUST_INTERVAL / 4U)
#endif

#define AUDIO_PLL_FRACTION_TICK_BASED_PRECISION (1U)
#define AUDIO_USB_FRO_TRIM_TICK_BASED_PRECISION (3U)
#define AUDIO_FRO_TRIM_DATA_BASED_INTERVAL      (16U)
#define AUDIO_PLL_ADJUST_DATA_BASED_INTERVAL    (16U)
#if (defined USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI)
#define AUDIO_PLL_ADJUST_DATA_BASED_STEP (64U)
#elif (defined USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI)
#define AUDIO_PLL_ADJUST_DATA_BASED_STEP (32U)
#endif
#endif /* USB_DEVICE_AUDIO_USE_SYNC_MODE */

/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
/*${prototype:end}*/

#endif /* _USB_AUDIO_CONFIG_H_ */
