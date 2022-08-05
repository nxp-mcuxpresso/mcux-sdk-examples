/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RTP_H__
#define __RTP_H__

#include <stdint.h>
#include "rpmsg_compiler.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief RTP header structure.
 */
typedef RL_PACKED_BEGIN struct _rtp_header
{
    /*!
     * 2 bits for version
     * P bit - padding
     * X bit - extension
     * 4 bits for CC - CSRC count
     */
    uint8_t version;

    /*!
     * M bit - marker
     * 7 bits for PT - payload type
     */
    uint8_t payload_type;

    /*!
     * Sequence number
     */
    uint16_t sequence_number;

    /*!
     * Timestamp
     */
    uint32_t timestamp;

    /*!
     * Synchronization source identifier
     */
    uint32_t ssrc_id;
} RL_PACKED_END rtp_header_t;

/*!
 * @brief RTP header size.
 */
#define RTP_HEADER_SIZE 12 // sizeof(rtp_header_t)

/*!
 * @brief Maximum RTP payload size within UDP packet.
 */
#define RTP_PAYLOAD_SIZE (1500 - 20 - 8 - RTP_HEADER_SIZE) // MTU - IP header - UDP header - RTP header

/*!
 * @brief RTP packet size.
 */
#define RTP_PACKET_SIZE (RTP_HEADER_SIZE + RTP_PAYLOAD_SIZE)

/*!
 * @brief RTP payload type header field value for G.711 u-Law.
 */
#define RTP_PAYLOAD_TYPE_PCMU 0

/*!
 * @brief RTP payload type header field value for G.711 A-Law.
 */
#define RTP_PAYLOAD_TYPE_PCMA 8

/*!
 * @brief RTP G.711 A-Law/u-Law sampling rate.
 */
#define RTP_SAMPLE_RATE 8000

#endif /* __RTP_H__ */
