/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "TSI_config.h"

/*
 * Key and TSI channel mapping table definition
 */
key_tsi_mapping_t key_TSI[] = {
    /* TSI_CH;channel_rx;touch_delta;key_state;state_debounce;baseline;value_current;value_sum; */
    /* User only need to config 3 parameters:
      .TSI_CH.TSI_channel, .TSI_channel_rx, .key_touch_delta */
    /* Note: .TSI_channel_rx = 0xFF, indicates it's selfmode */

    /* config key in self mode */
    {TSI0, {kTSI_Chnl_19}, 0xFFU, 0.05F, 0U, 0U, 0U, 0U, 0U}, /* KEY 0 */
    {TSI1, {kTSI_Chnl_3}, 0xFFU, 0.05F, 0U, 0U, 0U, 0U, 0U},  /* KEY 1 */

    /* Do NOT move {0xFFU, 0xFFU, 0xFFU, 0xFFU} which indicates the end of the array */
    {TSI0, {0xFFU}, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU} /* the end of the array */
};

/* If slider not needed, please fill 0xFF */
slider_tsi_mapping_t slider_TSI[] = {
    /* tsi0, tsi1, tsi0_baseline, tsi1_baseline, baseline_delta, touch_delta */
    /* {TSI_9, TSI_10, 0, 0, 5, 100},		 */
    /* {TSI_9, TSI_10, 0, 0, 5, 100},	 */

    /* do NOT move {0xFF, 0xFF, 0xFF, 0xFF} which indicates the end of the array */
    {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU} /* the end of the array */
};
