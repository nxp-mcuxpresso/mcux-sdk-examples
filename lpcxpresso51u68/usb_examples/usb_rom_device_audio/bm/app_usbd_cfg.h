/*
 * @brief Configuration file needed for USB ROM stack based applications.
 *
 * @note
 * Copyright  2013, NXP
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */
#include "lpc_types.h"
#include "error.h"

#ifndef __APP_USB_CFG_H_
#define __APP_USB_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup EXAMPLES_USBDROM_11U6X_CDC
 * @{
 */

/* Manifest constants used by USBD ROM stack. These values SHOULD NOT BE CHANGED
   for advance features which require usage of USB_CORE_CTRL_T structure.
   Since these are the values used for compiling USB stack.
 */
#define USB_MAX_IF_NUM         8    /*!< Max interface number used for building USBD ROM. DON'T CHANGE. */
#define USB_MAX_EP_NUM         5    /*!< Max number of EP used for building USBD ROM. DON'T CHANGE. */
#define USB_MAX_PACKET0        64   /*!< Max EP0 packet size used for building USBD ROM. DON'T CHANGE. */
#define USB_FS_MAX_BULK_PACKET 64   /*!< MAXP for FS bulk EPs used for building USBD ROM. DON'T CHANGE. */
#define USB_HS_MAX_BULK_PACKET 512  /*!< MAXP for HS bulk EPs used for building USBD ROM. DON'T CHANGE. */
#define USB_DFU_XFER_SIZE      2048 /*!< Max DFU transfer size used for building USBD ROM. DON'T CHANGE. */

/* Uncomment the following define to enable low power mode */
//#define LOWPOWEROPERATION

/* If using the LOWPOWEROPERATION definition, the normal operational
 * mode clock rate divider can be selected here. Legal values are 1,
 * 2, and 4 (based on 48MHz). If 4 is selected, the main clock will
 * be driven from the FRO12MHz clock. At 2, the main clock is 48MHz
 * with a CPU divider of 2. Not all combinations of LOWPOWERCLOCKMODE
 * and USEINRAMDEEPSLEEP may work.
 */
#define LOWPOWERCLOCKMODE 1

/* Uncomment this define to use the in-RAM version of Deep Sleep
 * for putting the system to sleep. If commented out, the ROM
 * vesion of deep power down will be used.
 */
#define USEINRAMDEEPSLEEP

/* Uncomment the following define to ENABLE target JTAG pins, leave
   commented to save power, but you won't be able to debug */
#define ENABLEJTAG

/**********************************************************************
 ** USB defines
 **********************************************************************/
#define USB_AUDIO_SYNC_MAXP 3

/* interface numbers */
//#define USB_iAP_IF_NUM          0
//#define USB_ADC_CIF_NUM     	1
//#define USB_ADC_SIF1_NUM    	2
//#define USB_ADC_SIF2_NUM    	3
#define USB_AUDIO_CIF_NUM  0
#define USB_AUDIO_SIF1_NUM 1
#define USB_AUDIO_SIF2_NUM 2

/* Manifest constants defining interface numbers and endpoints used by a
   particular interface in this application.
 */
#define USB_ADC_CTRL_IF    0
#define USB_ADC_SPEAKER_IF 1
#define USB_ADC_MIC_IF     2
#define USB_ADC_IN_EP      0x81
#define USB_ADC_OUT_EP     0x01

#define DEF_SAMPLE_RATE  48000 /* default sample rate */
#define DEF_RES_BITS     16    /* 16-Bits per sample per channel */
#define DEF_NUM_CHANNELS 2     /* 2-Channel Stereo in/out */

#define NUM_DTDS        32             /* For 44.1KHz this should be multiple of 10 [Typical 20 or 40] */
#define DATA_VALID_DIFF (NUM_DTDS / 2) /* minimum difference between index to set DATAVALID flag */
#define SILENCE_DATA    0              // 0x80008000
#define AUDIO_DEF_THRES (48 * 2)       /* Frequency adjust happens when we lead/lag by this rate in ms */

/* The following manifest constants are used to define this memory area to be used
   by USBD ROM stack.
 */
#define USB_STACK_MEM_BASE 0x20004000
#define USB_STACK_MEM_SIZE 0x800
/* USB IRQ priority */
#define USB_DEVICE_INTERRUPT_PRIORITY (3U)

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __APP_USB_CFG_H_ */
