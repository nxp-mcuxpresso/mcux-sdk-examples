/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_LIN_CFG_H_
#define SOURCE_LIN_CFG_H_

#define LIN_NUM_OF_SLAVE_IFCS  0U
#define LIN_NUM_OF_MASTER_IFCS 1U

/* Size of configuration in ROM and RAM used for interface: LI0 */
#define LI0_LIN_SIZE_OF_CFG 8U

/* Number of frames */
#define LIN_NUM_OF_FRMS 6U

/* Number of interfaces */
#define LIN_NUM_OF_IFCS 1U

/* Maximal timeout to settle in idle state */
#define MAX_IDLE_TIMEOUT_CNT 10000

/* frame buffer size */
#define LIN_FRAME_BUF_SIZE 19U
#define LIN_FLAG_BUF_SIZE  7U

/*! @brief Table to save LIN protocol state structure pointers */
extern const lin_protocol_user_config_t g_lin_protocol_user_cfg_array[LIN_NUM_OF_IFCS];

/*! @brief Table to save LIN protocol state structure pointers */
extern lin_protocol_state_t g_lin_protocol_state_array[LIN_NUM_OF_IFCS];

/*! @brief Frame signal array */
extern uint8_t g_lin_frame_data_buffer[LIN_FRAME_BUF_SIZE];

/*! @brief Table of interface flag handles */
extern uint8_t g_lin_flag_handle_tbl[LIN_FLAG_BUF_SIZE];

#endif /* SOURCE_LIN_CFG_H_ */
