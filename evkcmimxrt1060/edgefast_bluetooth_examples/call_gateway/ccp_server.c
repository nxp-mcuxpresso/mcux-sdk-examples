/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/types.h>
#include <stdio.h>
#include <stddef.h>
#include <errno/errno.h>
#include <toolchain.h>
#include <porting.h>

#include <bluetooth/audio/tbs.h>
#include <bluetooth/audio/audio.h>

#include "fsl_debug_console.h"

#include "fsl_shell.h"

#include "ringtone.h"

#include "unicast_audio_client.h"

#include "ccp_server.h"

#ifndef CCP_SERVER_RESTART_STREAM
#define CCP_SERVER_RESTART_STREAM 0
#endif /* CCP_SERVER_RESTART_STREAM */

static shell_handle_t s_shellHandle;

volatile uint8_t s_callIndex;

static struct bt_conn *default_conn;

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
};

static OSA_SEMAPHORE_HANDLE_DEFINE(semHandle);

enum
{
    KSTATUS_APP_CPP_SERVER_ORIGINATE_CALL = 1,
    KSTATUS_APP_CPP_SERVER_TERMINATE_CALL = 2,
    KSTATUS_APP_CPP_SERVER_JOINS_CALL = 3,
    KSTATUS_APP_CPP_SERVER_HOLD = 4,
    KSTATUS_APP_CPP_SERVER_ACCEPT = 5,
    KSTATUS_APP_CPP_SERVER_RETRIEVE = 6,
};

static volatile uint8_t commander_is_ready = 0;

static volatile uint8_t s_call_status;

extern int BOARD_StopVoice(void);
extern int BOARD_StartVoice(uint32_t simpleBitRate, uint32_t simpleBits);
extern int BOARD_StopRingTone(void);
extern int BOARD_StartRingTone(void);
extern int BOARD_VolSet(uint32_t vol);
extern int BOARD_VolUp(void);
extern int BOARD_VolDown(void);
extern int BOARD_SpeakerMute(void);
extern int BOARD_SpeakerUnmute(void);

static bool app_bt_tbs_originate_call_cb(struct bt_conn *conn,
                     uint8_t call_index,
                     const char *uri);

static void app_bt_tbs_terminate_call_cb(struct bt_conn *conn,
                     uint8_t call_index,
                     uint8_t reason);

static void app_bt_tbs_join_calls_cb(struct bt_conn *conn,
                     uint8_t call_index_count,
                     const uint8_t *call_indexes);

static void app_bt_tbs_call_hold_change_cb(struct bt_conn *conn,
                      uint8_t call_index);

static void app_bt_tbs_call_accept_change_cb(struct bt_conn *conn,
                      uint8_t call_index);

static void app_bt_tbs_call_retrieve_change_cb(struct bt_conn *conn,
                      uint8_t call_index);

static bool app_bt_tbs_authorize_cb(struct bt_conn *conn);

static shell_status_t vol_set(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t vol_up(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t vol_down(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t vol_mute(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t vol_unmute(shell_handle_t shellHandle, int32_t argc, char **argv);

static shell_status_t call_outgoing(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t call_term(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t call_accept(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t call_hold(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t call_retrieve(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t call_join(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t remote_call_incoming(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t remote_call_answer(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t remote_call_term(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t remote_call_hold(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t remote_call_retrieve(shell_handle_t shellHandle, int32_t argc, char **argv);

struct bt_tbs_cb app_tbs_cb = {
    app_bt_tbs_originate_call_cb,
    app_bt_tbs_terminate_call_cb,
    app_bt_tbs_call_hold_change_cb,
    app_bt_tbs_call_accept_change_cb,
    app_bt_tbs_call_retrieve_change_cb,
    app_bt_tbs_join_calls_cb,
    app_bt_tbs_authorize_cb,
};

static int start_ringtone(void);
static int stop_ringtone_light(void);
static int start_voice_light(void);
static int stop_call(void);
static int hold_call(void);
static int retrieve_call(void);

static int start_ringtone(void)
{
    int err = 0;

    if (commander_is_ready == 0)
    {
        err = -1;
        PRINTF("The unicast client is not ready.\n");
    }

    if (err >= 0)
    {
        err = unicast_client_retrieve();
    }

    if (err >= 0)
    {
        err = unicast_client_configure_streams();
    }

    if (err >= 0)
    {
        err = unicast_client_create_group();
    }

    if (err >= 0)
    {
        err = unicast_client_set_stream_qos();
    }

    if (err >= 0)
    {
        err = unicast_client_disable_streams();
    }

#if 0
    unicast_client_enable_stream_unidirectional(1, BT_AUDIO_CONTEXT_TYPE_RINGTONE);
    unicast_client_start_stream_unidirectional(1);
#else
    if (err >= 0)
    {
        err = unicast_client_enable_streams(BT_AUDIO_CONTEXT_TYPE_RINGTONE, BT_AUDIO_CONTEXT_TYPE_RINGTONE);
    }

    if (err >= 0)
    {
        err = unicast_client_start_streams();
    }
#endif
    if (err >= 0)
    {
        err = unicast_client_start_ringtone(music, sizeof(music));
    }

    if (err < 0)
    {
        stop_call();
    }

    return err;
}

static int stop_ringtone_light(void)
{
    unicast_client_stop_ringtone();
#if CCP_SERVER_RESTART_STREAM
    unicast_client_disable_streams();
#endif
    return 0;
}

static int start_voice_light(void)
{
    int err = 0;

#if CCP_SERVER_RESTART_STREAM
    if (err >= 0)
    {
        err = unicast_client_disable_streams();
    }
#endif

#if 0
    unicast_client_enable_stream_unidirectional(1, BT_AUDIO_CONTEXT_TYPE_RINGTONE);
    unicast_client_start_stream_unidirectional(1);
#else
#if CCP_SERVER_RESTART_STREAM
    if (err >= 0)
    {
        err = unicast_client_enable_streams(BT_AUDIO_CONTEXT_TYPE_CONVERSATIONAL, BT_AUDIO_CONTEXT_TYPE_CONVERSATIONAL);
    }

    if (err >= 0)
    {
        err = unicast_client_start_streams();
    }
#else
    if (err >= 0)
    {
        err = unicast_client_metadata(BT_AUDIO_CONTEXT_TYPE_CONVERSATIONAL, BT_AUDIO_CONTEXT_TYPE_CONVERSATIONAL);
    }
#endif
#endif
    if (err >= 0)
    {
        err = unicast_client_start_voice();
    }

    if (err < 0)
    {
        stop_call();
    }

    return err;
}

static int stop_call(void)
{
    unicast_client_stop_voice();
    unicast_client_stop_ringtone();
    unicast_client_disable_streams();
    unicast_client_release_streams();
    unicast_client_delete_group();
    return 0;
}

static int hold_call(void)
{
    unicast_client_hold();
    return 0;
}

static int retrieve_call(void)
{
    unicast_client_retrieve();
    return 0;
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    default_conn = conn;
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    if (s_callIndex > 0U)
    {
        unicast_client_stop_voice();
        unicast_client_stop_ringtone();
        bt_tbs_terminate(s_callIndex);
        s_callIndex = 0U;
    }

    default_conn = NULL;
    call_server_disable_command();
}

/* TBS callback */
static bool app_bt_tbs_originate_call_cb(struct bt_conn *conn,
                     uint8_t call_index,
                     const char *uri)
{
    uint32_t reg_mask;
#if 0
    int err;
#endif

    PRINTF("Start a outgoing call, call index %d, callee %s\r\n", call_index, uri);
    /* TODO: Start a new outgoing call */
    /* Check whether the new outgoing call could be started */
    /* Start the call */
    /* start the ringtone */
    /* case 1: call has been accepted, notify client */
    /* case 2: call has been terminated, notify client */
    /* Accept the call */
    if ((s_callIndex == 0))
    {
        s_callIndex = call_index;
        /* TODO: */
        reg_mask = DisableGlobalIRQ();
        s_call_status = KSTATUS_APP_CPP_SERVER_ORIGINATE_CALL;
        EnableGlobalIRQ(reg_mask);
        (void)OSA_SemaphorePost((osa_semaphore_handle_t)semHandle);
#if 0
        err = start_ringtone();
        if (err < 0)
        {
            PRINTF("Cannot start ringtone stream, will term the call %d\r\n", call_index);
            bt_tbs_terminate(call_index);
        }
#endif
    }
    return true;
}

static void app_bt_tbs_terminate_call_cb(struct bt_conn *conn,
                     uint8_t call_index,
                     uint8_t reason)
{
    uint32_t reg_mask;
    PRINTF("Terminate a call, call index %d, reason %d\r\n", call_index, reason);
    /* TODO: Terminate a call */
    if (s_callIndex == call_index)
    {
        s_callIndex = 0U;
        /* TODO: */
        reg_mask = DisableGlobalIRQ();
        s_call_status = KSTATUS_APP_CPP_SERVER_TERMINATE_CALL;
        EnableGlobalIRQ(reg_mask);
        (void)OSA_SemaphorePost((osa_semaphore_handle_t)semHandle);
#if 0
        stop_call();
#endif
    }
}

static void app_bt_tbs_join_calls_cb(struct bt_conn *conn,
                     uint8_t call_index_count,
                     const uint8_t *call_indexes)
{
    uint32_t reg_mask;
    PRINTF("Join the following calls (count %d): ", call_index_count);
    for (int i =0; i < call_index_count;i ++)
    {
        PRINTF("%d", call_indexes[i]);
        if ((call_index_count > 0) && (i < (call_index_count-1)))
        {
            PRINTF(", ");
        }
    }
    PRINTF("\r\n");
    /* TODO: Join the calls */
    reg_mask = DisableGlobalIRQ();
    s_call_status = KSTATUS_APP_CPP_SERVER_JOINS_CALL;
    EnableGlobalIRQ(reg_mask);
    (void)OSA_SemaphorePost((osa_semaphore_handle_t)semHandle);
}

static void app_bt_tbs_call_hold_change_cb(struct bt_conn *conn,
                      uint8_t call_index)
{
    uint32_t reg_mask;
    PRINTF("Hold a call, call index %d\r\n", call_index);
    /* TODO: Hold a call */
    if (s_callIndex == call_index)
    {
        /* TODO: */
        reg_mask = DisableGlobalIRQ();
        s_call_status = KSTATUS_APP_CPP_SERVER_HOLD;
        EnableGlobalIRQ(reg_mask);
        (void)OSA_SemaphorePost((osa_semaphore_handle_t)semHandle);
    }
}

static void app_bt_tbs_call_accept_change_cb(struct bt_conn *conn,
                      uint8_t call_index)
{
    uint32_t reg_mask;
#if 0
    int err;
#endif

    PRINTF("Accept a call, call index %d\r\n", call_index);
    /* TODO: Accept a call */
    if (s_callIndex == call_index)
    {
        /* TODO: */
        reg_mask = DisableGlobalIRQ();
        s_call_status = KSTATUS_APP_CPP_SERVER_ACCEPT;
        EnableGlobalIRQ(reg_mask);
        (void)OSA_SemaphorePost((osa_semaphore_handle_t)semHandle);
#if 0
        stop_ringtone_light();
        err = start_voice_light();

        if (0 > err)
        {
            PRINTF("Cannot start voice stream, will term the call %d\r\n", call_index);
            bt_tbs_terminate(call_index);
        }
#endif
    }
}

static void app_bt_tbs_call_retrieve_change_cb(struct bt_conn *conn,
                      uint8_t call_index)
{
    uint32_t reg_mask;
    PRINTF("Retrieve a call, call index %d\r\n", call_index);
    /* TODO: Retrieve a call */
    if (s_callIndex == call_index)
    {
        /* TODO: */
        reg_mask = DisableGlobalIRQ();
        s_call_status = KSTATUS_APP_CPP_SERVER_RETRIEVE;
        EnableGlobalIRQ(reg_mask);
        (void)OSA_SemaphorePost((osa_semaphore_handle_t)semHandle);
    }
}

static bool app_bt_tbs_authorize_cb(struct bt_conn *conn)
{
    return true;
}

/* Shell commander handler */
static shell_status_t vol_set(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int value = 0;

    for (int i = 0;i < strlen(argv[1]);i++)
    {
        if ('-' == argv[1][i])
        {
            SHELL_Printf(s_shellHandle, "invalid parameter\n");
            return kStatus_SHELL_Error;
        }
        if (('0' > argv[1][i]) || ('9' < argv[1][i]))
        {
            SHELL_Printf(s_shellHandle, "invalid parameter\n");
            return kStatus_SHELL_Error;
        }
    }

    value = (int)atoi(argv[1]);

     int ret = BOARD_VolSet((uint32_t)value);

    if(ret)
    {
        SHELL_Printf(s_shellHandle, "vol set error %d\n", ret);
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t vol_up(shell_handle_t shellHandle, int32_t argc, char **argv)
{
     int ret = BOARD_VolUp();

    if(ret)
    {
        SHELL_Printf(s_shellHandle, "vol up error %d\n", ret);
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t vol_down(shell_handle_t shellHandle, int32_t argc, char **argv)
{
     int ret = BOARD_VolDown();

    if(ret)
    {
        SHELL_Printf(s_shellHandle, "vol down error %d\n", ret);
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t vol_mute(shell_handle_t shellHandle, int32_t argc, char **argv)
{
     int ret = BOARD_SpeakerMute();

    if(ret)
    {
        SHELL_Printf(s_shellHandle, "vol mute error %d\n", ret);
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t vol_unmute(shell_handle_t shellHandle, int32_t argc, char **argv)
{
     int ret = BOARD_SpeakerUnmute();

    if(ret)
    {
        SHELL_Printf(s_shellHandle, "vol unmute error %d\n", ret);
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t call_outgoing(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char *bearer_string = NULL;
    char *callee_uri = NULL;
    long bearer = 0;
    int ret = 0;
    uint8_t call_index = 0;

    if (argc < 3)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "The Connection is broken\r\n");
        return kStatus_SHELL_Error;
    }

    bearer_string = argv[1];
    callee_uri = argv[2];

    bearer = strtol(bearer_string, NULL, 0);

    if ((bearer < 0) || (bearer >= CONFIG_BT_TBS_BEARER_COUNT))
    {
        SHELL_Printf(s_shellHandle, "Invalid bearer index (%ld)\r\n", bearer);
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(s_shellHandle, "outgoing call: callee uri %s\r\n", callee_uri);

    ret = bt_tbs_originate((uint8_t)bearer, callee_uri, &call_index);
    if ((ret >= 0) && (s_callIndex == 0))
    {
        s_callIndex = call_index;
        /* TODO: */
        ret = start_ringtone();

        if (0 > ret)
        {
            SHELL_Printf(s_shellHandle, "Cannot start tingtone stream, will term the call %d\r\n", call_index);
            bt_tbs_terminate(call_index);
        }
    }

    SHELL_Printf(s_shellHandle, "Return code %d, call index is %d\r\n", ret, call_index);

    return kStatus_SHELL_Success;
}

static shell_status_t call_term(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    long call_index = 0;
    char *call_index_string = NULL;
    int ret = 0;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "The Connection is broken\r\n");
        return kStatus_SHELL_Error;
    }

    call_index_string = argv[1];

    call_index = strtol(call_index_string, NULL, 0);

    if (0 == call_index)
    {
        SHELL_Printf(s_shellHandle, "Invalid call index (%ld)\r\n", call_index);
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(s_shellHandle, "terminate the call: call index %ld\r\n", call_index);

    ret = bt_tbs_terminate(call_index);

    if ((ret == 0) && (s_callIndex == call_index))
    {
        s_callIndex = 0U;
        /* TODO: */
        stop_call();
    }
    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);
    return kStatus_SHELL_Success;
}

static shell_status_t call_accept(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    long call_index = 0;
    char *call_index_string = NULL;
    int ret = 0;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "The Connection is broken\r\n");
        return kStatus_SHELL_Error;
    }

    call_index_string = argv[1];

    call_index = strtol(call_index_string, NULL, 0);

    if (0 == call_index)
    {
        SHELL_Printf(s_shellHandle, "Invalid call index (%ld)\r\n", call_index);
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(s_shellHandle, "accept the call: call index %ld\r\n", call_index);

    ret = bt_tbs_accept(call_index);
    if (0 == ret)
    {
        if (s_callIndex == call_index)
        {
            /* TODO: */
            stop_ringtone_light();
            ret = start_voice_light();

            if (0 > ret)
            {
                SHELL_Printf(s_shellHandle, "Cannot start voice stream, will term the call %d\r\n", call_index);
                bt_tbs_terminate(call_index);
            }
        }
    }

    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);
    return kStatus_SHELL_Success;
}

static shell_status_t call_hold(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    long call_index = 0;
    char *call_index_string = NULL;
    int ret = 0;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "The Connection is broken\r\n");
        return kStatus_SHELL_Error;
    }

    call_index_string = argv[1];

    call_index = strtol(call_index_string, NULL, 0);

    if (0 == call_index)
    {
        SHELL_Printf(s_shellHandle, "Invalid call index (%ld)\r\n", call_index);
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(s_shellHandle, "hold the call: call index %ld\r\n", call_index);

    ret = bt_tbs_hold(call_index);
    if ((ret == 0) && (s_callIndex == call_index))
    {
        /* TODO: */
        hold_call();
    }
    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);
    return kStatus_SHELL_Success;
}

static shell_status_t call_retrieve(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    long call_index = 0;
    char *call_index_string = NULL;
    int ret = 0;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "The Connection is broken\r\n");
        return kStatus_SHELL_Error;
    }

    call_index_string = argv[1];

    call_index = strtol(call_index_string, NULL, 0);

    if (0 == call_index)
    {
        SHELL_Printf(s_shellHandle, "Invalid call index (%ld)\r\n", call_index);
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(s_shellHandle, "retrieve the call: call index %ld\r\n", call_index);

    ret = bt_tbs_retrieve(call_index);
    if ((ret == 0) && (s_callIndex == call_index))
    {
        /* TODO: */
        retrieve_call();
    }
    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);
    return kStatus_SHELL_Success;
}

static shell_status_t call_join(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int ret;
    uint8_t call_index_list[CONFIG_BT_TBS_MAX_CALLS];

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "The Connection is broken\r\n");
        return kStatus_SHELL_Error;
    }

    if (argc > (CONFIG_BT_TBS_MAX_CALLS + 1))
    {
        SHELL_Printf(s_shellHandle, "Supported max call count is %d\r\n", CONFIG_BT_TBS_MAX_CALLS);
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(s_shellHandle, "Jion calls\r\n", CONFIG_BT_TBS_MAX_CALLS);
    for (int32_t index = 1;index < argc;index++)
    {
        call_index_list[index - 1] = (uint8_t)strtol(argv[index], NULL, 0);
        SHELL_Printf(s_shellHandle, "%d", (uint8_t)call_index_list[index - 1]);
        if (index >= (argc-1))
        {
            SHELL_Printf(s_shellHandle, "\r\n");
        }
        else
        {
            SHELL_Printf(s_shellHandle, ",");
        }
    }

    ret = bt_tbs_join(argc - 1, &call_index_list[0]);

    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);

    return kStatus_SHELL_Success;
}

static shell_status_t remote_call_incoming(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char *bearer_string = NULL;
    char *callee_uri = NULL;
    char *caller_uri = NULL;
    char *caller_name = NULL;
    long bearer = 0;
    int ret = 0;

    if (argc < 5)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "The Connection is broken\r\n");
        return kStatus_SHELL_Error;
    }

    bearer_string = argv[1];
    callee_uri = argv[2];
    caller_uri = argv[3];
    caller_name = argv[4];

    bearer = strtol(bearer_string, NULL, 0);

    if ((bearer < 0) || (bearer >= CONFIG_BT_TBS_BEARER_COUNT))
    {
        SHELL_Printf(s_shellHandle, "Invalid bearer index (%ld)\r\n", bearer);
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(s_shellHandle, "incoming call: callee uri %s, caller uri %s\r\n", callee_uri, caller_uri);

    ret = bt_tbs_remote_incoming((uint8_t)bearer, callee_uri, caller_uri, caller_name);

    if (ret < 0)
    {
        SHELL_Printf(s_shellHandle, "failed %d\r\n", ret);
        return kStatus_SHELL_Error;
    }
    else
    {
        if ((ret > 0) && (s_callIndex == 0))
        {
            s_callIndex = ret;
            /* TODO: */
            ret = start_ringtone();
            if (ret < 0)
            {
                PRINTF("Cannot start ringtone stream, will term the call %d\r\n", s_callIndex);
                bt_tbs_terminate(s_callIndex);
            }
        }
        SHELL_Printf(s_shellHandle, "done, call index is %d\r\n", ret);
    }
    return kStatus_SHELL_Success;
}

static shell_status_t remote_call_answer(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    long call_index = 0;
    char *call_index_string = NULL;
    int ret = 0;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "The Connection is broken\r\n");
        return kStatus_SHELL_Error;
    }

    call_index_string = argv[1];

    call_index = strtol(call_index_string, NULL, 0);

    if (0 == call_index)
    {
        SHELL_Printf(s_shellHandle, "Invalid call index (%ld)\r\n", call_index);
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(s_shellHandle, "Remove answer the call: call index %ld\r\n", call_index);

    ret = bt_tbs_remote_answer(call_index);
    if ((ret == 0) && (s_callIndex == call_index))
    {
        /* TODO: */
        stop_ringtone_light();
        ret = start_voice_light();

        if (0 > ret)
        {
            SHELL_Printf(s_shellHandle, "Cannot start voice stream, will term the call %d\r\n", call_index);
            bt_tbs_terminate(call_index);
        }
    }
    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);
    return kStatus_SHELL_Success;
}

static shell_status_t remote_call_term(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    long call_index = 0;
    char *call_index_string = NULL;
    int ret = 0;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "The Connection is broken\r\n");
        return kStatus_SHELL_Error;
    }

    call_index_string = argv[1];

    call_index = strtol(call_index_string, NULL, 0);

    if (0 == call_index)
    {
        SHELL_Printf(s_shellHandle, "Invalid call index (%ld)\r\n", call_index);
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(s_shellHandle, "Remove terminate the call: call index %ld\r\n", call_index);

    ret = bt_tbs_remote_terminate(call_index);

    if ((ret == 0) && (s_callIndex == call_index))
    {
        s_callIndex = 0U;
        /* TODO: */
        stop_call();
    }

    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);
    return kStatus_SHELL_Success;
}

static shell_status_t remote_call_hold(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    long call_index = 0;
    char *call_index_string = NULL;
    int ret = 0;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "The Connection is broken\r\n");
        return kStatus_SHELL_Error;
    }

    call_index_string = argv[1];

    call_index = strtol(call_index_string, NULL, 0);

    if (0 == call_index)
    {
        SHELL_Printf(s_shellHandle, "Invalid call index (%ld)\r\n", call_index);
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(s_shellHandle, "Remote hold the call: call index %ld\r\n", call_index);

    ret = bt_tbs_remote_hold(call_index);
    if ((ret == 0) && (s_callIndex == call_index))
    {
        /* TODO: */
        hold_call();
    }
    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);
    return kStatus_SHELL_Success;
}

static shell_status_t remote_call_retrieve(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    long call_index = 0;
    char *call_index_string = NULL;
    int ret = 0;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "The Connection is broken\r\n");
        return kStatus_SHELL_Error;
    }

    call_index_string = argv[1];

    call_index = strtol(call_index_string, NULL, 0);

    if (0 == call_index)
    {
        SHELL_Printf(s_shellHandle, "Invalid call index (%ld)\r\n", call_index);
        return kStatus_SHELL_Error;
    }

    SHELL_Printf(s_shellHandle, "Remote retrieve the call: call index %ld\r\n", call_index);

    ret = bt_tbs_remote_retrieve(call_index);
    if ((ret == 0) && (s_callIndex == call_index))
    {
        /* TODO: */
        retrieve_call();
    }
    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);
    return kStatus_SHELL_Success;
}

/* VOL Commander definition */
SHELL_COMMAND_DEFINE(vol_set,    "vol_set [0-100]\r\n", vol_set,     1);
SHELL_COMMAND_DEFINE(vol_up,     "vol_up\r\n",          vol_up,      0);
SHELL_COMMAND_DEFINE(vol_down,   "vol_down\r\n",        vol_down,    0);
SHELL_COMMAND_DEFINE(vol_mute,   "vol_mute\r\n",        vol_mute,    0);
SHELL_COMMAND_DEFINE(vol_unmute, "vol_unmute\r\n",      vol_unmute,  0);
/* local call Commander definition */
SHELL_COMMAND_DEFINE(call_accept,   "call_accept <callIndex>: Accept a incoming call\r\n",                                                               call_accept,      1);
SHELL_COMMAND_DEFINE(call_outgoing, "call_outgoing <telephone bearer index> <callee_URI>: Originate a call\r\n",                                         call_outgoing,    2);
SHELL_COMMAND_DEFINE(call_hold,     "call_hold <callIndex>: Hold a active call\r\n",                                                                     call_hold,        1);
SHELL_COMMAND_DEFINE(call_retrieve, "call_retrieve <callIndex>: Retrieve a active call\r\n",                                                             call_retrieve,    1);
SHELL_COMMAND_DEFINE(call_term,     "call_term <callIndex>: Terminate a call\r\n",                                                                       call_term,        1);
SHELL_COMMAND_DEFINE(call_join,     "call_join <callIndex1> [<callIndex2> <callIndex3> ...]: Join the calls\r\n",                                        call_join,        SHELL_IGNORE_PARAMETER_COUNT);
/* remote call Commander definition */
SHELL_COMMAND_DEFINE(remote_call_incoming, "remote_call_incoming <telephone bearer index> <callee_URI> <caller_URI> <caller_name>: Simulate a call\r\n", remote_call_incoming,    4);
SHELL_COMMAND_DEFINE(remote_call_answer,   "remote_call_answer <callIndex>: Simulate the outgoing has been accepted by the remote\r\n",                  remote_call_answer,      1);
SHELL_COMMAND_DEFINE(remote_call_term,     "remote_call_term <callIndex>: Terminate a call\r\n",                                                         remote_call_term,        1);
SHELL_COMMAND_DEFINE(remote_call_hold,     "remote_call_hold <callIndex>: Hold a active call\r\n",                                                       remote_call_hold,        1);
SHELL_COMMAND_DEFINE(remote_call_retrieve, "remote_call_retrieve <callIndex>: Retrieve a active call\r\n",                                               remote_call_retrieve,    1);

static void call_server_task(void* parm)
{
    uint32_t reg_mask;
    uint8_t status;
    int err;

    while (true)
    {
        (void)OSA_SemaphoreWait((osa_semaphore_handle_t)semHandle, osaWaitForever_c);

        reg_mask = DisableGlobalIRQ();
        status = s_call_status;
        s_call_status = 0;
        EnableGlobalIRQ(reg_mask);

        switch (status)
        {
            case (uint8_t)KSTATUS_APP_CPP_SERVER_ORIGINATE_CALL:
            {
                err = start_ringtone();
                if (err < 0)
                {
                    PRINTF("Cannot start ringtone stream, will term the call %d\r\n", s_callIndex);
                    if (0 != s_callIndex)
                    {
                        bt_tbs_terminate(s_callIndex);
                    }
                }
            }
            break;
            case (uint8_t)KSTATUS_APP_CPP_SERVER_TERMINATE_CALL:
            {
                stop_call();
            }
            break;
            case (uint8_t)KSTATUS_APP_CPP_SERVER_JOINS_CALL:
            {

            }
            break;
            case (uint8_t)KSTATUS_APP_CPP_SERVER_HOLD:
            {
                hold_call();
            }
            break;
            case (uint8_t)KSTATUS_APP_CPP_SERVER_ACCEPT:
            {
                stop_ringtone_light();
                err = start_voice_light();

                if (0 > err)
                {
                    PRINTF("Cannot start voice stream, will term the call %d\r\n", s_callIndex);
                    if (0 != s_callIndex)
                    {
                        bt_tbs_terminate(s_callIndex);
                    }
                }
            }
            break;
            case (uint8_t)KSTATUS_APP_CPP_SERVER_RETRIEVE:
            {
                retrieve_call();
            }
            break;
            default:
            {

            }
            break;
        }
    }
}

int call_server_init(shell_handle_t shellHandle)
{
    int ret = -1;

    s_shellHandle = shellHandle;

    if (xTaskCreate(call_server_task, "call_server_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        PRINTF("Fail to create call server task!\r\n");
        return ret;
    }

    if(KOSA_StatusSuccess != OSA_SemaphoreCreate((osa_semaphore_handle_t)semHandle, 0))
    {
        PRINTF("Fail to create call server semaphore!\r\n");
        return ret;
    }

    ret = bt_tbs_init();

    if (ret >= 0)
    {
        bt_tbs_register_cb(&app_tbs_cb);
    }

    bt_conn_cb_register(&conn_callbacks);



    return ret;
}

int call_server_enable_command(void)
{
    /* VOL setting commander */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_set));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_up));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_down));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_mute));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_unmute));
    /* Telephone call server commanders */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_accept));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_outgoing));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_hold));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_retrieve));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_term));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_join));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(remote_call_incoming));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(remote_call_term));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(remote_call_answer));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(remote_call_hold));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(remote_call_retrieve));

    commander_is_ready = 1;

    return 0;
}

int call_server_disable_command(void)
{
    commander_is_ready = 0;

    /* VOL setting commander */
    SHELL_UnregisterCommand(SHELL_COMMAND(vol_set));
    SHELL_UnregisterCommand(SHELL_COMMAND(vol_up));
    SHELL_UnregisterCommand(SHELL_COMMAND(vol_down));
    SHELL_UnregisterCommand(SHELL_COMMAND(vol_mute));
    SHELL_UnregisterCommand(SHELL_COMMAND(vol_unmute));
    /* Telephone call server commanders */
    SHELL_UnregisterCommand(SHELL_COMMAND(call_accept));
    SHELL_UnregisterCommand(SHELL_COMMAND(call_outgoing));
    SHELL_UnregisterCommand(SHELL_COMMAND(call_hold));
    SHELL_UnregisterCommand(SHELL_COMMAND(call_retrieve));
    SHELL_UnregisterCommand(SHELL_COMMAND(call_term));
    SHELL_UnregisterCommand(SHELL_COMMAND(call_join));
    SHELL_UnregisterCommand(SHELL_COMMAND(remote_call_incoming));
    SHELL_UnregisterCommand(SHELL_COMMAND(remote_call_term));
    SHELL_UnregisterCommand(SHELL_COMMAND(remote_call_answer));
    SHELL_UnregisterCommand(SHELL_COMMAND(remote_call_hold));
    SHELL_UnregisterCommand(SHELL_COMMAND(remote_call_retrieve));

    return 0;
}