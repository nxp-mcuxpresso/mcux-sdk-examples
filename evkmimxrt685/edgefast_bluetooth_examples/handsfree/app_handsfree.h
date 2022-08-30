/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APPL_PERIPHERAL_HFP_HF_MAIN_H__
#define __APPL_PERIPHERAL_HFP_HF_MAIN_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

void peripheral_hfp_hf_task(void *param);
void hfp_AnswerCall(void);
void hfp_RejectCall(void);
void hfp_dial(const char *number);
void dial_memory(int location);
void hfp_last_dial(void);
void hfp_start_voice_recognition(void);
void hfp_hf_get_last_voice_tag_number(void);
void hfp_stop_voice_recognition(void);
void hfp_volume_update(hf_volume_type_t type, int volume);
void hfp_enable_ccwa( uint8_t enable);
void hfp_enable_clip( uint8_t enable);
void hfp_multiparty_call_option(uint8_t option);

#endif /* __APPL_PERIPHERAL_HFP_HF_MAIN_H__ */
