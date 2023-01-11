/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifdef EAP_PROC

#include "fsl_debug_console.h"

#include "streamer_api.h"
#include "streamer_element_properties.h"
#include "streamer.h"

#include "app_streamer.h"
#include "eap_proc.h"
#include "eap_att.h"
#include "app_data.h"

#include "EAP_Parameter_AllEffectOff.h"
#include "EAP_Parameter_AutoVolumeLeveler.h"
#if (ALGORITHM_CS == 1)
#include "EAP_Parameter_ConcertSound.h"
#endif
#if (ALGORITHM_XO == 1)
#include "EAP_Parameter_Crossover2waySpeakers.h"
#include "EAP_Parameter_CrossoverForSubwoofer.h"
#endif

#include "EAP_Parameter_ToneGenerator.h"
#include "EAP_Parameter_LoudnessMaximiser.h"

#ifdef CPU_LPC55S69JBD100_cm33_core0
#include "EAP_Parameter_MusicEnhancerRMSLimiter_lowMIPS.h"
#include "EAP_Parameter_VoiceEnhancer_lowMIPS.h"
#else
#include "EAP_Parameter_MusicEnhancerRMSLimiter.h"
#include "EAP_Parameter_VoiceEnhancer.h"
#include "EAP_Parameter_Custom.h"
#endif

// EAP Lib
static LVM_Handle_t EAP_hInstance; /* Instance handle */
typedef struct _eap_preset_t
{
    char *name;
    LVM_ControlParams_t *controlParams;
    LVM_InstParams_t *instParams;
    LVM_HeadroomParams_t *headroomParams;
} eap_preset_t;

eap_preset_t eap_presets[] = {{.name           = "AllEffectOff",
                               .controlParams  = &ControlParamSet_allEffectOff,
                               .instParams     = &InstParams_allEffectOff,
                               .headroomParams = &HeadroomParams_allEffectOff},
                              {.name = "VoiceEnhancer",
#ifdef CPU_LPC55S69JBD100_cm33_core0
                               .controlParams  = &ControlParamSet_voiceEnhancer_lowMIPS,
                               .instParams     = &InstParams_voiceEnhancer_lowMIPS,
                               .headroomParams = &HeadroomParams_voiceEnhancer_lowMIPS},
#else
                               .controlParams  = &ControlParamSet_voiceEnhancer,
                               .instParams     = &InstParams_voiceEnhancer,
                               .headroomParams = &HeadroomParams_voiceEnhancer},
#endif
                              {.name = "MusicEnhancer",
#ifdef CPU_LPC55S69JBD100_cm33_core0
                               .controlParams  = &ControlParamSet_musicEnhancerRmsLimiter_lowMIPS,
                               .instParams     = &InstParams_musicEnhancerRmsLimiter_lowMIPS,
                               .headroomParams = &HeadroomParams_musicEnhancerRmsLimiter_lowMIPS},
#else
                               .controlParams  = &ControlParamSet_musicEnhancerRmsLimiter,
                               .instParams     = &InstParams_musicEnhancerRmsLimiter,
                               .headroomParams = &HeadroomParams_musicEnhancerRmsLimiter},
#endif
                              {.name           = "AutoVolumeLeveler",
                               .controlParams  = &ControlParamSet_autoVolumeLeveler,
                               .instParams     = &InstParams_autoVolumeLeveler,
                               .headroomParams = &HeadroomParams_autoVolumeLeveler},
                              {.name           = "LoudnessMaximiser",
                               .controlParams  = &ControlParamSet_loudnessMaximiser,
                               .instParams     = &InstParams_loudnessMaximiser,
                               .headroomParams = &HeadroomParams_loudnessMaximiser},
#if (ALGORITHM_CS == 1)
                              {.name           = "ConcertSound",
                               .controlParams  = &ControlParamSet_concertSound,
                               .instParams     = &InstParams_concertSound,
                               .headroomParams = &HeadroomParams_concertSound},
#endif
#ifndef CPU_LPC55S69JBD100_cm33_core0
                              {.name           = "Custom",
                               .controlParams  = &ControlParamSet_custom,
                               .instParams     = &InstParams_custom,
                               .headroomParams = &HeadroomParams_custom},
#endif
                              {.name           = "ToneGenerator",
                               .controlParams  = &ControlParamSet_toneGenerator,
                               .instParams     = &InstParams_toneGenerator,
                               .headroomParams = &HeadroomParams_toneGenerator},
#if (ALGORITHM_XO == 1)
                              {.name           = "Crossover2WaySpeakers",
                               .controlParams  = &ControlParamSet_Crossover2WaySpeaker,
                               .instParams     = &InstParams_Crossover2WaySpeaker,
                               .headroomParams = &HeadroomParams_Crossover2WaySpeaker},
                              {.name           = "CrossoverForSubwoofer",
                               .controlParams  = &ControlParamSet_CrossoverForSubwoofer,
                               .instParams     = &InstParams_CrossoverForSubwoofer,
                               .headroomParams = &HeadroomParams_CrossoverForSubwoofer}
#endif
};

LVM_ReturnStatus_en EAP_SetFSandChannels(ext_proc_args *args);

static bool first_exec;
static LVM_MemTab_t EAP_MemTab;            /* Memory allocation table */
static LVM_VersionInfo_st EAP_VersionInfo; /* Version info structure */
#if (ALGORITHM_XO == 1)
static uint16_t eap_xo_out_buf_size = 0U; /* Chunk size for which the xo out buffers are allocated */
static int8_t *eap_xo_out_buffer    = NULL;
#endif
int32_t *unaligned_buff[LVM_NR_MEMORY_REGIONS];

#ifdef ALGORITHM_AVL
static LVM_INT32 avlGain;
#endif

#ifdef ALGORITHM_PSA
LVM_INT16 EAP_PSA_Bands           = 16;
LVM_UINT32 EAP_PSA_AudioTime1     = 0;
LVM_UINT32 EAP_PSA_DisplayTimeInc = 60;
LVM_UINT32 EAP_PSA_StartOffset    = 200;

LVM_INT8 EAP_PSA_Data_Lock = 0;
LVM_INT8 EAP_PSA_CurrPeaks[LVM_PSA_MAX_NUMBANDS];
LVM_INT8 EAP_PSA_PastPeaks[LVM_PSA_MAX_NUMBANDS];
LVM_UINT16 EAP_PSA_BandFreq[LVM_PSA_MAX_NUMBANDS];
#endif

static LVM_UINT32 EAP_AudioTime = 0;

// malloc memory
static LVM_INT16 MallocAlign = 4; /* 4 byte Malloc alignment */

int EAP_Init(void *arg)
{
    LVM_ReturnStatus_en LVM_Status; /* Function call status */
    LVM_UINT16 i, minIdx;           /* loop index */
    LVM_INT32 temp32;               /* temporary address */
    LVM_INT16 j;
    LVM_UINT16 order[LVM_NR_MEMORY_REGIONS];

#if (ALGORITHM_XO == 1)
    /* Init variables */
    eap_xo_out_buf_size = 0U;
    eap_xo_out_buffer   = NULL;
#endif

    for (i = 0; i < LVM_NR_MEMORY_REGIONS; i++)
    {
        unaligned_buff[i] = NULL;
    }
    /******************************************************************************
    GET VERSION INFORMATION
    *******************************************************************************/
    LVM_Status = LVM_GetVersionInfo(&EAP_VersionInfo);
    if (LVM_Status != LVM_SUCCESS)
    {
        return LVM_Status;
    }

    /******************************************************************************
    SET THE INPUT PARAMETERS
    *******************************************************************************/
    /*
     * Select parameter configuration
     */
    ext_proc_args *args        = arg;
    eap_att_control_t *control = get_eap_att_control();
    int presetIndex            = get_app_data()->lastPreset;
    if (presetIndex > 0)
    {
        presetIndex -= 1; // normalize ID to index, ID=0 is reserved for no-preset reload
        if ((presetIndex < (sizeof(eap_presets) / sizeof(eap_presets[0]))))
        {
            eap_preset_t *preset = &(eap_presets[presetIndex]);
            PRINTF("[EAP_STREAMER] loading preset %s\r\n", preset->name);

#ifdef ALGORITHM_EQNB
            void *eqnbPtrBackup     = control->controlParam->pEQNB_BandDefinition;
            void *prodPtrBackup     = control->controlParam->pPR_EQNB_BandDefinition;
            void *headroomPtrBackup = control->headroomParams->pHeadroomDefinition;

            memcpy(headroomPtrBackup, preset->headroomParams->pHeadroomDefinition,
                   sizeof(*control->headroomParams->pHeadroomDefinition) * LVM_HEADROOM_MAX_NBANDS);
            memcpy(eqnbPtrBackup, preset->controlParams->pEQNB_BandDefinition,
                   sizeof(*control->controlParam->pEQNB_BandDefinition) * LVM_EQNB_MAX_BANDS_NBR);
            memcpy(prodPtrBackup, preset->controlParams->pPR_EQNB_BandDefinition,
                   sizeof(*control->controlParam->pPR_EQNB_BandDefinition) * LVM_EQNB_MAX_BANDS_NBR);

            memcpy(control->headroomParams, preset->headroomParams, sizeof(*control->headroomParams));
#endif

            memcpy(control->controlParam, preset->controlParams, sizeof(*control->controlParam));
            memcpy(control->instParams, preset->instParams, sizeof(*control->instParams));

#ifdef ALGORITHM_EQNB
            control->headroomParams->pHeadroomDefinition   = headroomPtrBackup;
            control->controlParam->pEQNB_BandDefinition    = eqnbPtrBackup;
            control->controlParam->pPR_EQNB_BandDefinition = prodPtrBackup;
#endif
        }
        else
        {
            PRINTF("[EAP_STREAMER] selected preset ID (%c, resolved as index %d) is out of defined presets.\r\n",
                   get_app_data()->lastPreset, presetIndex);
        }
    }

#ifdef ALGORITHM_PSA
    // EAP_PSA_Bands = (LVM_INT16)ControlParamSet_internal.PSA_NumBands;
#endif

    LVM_Status = EAP_SetFSandChannels(args);
    if (LVM_Status != LVM_SUCCESS)
    {
        return LVM_Status;
    }

    control->instParams->MaxBlockSize = MAX_SAMPLE_SIZE;

    /******************************************************************************
    Allocate memory
    Force alignment by allocating extra memory
    *******************************************************************************/
    LVM_Status = LVM_GetMemoryTable(LVM_NULL, &EAP_MemTab, control->instParams);
    if (LVM_Status != LVM_SUCCESS)
    {
        return LVM_Status;
    }

    /* Initialize order variable */
    for (i = 0; i < LVM_NR_MEMORY_REGIONS; i++)
    {
        order[i] = i;
    }

    /* Sort region indexes by region size */
    for (i = 0; i < (LVM_NR_MEMORY_REGIONS - 1); i++)
    {
        minIdx = i;
        for (j = i + 1; j < LVM_NR_MEMORY_REGIONS; j++)
            if (EAP_MemTab.Region[order[j]].Size < EAP_MemTab.Region[order[minIdx]].Size)
                minIdx = j;

        /* Swap indexes */
        temp32        = order[minIdx];
        order[minIdx] = order[i];
        order[i]      = temp32;
    }

    /* Allocate regions in order from largest to smallest */
    for (j = (LVM_NR_MEMORY_REGIONS - 1); j >= 0; j--)
    {
        /* Log the memory size */
        if (EAP_MemTab.Region[order[j]].Size != 0)
        {
            unaligned_buff[j] =
                (int32_t *)OSA_MemoryAllocate(EAP_MemTab.Region[order[j]].Size + (LVM_UINT32)MallocAlign);
            if (unaligned_buff[j] == NULL)
            {
                return LVM_NULLADDRESS;
            }
            EAP_MemTab.Region[order[j]].pBaseAddress = (void *)MEM_ALIGN(unaligned_buff[j], MallocAlign);
        }
    }

    /******************************************************************************
    Initialize
    *******************************************************************************/
    /*
     * Get an EAP Instance
     */
    control->instParams->Platform = PLATFORM_ID;
    EAP_hInstance                 = LVM_NULL;                             /* Initialise to NULL */
    LVM_Status                    = LVM_GetInstanceHandle(&EAP_hInstance, /* Init sets the instance handle */
                                       &EAP_MemTab, control->instParams);
    if (LVM_Status != LVM_SUCCESS)
    {
        return LVM_Status;
    }

#ifdef ALGORITHM_EQNB
    {
        // set headroom param config
        LVM_Status = LVM_SetHeadroomParams(EAP_hInstance, control->headroomParams);
        if (LVM_Status != LVM_SUCCESS)
        {
            return LVM_Status;
        }
    }
#endif

    /******************************************************************************
    Call set control parameters
     - propagate the configuration to EAP
    *******************************************************************************/
    LVM_Status = LVM_SetControlParameters(EAP_hInstance, control->controlParam);

    if (LVM_Status != LVM_SUCCESS)
    {
        return LVM_Status;
    }
    else
    {
        /* ready for first EAP execute */
        first_exec = true;
#if (ALGORITHM_XO == 1)
        get_app_data()->lastXOOperatingMode = control->controlParam->XO_OperatingMode;
#endif
        eap_att_register_handle(EAP_hInstance);
    }
    return LVM_Status;
}

int EAP_Execute(void *arg, void *inputBuffer, int size)
{
    StreamBuffer *buf                 = (StreamBuffer *)inputBuffer;
    int8_t *pkt_hdr_size              = arg;
    AudioPacketHeader *data_packet    = NULL;
    int8_t *data_ptr                  = NULL;
    int8_t num_channel                = 0;
    int8_t byte_width                 = 0;
    int8_t *outBuffer[NUM_OUT_BUFFES] = {NULL, NULL};
    LVM_ReturnStatus_en LVM_Status    = LVM_SUCCESS;

    if ((buf == NULL) || (pkt_hdr_size == NULL))
    {
        return LVM_NULLADDRESS;
    }

    if (*pkt_hdr_size != sizeof(AudioPacketHeader))
    {
        /* Others headers are not allowed in the EAP */
        return LVM_OUTOFRANGE_GENERAL_PARAMS;
    }

    /* Initialization of the variables */
    data_packet  = (AudioPacketHeader *)buf->buffer;
    data_ptr     = buf->buffer + *pkt_hdr_size;
    outBuffer[0] = (int8_t *)data_ptr;
    num_channel  = data_packet->num_channels;
    byte_width   = data_packet->bits_per_sample >> 3;

    if (num_channel < 1)
    {
        return LVM_OUTOFRANGE;
    }

#if (ALGORITHM_XO == 1)
    eap_att_control_t *control = get_eap_att_control();
    app_data_t *app_data       = get_app_data();
    if (size > (int)eap_xo_out_buf_size)
    {
        first_exec = true;
    }

    if (first_exec)
    {
        first_exec                    = false;
        app_data->eap_args.xo_enabled = false;

        if (control->controlParam->XO_OperatingMode == LVM_MODE_ON)
        {
            // indicate that xo is enabled to enable proper chunks delivery for MONO source format by maestro
            // streamer
            app_data->eap_args.xo_enabled = true;

            if (eap_xo_out_buffer != NULL)
            {
                OSA_MemoryFree(eap_xo_out_buffer);
                eap_xo_out_buffer = NULL;
            }

            eap_xo_out_buf_size = (uint16_t)size;
            eap_xo_out_buffer   = (int8_t *)OSA_MemoryAllocate(size * NUM_OUT_BUFFES + *pkt_hdr_size);
            if (eap_xo_out_buffer == NULL)
            {
                return LVM_NULLADDRESS;
            }
        }
    }
    if (control->controlParam->XO_OperatingMode == LVM_MODE_ON)
    {
        outBuffer[1] = (eap_xo_out_buffer + *pkt_hdr_size) + size;
    }
#endif
    size /= byte_width;

    EAP_AudioTime += LVM_FRAME_SIZE_MS;

    LVM_Status = LVM_Process(EAP_hInstance,           /* Instance handle */
                             (LVM_INT16 *)data_ptr,   /* Input buffer */
                             (LVM_INT16 **)outBuffer, /* Output buffer */
                             size / num_channel,      /* Number of samples to process */
                             EAP_AudioTime);          /* Audio Time*/

#if (ALGORITHM_XO == 1)

    if (control->controlParam->XO_OperatingMode == LVM_MODE_ON)
    {
        int8_t *data_out = eap_xo_out_buffer + *pkt_hdr_size;
        int8_t num_bytes = num_channel * byte_width;

#ifdef XO_USE_FULL_STEREO
        /* rearrangement of output values */
        for (int i = 0; i < size / num_channel; i++, outBuffer[0] += num_bytes, outBuffer[1] += num_bytes)
        {
            memcpy(data_out + num_bytes * 2 * i, outBuffer[0], num_bytes);
            memcpy(data_out + num_bytes * 2 * i + num_bytes, outBuffer[1], num_bytes);
        }

        /* Set the current valid values */
        data_packet->num_channels = 2 * num_channel;
        data_packet->chunk_size   = 2 * size * byte_width;
        memcpy(eap_xo_out_buffer, data_packet, *pkt_hdr_size);

        buf->buffer = eap_xo_out_buffer;
        buf->size   = data_packet->chunk_size + *pkt_hdr_size;
#else
        if (num_channel == 1)
        {
            /* rearrangement of output values */
            for (int i = 0; i < size / num_channel; i++, outBuffer[0] += num_bytes, outBuffer[1] += num_bytes)
            {
                memcpy(data_out + num_bytes * 2 * i, outBuffer[0], num_bytes);
                memcpy(data_out + num_bytes * 2 * i + num_bytes, outBuffer[1], num_bytes);
            }

            /* Set the current valid values */
            data_packet->num_channels = 2 * num_channel;
            data_packet->chunk_size   = 2 * size * byte_width;
            memcpy(eap_xo_out_buffer, data_packet, *pkt_hdr_size);

            buf->buffer = eap_xo_out_buffer;
            buf->size   = data_packet->chunk_size + *pkt_hdr_size;
        }
        else
        {
            /* Interleave data - mix low and high band output of crossover (in case of stereo input take just left
             * channel)*/
            for (int i = 0; i < size / num_channel; i++, outBuffer[0] += num_bytes, outBuffer[1] += num_bytes)
            {
                memcpy(data_out + byte_width * 2 * i, outBuffer[0], byte_width);
                memcpy(data_out + byte_width * 2 * i + byte_width, outBuffer[1], byte_width);
            }

            memcpy(eap_xo_out_buffer, data_packet, *pkt_hdr_size);
            buf->buffer = eap_xo_out_buffer;
        }
#endif
    }
#endif

#ifdef ALGORITHM_AVL
    LVM_GetAVLGain(EAP_hInstance, &avlGain);
#endif

#ifdef ALGORITHM_PSA
    if (EAP_AudioTime > (EAP_PSA_StartOffset + EAP_PSA_AudioTime1))
    {
        if (!EAP_PSA_Data_Lock)
        {
            // save calculated PSA data into global buffer
            LVM_GetSpectrum(EAP_hInstance, EAP_PSA_CurrPeaks, EAP_PSA_PastPeaks, EAP_PSA_BandFreq, EAP_PSA_AudioTime1);
        }
        EAP_PSA_AudioTime1 += (LVM_UINT32)EAP_PSA_DisplayTimeInc;
    }
#endif

    return LVM_Status;
}

LVM_ReturnStatus_en EAP_SetFSandChannels(ext_proc_args *args)
{
    LVM_ReturnStatus_en LVM_Status = LVM_SUCCESS;
    eap_att_control_t *control     = get_eap_att_control();
    switch (args->sample_rate)
    {
        case 4000:
            LVM_Status = LVM_OUTOFRANGE;
            break;
        case 8000:
            control->controlParam->SampleRate = LVM_FS_8000;
            break;
        case 11025:
            control->controlParam->SampleRate = LVM_FS_11025;
            break;
        case 12000:
            control->controlParam->SampleRate = LVM_FS_12000;
            break;
        case 16000:
            control->controlParam->SampleRate = LVM_FS_16000;
            break;
        case 22050:
            control->controlParam->SampleRate = LVM_FS_22050;
            break;
        case 24000:
            control->controlParam->SampleRate = LVM_FS_24000;
            break;
        case 32000:
            control->controlParam->SampleRate = LVM_FS_32000;
            break;
        case 44100:
            control->controlParam->SampleRate = LVM_FS_44100;
            break;
        case 48000:
            control->controlParam->SampleRate = LVM_FS_48000;
            break;
        case 64000:
        case 88200:
        case 96000:
        case 128000:
        case 176400:
        case 192000:
            LVM_Status = LVM_OUTOFRANGE;
            break;
    }
    switch (args->num_channels)
    {
        case 1:
            control->controlParam->SourceFormat = LVM_MONO;
            break;
        case 2:
            control->controlParam->SourceFormat = LVM_STEREO;
            break;
    }
    if (LVM_Status == LVM_OUTOFRANGE)
    {
        return LVM_Status;
    }
    else
    {
        return LVM_SUCCESS;
    }
}

int EAP_Deinit(void)
{
    LVM_ReturnStatus_en LVM_Status; /* Function call status */
    LVM_UINT16 i;                   /* loop index */

    /*
     * Free memory
     */

    LVM_Status = LVM_GetMemoryTable(EAP_hInstance, &EAP_MemTab, LVM_NULL);
    if (LVM_Status != LVM_SUCCESS)
    {
        PRINTF("EAP GetMemoryTable error: %d\r\n", LVM_Status);
    }
    for (i = 0; i < LVM_NR_MEMORY_REGIONS; i++)
    {
        if (unaligned_buff[i] != NULL)
        {
            OSA_MemoryFree(unaligned_buff[i]);
            unaligned_buff[i] = NULL;
        }
    }
#if (ALGORITHM_XO == 1)
    if (eap_xo_out_buffer != NULL)
    {
        OSA_MemoryFree(eap_xo_out_buffer);
        eap_xo_out_buffer   = NULL;
        eap_xo_out_buf_size = 0U;
    }
#endif

    EAP_hInstance = LVM_NULL;
    return LVM_Status;
}

eap_att_code_t register_post_process(void *streamer)
{
    ELEMENT_PROPERTY_T prop;
    get_app_data()->lastPreset = get_eap_att_control()->eapPreset;

    PRINTF("[EAP_STREAMER] registering post process EAP\r\n");

    EXT_PROCESS_DESC_T eap_proc = {EAP_Init, EAP_Execute, EAP_Deinit, &get_app_data()->eap_args};

    prop.prop = PROP_AUDIO_PROC_FUNCPTR;
    prop.val  = (uintptr_t)&eap_proc;

    if (streamer_set_property(streamer, prop, true) == 0)
    {
        return kEapAttCodeOk;
    }
    else
    {
        PRINTF("[EAP_STREAMER] EAP post processor registration failed\r\n");
    }
    return kEapAttCodeError;
}

#endif /* EAP_PROC */
