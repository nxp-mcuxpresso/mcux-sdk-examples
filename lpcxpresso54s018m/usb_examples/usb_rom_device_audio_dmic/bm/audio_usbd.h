/*
 * @brief Programming API used with USB Audio class module
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

#ifndef __AUDIO_USBD_H_
#define __AUDIO_USBD_H_

#include "app_usbd_cfg.h"
#include "usbd_rom_api.h"
#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup EXAMPLES_USBDROM_11UXX_ADC
 * @{
 */

/* FIXME: Do we need separate defines for IN and OUT (?) */
#define AUDIO_BYTES_PER_CHANNEL (((DEF_RES_BITS + 8) / 16) * 2)
#define AUDIO_BYTES_PER_SAMPLE  (DEF_NUM_CHANNELS * AUDIO_BYTES_PER_CHANNEL)
#define AUDIO_MAX_PKT_SZ        (32)

#define DTD_INVALID_IDX 0xFF

/* controller flags */
#define ADC_PLAY_MUTE      0x0001
#define ADC_PLAY_DATAVALID 0x0002
#define ADC_PLAYING        0x0008
#define ADC_REC_DATAVALID  0x0010
#define ADC_REC_BUF_BUSY   0x0020
#define ADC_RECORDING      0x0080
#define ADC_USB_SUSPEND    0x0100

/** sub-stream types */
#define SUBS_DMIC      1
#define MAX_SUBSTREAMS 2
/** ADC USB stream mode defines */
#define ADC_USB_WRITER 0
#define ADC_USB_READER 1

/** endpoint transfer descriptor */
typedef volatile struct _EP_TD
{
    uint32_t buf_ptr;
    uint32_t buf_length;
} EP_TD_T;

/** endpoint transfer descriptor */
typedef volatile struct _EP_Queue
{
    uint32_t td[2];
} EP_Queue_T;

/* structure to store Sub-stream control  data */
typedef struct _ADC_SUBSTREAM_T
{
    /* Endpoint TD's queue associated with stream */
    EP_TD_T eTD[32];
    uint32_t *epQH;                /* endpoint HW queue head */
    uint32_t sample_rate;          /* stream rate */
    uint16_t maxp;                 /* Max packet size */
    volatile uint8_t wr_idx;       /* writer index */
    volatile uint8_t wr_valid_idx; /* index+1 of TD from where we started reciving valid data */
    volatile uint8_t rd_idx;       /* reader index */
    uint8_t usb_idx;               /* id to track ping-pong on LPC11u USB TDs */
    uint32_t ep_index;             /* endpoint index */
    uint32_t ep_num;               /* endpoint number per USB spec */

    uint16_t packet_sz; /* Packet size computed for given sample rate and size. */

    int32_t total_diff; /* rolling average of sample rate difference between source & sink*/
    int32_t wr_count;   /* sample received/written to the buffer */
    int32_t rd_count;   /* rendered sample count*/

} ADC_SUBSTREAM_T;

/* Structure to store Audio class driver control data */
typedef struct _ADC_CTRL_T
{
    USBD_HANDLE_T hUsb;      /* handle to ROM stack */
    uint32_t *ep_list_hw;    /* LPC11u hardware TD list */
    ADC_SUBSTREAM_T subs[2]; /* pointers to sub-stream strcutres */
    uint16_t flags;          /* controller state flags */

} USB_ADC_CTRL_T;

/* structure to hold parameter for ADC_init() routine. */
typedef struct _ADC_INIT_PARAM_T
{
    USBD_HANDLE_T hUsb;           /* [IN] handle to USB stack */
    USB_ADC_CTRL_T *pAdcCtrl;     /* [OUT] pointer to ADC controller associated
                  with the stack handle. */
    USBD_API_INIT_PARAM_T *param; /* [IN] ADC_init() reuses some of the
               members of USBD_API_INIT_PARAM_T for it init. */
    uint32_t maxp[2];             /* [IN] Largest MAX audio packet size of the audio
                        endpoint inclusive of all alternate interfaces for
                        each substream. If this value is 0 then the substream
                        is absent.*/
    uint32_t ep_num[2];           /* [IN] enpoint address associated with the substream */

} ADC_INIT_PARAM_T;

#define INDEX_INC(x)       \
    if (++(x) >= NUM_DTDS) \
    {                      \
        (x) = 0;           \
    }
#define INDEX_DIFF(diff, x, y)         \
    if ((y) > (x))                     \
    {                                  \
        diff = (NUM_DTDS - (y)) + (x); \
    }                                  \
    else                               \
    {                                  \
        diff = (x) - (y);              \
    }

/* exported data */
extern USB_ADC_CTRL_T g_AdcCtrl;

/* Buffers to be used by audio codec */
extern void *const out_buff;
extern const uint32_t out_buff_sz;
extern void *const in_buff;
extern const uint32_t in_buff_sz;

/* exported routines */
/**
 * @brief	Audio device class initialization routine
 * @param	adc_param	: Pointer to ADC_INIT_PARAM_T struture
 * @return	LPC_OK on success.
 */
extern ErrorCode_t ADC_init(ADC_INIT_PARAM_T *adc_param);

/**
 * @brief	Virtual com port write routine
 * @param	pBuf	: Pointer to buffer to be written
 * @param	buf_len	: Length of the buffer passed
 * @return	Number of bytes written
 */
extern void ADC_start_xfr(USB_ADC_CTRL_T *pAdcCtrl, ADC_SUBSTREAM_T *pSubs, uint32_t mode);

/**
 * @brief	Virtual com port write routine
 * @param	hUsb	: Handle to USBD stack
 * @return	Number of bytes written
 */
extern void ADC_stop_xfr(USB_ADC_CTRL_T *pAdcCtrl, ADC_SUBSTREAM_T *pSubs, uint32_t mode);

/**
 * @brief	Interface event handler of USB audio class
 * @param	hUsb	: Handle to USBD stack
 * @return	Number of bytes written
 */
extern ErrorCode_t ADC_Interface_Event(USBD_HANDLE_T hUsb);

/**
 * @brief	Reset event handler of USB audio class
 * @param	hUsb	: Handle to USBD stack
 * @return	Number of bytes written
 */
extern ErrorCode_t ADC_Reset_Event(USBD_HANDLE_T hUsb);

/**
 * @brief	Suspend event handler of USB audio class
 * @param	hUsb	: Handle to USBD stack
 * @return	Number of bytes written
 */
extern ErrorCode_t ADC_Suspend_Event(USBD_HANDLE_T hUsb);

/**
 * @brief	Resume event handler of USB audio class
 * @param	hUsb	: Handle to USBD stack
 * @return	Number of bytes written
 */
extern ErrorCode_t ADC_Resume_Event(USBD_HANDLE_T hUsb);

/**
 * @brief	Start of Frame event
 * @param	hUsb	: Handle to USBD stack
 * @return	LPC_OK
 */
extern ErrorCode_t ADC_SOF_Event(USBD_HANDLE_T hUsb);

extern void ADC_Flush_Audio(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_USBD_H_ */
