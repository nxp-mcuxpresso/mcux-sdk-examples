/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LE_AUDIO_SYNC_H
#define __LE_AUDIO_SYNC_H

#if defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)

#include <stdint.h>
#include "appl_ga_utils.h"
#include "appl_ga_ucs.h"
#include "appl_ga_ucc.h"

typedef enum {
    LE_AUDIO_IS_BIG_ESTABLISHED,
    LE_AUDIO_ISO_INTERVAL,
    LE_AUDIO_SDU_INTERVAL,
    LE_AUDIO_TRANSPORT_LATENCY,
    LE_AUDIO_BIG_CIG_DELAY,
    LE_AUDIO_PRESENTATION_DELAY,
    LE_AUDIO_TOTAL_SYNC_DELAY,
    LE_AUDIO_SYNC_OFFSET,
    LE_AUDIO_SYSTEM_DELAY,
    LE_AUDIO_SAMPLE_RATE,
    LE_AUDIO_SAMPLE_PER_FRAME,
    LE_AUDIO_BITS_PER_SAMPLE,
	LE_AUDIO_NO_OF_CHANNELS,
    LE_AUDIO_LOCATION,
    LE_AUDIO_FRAMED_UNFRAMED,
    LE_AUDIO_BC_BN,
    LE_AUDIO_BC_PTO,
    LE_AUDIO_BC_IRC,
    LE_AUDIO_BC_NSE,
    LE_AUDIO_INVALID_PROP
}le_audio_pl_config_value_t;

typedef struct broadcast_sink_data {
    UINT16 sync_handle;
    UINT8 pa_sync_established;
    UINT8 big_sync_established;
    UINT32 big_sync_delay;
    UINT32 presentation_delay;
    UINT32 transport_latency;
    UINT32 iso_interval;
    UINT32 sdu_interval;
    UINT8 NSE;
    UINT8 BN;
    UINT8 PTO;
    UINT8 IRC;
    UINT8 framing;
    UINT8 num_of_bis;
}broadcast_sink_data_t;

typedef struct unicast_sink_data {
    UINT8 cis_sync_established;
    UINT32 iso_interval;
    UINT32 cig_sync_delay;
    UINT32 cis_sync_delay;
} unicast_sink_data_t;

typedef struct src_sync_data_t {
    UINT8 big_created;
    UINT32 big_sync_delay;
    UINT32 iso_interval;
    UINT32 sdu_interval;
    UINT8 framing;
}src_sync_data_t;

typedef struct {
    UINT8 qos_config_state;
    GA_QOS_CONF qos_config;
}qos_config_data_t;

typedef struct _audio_data
{
    HCI_ISO_HEADER iso_hdr;
    UCHAR          *data;
    UINT16          data_len;
}audio_data_t;

#define LE_AUDIO_SDU_FLAG_VALID  0
#define LE_AUDIO_SDU_FLAG_ERROR  1U
#define LE_AUDIO_SDU_FLAG_LOST   2U

#define AUDIO_SINK_ROLE_LEFT   1
#define AUDIO_SINK_ROLE_RIGHT  2
#define AUDIO_SYNC_CIG_ESTABLISHED 2
#define AUDIO_SYNC_BIG_ESTABLISHED 1
#define AUDIO_SYNC_BIG_CREATE	  3


void le_audio_sync_init(void);
void le_audio_sync_start(UINT8 ep, UINT32 sink_cnter);
void le_audio_sync_set_config(UINT32 iso_interval_us, UINT32 sync_delay_us, INT32 sample_rate, INT32 samples_per_frame, UINT32 presentation_delay_us, UINT32 sync_index_init, UINT32 bps, UINT8 cc);
void le_audio_snk_i2s_enqueue(
								/* IN */  UCHAR* data,
								/* IN */  UINT16  datalen
							 );
void le_audio_sync_stop(void);
void le_audio_pl_set_config_values (le_audio_pl_config_value_t prop , uint32_t value);
uint32_t le_audio_pl_get_config_values (le_audio_pl_config_value_t prop);
UINT16 le_audio_asrc_process(UINT8 bfi, HCI_ISO_HEADER *iso_hdr, INT16* in, UINT16 length, INT16* out, UINT8 curr_ch);

#endif /*defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)*/
#endif /* __LE_AUDIO_SYNC_H */
