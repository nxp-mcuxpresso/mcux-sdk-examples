/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018,2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _TSI_CONFIG_H_
#define _TSI_CONFIG_H_

#include "fsl_common.h"

/* Max debounce for touch, determine the touch filter time/response time */
#define MAX_TOUCH_DEBOUNCE 2U
/* Max debounce for baseline, determine the baseline tracking time */
#define MAX_BASELINE_DEBOUNCE 100U
/* Base line update frequency (1-255). The base line will be updated every BASELINE_UPDATE_FREQ times round key scan.*/
#define BASELINE_UPDATE_FREQ 128U
/* Number of TSI key defined by user in TSI_config.c */
#define MAX_KEY_NUM (sizeof(key_TSI) / sizeof(key_tsi_mapping_t) - 1U)
/* Number of TSI slider defined by user in TSI_config.c */
#define MAX_SLIDER_NUM (sizeof(slider_TSI) / sizeof(slider_tsi_mapping_t) - 1U)

/* TSI channel ID definition */
enum _tsi_channelId
{
    kTSI_Chnl_0 = 0U, /* Responding to TSI channel 0 */
    kTSI_Chnl_1,
    kTSI_Chnl_2,
    kTSI_Chnl_3,
    kTSI_Chnl_4,
    kTSI_Chnl_5,
    kTSI_Chnl_6,
    kTSI_Chnl_7,
    kTSI_Chnl_8,
    kTSI_Chnl_9,
    kTSI_Chnl_10,
    kTSI_Chnl_11,
    kTSI_Chnl_12,
    kTSI_Chnl_13,
    kTSI_Chnl_14,
    kTSI_Chnl_15,
    kTSI_Chnl_16,
    kTSI_Chnl_17,
    kTSI_Chnl_18,
    kTSI_Chnl_19,
    kTSI_Chnl_20,
    kTSI_Chnl_21,
    kTSI_Chnl_22,
    kTSI_Chnl_23,
    kTSI_Chnl_24,
    kTSI_Chnl_MAX_ID
};

/* Key event indicates momentary actions */
typedef enum _key_event
{
    kKey_Event_Idle = 0U, /* Key is not touched */
    kKey_Event_Touch,     /* Key is touched for the first time */
    kKey_Event_Touched,   /* Key has been touched */
    kKey_Event_Stick,     /* Key has been touched for a long time */
    kKey_Event_Release,   /* Key is released for the first time */
    kKey_Event_Released,  /* Key has been released */
    kKey_Event_Abnormal,  /* Key is in abnormal status */
} key_event_t;

/* Key state indicates long term status changes */
typedef enum _key_state
{
    kKey_State_Idle = 0U, /* TSI is not touched */
    kKey_State_Touch,     /* TSI is in touched status */
    kKey_State_Touched,   /* TSI has been touched for a long time */
    kKey_State_Released,  /* TSI is released */
    kKey_State_Abnormal   /* TSI is in abnormal status */
} key_state_t;

/* Slider event indicates momentary actions */
typedef enum _slider_event
{
    kSlider_Event_Idle = 0U, /* Slider no action */
    kSlider_Event_Touch,
    kSlider_Event_Touched,
    kSlider_Event_Inc,
    kSlider_Event_Dec,
    kSlider_Event_Release,
    kSlider_Event_Released,
    kSlider_Event_Filter
} slider_event_t;

typedef struct _key_tsi_mapping
{
    TSI_Type *TSI_base;
    union
    {
        uint8_t TSI_channel;    /* TSI channel for self mode */
        uint8_t TSI_channel_tx; /* TSI transmit channel for mutual mode */
    } TSI_CH;
    uint8_t TSI_channel_rx;     /* TSI receive channel for mutual mode */
    float key_touch_delta;      /* Delta percentage for touch sensing threshold */
    uint8_t key_state;          /* Key state machine */
    uint8_t key_state_debounce; /* Debounce counter to confirm touch event */
    uint16_t key_baseline;      /* Baseline tracking record */
    uint16_t key_value_current; /* Instant TSI sample value */
    uint32_t key_value_sum;     /* Touch sample value accumulate */
} key_tsi_mapping_t;

typedef struct _slider_tsi_mapping
{
    uint8_t TSI_channel_0;
    uint8_t TSI_channel_1;
    uint16_t TSI_baseline_0;
    uint16_t TSI_baseline_1;
    uint8_t slider_baseline_delta;
    uint16_t slider_touch_delta;
} slider_tsi_mapping_t;

extern key_tsi_mapping_t key_TSI[3];
#endif /* _TSI_CONFIG_H_ */
