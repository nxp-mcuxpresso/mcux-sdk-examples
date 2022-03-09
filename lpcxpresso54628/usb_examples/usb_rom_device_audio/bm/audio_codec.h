/*
 * @brief Codec interface used by USBD audio module
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

#include <stdio.h>
#include "audio_usbd.h"

#ifndef __ADC_CODEC_H__
#define __ADC_CODEC_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup EXAMPLES_USBDROM_11UXX_ADC
 * @{
 */

/* Codec Task defines - each bit signifies a task maximum 16 tasks can be scheduled */
#define CODEC_TASK_MUTE    (1UL << 0)
#define CODEC_TASK_UNMUTE  (1UL << 1)
#define CODEC_TASK_VOLUME  (1UL << 2)
#define CODEC_TASK_COMPUTE (1UL << 3)
#define CODEC_TASK_REROUTE (1UL << 4)

/* Codec state defines */
#define CODEC_ST_SUSPENDED   (1UL << 0)
#define CODEC_ST_IOH_ON      (1UL << 1)
#define CODEC_ST_INIT_DONE   (1UL << 2)
#define CODEC_ST_MUTE        (1UL << 3)
#define CODEC_ST_SRATE_CHNGD (1UL << 4)

typedef enum
{
    eCodecI2SOUT = 0,
    eCodecIN1OUT,
    eCodecIN2OUT,
    eCodecMicnI2SOUT,
} Codec_Route_t;

/* Interface to manage individual codecs */
typedef struct _CodecInterface_t
{
    uint16_t volumeMin;
    uint16_t volumeMax;
    uint16_t volumeRes;
    uint16_t volumeDef;
} CodecInterface_t;

typedef struct __Codec_Ctrl
{
    const CodecInterface_t *hw; /* pointer to individual codec interface */
    uint16_t state;             /* Codec state */
    uint16_t cur_rate;
    uint16_t cnt44k; /* count to track 44.1 packets since every 10th packet we need to send 45 samples */
    volatile uint16_t tasks; /* Codec tasks schedule from interrupt context and executed in main thread */ // KWW
    int16_t volume; /* Volume Current Value */
    Codec_Route_t route;
} Codec_Ctrl_t;

/* exported data */
extern Codec_Ctrl_t g_codec;
/* list of codec interfaces pre-built in flash image */
extern const CodecInterface_t g_wm8960Ctrl;

/**
 * @brief	Initialize Codec sub-system
 * @param	hwCtrl	: Pointer to codec interface structure
 * @return	Nothing
 * @note	This module uses SSP1 and Timer32B0 blocks.
 */
extern ErrorCode_t Codec_Init(const CodecInterface_t *hwCtrl);

/**
 * @brief	Executes scheduled codec tasks which can't be done in IRQ context.
 * @return	Nothing
 * @note	This routine executes in user/system task context.
 */
extern ErrorCode_t Codec_Tasks(void);

extern ErrorCode_t Codec_SetSampleRate(USB_ADC_CTRL_T *pAdcCtrl, uint16_t rate);

extern ErrorCode_t Codec_Record(void *pAdcCtrl, uint32_t altId);

extern ErrorCode_t Codec_Play(void *pCtrl, uint32_t altId);

extern ErrorCode_t Codec_usb_rx_done(void *pAdcCtrl, ADC_SUBSTREAM_T *pSubs, uint32_t len);

/**
 * @brief	Routine to handle codec clean-up in USB reset event.
 * @return	Nothing
 * @note	This routine should be called from USB reset event handler.
 */
extern void Codec_Reset(void);

/**
 * @brief	Suspend codec and corresponding clocks
 * @return	Nothing
 * @note
 */
extern void Codec_Suspend(void);

/**
 * @brief	Resume codec and corresponding clocks
 * @return	Nothing
 * @note
 */
extern void Codec_Resume(void);

/**
 * @brief	Get the minimum volume supported by codec
 * @return	Returns minimum volume setting supported by the codec.
 * @note	This routine returns, volume value based on USB Audio 1.0 class specification.
 */
static INLINE uint16_t Codec_GetVolumeMin(void)
{
    return g_codec.hw->volumeMin;
}

/**
 * @brief	Get the maximum volume supported by codec
 * @return	Returns maximum volume setting supported by the codec.
 * @note	This routine returns, volume value based on USB Audio 1.0 class specification.
 */
static INLINE uint16_t Codec_GetVolumeMax(void)
{
    return g_codec.hw->volumeMax;
}

/**
 * @brief	Get the volume resolution supported by codec
 * @return	Returns volume resolution supported by the codec.
 * @note	This routine returns, volume value based on USB Audio 1.0 class specification.
 */
static INLINE uint16_t Codec_GetVolumeRes(void)
{
    return g_codec.hw->volumeRes;
}

/**
 * @brief	Get the default volume
 * @return	Returns the default volume
 * @note	This routine returns, volume value based on USB Audio 1.0 class specification.
 */
static INLINE uint16_t Codec_GetVolumeDef(void)
{
    return g_codec.hw->volumeDef;
}

/**
 * @brief	Get current volume
 * @return	Returns current volume
 * @note	This routine returns, volume value based on USB Audio 1.0 class specification.
 */
static INLINE uint16_t Codec_GetVolume(void)
{
    /* store new volume value */
    return g_codec.volume;
}

/**
 * @brief	Mute/un-mute the codec output
 * @param	enable	: When 1 mutes the codec, 0 - un-mutes the codec
 * @return	Nothing
 * @note	This mutes playback only. Mute control for capture is not supported.
 */
static INLINE void Codec_Mute(uint32_t enable)
{
    if (enable)
    {
        g_codec.tasks |= CODEC_TASK_MUTE;
    }
    else
    {
        g_codec.tasks |= CODEC_TASK_UNMUTE;
    }
}

/**
 * @brief	Set the volume level of the codec
 * @param	volume	: Volume level to be set
 * @return	Nothing
 * @note
 */
static INLINE void Codec_SetVolume(uint16_t volume)
{
    /* store new volume value */
    if (g_codec.volume != (int16_t)volume)
    {
        g_codec.volume = (int16_t)volume;
        /* schedule volume update */
        g_codec.tasks |= CODEC_TASK_VOLUME;
    }
}

/* Increase audio frequency */
void audio_trim_up(void);

/* Decrease audio frequency */
void audio_trim_down(void);

void audio_pll_setup(void);
void audio_pll_stop(void);
void audio_pll_start(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_CODEC_H__ */
