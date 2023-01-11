/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license for this file can be found in the LICENSE.txt file included with this distribution
 * or at https://spdx.org/licenses/BSD-3-Clause.html#licenseText
 */
#include "eap_att.h"

#include <string.h>
#include <stdio.h>

#if (defined EAP_PROC || defined EAP32_PROC)
// @formatter:off
LVM_EQNB_BandDef_t EQNB_BandDefs_UserEq1_internal[LVM_EQNB_MAX_BANDS_NBR] = {
    {-15, 50, 50}, // gain(dB), freq(Hz) , Qfactor *100
    {5, 400, 100}, {-3, 3999, 80}, {0, 1400, 96}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

LVM_EQNB_BandDef_t EQNB_BandDefs_ProductEq1_internal[LVM_EQNB_MAX_BANDS_NBR] = {
    {3, 700, 1000}, // gain(dB), freq(Hz) , Qfactor *100
    {0, 0, 0},      {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

LVM_InstParams_t InstParams_internal = {
    .BufferMode       = LVM_MANAGED_BUFFERS,    /* Buffer management mode */
    .MaxBlockSize     = 480,                    /* Maximum processing block size max = 480 */
    .EQNB_NumBands    = LVM_EQNB_MAX_BANDS_NBR, /* Maximum number of band for the equalizer */
    .PR_EQNB_NumBands = LVM_EQNB_MAX_BANDS_NBR, /* Maximum number of band for the equalizer */
#ifdef ALGORITHM_PSA
    .PSA_HistorySize        = 1000,       /* PSA History size in ms: 200 to 5000 */
    .PSA_MaxBands           = 64,         /* Maximum number of bands: 6 to 64 */
    .PSA_SpectrumUpdateRate = 25,         /* Spectrum update rate : 10 to 25*/
    .PSA_Included           = LVM_PSA_ON, /* Controls the instance memory allocation for PSA: ON/OFF */
#endif
};

#ifdef ALGORITHM_EQNB
LVM_HeadroomBandDef_t HeadroomBandDef_internal[LVM_HEADROOM_MAX_NBANDS] = {
    {20, 4999, 3}, /* Limit_Low, Limit_High, Headroom_Offset */
    {5000, 24000, 5},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0}};

LVM_HeadroomParams_t HeadroomParams_internal = {
    .Headroom_OperatingMode = LVM_HEADROOM_OFF,             /* Headroom Control On/Off */
    .pHeadroomDefinition    = &HeadroomBandDef_internal[0], /* Headroom band definition */
    .NHeadroomBands         = 0,                            /* Number of headroom bands */
};
#endif

LVM_ControlParams_t ControlParamSet_internal = {
    /* General parameters */
    .OperatingMode = LVM_MODE_ON,  // LVM_MODE_ON or LVM_MODE_OFF / Bundle operating mode On/Bypass
    .SampleRate    = LVM_FS_48000, // LVM_FS_8000, LVM_FS_11025, LVM_FS_12000, LVM_FS_16000, LVM_FS_22050, LVM_FS_24000,
                                   // LVM_FS_32000, LVM_FS_44100, LVM_FS_48000
    .SourceFormat = LVM_STEREO,    // LVM_MONO or LVM_MONOINSTEREO or LVM_STEREO
    .SpeakerType  = LVM_HEADPHONES, // LVM_HEADPHONES or LVM_MOBILE_SPEAKERS_SMALL or LVM_MOBILE_SPEAKERS_MEDIUM or
                                    // LVM_MOBILE_SPEAKERS_LARGE
    .SpeakerTypeInternal = LVM_SPEAKER_STEREO, // Device speaker type, LVM_SPEAKER_MONO or LVM_SPEAKER_STEREO
#ifdef ALGORITHM_3DWIDENING
    /* Concert Sound Virtualizer parameters */
    .VirtualizerOperatingMode = LVM_MODE_OFF,   // LVM_MODE_ON or LVM_MODE_OFF;
    .VirtualizerType = LVM_CONCERTSOUND,        // LVM_CONCERTSOUND or LVM_CINEMASOUND_MUSIC or LVM_CINEMASOUND_MOVIE or
                                                // LVM_CINEMASOUND_STEREO;
    .VirtualizerReverbLevel = (LVM_UINT16)100,  // Virtualizer reverb level min = 0% to max = 100%
    .CS_EffectLevel         = (LVM_INT16)32767, // Concert Sound effect level min = 0 to max = 32767

// Concert sound advanced parameter
#ifdef ALGORITHM_CS
    .CS_AP_Mode          = LVM_AP_DEFAULT, // concert sound advanced parameter mode: LVM_AP_DEFAULT or LVM_AP_MANUAL
    .CS_AP_MidGain       = 0,              // MidChannelGain: -10 to 10 dB
    .CS_AP_MidCornerFreq = 500,            // Shelving Filter Corner Frequency: 20 to 24000 Hz
    .CS_AP_SideHighPassCutoff = 600,       // SideBoost HighPassFilter Corner Frequency: 20 to 24000 Hz
    .CS_AP_SideLowPassCutoff  = 1544,      // SideBoost LowPassFilter Corner Frequency: 20 to 24000 Hz
    .CS_AP_SideGain           = 10,        // Side Channel Gain: 0 to 15 dB
#endif                                     /* ALGORITHM_CS */
#endif                                     /* ALGORITHM_3DWIDENING */

#ifdef ALGORITHM_BASS
    /* Bass Enhancement parameters */
    .BE_OperatingMode = LVM_BE_OFF, // LVM_BE_ON or LVM_BE_OFF;
    .BE_EffectLevel =
        (LVM_INT16)LVM_BE_9DB, // LVM_BE_0DB or LVM_BE_3DB or LVM_BE_6DB or LVM_BE_9DB or LVM_BE_12DB or LVM_BE_15DB;
    .BE_CentreFreq =
        LVM_BE_CENTRE_90Hz,   // LVM_BE_CENTRE_55Hz or LVM_BE_CENTRE_66Hz or LVM_BE_CENTRE_78Hz or LVM_BE_CENTRE_90Hz;
    .BE_HPF = LVM_BE_HPF_OFF, // LVM_BE_HPF_ON or LVM_BE_HPF_OFF;
#endif                        /* ALGORITHM_BASS */

    /* Volume Control parameters */
    .VC_EffectLevel = (LVM_INT16)0, // Volume Control setting in dBs -96 to 0 dB
    .VC_Balance     = (LVM_INT16)0, // Left Right Balance control in dB (-96 to 96 dB)

#ifdef ALGORITHM_TE
    /* Treble Enhancement parameters */
    .TE_OperatingMode = LVM_TE_OFF,   // LVM_TE_ON or LVM_TE_OFF
    .TE_EffectLevel   = (LVM_INT16)4, // Treble Enhancement gain 0dB to 15dB or LVM_TE_LOW_MIPS for saving MIPS
#endif                                /* ALGORITHM_TE */

#ifdef ALGORITHM_LM
    /* Loudness Maximizer parameters */
    .LM_OperatingMode  = LVM_LM_OFF,    // LVM_LM_ON or LVM_LM_OFF
    .LM_EffectLevel    = LVM_LM_GENTLE, // LVM_LM_GENTLE or LVM_LM_MEDIUM or LVM_LM_EXTREME
    .LM_Attenuation    = (LVM_UINT16)0, // 0 to 6 ; Loudness Maximizer output attenuation
    .LM_CompressorGain = (LVM_UINT16)2, // 0 to 6 ; Loudness Maximizer output compressor gain
    .LM_SpeakerCutOff  = (LVM_UINT16)0, // 150 to 1100; Loudness Maximizer speaker cut off frequency
#endif                                  /* ALGORITHM_LM */

#ifdef ALGORITHM_AVL
    /* AVL parameters */
    .AVL_OperatingMode = LVM_AVL_OFF, // LVM_AVL_ON or LVM_AVL_OFF
#endif                                /* ALGORITHM_AVL */

#ifdef ALGORITHM_TG
    /* Tone Generator parameters */
    .TG_OperatingMode  = LVM_TG_OFF,       // LVM_TG_OFF or LVM_TG_CONTINUOUS or LVM_TG_ONESHOT
    .TG_SweepMode      = LVM_TG_SWEEPLOG,  // LVM_TG_SWEEPLIN or LVM_TG_SWEEPLOG
    .TG_StartFrequency = (LVM_UINT16)20,   // Tone Generator Sweep Start Frequency 20 to 24000 Hz
    .TG_StartAmplitude = (LVM_INT16)0,     // Tone Generator Sweep Start Amplitude -96 to 0 dB
    .TG_StopFrequency  = (LVM_UINT16)2200, // Tone Generator Sweep Stop Frequency 20 to 24000 Hz
    .TG_StopAmplitude  = (LVM_INT16)0,     // Tone Generator Sweep Stop Amplitude -96 to 0 dB
    .TG_SweepDuration =
        (LVM_UINT16)60, // Tone Generator Sweep Duration; Sweep duration in seconds, 0 for infinite duration tone
    .pTG_CallBack   = LVM_NULL,         // WARNING Callback not supported; End of sweep callback
    .TG_CallBackID  = 0x100,            // Callback ID
    .pTGAppMemSpace = (void *)LVM_NULL, // Application instance handle or memory area
#endif                                  /* ALGORITHM_TG */

#ifdef ALGORITHM_PSA
    /* General Control */
    .PSA_Enable = LVM_PSA_OFF, // LVM_PSA_ON or LVM_PSA_OFF;
    /* Spectrum Analyzer parameters */
    .PSA_PeakDecayRate =
        LVM_PSA_SPEED_MEDIUM, // LVM_PSA_SPEED_SLOW or LVM_PSA_SPEED_MEDIUM or LVM_PSA_SPEED_FAST; Peak value decay rate
    .PSA_NumBands = (LVM_UINT16)32, // 6 to 64; Number of Bands
#endif                              /* ALGORITHM_PSA */

#ifdef ALGORITHM_LIMP
    .LIMP_OperatingMode = LVM_LIMP_OFF, // LIMP operating mode: LVM_LIMP_ON or LVM_LIMP_OFF
    .LIMP_Threshold     = -3,           // LIMP threshold in dB: -24dB to 0dB
#endif                                  /* ALGORITHM_LIMP */

#ifdef ALGORITHM_LIMR
    .LIMR_OperatingMode = LVM_LIMR_OFF,       // LVM_LIMR_ON or LVM_LIMR_OFF
    .LIMR_Reference     = LVM_LIMR_REF_0DBFS, // LIMR reference input: LVM_LIMR_REF_INPUT or LVM_LIMR_REF_0DBFS
    .LIMR_Threshold     = -24,                // LIMR threshold in dB: -24dB to 0dB
#endif                                        /* ALGORITHM_LIMR */

#ifdef ALGORITHM_EQNB
    /* N-Band Equalizer parameters */
    .EQNB_OperatingMode   = LVM_EQNB_OFF,                       // LVM_EQNB_ON or LVM_EQNB_OFF;
    .EQNB_LPF_Mode        = LVM_EQNB_FILTER_OFF,                // LVM_EQNB_FILTER_ON or LVM_EQNB_FILTER_OFF;
    .EQNB_LPF_CornerFreq  = (LVM_INT16)2000,                    // EQNB LowPass Corner Frequency;
    .EQNB_HPF_Mode        = LVM_EQNB_FILTER_OFF,                // LVM_EQNB_FILTER_ON or LVM_EQNB_FILTER_OFF;
    .EQNB_HPF_CornerFreq  = (LVM_INT16)20,                      // EQNB HighPass Corner Frequency;
    .EQNB_NBands          = (LVM_UINT16)0,                      // Number of bands 0 to LVM_EQNB_MAX_BANDS_NBR
    .pEQNB_BandDefinition = &EQNB_BandDefs_UserEq1_internal[0], // EQ band configuration
#endif                                                          /* ALGORITHM_EQNB */

#ifdef ALGORITHM_PR_EQNB
    /* N-Band Equalizer parameters */
    .PR_EQNB_OperatingMode   = LVM_EQNB_OFF,                          // LVM_EQNB_ON or LVM_EQNB_OFF;
    .PR_EQNB_LPF_Mode        = LVM_EQNB_FILTER_OFF,                   // LVM_EQNB_FILTER_ON or LVM_EQNB_FILTER_OFF;
    .PR_EQNB_LPF_CornerFreq  = (LVM_INT16)2000,                       // EQNB LowPass Corner Frequency;
    .PR_EQNB_HPF_Mode        = LVM_EQNB_FILTER_OFF,                   // LVM_EQNB_FILTER_ON or LVM_EQNB_FILTER_OFF;
    .PR_EQNB_HPF_CornerFreq  = (LVM_INT16)20,                         // EQNB HighPass Corner Frequency;
    .PR_EQNB_NBands          = (LVM_UINT16)0,                         // Number of bands 0 to MAX_EQNB_BANDS
    .pPR_EQNB_BandDefinition = &EQNB_BandDefs_ProductEq1_internal[0], // EQ band configuration
#endif                                                                /* ALGORITHM_PR_EQNB */

#ifdef ALGORITHM_XO
    .XO_OperatingMode   = LVM_MODE_OFF, // LVM_MODE_ON or LVM_MODE_OFF
    .XO_cutoffFrequency = 100,          // Cutoff frequency in Hz (range  = [60 Hz - 6 000 Hz])
#endif
};
// @formatter:on

static eap_att_code_t normalize_params(void);
static eap_att_code_t set_control_params(void);
static eap_att_code_t get_control_params(LVM_HeadroomParams_t *headroom, LVM_ControlParams_t *control);
static eap_att_code_t update_wrapper(void);

#endif /* EAP_PROC */

static eap_att_code_t play_wrapper(void);
static eap_att_code_t pause_wrapper(void);
static eap_att_code_t resume_wrapper(void);
static eap_att_code_t reset_wrapper(void);
static eap_att_code_t stop_wrapper(void);
static eap_att_code_t set_volume_wrapper(int value);
static eap_att_code_t seek_wrapper(int32_t seek_time);

static eap_att_code_t update(void);
static eap_att_code_t set_volume(int value);
static eap_att_code_t seek(int32_t seek_time);

static void progress(int current, int total);

int attProcessIterator = 0;

eap_att_control_t att_control = {.attVersion      = 4,
                                 .eapVersion      = "0.0.0",
                                 .command         = kAttCmdNone,
                                 .status          = kAttIdle,
                                 .lastError       = 0,
                                 .eapPreset       = 0,
                                 .generatePSA     = 0,
                                 .input           = "",
                                 .trackTotal      = 0,
                                 .trackCurrent    = 0,
                                 .availableInputs = {{0}},
                                 .progress        = &progress,
                                 .update          = &update,
                                 .set_volume      = &set_volume,
                                 .seek            = &seek,
                                 .logme           = printf,
                                 .volume          = 75,
                                 .seek_time       = 0,

#if (defined EAP_PROC || defined EAP32_PROC)
                                 .controlParam     = &ControlParamSet_internal,
                                 .instParams       = &InstParams_internal,
                                 .headroomParams   = &HeadroomParams_internal,
                                 .normalize_params = &normalize_params
#endif
};

eap_att_control_t *get_eap_att_control(void)
{
    return &att_control;
}

void eap_att_process(void)
{
#if (defined EAP_PROC || defined EAP32_PROC)
    if (attProcessIterator++ == 0)
    {
        LVM_VersionInfo_st eapVersionInfo;
        if (LVM_GetVersionInfo(&eapVersionInfo) == LVM_SUCCESS)
        {
            strcpy(att_control.eapVersion, eapVersionInfo.pVersionNumber);
        }
    }
#endif

    switch (att_control.command)
    {
        case kAttCmdNone:
        {
            // some waiting info could be printed here
            break;
        }
        case kAttCmdStart:
        {
            if (att_control.status == kAttPaused)
            {
                att_control.lastError = resume_wrapper();
            }
            else
            {
                if (att_control.status == kAttRunning)
                {
                    att_control.lastError    = stop_wrapper();
                    att_control.trackCurrent = 0;
                    att_control.trackTotal   = 0;
                }
                if (att_control.lastError == kEapAttCodeOk)
                {
                    att_control.lastError = play_wrapper();
                }
            }

            att_control.status = kAttRunning;

            att_control.logme("[EAP_ATT] Playback started for %s\r\n", att_control.input);

            break;
        }
        case kAttCmdStop:
        {
            if (att_control.status == kAttRunning || att_control.status == kAttPaused)
            {
                att_control.lastError    = stop_wrapper();
                att_control.trackCurrent = 0;
                att_control.trackTotal   = 0;
                att_control.status       = kAttIdle;

                att_control.logme("[EAP_ATT] Playback stopped\r\n");
            }
            break;
        }
        case kAttCmdPause:
        {
            if (att_control.status == kAttPaused)
            {
                att_control.lastError = resume_wrapper();
                att_control.status    = kAttRunning;
                att_control.logme("[EAP_ATT] Playback continued\r\n");
            }
            else if (att_control.status == kAttRunning)
            {
                att_control.lastError = pause_wrapper();
                att_control.status    = kAttPaused;
                att_control.logme("[EAP_ATT] Playback paused\r\n");
            }
            break;
        }
        case kAttCmdSeek:
            att_control.lastError = seek_wrapper(att_control.seek_time);
            break;
#if (defined EAP_PROC || defined EAP32_PROC)
        case kAttCmdSetConfig:
        {
            att_control.logme("[EAP_ATT] Set config\r\n");
            att_control.lastError = set_control_params();
            break;
        }
        case kAttCmdGetConfig:
        {
            att_control.logme("[EAP_ATT] Get config\r\n");
            att_control.lastError = get_control_params(0, 0);
            break;
        }
#endif
        case kAttCmdReset:
        {
            /* currently used only in tests */
            att_control.command   = kAttCmdNone;
            att_control.lastError = reset_wrapper();
            while (1)
                ; // block until restarted
            break;
        }
        case kAttCmdVolume:
        {
            if (att_control.status == kAttRunning || att_control.status == kAttPaused)
            {
                att_control.lastError = set_volume_wrapper(att_control.volume);
            }
            else
            {
                att_control.logme("[EAP_ATT] First, play an audio file.\r\n");
            }
            break;
        }
        default:
        {
            att_control.logme("[EAP_ATT] Error: Undefined command has been detected: %d\r\n", att_control.command);
            att_control.lastError = kEapAttCodeUndefinedCommand;
        }
    }

    if (att_control.lastError != kEapAttCodeOk && att_control.status != kAttError)
    {
        att_control.logme("[EAP_ATT] Error occurred %d for command %d\r\n", att_control.lastError, att_control.command);
        if (att_control.status == kAttRunning)
        {
            att_control.logme("[EAP_ATT] Error occurred, trying to stop...\r\n");
            stop_wrapper();
        }
        att_control.status = kAttError;
    }

    att_control.command = kAttCmdNone;
}

static void progress(int current, int total)
{
    att_control.trackCurrent = current;
    att_control.trackTotal   = total;
}

/* wrap functions by safe invoker */
static eap_att_code_t play_wrapper(void)
{
    if (att_control.play != NULL)
    {
        return (*att_control.play)();
    }
    return kEapAttCodeMissingHandler;
}

static eap_att_code_t pause_wrapper(void)
{
    if (att_control.pause != NULL)
    {
        return (*att_control.pause)();
    }
    return kEapAttCodeMissingHandler;
}

static eap_att_code_t resume_wrapper(void)
{
    if (att_control.resume != NULL)
    {
        return (*att_control.resume)();
    }
    return kEapAttCodeMissingHandler;
}

static eap_att_code_t reset_wrapper(void)
{
    if (att_control.reset != NULL)
    {
        return (*att_control.reset)();
    }
    return kEapAttCodeMissingHandler;
}

static eap_att_code_t stop_wrapper(void)
{
    if (att_control.stop != NULL)
    {
        return (*att_control.stop)();
    }
    return kEapAttCodeMissingHandler;
}

static eap_att_code_t seek_wrapper(int32_t seek_time)
{
    if (att_control.seek != NULL)
    {
        return (*att_control.seek)(seek_time);
    }
    return kEapAttCodeMissingHandler;
}

eap_att_code_t seek(int32_t seek_time)
{
    return kEapAttCodeOk; // let implementation on user if needed
}

static eap_att_code_t update(void)
{
    return kEapAttCodeOk; // let implementation on user if needed
}

static eap_att_code_t set_volume_wrapper(int volume)
{
    if (att_control.set_volume != NULL)
    {
        return (*att_control.set_volume)(volume);
    }
    return kEapAttCodeMissingHandler;
}

eap_att_code_t set_volume(int value)
{
    return kEapAttCodeOk; // let implementation on user if needed
}

#if (defined EAP_PROC || defined EAP32_PROC)

static eap_att_code_t update_wrapper(void)
{
    if (att_control.update != NULL)
    {
        return (*att_control.update)();
    }
    return kEapAttCodeMissingHandler;
}

void eap_att_register_handle(LVM_Handle_t *handle)
{
    LVM_VersionInfo_st eapVersionInfo;
    if (LVM_GetVersionInfo(&eapVersionInfo) == LVM_SUCCESS)
    {
        strcpy(att_control.eapVersion, eapVersionInfo.pVersionNumber);
    }

    att_control.logme("[EAP_ATT] Handle registered into ATT control (v%d, eap: %s)\r\n", att_control.attVersion,
                      att_control.eapVersion);
    att_control.handle = handle;
}

static eap_att_code_t normalize_params(void)
{
    LVM_ControlParams_t *cp = att_control.controlParam;
#ifdef ALGORITHM_EQNB
    for (cp->EQNB_NBands = 0; cp->EQNB_NBands < LVM_EQNB_MAX_BANDS_NBR; cp->EQNB_NBands++)
    {
        if (cp->pEQNB_BandDefinition[cp->EQNB_NBands].Frequency == 0)
        {
            break;
        }
    }
    LVM_HeadroomParams_t *hp = att_control.headroomParams;
    for (hp->NHeadroomBands = 0; hp->NHeadroomBands < LVM_EQNB_MAX_BANDS_NBR; hp->NHeadroomBands++)
    {
        if (hp->pHeadroomDefinition[hp->NHeadroomBands].Limit_Low == 0)
        {
            break;
        }
    }
#endif
#ifdef ALGORITHM_PR_EQNB
    for (cp->PR_EQNB_NBands = 0; cp->PR_EQNB_NBands < LVM_EQNB_MAX_BANDS_NBR; cp->PR_EQNB_NBands++)
    {
        if (cp->pPR_EQNB_BandDefinition[cp->PR_EQNB_NBands].Frequency == 0)
        {
            break;
        }
    }
#endif
    return kEapAttCodeOk;
}

static eap_att_code_t normalize_params_wrapper(void)
{
    if (att_control.normalize_params != NULL)
    {
        return (*att_control.normalize_params)();
    }
    return kEapAttCodeMissingHandler;
}

/* internal */
static eap_att_code_t set_control_params(void)
{
    int status = LVM_SUCCESS;

    if (att_control.handle != 0)
    {
        // pointer has to be initialized because memcpy during get will fail (looks like caused by get LVM param);
        // use {} initializer will be good solution but only for gcc|clang but fail during compilation by msvc,
        // thus using this old-school init
        LVM_HeadroomParams_t headroom;
        LVM_ControlParams_t control;
        LVM_HeadroomBandDef_t HeadroomBandDef_placeholder[LVM_HEADROOM_MAX_NBANDS] = {0};
        LVM_EQNB_BandDef_t EQNB_BandDef_placeholder[LVM_EQNB_MAX_BANDS_NBR]        = {0};
        LVM_EQNB_BandDef_t PR_EQNB_BandDef_placeholder[LVM_EQNB_MAX_BANDS_NBR]     = {0};
        headroom.pHeadroomDefinition                                               = HeadroomBandDef_placeholder;
        control.pEQNB_BandDefinition                                               = EQNB_BandDef_placeholder;
        control.pPR_EQNB_BandDefinition                                            = PR_EQNB_BandDef_placeholder;

        eap_att_code_t code = get_control_params(&headroom, &control);
        if (code != kEapAttCodeOk)
        {
            att_control.logme("[EAP_ATT] Parameters write failed in getting current params phase: %d\r\n", code);
            return code;
        }

        code = normalize_params_wrapper();
        if (code != kEapAttCodeOk)
        {
            att_control.logme("[EAP_ATT] Parameters normalization failed: %d\r\n", code);
            return code;
        }

#ifdef ALGORITHM_EQNB
        // headroomParams restricted parameters filter could be here
        status = LVM_SetHeadroomParams(att_control.handle, att_control.headroomParams);
        if (status != LVM_SUCCESS)
        {
            att_control.logme("[EAP_ATT] Headroom parameters write failed: %d\r\n", status);
            return kEapAttCodeHeadroomSetFailed;
        }
#endif
        // controlParams restricted parameters filter
        if (control.SampleRate != att_control.controlParam->SampleRate ||
            control.SourceFormat != att_control.controlParam->SourceFormat)
        {
            att_control.controlParam->SampleRate   = control.SampleRate;
            att_control.controlParam->SourceFormat = control.SourceFormat;
            att_control.logme(
                "[EAP_ATT] SampleRate or SourceFormat not matching, replacing by active one from EAP config.\r\n");
        }
        status = LVM_SetControlParameters(att_control.handle, att_control.controlParam);

        if (status != LVM_SUCCESS)
        {
            att_control.logme("[EAP_ATT] Control parameters write failed: %d\r\n", status);
            return kEapAttCodeControlParamSetFailed;
        }

        att_control.logme("[EAP_ATT] Parameters written.\r\n");
    }
    else
    {
        att_control.logme("[EAP_ATT] Parameters write skipped. EAP instance not registered or is null.\r\n");
    }
    return update_wrapper();
}

static eap_att_code_t get_control_params(LVM_HeadroomParams_t *headroom, LVM_ControlParams_t *control)
{
    int status = LVM_SUCCESS;
    if (att_control.handle != 0)
    {
        LVM_ControlParams_t *controlDest = control;
        if (control == 0)
        {
            controlDest = att_control.controlParam;
        }

#ifdef ALGORITHM_EQNB
        LVM_HeadroomParams_t *headroomDest = headroom;
        if (headroom == 0)
        {
            headroomDest = att_control.headroomParams;
        }
        LVM_HeadroomBandDef_t *headroomPtrBackup = headroomDest->pHeadroomDefinition;

        status = LVM_GetHeadroomParams(att_control.handle, headroomDest);
        if (status != LVM_SUCCESS)
        {
            att_control.logme("[EAP_ATT] Headroom parameters read failed: %d\r\n", status);
            return kEapAttCodeHeadroomGetFailed;
        }
        if (headroomPtrBackup != headroomDest->pHeadroomDefinition)
        {
            // copy current headroom band definition if array is on different pointer than internal definition
            memcpy(headroomPtrBackup, headroomDest->pHeadroomDefinition,
                   sizeof(*headroomDest->pHeadroomDefinition) * headroomDest->NHeadroomBands);
            headroomDest->pHeadroomDefinition = headroomPtrBackup;
        }
        LVM_EQNB_BandDef_t *eqnbPtrBackup = controlDest->pEQNB_BandDefinition;
#endif
#ifdef ALGORITHM_PR_EQNB
        LVM_EQNB_BandDef_t *prodPtrBackup = controlDest->pPR_EQNB_BandDefinition;
#endif
        status = LVM_GetControlParameters(att_control.handle, controlDest);

        if (status != LVM_SUCCESS)
        {
            att_control.logme("[EAP_ATT] Control parameters read failed: %d\r\n", status);
            return kEapAttCodeControlParamGetFailed;
        }
#ifdef ALGORITHM_EQNB
        if (eqnbPtrBackup != controlDest->pEQNB_BandDefinition)
        {
            memcpy(eqnbPtrBackup, controlDest->pEQNB_BandDefinition,
                   sizeof(*controlDest->pEQNB_BandDefinition) * controlDest->EQNB_NBands);
            controlDest->pEQNB_BandDefinition = eqnbPtrBackup;
        }
#endif
#ifdef ALGORITHM_PR_EQNB
        if (prodPtrBackup != controlDest->pPR_EQNB_BandDefinition)
        {
            memcpy(prodPtrBackup, controlDest->pPR_EQNB_BandDefinition,
                   sizeof(*controlDest->pPR_EQNB_BandDefinition) * controlDest->PR_EQNB_NBands);
            controlDest->pPR_EQNB_BandDefinition = prodPtrBackup;
        }
#endif
        att_control.logme("[EAP_ATT] Parameters synchronized.\r\n");
    }
    else
    {
        att_control.logme("[EAP_ATT] Parameters read skipped. EAP instance not registered or is null.\r\n");
    }
    return kEapAttCodeOk;
}
#endif /* EAP_PROC */
