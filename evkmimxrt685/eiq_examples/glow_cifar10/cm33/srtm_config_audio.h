/*
 * Copyright 2019-2021.2023 NXP
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
    SRTM_Command_Decode,          /*!< Decode audio data stored in memory */
    SRTM_Command_Encode,          /*!< Encode PCM data stored in memory */
    SRTM_Command_MIX,             /*!< Mix of two channels */
    SRTM_Command_SRC,             /*!< Sampling rate converter */
    SRTM_Command_GAIN,            /*!< PCM Gain control */
    SRTM_Command_REC_DMIC,        /*!< Record from DMIC */
    SRTM_Command_REC_I2S,         /*!< Record from I2S */
    SRTM_Command_FileStart,       /*!< Start streaming decode from a remote file */
    SRTM_Command_FileData,        /*!< Data frame from remote file playback */
    SRTM_Command_FileEnd,         /*!< Streaming decode from remote file complete */
    SRTM_Command_FileStop,        /*!< Stop playback */
    SRTM_Command_FileError,       /*!< Stop playback due to file error*/
    SRTM_Command_AUDIO_MAX,       /*!< Request message */
    SRTM_Command_VIT,             /*!< VIT started */
    SRTM_Print_String,            /*!< Print out string */
    SRTM_Command_UsbSpeakerStart, /*!< Start playing from USB */
    SRTM_Command_UsbSpeakerData,  /*!< Data frame from remote USB speaker */
    SRTM_Command_UsbSpeakerEnd,   /*!< Playing from USB complete */
    SRTM_Command_UsbSpeakerStop,  /*!< Stop playing from USB */
    SRTM_Command_UsbSpeakerError, /*!< Stop playback due to USB error*/
    SRTM_Command_UsbMicStart,     /*!< Start usb recording */
    SRTM_Command_UsbMicData,      /*!< Data frame to remote USB microphone */
    SRTM_Command_UsbMicEnd,       /*!< Usb recording complete */
    SRTM_Command_UsbMicStop,      /*!< Stop recording from USB */
    SRTM_Command_UsbMicError,     /*!< Stop record due to USB error*/
} srtm_audio_command_t;

typedef enum _srtm_rt600_audio_decoder
{
    SRTM_Decoder_MP3 = 0,    /*!< MP3 decoder */
    SRTM_Decoder_AAC,        /*!< AAC+ decoder */
    SRTM_Decoder_OPUS,       /*!< OPUS decoder */
    SRTM_Decoder_SBC,        /*!< SBC decoder */
    SRTM_Decoder_VORBIS_OGG, /*!< OGG VORBIS decoder */
    SRTM_Decoder_VORBIS_RAW, /*!< RAW VORBIS decoder */
    SRTM_Decoder_MAX,        /*!< Maximum number of supported decoders */
} srtm_audio_decoder_t;

typedef enum _srtm_rt600_audio_encoder
{
    SRTM_Encoder_OPUS = 0, /*!< OPUS encoder */
    SRTM_Encoder_SBC,      /*!< SBC encoder */
    SRTM_Encoder_MAX,      /*!< Maximum number of supported encoders */
} srtm_audio_encoder_t;

#endif /* __SRTM_CONFIG_AUDIO_H__ */
