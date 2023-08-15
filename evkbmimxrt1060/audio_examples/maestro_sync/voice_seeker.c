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
#include "streamer_element_properties.h"

#include "file_utils.h"

#include "voice_seeker.h"
#include "libVoiceSeekerLight.h"
#include "RdspCycleCounter.h"
#include "RdspDeviceConfig.h"
#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
#include "fsl_cache.h"
#endif

#if defined(PLATFORM_RT1060)
#define DEVICE_ID    Device_IMXRT1060_CM7
#define RDSP_NUM_MIC 1
#else
#error "No platform selected"

#endif

RETUNE_VOICESEEKERLIGHT_plugin_t vsl             = {0};
static rdsp_voiceseekerlight_config_t vsl_config = {0};
static uint32_t num_mics                         = 0;
static uint32_t num_spks                         = 0;
static uint32_t framesize_in                     = 0;
static uint32_t framesize_out                    = 0;
static uint32_t samplerate                       = 0;

static bool debugging = false;
typedef struct _VoiceSeekerDataDump
{
    int8_t *mic_data_ptr;       /*!< @brief Microphone data pointer */
    int8_t *spk_data_ptr;       /*!< @brief Speaker data pointer */
    uint32_t mic_data_ptr_size; /*!< @brief Microphone data buffer size */
    uint32_t spk_data_ptr_size; /*!< @brief Speaker data buffer size */
    uint32_t mic_size;          /*!< @brief Size of stored bytes */
    uint32_t spk_size;          /*!< @brief Size of stored bytes */
} VoiceSeekerDataDump;
static VoiceSeekerDataDump VoiceSeeker_data_dump;

typedef struct
{
    int num_buffers; /* Number of all required buffers */
    int head;
    int tail;
    int count;
    AudioRefData_t *ref_data; /* Reference data itself */
} VoiceSeeker_RefData_T;
static VoiceSeeker_RefData_T VoiceSeeker_refData;
AudioRefData_t VoiceSeeker_RefData_Pull(void);

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
    VoiceSeekerLight_GetLibVersion(&vsl, &vsl_version.major, &vsl_version.minor, &vsl_version.patch);
    PRINTF("VoiceSeekerLight_GetLibVersion: v%i.%i.%i\n", vsl_version.major, vsl_version.minor, vsl_version.patch);

    // Unpack configuration
    framesize_in  = vsl_constants.framesize_in;
    samplerate    = vsl_constants.samplerate;
    framesize_out = vsl_config.framesize_out;
    num_mics      = vsl_config.num_mics;
    num_spks      = vsl_config.num_spks;

    /*
     * Map mic_in pointers to mic buffer
     */
    // Check right config of VoiceSeeker in and out buffers
    if ((framesize_out % framesize_in) != 0)
        PRINTF("VoiceSeeker in = %d not a modulo of VoiceSeeker out = %d", framesize_in, framesize_out);

    VoiceSeekerLight_PrintConfig(&vsl);
    VoiceSeekerLight_PrintMemOverview(&vsl);

    VoiceSeeker_data_dump.mic_data_ptr = (int8_t *)DEMO_SEMC_START_ADDRESS; /* SDRAM start address. */
    VoiceSeeker_data_dump.spk_data_ptr =
        (int8_t *)(DEMO_SEMC_START_ADDRESS + (DEMO_SEMC_SIZE / 2));         /* SDRAM start address. */
    VoiceSeeker_data_dump.mic_data_ptr_size = DEMO_SEMC_SIZE / 2;
    VoiceSeeker_data_dump.spk_data_ptr_size = DEMO_SEMC_SIZE / 2;
    VoiceSeeker_data_dump.mic_size          = 0;
    VoiceSeeker_data_dump.spk_size          = 0;

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    DCACHE_InvalidateByRange(DEMO_SEMC_START_ADDRESS, DEMO_SEMC_SIZE);
#endif

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
    int32_t TempData_Spk[FRAME_SIZE * RDSP_NUM_SPK] = {0};
    float *InputTempDataFloat_Mic                   = NULL;
    int16_t *InputTempDataINT16_Mic                 = NULL;
    float *InputTempDataFloat_Spk                   = NULL;
    int16_t *InputTempDataINT16_Spk                 = NULL;
    int16_t *outBuffer                              = NULL;
    float VoiceSeekerIteration                      = (float)framesize_out / (float)framesize_in;
    float *mic_in[num_mics];
    float *spk_in[num_spks];
    float *vsl_out         = NULL;
    AudioRefData_t refData = {NULL, 0};

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

    if (buffer_size > 0)
    {
        refData = VoiceSeeker_RefData_Pull();
    }
    else
    {
        return OK;
    }

    if (debugging)
    {
        if (VoiceSeeker_data_dump.mic_data_ptr != NULL)
        {
            uint16_t size = framesize_out * num_mics * (data_packet->bits_per_sample / 8);

            if ((VoiceSeeker_data_dump.mic_size + size <= VoiceSeeker_data_dump.mic_data_ptr_size) && (size > 0))
            {
                memcpy(VoiceSeeker_data_dump.mic_data_ptr + VoiceSeeker_data_dump.mic_size, buffer, size);
                VoiceSeeker_data_dump.mic_size += size;
            }
        }

        if (VoiceSeeker_data_dump.spk_data_ptr != NULL)
        {
            uint16_t size = framesize_out * num_spks * (data_packet->bits_per_sample / 8);

            if ((VoiceSeeker_data_dump.spk_size + size <= VoiceSeeker_data_dump.spk_data_ptr_size) && (size > 0))
            {
                memcpy(VoiceSeeker_data_dump.spk_data_ptr + VoiceSeeker_data_dump.spk_size, refData.buffer, size);
                VoiceSeeker_data_dump.spk_size += size;
            }
        }
    }

    if (data_packet->bits_per_sample == 16)
    {
        DeInterleave16((int16_t *)buffer, (int16_t *)TempData_Mic, framesize_out, num_mics);
        if (refData.buffer != NULL)
        {
            DeInterleave16((int16_t *)(refData.buffer), (int16_t *)TempData_Spk, framesize_out, num_spks);
        }
    }
    else
    {
        DeInterleave32((int16_t *)buffer, (int16_t *)TempData_Mic, framesize_out, num_mics);
        if (refData.buffer != NULL)
        {
            DeInterleave32((int16_t *)(refData.buffer), (int16_t *)TempData_Spk, framesize_out, num_spks);
        }
    }

    InputTempDataFloat_Mic = (float *)TempData_Mic;
    InputTempDataINT16_Mic = (int16_t *)TempData_Mic;

    // Convert  INT16 Q15 data to Float data
    for (uint16_t i = framesize_out * num_mics; i > 0; i--)
    {
        InputTempDataFloat_Mic[i - 1] = ((float)InputTempDataINT16_Mic[i - 1]) * INV_32768;
    }

    if (refData.buffer != NULL)
    {
        InputTempDataFloat_Spk = (float *)TempData_Spk;
        InputTempDataINT16_Spk = (int16_t *)TempData_Spk;

        for (uint16_t i = framesize_out * num_spks; i > 0; i--)
        {
            InputTempDataFloat_Spk[i - 1] = ((float)InputTempDataINT16_Spk[i - 1]) * INV_32768;
        }
    }

    // Initialize multichannel input pointers
    for (uint32_t imic = 0; imic < num_mics; imic++)
    {
        mic_in[imic] = &(InputTempDataFloat_Mic[imic * framesize_out]);
    }

    if (refData.buffer != NULL)
    {
        for (uint32_t ispk = 0; ispk < num_spks; ispk++)
        {
            spk_in[ispk] = &(InputTempDataFloat_Spk[ispk * framesize_out]);
        }
    }
    else
    {
        for (uint32_t ispk = 0; ispk < num_spks; ispk++)
        {
            spk_in[ispk] = NULL;
        }
    }

    while (VoiceSeekerIteration)
    {
        /*
         * VoiceSeeker Light process
         */
        voiceseeker_status = VoiceSeekerLight_Process(&vsl, mic_in, spk_in, &vsl_out);
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
        for (uint32_t ispk = 0; ispk < num_spks; ispk++)
        {
            if (spk_in[ispk] != NULL)
            {
                spk_in[ispk] += framesize_in; // increase address by VoiceSeeker input size i.e 32 samples
            }
        }
        VoiceSeekerIteration--;
    }

    if (vsl_out != NULL)
    {
        for (uint16_t i = 0; i < framesize_out; i++)
        {
            outBuffer[i] = (int16_t)(vsl_out[i] * FLOAT_TO_INT);
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

    if (VoiceSeeker_refData.ref_data != NULL)
    {
        OSA_MemoryFree(VoiceSeeker_refData.ref_data);
        VoiceSeeker_refData.num_buffers = 0;
        VoiceSeeker_refData.head        = 0;
        VoiceSeeker_refData.tail        = 0;
        VoiceSeeker_refData.count       = 0;
        VoiceSeeker_refData.ref_data    = NULL;
    }

    debugging = false;

    if ((VoiceSeeker_data_dump.mic_data_ptr != NULL) && (VoiceSeeker_data_dump.mic_size > 0))
    {
        fd = file_open("mic_data.pcm", FILE_WRONLY | FILE_TRUNC);
        if (fd >= 0)
        {
            PRINTF("Please wait, the microphone data is being written to the SD card\r\n");
            if (file_write(fd, VoiceSeeker_data_dump.mic_data_ptr, VoiceSeeker_data_dump.mic_size) ==
                VoiceSeeker_data_dump.mic_size)
            {
                PRINTF("Microphone data written successfully.\r\n");
            }
            else
            {
                PRINTF("Error writing microphone data.\r\n");
            }

            file_close(fd);
            fd = -1;
        }
        else
        {
            PRINTF("Error opening file to save microphone data.\r\n");
        }

        VoiceSeeker_data_dump.mic_data_ptr      = NULL;
        VoiceSeeker_data_dump.mic_data_ptr_size = 0;
        VoiceSeeker_data_dump.mic_size          = 0;
    }

    if ((VoiceSeeker_data_dump.spk_data_ptr != NULL) && (VoiceSeeker_data_dump.spk_size > 0))
    {
        fd = file_open("ref_data.pcm", FILE_WRONLY | FILE_TRUNC);
        if (fd >= 0)
        {
            PRINTF("Please wait, the reference data is being written to the SD card\r\n");
            if (file_write(fd, VoiceSeeker_data_dump.spk_data_ptr, VoiceSeeker_data_dump.spk_size) ==
                VoiceSeeker_data_dump.spk_size)
            {
                PRINTF("Reference data written successfully.\r\n");
            }
            else
            {
                PRINTF("Error writing reference data.\r\n");
            }

            file_close(fd);
            fd = -1;
        }
        else
        {
            PRINTF("Error opening file to save reference data.\r\n");
        }

        VoiceSeeker_data_dump.spk_data_ptr      = NULL;
        VoiceSeeker_data_dump.spk_data_ptr_size = 0;
        VoiceSeeker_data_dump.spk_size          = 0;
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

int VoiceSeeker_RefData_Set_Num_Buff(int numBuff)
{
    if (numBuff < 0)
    {
        return -1;
    }

    if (VoiceSeeker_refData.ref_data != NULL)
    {
        OSA_MemoryFree(VoiceSeeker_refData.ref_data);
        VoiceSeeker_refData.ref_data = NULL;
    }

    VoiceSeeker_refData.ref_data = (AudioRefData_t *)OSA_MemoryAllocate(sizeof(AudioRefData_t) * numBuff);
    if (VoiceSeeker_refData.ref_data == NULL)
    {
        return -1;
    }

    /* Set default values */
    VoiceSeeker_refData.num_buffers = numBuff;
    VoiceSeeker_refData.head        = 0;
    VoiceSeeker_refData.tail        = 0;
    VoiceSeeker_refData.count       = 0;

    for (int i = 0; i < VoiceSeeker_refData.num_buffers; i++)
    {
        VoiceSeeker_refData.ref_data[i].buffer = NULL;
        VoiceSeeker_refData.ref_data[i].size   = 0;
    }

    return 0;
}

int VoiceSeeker_RefData_Push(void *refData)
{
    AudioRefData_t *data = (AudioRefData_t *)refData;

    if (VoiceSeeker_refData.num_buffers <= 0)
    {
        return -1;
    }

    if (VoiceSeeker_refData.count == VoiceSeeker_refData.num_buffers)
    {
        return -1; /* Queue is full*/
    }

    VoiceSeeker_refData.ref_data[VoiceSeeker_refData.head].buffer = data->buffer;
    VoiceSeeker_refData.ref_data[VoiceSeeker_refData.head].size   = data->size;
    VoiceSeeker_refData.head = (VoiceSeeker_refData.head + 1) % VoiceSeeker_refData.num_buffers;
    VoiceSeeker_refData.count++;

    return 0;
}

AudioRefData_t VoiceSeeker_RefData_Pull(void)
{
    AudioRefData_t refData = {NULL, 0};

    if (VoiceSeeker_refData.ref_data == NULL)
    {
        return refData;
    }

    if (0 == VoiceSeeker_refData.count)
    {
        return refData; /* Queue is empty */
    }

    refData.buffer           = VoiceSeeker_refData.ref_data[VoiceSeeker_refData.tail].buffer;
    refData.size             = VoiceSeeker_refData.ref_data[VoiceSeeker_refData.tail].size;
    VoiceSeeker_refData.tail = (VoiceSeeker_refData.tail + 1) % VoiceSeeker_refData.num_buffers;
    VoiceSeeker_refData.count--;

    return refData;
}

int VoiceSeeker_Set_Debugging(bool set_debugging)
{
#if defined(PLATFORM_RT1060) && (RDSP_NUM_MIC > 1)
    PRINTF("Debugging is not Allowed with Expansion board\r\n");
    return 0;
#endif

    debugging = set_debugging;

    return 0;
}

VoiceSeeker_Initialize_T VoiceSeeker_Initialize_func = VoiceSeeker_Initialize;
VoiceSeeker_Execute_T VoiceSeeker_Execute_func       = VoiceSeeker_Execute;
VoiceSeeker_Deinit_T VoiceSeeker_Deinit_func         = VoiceSeeker_Deinit;

VoiceSeeker_RefData_Set_NumBuff_T VoiceSeeker_RefData_Set_NumBuff_func = VoiceSeeker_RefData_Set_Num_Buff;
VoiceSeeker_RefData_Push_T VoiceSeeker_RefData_Push_func               = VoiceSeeker_RefData_Push;
VoiceSeeker_Set_Debugging_T VoiceSeeker_Set_Debugging_func             = VoiceSeeker_Set_Debugging;

#endif
