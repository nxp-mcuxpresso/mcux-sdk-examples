/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_lin.h"
#include "lin_cfg.h"

/* definition and initialization of signal array */
uint8_t g_lin_frame_data_buffer[LIN_FRAME_BUF_SIZE] = {'L', 'I', 'N', ' ', 'D', 'E', 'M', 'O', '*', '*',
                                                       '*', '*', '*', 'M', 'A', 'S', 'T', 'E', 'R'};

uint8_t g_lin_go_to_sleep_buffer[8] = {0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* definition and initialization of signal array */
uint8_t g_lin_flag_handle_tbl[LIN_FLAG_BUF_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/**********************************  Frame table **********************************/
static const lin_frame_struct lin_frame_tbl[LIN_NUM_OF_FRMS] = {

    {LIN_FRM_UNCD, 8U, LIN_RES_PUB, 0U, 0U, 1U, 15U, 0U}

    ,
    {LIN_FRM_UNCD, 5U, LIN_RES_SUB, 8U, 1U, 1U, 15U, 0U}

    ,
    {LIN_FRM_UNCD, 8U, LIN_RES_SUB, 0U, 2U, 1U, 15U, 0U}

    ,
    {LIN_FRM_UNCD, 6U, LIN_RES_PUB, 13U, 6U, 1U, 15U, 0U}

    ,
    {LIN_FRM_DIAG, 8U, LIN_RES_PUB, 0U, 0U, 0U, 5U, g_lin_go_to_sleep_buffer}

    ,
    {LIN_FRM_DIAG, 8, LIN_RES_SUB, 0U, 0U, 0U, 5U, 0U}

};

static uint8_t LI0_lin_configuration_RAM[LI0_LIN_SIZE_OF_CFG] = {0x00, 0x30, 0x33, 0x36, 0x2D, 0x3C, 0x3D, 0xFF};
const uint16_t LI0_lin_configuration_ROM[LI0_LIN_SIZE_OF_CFG] = {0x0000, 0x30, 0x33, 0x36, 0x2D, 0x3C, 0x3D, 0xFFFF};

/****************************LIN interface configuration ****************************/
const lin_protocol_user_config_t g_lin_protocol_user_cfg_array[LIN_NUM_OF_IFCS] = {

    /* Interface_name = LI0 */
    {
        .protocol_version = LIN_PROTOCOL_21,                        /*lin_protocol_version */
        .language_version = LIN_PROTOCOL_21,                        /*lin_language_version */

        .number_of_configurable_frames = 7,                         /*  num_of_frames */
        .frame_start                   = 0,                         /*  frame_start */
        .frame_tbl_ptr                 = lin_frame_tbl,             /*  frame_tbl */
        .list_identifiers_ROM_ptr      = LI0_lin_configuration_ROM, /*  *configuration_ROM */
        .list_identifiers_RAM_ptr      = LI0_lin_configuration_RAM, /*  *configuration_RAM */
        .max_idle_timeout_cnt          = 10000,                     /* Max Idle Timeout Count */
        .max_message_length            = 6                          /* Max message length */
    }};

lin_protocol_state_t g_lin_protocol_state_array[LIN_NUM_OF_IFCS] = {

    /* Interface_name = LI0 */
    {.successful_transfer         = 0U,
     .num_of_processed_frame      = 0U,
     .num_of_successfull_frame    = 0U,
     .diagnostic_mode             = DIAG_INTERLEAVE_MODE,
     .error_in_response           = 0U,
     .timeout_in_response         = 0U,
     .transmit_error_resp_sig_flg = 0U,
     .go_to_sleep_flg             = 0U,
     .next_transmit_tick          = 0U,
     .save_config_flg             = 0U,
     .event_trigger_collision_flg = 0U}};
