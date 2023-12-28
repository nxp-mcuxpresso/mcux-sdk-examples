/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <string.h>
#include <math.h>

#include "app_definitions.h"
#include "fsl_debug_console.h"
#include "fsl_os_abstraction.h"

#include "streamer.h"
#include "vit_proc.h"
#include "VIT.h"
#include "VIT_Model_en.h"
#include "VIT_Model_cn.h"

#ifdef VOICE_SEEKER_PROC
#include "libVoiceSeekerLight.h"
#endif

#define BYTE_DEPTH         2
#define NUMBER_OF_CHANNELS 1
#define VIT_CMD_TIME_SPAN  3.0
#define VIT_OPERATING_MODE VIT_WAKEWORD_ENABLE | VIT_VOICECMD_ENABLE

#if defined(PLATFORM_RT1060)
#define DEVICE_ID      VIT_IMXRT1060
#define MODEL_LOCATION VIT_MODEL_IN_SLOW_MEM

#elif defined(PLATFORM_RT1170_EVKB)
#define DEVICE_ID      VIT_IMXRT1170
#define MODEL_LOCATION VIT_MODEL_IN_SLOW_MEM

#else
#error "No platform selected"

#endif

#ifdef VOICE_SEEKER_PROC
#define SAMPLES_PER_FRAME VIT_SAMPLES_PER_30MS_FRAME
extern RETUNE_VOICESEEKERLIGHT_plugin_t vsl;
#else
#define SAMPLES_PER_FRAME VIT_SAMPLES_PER_10MS_FRAME
#endif

#define MEMORY_ALIGNMENT 8                    // in bytes

static VIT_Handle_t VITHandle = PL_NULL;      // VIT handle pointer
static VIT_InstanceParams_st VITInstParams;   // VIT instance parameters structure
static VIT_ControlParams_st VITControlParams; // VIT control parameters structure
static PL_MemoryTable_st VITMemoryTable;      // VIT memory table descriptor
static PL_BOOL InitPhase_Error = PL_FALSE;
// static VIT_DataIn_st VIT_InputBuffers = {PL_NULL, PL_NULL,
//                                         PL_NULL}; // Resetting Input Buffer addresses provided to VIT_process() API
static PL_INT8 *pMemory[PL_NR_MEMORY_REGIONS];

VIT_ReturnStatus_en VIT_ModelInfo(void)
{
    VIT_ReturnStatus_en VIT_Status;
    VIT_ModelInfo_st Model_Info;
    VIT_Status = VIT_GetModelInfo(&Model_Info);
    if (VIT_Status != VIT_SUCCESS)
    {
        PRINTF("VIT_GetModelInfo error: %d\r\n", VIT_Status);
        return VIT_INVALID_MODEL;
    }

    PRINTF("\n  VIT Model info\r\n");
    PRINTF("  VIT Model Release: 0x%04x\r\n", Model_Info.VIT_Model_Release);
    if (Model_Info.pLanguage != PL_NULL)
    {
        PRINTF("  Language supported: %s \r\n", Model_Info.pLanguage);
    }
    PRINTF("  Number of WakeWords supported : %d \r\n", Model_Info.NbOfWakeWords);
    PRINTF("  Number of Commands supported : %d \r\n", Model_Info.NbOfVoiceCmds);

    if (!Model_Info.WW_VoiceCmds_Strings) // Check here if Model is containing WW and CMDs strings
    {
        PRINTF("  VIT_Model integrating WakeWord and Voice Commands strings: NO\r\n");
    }
    else
    {
        const char *ptr;

        PRINTF("  VIT_Model integrating WakeWord and Voice Commands strings: YES\r\n");
        PRINTF("  WakeWords supported : \r\n");
        ptr = Model_Info.pWakeWord_List;
        if (ptr != PL_NULL)
        {
            for (PL_UINT16 i = 0; i < Model_Info.NbOfWakeWords; i++)
            {
                PRINTF("   '%s' \r\n", ptr);
                ptr += strlen(ptr) + 1; // to consider NULL char
            }
        }
        PRINTF("  Voice commands supported: \r\n");
        ptr = Model_Info.pVoiceCmds_List;
        if (ptr != PL_NULL)
        {
            for (PL_UINT16 i = 0; i < Model_Info.NbOfVoiceCmds; i++)
            {
                PRINTF("   '%s' \r\n", ptr);
                ptr += strlen(ptr) + 1; // to consider NULL char
            }
        }
    }
    /*
     *   VIT Get Library information
     */
    VIT_LibInfo_st Lib_Info;
    VIT_Status = VIT_GetLibInfo(&Lib_Info);
    if (VIT_Status != VIT_SUCCESS)
    {
        PRINTF("VIT_GetLibInfo error: %d\r\n", VIT_Status);
        return VIT_INVALID_STATE;
    }
    PRINTF("\n  VIT Lib Info\r\n");
    PRINTF("  VIT LIB Release: 0x%04x\r\n", Lib_Info.VIT_LIB_Release);
    PRINTF("  VIT Features supported by the lib: 0x%04x\r\n", Lib_Info.VIT_Features_Supported);
    PRINTF("  Number of channels supported by VIT lib: %d\r\n", Lib_Info.NumberOfChannels_Supported);

    /*
     *   Configure VIT Instance Parameters
     */
    // Check that NUMBER_OF_CHANNELS is supported by VIT
    // Retrieve from VIT_GetLibInfo API the number of channel supported by the VIT lib
    PL_UINT16 max_nb_of_Channels = Lib_Info.NumberOfChannels_Supported;
    if (NUMBER_OF_CHANNELS > max_nb_of_Channels)
    {
        PRINTF("VIT lib is supporting only: %d channels\r\n", max_nb_of_Channels);
        return VIT_INVALID_PARAMETER_OUTOFRANGE;
    }
    return VIT_SUCCESS;
}

int VIT_Initialize(void *arg)
{
    VIT_ReturnStatus_en VIT_Status;
    uint16_t i, minIdx; /* loop index */
    int32_t temp32;     /* temporary address */
    int16_t j;          /* loop index */
    uint16_t order[PL_NR_MEMORY_REGIONS];

    switch (Vit_Language)
    {
        case CN:
            VIT_Status = VIT_SetModel(VIT_Model_cn, MODEL_LOCATION);
            break;
        default:
            VIT_Status = VIT_SetModel(VIT_Model_en, MODEL_LOCATION);
    }
    if (VIT_Status != VIT_SUCCESS)
    {
        return VIT_Status;
    }

    VIT_Status = VIT_ModelInfo();
    if (VIT_Status != VIT_SUCCESS)
    {
        return VIT_Status;
    }
    /*
     *   Configure VIT Instance Parameters
     */
    VITInstParams.SampleRate_Hz   = VIT_SAMPLE_RATE;
    VITInstParams.SamplesPerFrame = SAMPLES_PER_FRAME;
    VITInstParams.NumberOfChannel = NUMBER_OF_CHANNELS;
    VITInstParams.DeviceId        = DEVICE_ID;
    VITInstParams.APIVersion      = VIT_API_VERSION;

    /*
     *   VIT get memory table: Get size info per memory type
     */
    VIT_Status = VIT_GetMemoryTable(PL_NULL, // VITHandle param should be NULL
                                    &VITMemoryTable, &VITInstParams);
    if (VIT_Status != VIT_SUCCESS)
    {
        PRINTF("VIT_GetMemoryTable error: %d\r\n", VIT_Status);
        return VIT_Status;
    }

    /* Initialize order variable */
    for (i = 0; i < PL_NR_MEMORY_REGIONS; i++)
    {
        order[i] = i;
    }

    /* Sort region indexes by region size */
    for (i = 0; i < (PL_NR_MEMORY_REGIONS - 1); i++)
    {
        minIdx = i;
        for (j = i + 1; j < PL_NR_MEMORY_REGIONS; j++)
            if (VITMemoryTable.Region[order[j]].Size < VITMemoryTable.Region[order[minIdx]].Size)
                minIdx = j;

        /* Swap indexes */
        temp32        = order[minIdx];
        order[minIdx] = order[i];
        order[i]      = temp32;
    }

    /*
     *   Reserve memory space: Malloc for each memory type
     */
    for (j = (PL_NR_MEMORY_REGIONS - 1); j >= 0; j--)
    {
        /* Log the memory size */
        if (VITMemoryTable.Region[order[j]].Size != 0)
        {
            // reserve memory space
            // NB: VITMemoryTable.Region[PL_MEMREGION_PERSISTENT_FAST_DATA] should be allocated
            //      in the fastest memory of the platform (when possible) - this is not the case in this example.
            pMemory[j] = OSA_MemoryAllocate(VITMemoryTable.Region[order[j]].Size + MEMORY_ALIGNMENT);
            if (!pMemory[j])
            {
                return VIT_INVALID_NULLADDRESS;
            }
            VITMemoryTable.Region[order[j]].pBaseAddress = (void *)pMemory[j];
        }
    }

    /*
     *    Create VIT Instance
     */
    VITHandle  = PL_NULL; // force to null address for correct memory initialization
    VIT_Status = VIT_GetInstanceHandle(&VITHandle, &VITMemoryTable, &VITInstParams);
    if (VIT_Status != VIT_SUCCESS)
    {
        InitPhase_Error = PL_TRUE;
        PRINTF("VIT_GetInstanceHandle error: %d\r\n", VIT_Status);
    }

    /*
     *    Test the reset (OPTIONAL)
     */
    if (!InitPhase_Error)
    {
        VIT_Status = VIT_ResetInstance(VITHandle);
        if (VIT_Status != VIT_SUCCESS)
        {
            InitPhase_Error = PL_TRUE;
            PRINTF("VIT_ResetInstance error: %d\r\n", VIT_Status);
        }
    }

    /*
     *   Set and Apply VIT control parameters
     */
    VITControlParams.OperatingMode     = VIT_OPERATING_MODE;
    VITControlParams.Feature_LowRes    = PL_FALSE;
    VITControlParams.Command_Time_Span = VIT_CMD_TIME_SPAN;

    if (!InitPhase_Error)
    {
        VIT_Status = VIT_SetControlParameters(VITHandle, &VITControlParams);
        if (VIT_Status != VIT_SUCCESS)
        {
            InitPhase_Error = PL_TRUE;
            PRINTF("VIT_SetControlParameters error: %d\r\n", VIT_Status);
        }
    }
    /*
        //Public call to VIT_GetStatusParameters
        VIT_StatusParams_st* pVIT_StatusParam_Buffer = (VIT_StatusParams_st*)&VIT_StatusParams_Buffer;

        VIT_GetStatusParameters(VITHandle, pVIT_StatusParam_Buffer, sizeof(VIT_StatusParams_Buffer));
        PRINTF("\nVIT Status Params\n");
        PRINTF(" VIT LIB Release   = 0x%04x\n", pVIT_StatusParam_Buffer->VIT_LIB_Release);
        PRINTF(" VIT Model Release = 0x%04x\n", pVIT_StatusParam_Buffer->VIT_MODEL_Release);
        PRINTF(" VIT Features supported by the lib = 0x%04x\n", pVIT_StatusParam_Buffer->VIT_Features_Supported);
        PRINTF(" VIT Features Selected             = 0x%04x\n", pVIT_StatusParam_Buffer->VIT_Features_Selected);
        PRINTF(" Number of channels supported by VIT lib = %d\n", pVIT_StatusParam_Buffer->NumberOfChannels_Supported);
        PRINTF(" Device Selected: device id = %d\n", pVIT_StatusParam_Buffer->Device_Selected);
        if (pVIT_StatusParam_Buffer->WakeWord_In_Text2Model)
        {
            PRINTF(" VIT WakeWord in Text2Model\n ");
        }
        else
        {
            PRINTF(" VIT WakeWord in Audio2Model\n ");
        }
    */
    return VIT_Status;
}

int VIT_Execute(void *arg, void *inputBuffer, int size)
{
    StreamBuffer *buf    = (StreamBuffer *)inputBuffer;
    int8_t *pkt_hdr_size = (int8_t *)arg;
    void *buffer         = NULL;
    VIT_ReturnStatus_en VIT_Status;
    VIT_VoiceCommand_st VoiceCommand;                               // Voice Command info
    VIT_WakeWord_st WakeWord;                                       // Wakeword info
    VIT_DetectionStatus_en VIT_DetectionResults = VIT_NO_DETECTION; // VIT detection result

    if ((buf == NULL) || (pkt_hdr_size == NULL))
    {
        return VIT_INVALID_NULLADDRESS;
    }

    /* Initialization of the variables */
    buffer = (void *)(buf->buffer + *pkt_hdr_size);

    if (size != SAMPLES_PER_FRAME * NUMBER_OF_CHANNELS * BYTE_DEPTH)
    {
        PRINTF("Input buffer format issue\r\n");
        return VIT_INVALID_FRAME_SIZE;
    }

    VIT_Status = VIT_Process(VITHandle,
                             buffer, // temporal audio input data
                             &VIT_DetectionResults);

    if (VIT_Status != VIT_SUCCESS)
    {
        PRINTF("VIT_Process error: %d\r\n", VIT_Status);
        return VIT_Status; // will stop processing VIT and go directly to MEM free
    }

    if (VIT_DetectionResults == VIT_WW_DETECTED)
    {
        // Retrieve id of the WakeWord detected
        // String of the Command can also be retrieved (when WW and CMDs strings are integrated in Model)
        VIT_Status = VIT_GetWakeWordFound(VITHandle, &WakeWord);
        if (VIT_Status != VIT_SUCCESS)
        {
            PRINTF("VIT_GetWakeWordFound error : %d\r\n", VIT_Status);
            return VIT_Status; // will stop processing VIT and go directly to MEM free
        }
        else
        {
            PRINTF(" - WakeWord detected %d", WakeWord.Id);

            // Retrieve WakeWord Name : OPTIONAL
            // Check first if WakeWord string is present
            if (WakeWord.pName != PL_NULL)
            {
                PRINTF(" %s\r\n", WakeWord.pName);
            }
#ifdef VOICE_SEEKER_PROC
            VoiceSeekerLight_TriggerFound(&vsl, WakeWord.StartOffset);
#endif
        }
    }
    else if (VIT_DetectionResults == VIT_VC_DETECTED)
    {
        // Retrieve id of the Voice Command detected
        // String of the Command can also be retrieved (when WW and CMDs strings are integrated in Model)
        VIT_Status = VIT_GetVoiceCommandFound(VITHandle, &VoiceCommand);
        if (VIT_Status != VIT_SUCCESS)
        {
            PRINTF("VIT_GetVoiceCommandFound error: %d\r\n", VIT_Status);
            return VIT_Status; // will stop processing VIT and go directly to MEM free
        }
        else
        {
            PRINTF(" - Voice Command detected %d", VoiceCommand.Id);

            // Retrieve CMD Name: OPTIONAL
            // Check first if CMD string is present
            if (VoiceCommand.pName != PL_NULL)
            {
                PRINTF(" %s\r\n", VoiceCommand.pName);
            }
            else
            {
                PRINTF("\r\n");
            }
        }
    }
    return VIT_Status;
}

int VIT_Deinit(void)
{
    VIT_ReturnStatus_en VIT_Status;   /* Function call status */
                                      // retrieve size of the different MEM tables allocated
    VIT_Status =
        VIT_GetMemoryTable(VITHandle, // Should provide VIT_Handle to retrieve the size of the different MemTabs
                           &VITMemoryTable, &VITInstParams);
    if (VIT_Status != VIT_SUCCESS)
    {
        PRINTF("VIT_GetMemoryTable error: %d\r\n", VIT_Status);
    }

    // Free the MEM tables
    for (int i = 0; i < PL_NR_MEMORY_REGIONS; i++)
    {
        if (pMemory[i] != NULL)
        {
            OSA_MemoryFree((PL_INT8 *)pMemory[i]);
            pMemory[i] = NULL;
        }
    }
    return VIT_Status;
}

VIT_Initialize_T VIT_Initialize_func = VIT_Initialize;
VIT_Execute_T VIT_Execute_func       = VIT_Execute;
VIT_Deinit_T VIT_Deinit_func         = VIT_Deinit;
VIT_Language_T Vit_Language;
