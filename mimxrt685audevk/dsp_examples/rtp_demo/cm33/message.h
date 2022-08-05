/*
 * Copyright 2019-2020 NXP
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
 * @brief Command sent over SPI
 */
typedef RL_PACKED_BEGIN struct _message
{
    uint8_t command;                         /*!< Command */
    uint8_t _pad[3];                         /*!< Padding */
    uint32_t params[MESSAGE_PARAMS_MAX * 2]; /*!< User defined msg params */
} RL_PACKED_END message_t;

/**
 * @brief Audio commands from remote side to CM33 and/or to DSP
 */
typedef enum _audio_command
{
    AudioCommand_PlayPacket, /*!< Send RTP packet with playback data */
} audio_command_t;

/**
 * @brief Audio events from DSP to CM33
 */
typedef enum _audio_event
{
    AudioEvent_PacketConsumed, /*!< RPT packet data consumed (played back/dropped) */
} audio_event_t;

/**
 * @brief Message parameters
 */
typedef enum _message_param
{
    MessageParam_NULL = 0,      /*!< Used as end of params marker */
    MessageParam_PacketAddress, /*!< Shared memory address of the transferred RTP packet */
    MessageParam_PacketSize,    /*!< Size of the transferred RTP packet */
} message_param_t;

#endif /* __MESSAGE_H__ */
