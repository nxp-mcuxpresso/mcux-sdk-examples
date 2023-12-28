/*
 * Copyright 2023 NXP
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
#elif defined(PLATFORM_RT1170_EVKB)
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

    if (vsl_constants.max_num_mics < RDSP_NUM_MIC)
    {
        PRINTF("Error VoiceSeeker is supporting only %d mics", vsl_constants.max_num_mics);
        return GENERAL_ERROR;
    }

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

#if (defined(PLATFORM_RT1060))
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
#elif defined(PLATFORM_RT1170_EVKB)
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
        vsl_config.mic_xyz_mm[1][0] = 0.0f;  // X
        vsl_config.mic_xyz_mm[1][1] = 70.0f; // Y
        vsl_config.mic_xyz_mm[1][2] = 0.0f;  // Z
    }
#endif

    /*
     * Query how much heap memory is required for the configuration
     */
    uint32_t heap_req_bytes = VoiceSeekerLight_GetRequiredHeapMemoryBytes(&vsl, &vsl_config);
    PRINTF("VoiceSeekerLight_GetRequiredHeapMemoryBytes: %i bytes\r\n", heap_req_bytes);

    // Allocate needed memory
    void *heap_memory = OSA_MemoryAllocate(heap_req_bytes);
    if (heap_memory == NULL)
    {
        OSA_MemoryFree(vsl_config.mic_xyz_mm);
        vsl_config.mic_xyz_mm = NULL;
        return MALLOC_FAIL;
    }
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
        OSA_MemoryFree(heap_memory);
        heap_memory = NULL;
        OSA_MemoryFree(vsl_config.mic_xyz_mm);
        vsl_config.mic_xyz_mm = NULL;
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
    VoiceSeekerLight_GetLibVersion(&vsl, &vsl_version);
    PRINTF("VoiceSeekerLight_GetLibVersion: v%i.%i.%i\n", vsl_version.major, vsl_version.minor, vsl_version.patch);

    // Unpack configuration
    framesize_in  = vsl_constants.framesize_in;
    samplerate    = vsl_constants.samplerate;
    framesize_out = vsl_config.framesize_out;
    num_mics      = vsl_config.num_mics;

    /*
     * Map mic_in pointers to mic buffer
     */
    // Check right config of VoiceSeeker in and out buffers
    if ((framesize_out % framesize_in) != 0)
        PRINTF("VoiceSeeker in = %d not a modulo of VoiceSeeker out = %d", framesize_in, framesize_out);

    VoiceSeekerLight_PrintConfig(&vsl);
    VoiceSeekerLight_PrintMemOverview(&vsl);

    return status;
}

int VoiceSeeker_Execute(void *arg, void *inputOutputBuffer, int bufferSize)
{
    RdspStatus voiceseeker_status                   = 0;
    StreamBuffer *buf                               = (StreamBuffer *)inputOutputBuffer;
    int32_t *buffer                                 = NULL;
    uint32_t buffer_size                            = 0;
    AudioPacketHeader *data_packet                  = NULL;
    int8_t *pkt_hdr_size                            = (int8_t *)arg;
    int32_t TempData_Mic[FRAME_SIZE * RDSP_NUM_MIC] = {0};
    float *InputTempDataFloat_Mic                   = NULL;
    int16_t *InputTempDataINT16_Mic                 = NULL;
    int16_t *outBuffer                              = NULL;
    float VoiceSeekerIteration                      = (float)framesize_out / (float)framesize_in;
    float *mic_in[num_mics];
    float *vsl_out = NULL;
#if DEMO_CODEC_CS42448
    int16_t DeInterleavedBuffer[framesize_out * DEMO_CHANNEL_NUM];
#endif
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
    buffer_size = buf->size - *pkt_hdr_size;
    outBuffer   = (int16_t *)(buf->buffer + *pkt_hdr_size);

    if (buffer_size <= 0)
    {
        data_packet->chunk_size      = framesize_out * sizeof(int16_t);
        data_packet->num_channels    = 1;
        data_packet->bits_per_sample = 16;
        return OK;
    }

    if (data_packet->bits_per_sample == 16)
    {
        DeInterleave16((int16_t *)buffer, (int16_t *)TempData_Mic, framesize_out, num_mics);
    }
    else
    {
#if DEMO_CODEC_CS42448
        DeInterleave32((int16_t *)buffer, DeInterleavedBuffer, framesize_out, DEMO_CHANNEL_NUM);
        InputTempDataFloat_Mic = (float *)(&DeInterleavedBuffer[4 * framesize_out]);
        InputTempDataINT16_Mic = (int16_t *)(&DeInterleavedBuffer[4 * framesize_out]);
    }
#else
        DeInterleave32((int16_t *)buffer, (int16_t *)TempData_Mic, framesize_out, num_mics);
    }

    InputTempDataFloat_Mic = (float *)TempData_Mic;
    InputTempDataINT16_Mic = (int16_t *)TempData_Mic;
#endif

    // Convert  INT16 Q15 data to Float data
    for (uint16_t i = framesize_out * num_mics; i > 0; i--)
    {
        InputTempDataFloat_Mic[i - 1] = ((float)InputTempDataINT16_Mic[i - 1]) * INV_32768;
    }

    // Initialize multichannel input pointers
    for (uint32_t imic = 0; imic < num_mics; imic++)
    {
        mic_in[imic] = &(InputTempDataFloat_Mic[imic * framesize_out]);
    }

    while (VoiceSeekerIteration)
    {
        /*
         * VoiceSeeker Light process
         */
        voiceseeker_status = VoiceSeekerLight_Process(&vsl, mic_in, NULL, &vsl_out);
        if (voiceseeker_status != OK)
        {
            if (voiceseeker_status == RDSP_VOICESEEKER_LICENSE_EXPIRED)
            {
                PRINTF("VoiceSeekerLight_Process: voiceseeker_status = Licence expired\r\n");
            }
            else
            {
                PRINTF("VoiceSeekerLight_Process: voiceseeker_status = %d\r\n", (int)voiceseeker_status);
            }
            return GENERAL_ERROR;
        }
        // Update multichannel input pointers
        // input pointers incremented by Voiceseeker framesize_in (i.e +32 float*)
        for (uint32_t imic = 0; imic < num_mics; imic++)
        {
            mic_in[imic] += framesize_in; // increase address by VoiceSeeker input size i.e 32 samples
        }

        VoiceSeekerIteration--;
    }

    if (vsl_out != NULL)
    {
        for (uint16_t i = 0; i < framesize_out; i++)
        {
            outBuffer[i] = (int16_t)(vsl_out[i] * FLOAT_TO_INT);
        }

        data_packet->chunk_size      = framesize_out * sizeof(int16_t);
        data_packet->num_channels    = 1;
        data_packet->bits_per_sample = 16;
        buf->size                    = data_packet->chunk_size + *pkt_hdr_size;
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
    int32_t fd = -1;
    int status = 0;

    VoiceSeekerLight_Destroy(&vsl);
    if (vsl.mem.pPrivateDataBase != NULL)
    {
        OSA_MemoryFree(vsl.mem.pPrivateDataBase);
        vsl.mem.pPrivateDataBase = NULL;
    }
    if (vsl_config.mic_xyz_mm != NULL)
    {
        OSA_MemoryFree(vsl_config.mic_xyz_mm);
        vsl_config.mic_xyz_mm = NULL;
    }

    return status;
}

//  de-Interleave Multichannel signal
//   example:  A1.B1.C1.A2.B2.C2.A3.B3.C3....An.Bn.Cn   (3 Channels case : A, B, C)
//             will become
//             A1.A2.A3....An.B1.B2.B3....Bn.C1.C2.C3....Cn

// Simple helper function for de-interleaving Multichannel stream
// The caller function shall ensure that all arguments are correct.
// This function assumes the input data as 32 bit width and transforms it into 16 bit width
void DeInterleave32(const int16_t *pDataInput, int16_t *pDataOutput, uint16_t FrameSize, uint16_t ChannelNumber)
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

void DeInterleave16(const int16_t *pDataInput, int16_t *pDataOutput, uint16_t FrameSize, uint16_t ChannelNumber)
{
    for (uint16_t ichan = 0; ichan < ChannelNumber; ichan++)
    {
        for (uint16_t i = 0; i < FrameSize; i++)
        {
            pDataOutput[i + (ichan * FrameSize)] = pDataInput[(i * ChannelNumber) + ichan];
        }
    }
    return;
}

VoiceSeeker_Initialize_T VoiceSeeker_Initialize_func = VoiceSeeker_Initialize;
VoiceSeeker_Execute_T VoiceSeeker_Execute_func       = VoiceSeeker_Execute;
VoiceSeeker_Deinit_T VoiceSeeker_Deinit_func         = VoiceSeeker_Deinit;

#endif
