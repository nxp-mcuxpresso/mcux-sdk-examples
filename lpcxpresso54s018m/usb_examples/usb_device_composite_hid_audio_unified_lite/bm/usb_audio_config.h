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
/* pll adjustment and fro trim calculate interval */
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#define AUDIO_PLL_ADJUST_INTERVAL (8U)
#else
#define AUDIO_PLL_ADJUST_INTERVAL (2U)
#endif
#define AUDIO_FRO_ADJUST_INTERVAL (1024U)

/* the threshold transfer count that can tolerance by frame */
#define AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD (4U)

#define AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL  (16 * 2)
#define AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL (16 * 2)

/**********************************************************************
Audio PLL contants
      AUDIO_PLL_USB_SOF_INTERVAL_COUNT
      The Audio PLL clock is 24.576Mhz, and the USB SOF TOGGLE frequency is 4kHz when the high speed device is attached,
      so AUDIO_PLL_USB_SOF_INTERVAL_COUNT = (24576000 * AUDIO_PLL_ADJUST_INTERVAL
      (stands for counter interval) / 4000 = 49152.
**********************************************************************/
#define AUDIO_PLL_FRACTIONAL_DIVIDER          (0x16872b)
#define AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT (49152U)
#define AUDIO_PLL_FRACTIONAL_CHANGE_STEP      (((12000000 * 100) / (32768 * 11 * 4000)) + 1)

#define AUDIO_PLL_FRACTION_TICK_BASED_PRECISION (1U)
#define AUDIO_PLL_ADJUST_DATA_BASED_INTERVAL    (16U)
#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
#define AUDIO_PLL_ADJUST_DATA_BASED_STEP (32U)
#elif (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
#define AUDIO_PLL_ADJUST_DATA_BASED_STEP (1U)
#endif

/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
/*${prototype:end}*/

#endif /* _USB_AUDIO_CONFIG_H_ */
