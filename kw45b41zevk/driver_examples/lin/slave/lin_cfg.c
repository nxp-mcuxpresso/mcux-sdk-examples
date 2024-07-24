/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_lin.h"
#include "lin_cfg.h"

/* definition and initialization of signal array */
uint8_t g_lin_frame_data_buffer[LIN_FRAME_BUF_SIZE] = {'*', '*', '*', '*', '*', '*', '*', '*', 'S', 'L',
                                                       'A', 'V', 'E', '*', '*', '*', '*', '*', '*'};

/* definition and initialization of signal array flags */
uint8_t g_lin_flag_handle_tbl[LIN_FLAG_BUF_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/**********************************  Frame table **********************************/
static const lin_frame_struct lin_frame_tbl[LIN_NUM_OF_FRMS] = {

    {LIN_FRM_UNCD, 8, LIN_RES_SUB, 0, 0, 1, 10, 0}

    ,
    {LIN_FRM_UNCD, 5, LIN_RES_PUB, 8, 1, 1, 10, 0}

    ,
    {LIN_FRM_UNCD, 8, LIN_RES_PUB, 0, 2, 1, 10, 0}

    ,
    {LIN_FRM_UNCD, 6, LIN_RES_SUB, 13, 6, 1, 10, 0}

    ,
    {LIN_FRM_DIAG, 8, LIN_RES_SUB, 0, 0, 0, 2, 0}

    ,
    {LIN_FRM_DIAG, 8, LIN_RES_PUB, 0, 0, 0, 2, 0}

};
/* volatile memory of frame IDs */
static uint8_t LI0_lin_configuration_RAM[LI0_LIN_SIZE_OF_CFG] = {0x00, 0x30, 0x33, 0x36, 0x2D, 0x3C, 0x3D, 0xFF};
/* non-volatile memory of frame IDs - do not change while runtime */
static const uint16_t LI0_lin_configuration_ROM[LI0_LIN_SIZE_OF_CFG] = {0x0000, 0x30, 0x33, 0x36,
                                                                        0x2D,   0x3C, 0x3D, 0xFFFF};
/* node address */
static uint8_t LI0_lin_configured_NAD = 0x02;
/* response signal ID */
static uint8_t LI0_lin_frm_err_resp_sig[1] = {0x31};

/**************** Node attributes Initialization  ****************************/
const lin_node_attribute g_lin_node_attribute_array[LIN_NUM_OF_SLAVE_IFCS] = {
    /** LI0 **/
    {
        .configured_NAD_ptr         = &LI0_lin_configured_NAD,  /*configured_NAD*/
        .initial_NAD                = 0x0A,                     /*initial_NAD*/
        .serial_number              = {0x49, 0x02, 0x00, 0x00}, /* Serial number{<D1>,<D2>,<D3>,<D4>} */
        .product_id                 = {0x001E, 0x0001, 0x00},   /*{<supplier_id>,<function_id>,<variant>}*/
        .resp_err_frm_id_ptr        = LI0_lin_frm_err_resp_sig, /* list index of frame error*/
        .num_frame_have_esignal     = 1,                        /* number of frame contain error signal*/
        .response_error_byte_offset = 0,                        /* Byte offset of response error signal */
        .response_error_bit_offset  = 0,                        /* Bit offset of response error signal */
        .num_of_fault_state_signal  = 0,                        /* Number of Fault state signal */
        .P2_min                     = 0,                        /* P2 min */
        .ST_min                     = 0,                        /* ST min */
        .N_As_timeout               = 1000,                     /* N_As_timeout */
        .N_Cr_timeout               = 1000,                     /* N_Cr_timeout */
    }};

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
