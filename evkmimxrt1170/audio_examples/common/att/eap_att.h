/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license for this file can be found in the LICENSE.txt file included with this distribution
 * or at https://spdx.org/licenses/BSD-3-Clause.html#licenseText
 */

/**
 * Please read notes inside this file to get all needed information to embed the EAP ATT driver into custom application.
 *
 * This driver contains state machine which could be linked with play/pause/stop/.. features inside your code to obtain
 * full control from Audio Tuning Tool. If these features not needed on your side then all handlers (i.e.
 * eap_att_control_t.play|pause|resume|..) should be defined as function which return kEapAttCodeOk.
 *
 * In the case that custom application has its own LVM params definitions (not using
 * eap_att_control_t.<inst|headroom|control>Params) then at least LVM_Handle_t has to be registered (use
 * eap_att_register_handle(..) function) and eap_att_process() should be polled periodically (note: this causes that
 * Audio Tuning Tool can call LVM set/get methods when needed - during push/pull from UI; for more info please see
 * enclosed eap_att.c at the very end of this file is part marked as internal).
 *
 * Your application can also maintain parameters synchronization itself. In this situation (also without audio stream
 * control features) the eap_att_process() is not necessary and Audio Tuning Tool will only read/write data in original
 * reference at eap_att_control_t.<inst|headroom|control>Params. WARNING: Be sure that your data are stored inside
 * original destination of these pointers (internal structures in eap_att.c) because communication with this driver now
 * uses memory read/write over debugger controlled by FreeMASTER so this communication channel operates directly with
 * these internal structures. In other words, if you replace eap_att_control_t.<inst|headroom|control>Params with
 * another pointer then these new structure references could ignored (use memcpy(..) instead)!!
 *
 * And finally: If you do not need any of mentioned features and you have your own
 * LVM_HeadroomParams_t|LVM_InstParams_t| ControlParamSet_internal structures inside your application then you can use
 * Advanced tuning profile in Audio Tuning Tool with custom structure name. This has several requirements:
 *  - do not add eap_att.h in to compiler setup (if the internal eap_att_control_t attControl structure will be found in
 * ELF then Audio Tuning Tool can forbid this mode of tuning)
 *  - define LVM structures in main context (has to be found in ELF so definitions i.e. in function block will not work)
 * which has to match this patterns (<type> <prefix><BOARD-SOLUTION-NAME-IN-ATT>):
 *      - LVM_EQNB_BandDef_t EQNB_BandDefs_UserEq1_<BOARD-SOLUTION-NAME-IN-ATT>
 *      - LVM_EQNB_BandDef_t EQNB_BandDefs_ProductEq1_<BOARD-SOLUTION-NAME-IN-ATT>
 *      - LVM_InstParams_t InstParams_<BOARD-SOLUTION-NAME-IN-ATT>
 *      - LVM_HeadroomBandDef_t HeadroomBandDef_<BOARD-SOLUTION-NAME-IN-ATT>
 *      - LVM_HeadroomParams_t HeadroomParams_<BOARD-SOLUTION-NAME-IN-ATT>
 *      - LVM_ControlParams_t ControlParamSet_<BOARD-SOLUTION-NAME-IN-ATT>
 */
#ifndef _EAP_ATT_H_
#define _EAP_ATT_H_

#if defined EAP_PROC
#include <EAP16.h>
#elif defined EAP32_PROC
#include <EAP32.h>
#endif
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_FILES_LIST
#define MAX_FILES_LIST 16
#endif

#ifndef MAX_FILE_NAME_LENGTH
#define MAX_FILE_NAME_LENGTH 32
#endif

enum
{
    kEapAttCodeOk                    = 0, /*!< No problem */
    kEapAttCodeError                 = 1, /*!< Status for generic error */
    kEapAttCodeMissingHandler        = 2, /*!< Missing EAP ATT control handler definition */
    kEapAttCodeUndefinedCommand      = 3, /*!< Undefined ATT command was been found requested */
    kEapAttCodeNotImplemented        = 4, /*!< Not implemented handler */
    kEapAttCodeStreamCreateFailed    = 5, /*!< Audio stream can not be created */
    kEapAttCodeStreamControlFailure  = 6, /*!< Audio stream control issue (play/pause/resume/stop) */
    kEapAttCodeHeadroomSetFailed     = 7, /*!< Headroom params write failed  */
    kEapAttCodeControlParamSetFailed = 8, /*!< ControlParams write failed */
    kEapAttCodeHeadroomGetFailed     = 9, /*!< Headroom params read failed */
    kEapAttCodeControlParamGetFailed = 10 /*!< ControlParams read failed */
};

typedef int eap_att_code_t;

/* EAP ATT state machine control commands */
typedef enum _eap_att_command
{
    kAttCmdNone      = 0, /*!< default for no action */
    kAttCmdStart     = 1, /*!< starts application and then goes to running */
    kAttCmdStop      = 2, /*!< stops application and then goes to idle */
    kAttCmdPause     = 3, /*!< toggle (pause/continue) audio playing */
    kAttCmdSetConfig = 4, /*!< save EAP parameters into LVM library */
    kAttCmdGetConfig = 5, /*!< load actual EAP parameters from LVM library */
    kAttCmdReset     = 6, /*!< stops application and starts again with last values
         (also critical properties like sampling frequency could be changed by this)*/
    kAttCmdVolume = 7,    /*!< apply current volume */
    kAttCmdSeek   = 8     /*!< seek current paused track */
} eap_att_command_t;

/* EAP ATT state machine statuses */
typedef enum _eap_att_status
{
    kAttIdle    = 0, /*!< application is idle */
    kAttRunning = 1, /*!< application is running */
    kAttPaused  = 2, /*!< application is running with paused playback */
    kAttError   = -1 /*!< signals any kind of error, read error message and restart*/
} eap_att_status_t;

/* EAP ATT control structure */
typedef struct _eap_att_control
{
    int attVersion;      /* ATT driver version, do not modify! */
    char eapVersion[16]; /* EAP library version. Read during handle registration. */

    int isLocked;
    eap_att_status_t status;
    eap_att_command_t command; /*!< this should goes to default value (kAttCmdNone=0) immediately
     after processed by observing loop */
    eap_att_code_t lastError;
    int generatePSA; /* force turn on PSA data generation */

    char input[MAX_FILE_NAME_LENGTH]; /* Set default input on startup or retrieve current value modified in ATT UI. */
    char availableInputs[MAX_FILES_LIST][MAX_FILE_NAME_LENGTH]; /* Fill available inputs data if wanted. */

    int trackTotal;   // audio track duration in [ms]
    int trackCurrent; // audio track actual time in [ms]

    // preset definition, '0' is reserved for default, used as optional storage or by tests
    char eapPreset;

    int volume;        // Volume in range 0-100%, 0 is muted
    int32_t seek_time; // Seek time

    // control handlers
    eap_att_code_t (*play)(void);   /* Register playback start function. */
    eap_att_code_t (*pause)(void);  /* Pause audio stream and save current position. */
    eap_att_code_t (*resume)(void); /* Continue playback of audio stream which was previously paused. */
    eap_att_code_t (*reset)(void);  /* Optional handler for master reset. (currently used only in tests) */
    eap_att_code_t (*stop)(void);   /* Stop audio stream. This is called i.e. before play() for new audio input. */
    eap_att_code_t (*seek)(int32_t seek_time); /* Seek current paused track. */
    eap_att_code_t (*destroy)(void); /* Destroy audio stream. Return kEapAttCodeOk if this behavior is not needed. */
    void (*progress)(int current, int total); /* Call periodically on audio stream progress changed. */
    eap_att_code_t (*set_volume)(int value);  /* Handler for volume control */

    // advanced overrides
    eap_att_code_t (*update)(void);       /* This is called when EAP config structures were changed by the tool. */
    int (*logme)(const char *fmt_s, ...); /* This function is mapped to stdio::printf() by default. */

#if (defined EAP_PROC || defined EAP32_PROC)
    eap_att_code_t (*normalize_params)(void); /* Normalizes params definition structures i.e. bands elements count. */

    // EAP references
    LVM_Handle_t handle;
    LVM_InstParams_t *instParams;
    LVM_HeadroomParams_t *headroomParams;
    LVM_ControlParams_t *controlParam;
#endif
} eap_att_control_t;

/*
 * This function is main accessor to internal control structure singleton. Please use this where needed.
 * @return Returns pointer to ATT control structure singleton.
 */
eap_att_control_t *get_eap_att_control(void);

/*
 * Main EAP ATT state machine process method. Should be called periodically.
 * Selected period defines how fast will be reaction on commands from Audio Tuning Tool.
 * Recommended period is between 1-100ms (higher values can leeds into too slow reaction).
 */
void eap_att_process(void);

#if (defined EAP_PROC || defined EAP32_PROC)
/*
 * Register the LVM handle into EAP ATT control structure when this handle will be available.
 * Be sure that handle is registered at least immediately after eap_att_control_t.play() handler will be called.
 * Otherwise, an automatic sync of the LVM parameters into LVM internals will not works properly.
 */
void eap_att_register_handle(LVM_Handle_t *handle);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _EAP_ATT_H_ */
