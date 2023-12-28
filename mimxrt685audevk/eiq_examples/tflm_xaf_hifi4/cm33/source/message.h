/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stddef.h>
#include <stdint.h>

#include "rpmsg_compiler.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define MESSAGE_PARAMS_MAX 4

/**
 * @brief Command sent over IPC
 */
typedef RL_PACKED_BEGIN struct _message
{
    uint8_t command;                         /*!< Command */
    uint8_t _pad[3];                         /*!< Padding */
    uint32_t params[MESSAGE_PARAMS_MAX * 2]; /*!< User defined msg params */
} RL_PACKED_END message_t;

#endif /* __MESSAGE_H__ */
