/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_CONFIG_AUDIO_H__
#define __SRTM_CONFIG_AUDIO_H__

/**
 * @brief SRTM audio command fields
 */
typedef enum _srtm_audio_component
{
    DSP_COMPONENT_MP3,
    DSP_COMPONENT_AAC,
    DSP_COMPONENT_VORBIS,
    DSP_COMPONENT_OPUS
} srtm_audio_component_t;

typedef enum _srtm_rt600_audio_command
{
    SRTM_Command_MP3,       /*!< MP3 decoder */
    SRTM_Command_AAC,       /*!< AAC+ decoder */
    SRTM_Command_VORBIS,    /*!< VORBIS decoder */
    SRTM_Command_OPUS_DEC,  /*!< OPUS decoder */
    SRTM_Command_OPUS_ENC,  /*!< OPUS encoder */
    SRTM_Command_SBC_DEC,   /*!< SBC decoder */
    SRTM_Command_SBC_ENC,   /*!< SBC encoder */
    SRTM_Command_MIX,       /*!< Mix of two channels */
    SRTM_Command_SRC,       /*!< Sampling rate converter */
    SRTM_Command_GAIN,      /*!< PCM Gain control */
    SRTM_Command_REC_DMIC,  /*!< Record from DMIC */
    SRTM_Command_REC_I2S,   /*!< Record from I2S */
    SRTM_Command_FileStart, /*!< Start streaming decode from a remote file */
    SRTM_Command_FileData,  /*!< Data frame from remote file playback */
    SRTM_Command_FileEnd,   /*!< Streaming decode from remote file complete */
    SRTM_Command_FileStop,  /*!< Stop playback */
    SRTM_Command_FileError, /*!< Stop playback due to file error*/
    SRTM_Command_AUDIO_MAX, /*!< Request message */
    SRTM_Command_VIT,       /*!< VIT started */
    SRTM_Print_String,      /*!< Print out string */
} srtm_audio_command_t;

#endif /* __SRTM_CONFIG_AUDIO_H__ */
