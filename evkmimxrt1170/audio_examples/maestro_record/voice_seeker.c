/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef VOICE_SEEKER_PROC

#include "app_definitions.h"

#include "fsl_debug_console.h"
#include "fsl_os_abstraction.h"

#include "streamer.h"

#include "voice_seeker.h"
#include "libVoiceSeekerLight.h"
#include "RdspCycleCounter.h"
#include "RdspDeviceConfig.h"

#if defined(PLATFORM_RT1060)
#define DEVICE_ID    Device_IMXRT1060_CM7
#define RDSP_NUM_MIC 2

#elif defined(PLATFORM_RT1160)
#define DEVICE_ID    Device_IMXRT1160_CM7
#define RDSP_NUM_MIC 2

#elif defined(PLATFORM_RT1170)
#define DEVICE_ID    Device_IMXRT1170_CM7
#define RDSP_NUM_MIC 2

#else
#error "No platform selected"

#endif

RETUNE_VOICESEEKERLIGHT_plugin_t vsl             = {0};
static rdsp_voiceseekerlight_config_t vsl_config = {0};
static uint32_t num_mics                         = 0;
static uint32_t framesize_in                     = 0;
static uint32_t framesize_out                    = 0;
static uint32_t samplerate                       = 0;

int VoiceSeeker_Initialize(void *arg)
{
    int status = 0;
    /*
     * Query VoiceSeeker library for the supported configuration
     */

    rdsp_voiceseekerlight_constants_t vsl_constants;
    VoiceSeekerLight_GetConstants(&vsl_constants);

    /*
     * VoiceSeekerLight plugin configuration
     */

    vsl_config.num_mics             = RDSP_NUM_MIC;
    vsl_config.num_spks             = RDSP_NUM_SPK;
    vsl_config.framesize_out        = FRAME_SIZE;
    vsl_config.create_aec           = RDSP_ENABLE_AEC;
    vsl_config.create_doa           = RDSP_ENABLE_DOA;
    vsl_config.buffer_length_sec    = RDSP_BUFFER_LENGTH_SEC;
    vsl_config.aec_filter_length_ms = RDSP_AEC_FILTER_LENGTH_MS;
    vsl_config.device_id            = DEVICE_ID;

    // Specify mic geometry
    vsl_config.mic_xyz_mm = (rdsp_xyz_t *)OSA_MemoryAllocate(sizeof(rdsp_xyz_t) * vsl_config.num_mics);
    if (vsl_config.mic_xyz_mm == NULL)
    {
        return MALLOC_FAIL;
    }

#if defined(PLATFORM_RT1060)
    /* AUD-EXP-42448 board rev B */
    // MIC0
    if (vsl_config.num_mics > 0)
    {
        vsl_config.mic_xyz_mm[0][0] = 0.0f; // X
        vsl_config.mic_xyz_mm[0][1] = 0.0f; // Y
        vsl_config.mic_xyz_mm[0][2] = 0.0f; // Z
    }
    // MIC1
    if (vsl_config.num_mics > 1)
    {
        vsl_config.mic_xyz_mm[1][0] = 54.0f; // X
        vsl_config.mic_xyz_mm[1][1] = 38.0f; // Y
        vsl_config.mic_xyz_mm[1][2] = 0.0f;  // Z
    }
#elif defined(PLATFORM_RT1160)
    // MIC0
    if (vsl_config.num_mics > 0)
    {
        vsl_config.mic_xyz_mm[0][0] = 0.0f; // X
        vsl_config.mic_xyz_mm[0][1] = 0.0f; // Y
        vsl_config.mic_xyz_mm[0][2] = 0.0f; // Z
    }
    // MIC1
    if (vsl_config.num_mics > 1)
    {
        vsl_config.mic_xyz_mm[1][0] = 71.0f; // X
        vsl_config.mic_xyz_mm[1][1] = 0.0f;  // Y
        vsl_config.mic_xyz_mm[1][2] = 0.0f;  // Z
    }
#elif defined(PLATFORM_RT1170)
    // MIC0
    if (vsl_config.num_mics > 0)
    {
        vsl_config.mic_xyz_mm[0][0] = 0.0f; // X
        vsl_config.mic_xyz_mm[0][1] = 0.0f; // Y
        vsl_config.mic_xyz_mm[0][2] = 0.0f; // Z
    }
    // MIC1
    if (vsl_config.num_mics > 1)
    {
        vsl_config.mic_xyz_mm[1][0] = 8.0f; // X
        vsl_config.mic_xyz_mm[1][1] = 0.0f; // Y
        vsl_config.mic_xyz_mm[1][2] = 0.0f; // Z
    }
#endif

    /*
     * Query how much heap memory is required for the configuration
     */

    uint32_t heap_req_bytes = VoiceSeekerLight_GetRequiredHeapMemoryBytes(&vsl, &vsl_config);
    PRINTF("VoiceSeekerLight_GetRequiredHeapMemoryBytes: %i bytes\r\n", heap_req_bytes);

    // Allocate needed memory
    void *heap_memory           = OSA_MemoryAllocate(heap_req_bytes);
    vsl.mem.pPrivateDataBase    = heap_memory;
    vsl.mem.pPrivateDataNext    = heap_memory;
    vsl.mem.FreePrivateDataSize = heap_req_bytes;

    /*
     * VoiceSeekerLight creation
     */

    RdspStatus voiceseeker_status = VoiceSeekerLight_Create(&vsl, &vsl_config);
    PRINTF("VoiceSeekerLight_Create: voiceseeker_status = %d\r\n", voiceseeker_status);
    if (voiceseeker_status != OK)
    {
        return voiceseeker_status;
    }

    /*
     * VoiceSeekerLight initialization
     */

    VoiceSeekerLight_Init(&vsl);

    /*
     * Retrieve VoiceSeekerLight version and configuration
     */

    rdsp_voiceseekerlight_ver_struct_t vsl_version;
    VoiceSeekerLight_GetLibVersion(&vsl, &vsl_version.major, &vsl_version.minor, &vsl_version.patch);
    PRINTF("VoiceSeekerLight_GetLibVersion: v%i.%i.%i\n", vsl_version.major, vsl_version.minor, vsl_version.patch);

    // Unpack configuration
    samplerate    = vsl_constants.samplerate;
    num_mics      = vsl_config.num_mics;
    framesize_in  = vsl_constants.framesize_in;
    framesize_out = vsl_config.framesize_out;

    if ((framesize_out % framesize_in) != 0)
        PRINTF("VoiceSeeker in = %d not a modulo of VoiceSeeker out = %d", framesize_in, framesize_out);

    VoiceSeekerLight_PrintConfig(&vsl);
    VoiceSeekerLight_PrintMemOverview(&vsl);

    return status;
}

int VoiceSeeker_Execute(void *arg, void *inputOutputBuffer, int bufferSize)
{
    StreamBuffer *buf              = (StreamBuffer *)inputOutputBuffer;
    int8_t *pkt_hdr_size           = arg;
    AudioPacketHeader *data_packet = NULL;
    int32_t *buffer                = NULL;
    RdspStatus voiceseeker_status  = 0;
    int VoiceSeekerIteration       = framesize_out / framesize_in;
    int16_t *mic_in_int16[num_mics];
    float *mic_in_float[num_mics];
    float mic_in_float_buffer[num_mics][framesize_out];
    const uint32_t framerate_in = samplerate / framesize_in;
    float *vsl_out              = NULL;
    int16_t DeInterleavedBuffer[framesize_out * DEMO_CHANNEL_NUM];
    int16_t *outBuffer;

    if ((buf == NULL) || (pkt_hdr_size == NULL))
    {
        return INVALID_PARAMETERS;
    }

    if (*pkt_hdr_size != sizeof(AudioPacketHeader))
    {
        /* Others headers are not allowed */
        return INVALID_PARAMETERS;
    }

    /* Initialization of the variables */
    data_packet = (AudioPacketHeader *)buf->buffer;
    buffer      = (int32_t *)(buf->buffer + *pkt_hdr_size);
    outBuffer   = (int16_t *)(buf->buffer + *pkt_hdr_size);

    if (bufferSize < (framesize_out * num_mics * sizeof(float)))
        return GENERAL_ERROR;

    DeInterleave((void *)buffer, DeInterleavedBuffer, framesize_out, DEMO_CHANNEL_NUM);

    // Initialize multichannel input pointers
#if DEMO_CODEC_CS42448
    mic_in_int16[0] = &DeInterleavedBuffer[framesize_out * 4];
    mic_in_int16[1] = &DeInterleavedBuffer[framesize_out * 5];
#else
    for (uint16_t imic = 0; imic < num_mics; imic++)
    {
        mic_in_int16[imic] = &DeInterleavedBuffer[imic * framesize_out];
    }
#endif
    // Convert fixed point input to floating point
    for (uint16_t ich = 0; ich < num_mics; ich++)
    {
        for (uint16_t is = 0; is < framesize_out; is++)
        {
            mic_in_float_buffer[ich][is] = ((float)mic_in_int16[ich][is]) * INT_TO_FLOAT;
        }
        mic_in_float[ich] = mic_in_float_buffer[ich];
    }

    while (VoiceSeekerIteration)
    {
        /*
         * VoiceSeeker Light process
         */
        voiceseeker_status = VoiceSeekerLight_Process(&vsl, mic_in_float, NULL, &vsl_out);
        if (voiceseeker_status != OK)
        {
            PRINTF("VoiceSeekerLight_Process: voiceseeker_status = %d\r\n", (int32_t)voiceseeker_status);
            return GENERAL_ERROR;
        }
        // Update multichannel input pointers
        // input pointers incremented by Voiceseeker framesize_in (i.e +32 float*)
        for (uint32_t imic = 0; imic < num_mics; imic++)
        {
            mic_in_float[imic] += framesize_in; // increase address by VoiceSeeker input size i.e 32 samples
        }
        VoiceSeekerIteration--;
    }
    float temp = 0;
    if (vsl_out != NULL)
    {
        for (uint16_t is = 0; is < framesize_out; is++)
        {
            temp          = (float)vsl_out[is] * FLOAT_TO_INT;
            outBuffer[is] = (int16_t)temp;
        }
        data_packet->chunk_size = framesize_out * sizeof(int16_t);
        buf->size               = data_packet->chunk_size + *pkt_hdr_size;
    }
    else
    {
        PRINTF("Wrong config between VoiceSeeker input = %zd and out = %d samples.", framesize_in, framesize_out);
        return GENERAL_ERROR;
    }

    return voiceseeker_status;
}

int VoiceSeeker_Deinit(void)
{
    int status = 0;
    VoiceSeekerLight_Destroy(&vsl);
    OSA_MemoryFree(vsl.mem.pPrivateDataBase);
    OSA_MemoryFree(vsl_config.mic_xyz_mm);
    return status;
}

//  de-Interleave Multichannel signal
//   example:  A1.B1.C1.A2.B2.C2.A3.B3.C3....An.Bn.Cn   (3 Channels case : A, B, C)
//             will become
//             A1.A2.A3....An.B1.B2.B3....Bn.C1.C2.C3....Cn

// Simple helper function for de-interleaving Multichannel stream
// The caller function shall ensure that all arguments are correct.
// This function assumes the input data as 32 bit width and transforms it into 16 bit width
void DeInterleave(const int16_t *pDataInput, int16_t *pDataOutput, uint16_t FrameSize, uint16_t ChannelNumber)
{
    for (uint16_t ichan = 0; ichan < ChannelNumber; ichan++)
    {
        for (uint16_t i = 0; i < FrameSize; i++)
        {
            /* Select the 16 MSB of the 32 input bits */
            pDataOutput[i + (ichan * FrameSize)] = pDataInput[(i * 2 * ChannelNumber) + (ichan * 2) + 1];
        }
    }
    return;
}

VoiceSeeker_Initialize_T VoiceSeeker_Initialize_func = VoiceSeeker_Initialize;
VoiceSeeker_Execute_T VoiceSeeker_Execute_func       = VoiceSeeker_Execute;
VoiceSeeker_Deinit_T VoiceSeeker_Deinit_func         = VoiceSeeker_Deinit;

#endif
