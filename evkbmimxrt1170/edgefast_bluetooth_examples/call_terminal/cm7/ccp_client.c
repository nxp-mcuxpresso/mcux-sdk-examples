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

#include "fsl_debug_console.h"

#include "fsl_shell.h"

#include "ccp_client.h"

static shell_handle_t s_shellHandle;

static volatile uint8_t s_currentCallIndex;

static struct bt_conn *default_conn;

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
};

extern int BOARD_VolSet(uint32_t vol);
extern int BOARD_VolUp(void);
extern int BOARD_VolDown(void);
extern int BOARD_SpeakerMute(void);
extern int BOARD_SpeakerUnmute(void);

static void app_bt_tbs_client_discover_cb_discover(struct bt_conn *conn, int err,
                      uint8_t tbs_count, bool gtbs_found);

#if (defined(CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL) && (CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL > 0))
    static void app_bt_tbs_client_cp_cb_originate_call(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint8_t call_index);
#endif /* defined(CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_TERMINATE_CALL) && (CONFIG_BT_TBS_CLIENT_TERMINATE_CALL > 0))
    static void app_bt_tbs_client_cp_cb_terminate_call(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint8_t call_index);
#endif /* defined(CONFIG_BT_TBS_CLIENT_TERMINATE_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_HOLD_CALL) && (CONFIG_BT_TBS_CLIENT_HOLD_CALL > 0))
    static void app_bt_tbs_client_cp_cb_hold_call(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint8_t call_index);
#endif /* defined(CONFIG_BT_TBS_CLIENT_HOLD_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_ACCEPT_CALL) && (CONFIG_BT_TBS_CLIENT_ACCEPT_CALL > 0))
    static void app_bt_tbs_client_cp_cb_accept_call(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint8_t call_index);
#endif /* defined(CONFIG_BT_TBS_CLIENT_ACCEPT_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL) && (CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL > 0))
    static void app_bt_tbs_client_cp_cb_retrieve_call(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint8_t call_index);
#endif /* defined(CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_JOIN_CALLS) && (CONFIG_BT_TBS_CLIENT_JOIN_CALLS > 0))
    static void app_bt_tbs_client_cp_cb_join_calls(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint8_t call_index);
#endif /* defined(CONFIG_BT_TBS_CLIENT_JOIN_CALLS) */

#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_PROVIDER_NAME) && (CONFIG_BT_TBS_CLIENT_BEARER_PROVIDER_NAME > 0))
    void app_bt_tbs_client_read_string_cb_bearer_provider_name(struct bt_conn *conn, int err,
                         uint8_t inst_index,
                         const char *value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_PROVIDER_NAME) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_UCI) && (CONFIG_BT_TBS_CLIENT_BEARER_UCI > 0))
    void app_bt_tbs_client_read_string_cb_bearer_uci(struct bt_conn *conn, int err,
                         uint8_t inst_index,
                         const char *value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_UCI) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_TECHNOLOGY) && (CONFIG_BT_TBS_CLIENT_BEARER_TECHNOLOGY > 0))
    void app_bt_tbs_client_read_value_cb_technology(struct bt_conn *conn, int err,
                        uint8_t inst_index, uint32_t value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_TECHNOLOGY) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_URI_SCHEMES_SUPPORTED_LIST) && (CONFIG_BT_TBS_CLIENT_BEARER_URI_SCHEMES_SUPPORTED_LIST > 0))
    void app_bt_tbs_client_read_string_cb_uri_list(struct bt_conn *conn, int err,
                         uint8_t inst_index,
                         const char *value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_URI_SCHEMES_SUPPORTED_LIST) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_SIGNAL_STRENGTH) && (CONFIG_BT_TBS_CLIENT_BEARER_SIGNAL_STRENGTH > 0))
    void app_bt_tbs_client_read_value_cb_signal_strength(struct bt_conn *conn, int err,
                        uint8_t inst_index, uint32_t value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_SIGNAL_STRENGTH) */
#if (defined(CONFIG_BT_TBS_CLIENT_READ_BEARER_SIGNAL_INTERVAL) && (CONFIG_BT_TBS_CLIENT_READ_BEARER_SIGNAL_INTERVAL > 0))
    void app_bt_tbs_client_read_value_cb_signal_interval(struct bt_conn *conn, int err,
                        uint8_t inst_index, uint32_t value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_READ_BEARER_SIGNAL_INTERVAL) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_LIST_CURRENT_CALLS) && (CONFIG_BT_TBS_CLIENT_BEARER_LIST_CURRENT_CALLS > 0))
    void app_bt_tbs_client_current_calls_cb_current_calls(struct bt_conn *conn, int err,
                           uint8_t inst_index,
                           uint8_t call_count,
                           const struct bt_tbs_client_call *calls);
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_LIST_CURRENT_CALLS) */
#if (defined(CONFIG_BT_TBS_CLIENT_CCID) && (CONFIG_BT_TBS_CLIENT_CCID > 0))
    void app_bt_tbs_client_read_value_cb_ccid(struct bt_conn *conn, int err,
                        uint8_t inst_index, uint32_t value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_CCID) */
#if (defined(CONFIG_BT_TBS_CLIENT_INCOMING_URI) && (CONFIG_BT_TBS_CLIENT_INCOMING_URI > 0))
    void app_bt_tbs_client_read_string_cb_call_uri(struct bt_conn *conn, int err,
                         uint8_t inst_index,
                         const char *value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_INCOMING_URI) */
#if (defined(CONFIG_BT_TBS_CLIENT_STATUS_FLAGS) && (CONFIG_BT_TBS_CLIENT_STATUS_FLAGS > 0))
    void app_bt_tbs_client_read_value_cb_status_flags(struct bt_conn *conn, int err,
                        uint8_t inst_index, uint32_t value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_STATUS_FLAGS) */
    void app_bt_tbs_client_call_states_cb_call_state(struct bt_conn *conn, int err,
                         uint8_t inst_index,
                         uint8_t call_count,
                         const struct bt_tbs_client_call_state *call_states);
#if (defined(CONFIG_BT_TBS_CLIENT_OPTIONAL_OPCODES) && (CONFIG_BT_TBS_CLIENT_OPTIONAL_OPCODES > 0))
    void app_bt_tbs_client_read_value_cb_optional_opcodes(struct bt_conn *conn, int err,
                        uint8_t inst_index, uint32_t value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_OPTIONAL_OPCODES) */
    void app_bt_tbs_client_termination_reason_cb_termination_reason(struct bt_conn *conn,
                            int err, uint8_t inst_index,
                            uint8_t call_index,
                            uint8_t reason);
#if (defined(CONFIG_BT_TBS_CLIENT_INCOMING_CALL) && (CONFIG_BT_TBS_CLIENT_INCOMING_CALL > 0))
    void app_bt_tbs_client_read_string_cb_remote_uri(struct bt_conn *conn, int err,
                         uint8_t inst_index,
                         const char *value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_INCOMING_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_CALL_FRIENDLY_NAME) && (CONFIG_BT_TBS_CLIENT_CALL_FRIENDLY_NAME > 0))
    void app_bt_tbs_client_read_string_cb_friendly_name(struct bt_conn *conn, int err,
                         uint8_t inst_index,
                         const char *value);
#endif /* defined(CONFIG_BT_TBS_CLIENT_CALL_FRIENDLY_NAME) */

static struct bt_tbs_client_cb s_btTbsClientCb =
{
    app_bt_tbs_client_discover_cb_discover,
#if (defined(CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL) && (CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL > 0))
    app_bt_tbs_client_cp_cb_originate_call,
#endif /* defined(CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_TERMINATE_CALL) && (CONFIG_BT_TBS_CLIENT_TERMINATE_CALL > 0))
    app_bt_tbs_client_cp_cb_terminate_call,
#endif /* defined(CONFIG_BT_TBS_CLIENT_TERMINATE_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_HOLD_CALL) && (CONFIG_BT_TBS_CLIENT_HOLD_CALL > 0))
    app_bt_tbs_client_cp_cb_hold_call,
#endif /* defined(CONFIG_BT_TBS_CLIENT_HOLD_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_ACCEPT_CALL) && (CONFIG_BT_TBS_CLIENT_ACCEPT_CALL > 0))
    app_bt_tbs_client_cp_cb_accept_call,
#endif /* defined(CONFIG_BT_TBS_CLIENT_ACCEPT_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL) && (CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL > 0))
    app_bt_tbs_client_cp_cb_retrieve_call,
#endif /* defined(CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_JOIN_CALLS) && (CONFIG_BT_TBS_CLIENT_JOIN_CALLS > 0))
    app_bt_tbs_client_cp_cb_join_calls,
#endif /* defined(CONFIG_BT_TBS_CLIENT_JOIN_CALLS) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_PROVIDER_NAME) && (CONFIG_BT_TBS_CLIENT_BEARER_PROVIDER_NAME > 0))
    app_bt_tbs_client_read_string_cb_bearer_provider_name,
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_PROVIDER_NAME) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_UCI) && (CONFIG_BT_TBS_CLIENT_BEARER_UCI > 0))
    app_bt_tbs_client_read_string_cb_bearer_uci,
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_UCI) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_TECHNOLOGY) && (CONFIG_BT_TBS_CLIENT_BEARER_TECHNOLOGY > 0))
    app_bt_tbs_client_read_value_cb_technology,
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_TECHNOLOGY) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_URI_SCHEMES_SUPPORTED_LIST) && (CONFIG_BT_TBS_CLIENT_BEARER_URI_SCHEMES_SUPPORTED_LIST > 0))
    app_bt_tbs_client_read_string_cb_uri_list,
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_URI_SCHEMES_SUPPORTED_LIST) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_SIGNAL_STRENGTH) && (CONFIG_BT_TBS_CLIENT_BEARER_SIGNAL_STRENGTH > 0))
    app_bt_tbs_client_read_value_cb_signal_strength,
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_SIGNAL_STRENGTH) */
#if (defined(CONFIG_BT_TBS_CLIENT_READ_BEARER_SIGNAL_INTERVAL) && (CONFIG_BT_TBS_CLIENT_READ_BEARER_SIGNAL_INTERVAL > 0))
    app_bt_tbs_client_read_value_cb_signal_interval,
#endif /* defined(CONFIG_BT_TBS_CLIENT_READ_BEARER_SIGNAL_INTERVAL) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_LIST_CURRENT_CALLS) && (CONFIG_BT_TBS_CLIENT_BEARER_LIST_CURRENT_CALLS > 0))
    app_bt_tbs_client_current_calls_cb_current_calls,
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_LIST_CURRENT_CALLS) */
#if (defined(CONFIG_BT_TBS_CLIENT_CCID) && (CONFIG_BT_TBS_CLIENT_CCID > 0))
    app_bt_tbs_client_read_value_cb_ccid,
#endif /* defined(CONFIG_BT_TBS_CLIENT_CCID) */
#if (defined(CONFIG_BT_TBS_CLIENT_INCOMING_URI) && (CONFIG_BT_TBS_CLIENT_INCOMING_URI > 0))
    app_bt_tbs_client_read_string_cb_call_uri,
#endif /* defined(CONFIG_BT_TBS_CLIENT_INCOMING_URI) */
#if (defined(CONFIG_BT_TBS_CLIENT_STATUS_FLAGS) && (CONFIG_BT_TBS_CLIENT_STATUS_FLAGS > 0))
    app_bt_tbs_client_read_value_cb_status_flags,
#endif /* defined(CONFIG_BT_TBS_CLIENT_STATUS_FLAGS) */
    app_bt_tbs_client_call_states_cb_call_state,
#if (defined(CONFIG_BT_TBS_CLIENT_OPTIONAL_OPCODES) && (CONFIG_BT_TBS_CLIENT_OPTIONAL_OPCODES > 0))
    app_bt_tbs_client_read_value_cb_optional_opcodes,
#endif /* defined(CONFIG_BT_TBS_CLIENT_OPTIONAL_OPCODES) */
    app_bt_tbs_client_termination_reason_cb_termination_reason,
#if (defined(CONFIG_BT_TBS_CLIENT_INCOMING_CALL) && (CONFIG_BT_TBS_CLIENT_INCOMING_CALL > 0))
    app_bt_tbs_client_read_string_cb_remote_uri,
#endif /* defined(CONFIG_BT_TBS_CLIENT_INCOMING_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_CALL_FRIENDLY_NAME) && (CONFIG_BT_TBS_CLIENT_CALL_FRIENDLY_NAME > 0))
    app_bt_tbs_client_read_string_cb_friendly_name,
#endif /* defined(CONFIG_BT_TBS_CLIENT_CALL_FRIENDLY_NAME) */
};

static void connected(struct bt_conn *conn, uint8_t err)
{
    int ret;

    default_conn = conn;

    PRINTF("Starting TBS server discover\r\n");

    ret = bt_tbs_client_discover(default_conn);
    if (0 != ret)
    {
        PRINTF("Cannot start discover(err %d)\r\n", ret);
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    default_conn = NULL;
}

static void app_bt_tbs_client_discover_cb_discover(struct bt_conn *conn, int err,
                      uint8_t tbs_count, bool gtbs_found)
{
    PRINTF("Discover complete (err %d)! TBS count %d, GTBS found? %s\r\n", err, tbs_count, (gtbs_found == true)?"Yes":"No");
}

#if (defined(CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL) && (CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL > 0))
static void app_bt_tbs_client_cp_cb_originate_call(struct bt_conn *conn, int err,
                uint8_t inst_index, uint8_t call_index)
{
    PRINTF("Control Point status update. A call outgoing (err %d). TBS Index %d, call index %d\r\n", err, inst_index, call_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_TERMINATE_CALL) && (CONFIG_BT_TBS_CLIENT_TERMINATE_CALL > 0))
static void app_bt_tbs_client_cp_cb_terminate_call(struct bt_conn *conn, int err,
                uint8_t inst_index, uint8_t call_index)
{
    PRINTF("Control Point status update. A call has been terminated (err %d). TBS Index %d, call index %d\r\n", err, inst_index, call_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_TERMINATE_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_HOLD_CALL) && (CONFIG_BT_TBS_CLIENT_HOLD_CALL > 0))
static void app_bt_tbs_client_cp_cb_hold_call(struct bt_conn *conn, int err,
                uint8_t inst_index, uint8_t call_index)
{
    PRINTF("Control Point status update. A call has been hold (err %d). TBS Index %d, call index %d\r\n", err, inst_index, call_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_HOLD_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_ACCEPT_CALL) && (CONFIG_BT_TBS_CLIENT_ACCEPT_CALL > 0))
static void app_bt_tbs_client_cp_cb_accept_call(struct bt_conn *conn, int err,
                uint8_t inst_index, uint8_t call_index)
{
    PRINTF("Control Point status update. A call has been accepted (err %d). TBS Index %d, call index %d\r\n", err, inst_index, call_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_ACCEPT_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL) && (CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL > 0))
static void app_bt_tbs_client_cp_cb_retrieve_call(struct bt_conn *conn, int err,
                uint8_t inst_index, uint8_t call_index)
{
    PRINTF("Control Point status update. A call has been retrieved (err %d). TBS Index %d, call index %d\r\n", err, inst_index, call_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_JOIN_CALLS) && (CONFIG_BT_TBS_CLIENT_JOIN_CALLS > 0))
static void app_bt_tbs_client_cp_cb_join_calls(struct bt_conn *conn, int err,
                uint8_t inst_index, uint8_t call_index)
{
    PRINTF("Control Point status update. A call has been retrieved (err %d). TBS Index %d, call index %d\r\n", err, inst_index, call_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_JOIN_CALLS) */

#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_PROVIDER_NAME) && (CONFIG_BT_TBS_CLIENT_BEARER_PROVIDER_NAME > 0))
void app_bt_tbs_client_read_string_cb_bearer_provider_name(struct bt_conn *conn, int err,
                        uint8_t inst_index,
                        const char *value)
{
    PRINTF("Read Bearer Provider name %s (err %d). TBS Index %d.\r\n", value, err, inst_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_PROVIDER_NAME) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_UCI) && (CONFIG_BT_TBS_CLIENT_BEARER_UCI > 0))
void app_bt_tbs_client_read_string_cb_bearer_uci(struct bt_conn *conn, int err,
                        uint8_t inst_index,
                        const char *value)
{
    PRINTF("Read Bearer uci %s (err %d). TBS Index %d.\r\n", value, err, inst_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_UCI) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_TECHNOLOGY) && (CONFIG_BT_TBS_CLIENT_BEARER_TECHNOLOGY > 0))
void app_bt_tbs_client_read_value_cb_technology(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint32_t value)
{
    PRINTF("Read technology %d (err %d). TBS Index %d.\r\n", value, err, inst_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_TECHNOLOGY) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_URI_SCHEMES_SUPPORTED_LIST) && (CONFIG_BT_TBS_CLIENT_BEARER_URI_SCHEMES_SUPPORTED_LIST > 0))
void app_bt_tbs_client_read_string_cb_uri_list(struct bt_conn *conn, int err,
                        uint8_t inst_index,
                        const char *value)
{
    PRINTF("Read uri list %s (err %d). TBS Index %d.\r\n", value, err, inst_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_URI_SCHEMES_SUPPORTED_LIST) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_SIGNAL_STRENGTH) && (CONFIG_BT_TBS_CLIENT_BEARER_SIGNAL_STRENGTH > 0))
void app_bt_tbs_client_read_value_cb_signal_strength(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint32_t value)
{
    PRINTF("Read signal strength %d (err %d). TBS Index %d.\r\n", value, err, inst_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_SIGNAL_STRENGTH) */
#if (defined(CONFIG_BT_TBS_CLIENT_READ_BEARER_SIGNAL_INTERVAL) && (CONFIG_BT_TBS_CLIENT_READ_BEARER_SIGNAL_INTERVAL > 0))
void app_bt_tbs_client_read_value_cb_signal_interval(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint32_t value)
{
    PRINTF("Read signal interval %d (err %d). TBS Index %d.\r\n", value, err, inst_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_READ_BEARER_SIGNAL_INTERVAL) */
#if (defined(CONFIG_BT_TBS_CLIENT_BEARER_LIST_CURRENT_CALLS) && (CONFIG_BT_TBS_CLIENT_BEARER_LIST_CURRENT_CALLS > 0))
void app_bt_tbs_client_current_calls_cb_current_calls(struct bt_conn *conn, int err,
                        uint8_t inst_index,
                        uint8_t call_count,
                        const struct bt_tbs_client_call *calls)
{
    PRINTF("List current calls (err %d). TBS Index %d, call count %d, call list,\r\n", err, inst_index, call_count);
    for (uint32_t index = 0U; index < call_count; index++)
    {
        PRINTF("call index %d, state %d, flags %d, remote uri %s\r\n", calls[index].call_info.index, calls[index].call_info.state, calls[index].call_info.flags, calls[index].remote_uri);
    }
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_BEARER_LIST_CURRENT_CALLS) */
#if (defined(CONFIG_BT_TBS_CLIENT_CCID) && (CONFIG_BT_TBS_CLIENT_CCID > 0))
void app_bt_tbs_client_read_value_cb_ccid(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint32_t value)
{
    PRINTF("Read Content Control ID %d (err %d). TBS Index %d.\r\n", value, err, inst_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_CCID) */
#if (defined(CONFIG_BT_TBS_CLIENT_INCOMING_URI) && (CONFIG_BT_TBS_CLIENT_INCOMING_URI > 0))
void app_bt_tbs_client_read_string_cb_call_uri(struct bt_conn *conn, int err,
                        uint8_t inst_index,
                        const char *value)
{
    PRINTF("Read incoming call URI %s (err %d). TBS Index %d.\r\n", value, err, inst_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_INCOMING_URI) */
#if (defined(CONFIG_BT_TBS_CLIENT_STATUS_FLAGS) && (CONFIG_BT_TBS_CLIENT_STATUS_FLAGS > 0))
void app_bt_tbs_client_read_value_cb_status_flags(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint32_t value)
{
    PRINTF("Read status flags %d (err %d). TBS Index %d.\r\n", value, err, inst_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_STATUS_FLAGS) */
void app_bt_tbs_client_call_states_cb_call_state(struct bt_conn *conn, int err,
                        uint8_t inst_index,
                        uint8_t call_count,
                        const struct bt_tbs_client_call_state *call_states)
{
    PRINTF("List current state of current calls (err %d). TBS Index %d, call count %d, call state list,\r\n", err, inst_index, call_count);
    for (uint32_t index = 0U; index < call_count; index++)
    {
        PRINTF("call index %d, state %d, flags %d.\r\n", call_states[index].index, call_states[index].state, call_states[index].flags);
        if ((0 == s_currentCallIndex) || (s_currentCallIndex = call_states[index].index))
        {
            s_currentCallIndex = call_states[index].index;
            switch (call_states[index].state)
            {
            case (BT_TBS_CALL_STATE_INCOMING):
              /* TODO: */
              break;
            case (BT_TBS_CALL_STATE_DIALING):
              break;
            case (BT_TBS_CALL_STATE_ALERTING):
              /* TODO: */

              break;
            case (BT_TBS_CALL_STATE_ACTIVE):
              /* TODO: */
              break;
            case (BT_TBS_CALL_STATE_LOCALLY_HELD):
              /* TODO: */
              break;
            case (BT_TBS_CALL_STATE_REMOTELY_HELD):
              break;
            case (BT_TBS_CALL_STATE_LOCALLY_AND_REMOTELY_HELD):
              /* TODO: */
              break;
            }
        }
    }
}
#if (defined(CONFIG_BT_TBS_CLIENT_OPTIONAL_OPCODES) && (CONFIG_BT_TBS_CLIENT_OPTIONAL_OPCODES > 0))
void app_bt_tbs_client_read_value_cb_optional_opcodes(struct bt_conn *conn, int err,
                    uint8_t inst_index, uint32_t value)
{
    PRINTF("Read call control point optional opcodes %d (err %d). TBS Index %d.\r\n", value, err, inst_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_OPTIONAL_OPCODES) */
void app_bt_tbs_client_termination_reason_cb_termination_reason(struct bt_conn *conn,
                        int err, uint8_t inst_index,
                        uint8_t call_index,
                        uint8_t reason)
{
    PRINTF("Call terminated(err %d). TBS Index %d, call index %d, reason %d.\r\n", err, inst_index, call_index, reason);
    if (s_currentCallIndex == call_index)
    {
        s_currentCallIndex = 0U;
        /* TODO: */
        BOARD_SpeakerMute();
    }
}
#if (defined(CONFIG_BT_TBS_CLIENT_INCOMING_CALL) && (CONFIG_BT_TBS_CLIENT_INCOMING_CALL > 0))
void app_bt_tbs_client_read_string_cb_remote_uri(struct bt_conn *conn, int err,
                        uint8_t inst_index,
                        const char *value)
{
    char call_index = 0;
    if (NULL != value)
    {
        call_index = value[0];
        PRINTF("incoming call inst_index %d, call_index = %d, uri %s\r\n", inst_index, call_index, &value[1]);
    }
    else
    {
        PRINTF("incoming call (inst_index %d) is invalid\r\n", inst_index);
    }
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_INCOMING_CALL) */
#if (defined(CONFIG_BT_TBS_CLIENT_CALL_FRIENDLY_NAME) && (CONFIG_BT_TBS_CLIENT_CALL_FRIENDLY_NAME > 0))
void app_bt_tbs_client_read_string_cb_friendly_name(struct bt_conn *conn, int err,
                        uint8_t inst_index,
                        const char *value)
{
    PRINTF("Read Friendly name %s (err %d). TBS Index %d.\r\n", value, err, inst_index);
}
#endif /* defined(CONFIG_BT_TBS_CLIENT_CALL_FRIENDLY_NAME) */

static shell_status_t call_discover(shell_handle_t shellHandle, int32_t argc, char **argv);
#if (defined(CONFIG_BT_TBS_CLIENT_ACCEPT_CALL) && (CONFIG_BT_TBS_CLIENT_ACCEPT_CALL > 0))
static shell_status_t call_accept(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif /* CONFIG_BT_TBS_CLIENT_ACCEPT_CALL */
#if (defined(CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL) && (CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL > 0))
static shell_status_t call_outgoing(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif /* CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL */
#if (defined(CONFIG_BT_TBS_CLIENT_HOLD_CALL) && (CONFIG_BT_TBS_CLIENT_HOLD_CALL > 0))
static shell_status_t call_hold(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif /* CONFIG_BT_TBS_CLIENT_HOLD_CALL */
#if (defined(CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL) && (CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL > 0))
static shell_status_t call_retrieve(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif /* CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL */
#if (defined(CONFIG_BT_TBS_CLIENT_TERMINATE_CALL) && (CONFIG_BT_TBS_CLIENT_TERMINATE_CALL > 0))
static shell_status_t call_term(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif /* CONFIG_BT_TBS_CLIENT_TERMINATE_CALL */
#if (defined(CONFIG_BT_TBS_CLIENT_JOIN_CALLS) && (CONFIG_BT_TBS_CLIENT_JOIN_CALLS > 0))
static shell_status_t call_join(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif /* CONFIG_BT_TBS_CLIENT_JOIN_CALLS */


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

static shell_status_t call_discover(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char *subscribe_flag_string = NULL;
    long subscribe_flag;
    int ret = 0;

    if (argc < 2)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "Invalid connection\r\n");
        return kStatus_SHELL_Error;
    }

    subscribe_flag_string = argv[1];

    subscribe_flag = strtol(subscribe_flag_string, NULL, 0);

    SHELL_Printf(s_shellHandle, "discover TBS server features, subscribe ? %s.\r\n", (subscribe_flag > 0) ? "Yes" : "No");

    ret = bt_tbs_client_discover(default_conn);

    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);

    return kStatus_SHELL_Success;
}

#if (defined(CONFIG_BT_TBS_CLIENT_ACCEPT_CALL) && (CONFIG_BT_TBS_CLIENT_ACCEPT_CALL > 0))
static shell_status_t call_accept(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char *inst_index_string = NULL;
    char *call_index_string = NULL;
    long inst_index = 0;
    long call_index = 0;
    int ret = 0;

    if (argc < 3)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "Invalid connection\r\n");
        return kStatus_SHELL_Error;
    }
    inst_index_string = argv[1];
    call_index_string = argv[2];

    inst_index = strtol(inst_index_string, NULL, 0);
    call_index = strtol(call_index_string, NULL, 0);

    SHELL_Printf(s_shellHandle, "accept call: TBS index %s, call index %d\r\n", (uint8_t)inst_index, (uint8_t)call_index);

    ret = bt_tbs_client_accept_call(default_conn, (uint8_t)inst_index, (uint8_t)call_index);

    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);

    return kStatus_SHELL_Success;
}
#endif /* CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL */

#if (defined(CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL) && (CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL > 0))
static shell_status_t call_outgoing(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char *inst_string = NULL;
    char *callee_uri = NULL;
    long inst_index = 0;
    int ret = 0;

    if (argc < 3)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "Invalid connection\r\n");
        return kStatus_SHELL_Error;
    }
    inst_string = argv[1];
    callee_uri = argv[2];

    inst_index = strtol(inst_string, NULL, 0);

    SHELL_Printf(s_shellHandle, "outgoing call: callee uri %s, TBS index %d\r\n", callee_uri, (uint8_t)inst_index);

    ret = bt_tbs_client_originate_call(default_conn, (uint8_t)inst_index, callee_uri);

    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);

    return kStatus_SHELL_Success;
}
#endif /* CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL */

#if (defined(CONFIG_BT_TBS_CLIENT_HOLD_CALL) && (CONFIG_BT_TBS_CLIENT_HOLD_CALL > 0))
static shell_status_t call_hold(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char *inst_index_string = NULL;
    char *call_index_string = NULL;
    long inst_index = 0;
    long call_index = 0;
    int ret = 0;

    if (argc < 3)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "Invalid connection\r\n");
        return kStatus_SHELL_Error;
    }
    inst_index_string = argv[1];
    call_index_string = argv[2];

    inst_index = strtol(inst_index_string, NULL, 0);
    call_index = strtol(call_index_string, NULL, 0);

    SHELL_Printf(s_shellHandle, "hold call: TBS index %s, call index %d\r\n", (uint8_t)inst_index, (uint8_t)call_index);

    ret = bt_tbs_client_hold_call(default_conn, (uint8_t)inst_index, (uint8_t)call_index);

    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);

    return kStatus_SHELL_Success;
}
#endif /* CONFIG_BT_TBS_CLIENT_HOLD_CALL */

#if (defined(CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL) && (CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL > 0))
static shell_status_t call_retrieve(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char *inst_index_string = NULL;
    char *call_index_string = NULL;
    long inst_index = 0;
    long call_index = 0;
    int ret = 0;

    if (argc < 3)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "Invalid connection\r\n");
        return kStatus_SHELL_Error;
    }
    inst_index_string = argv[1];
    call_index_string = argv[2];

    inst_index = strtol(inst_index_string, NULL, 0);
    call_index = strtol(call_index_string, NULL, 0);

    SHELL_Printf(s_shellHandle, "retrieve call: TBS index %s, call index %d\r\n", (uint8_t)inst_index, (uint8_t)call_index);

    ret = bt_tbs_client_retrieve_call(default_conn, (uint8_t)inst_index, (uint8_t)call_index);

    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);

    return kStatus_SHELL_Success;
}
#endif /* CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL */

#if (defined(CONFIG_BT_TBS_CLIENT_TERMINATE_CALL) && (CONFIG_BT_TBS_CLIENT_TERMINATE_CALL > 0))
static shell_status_t call_term(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char *inst_index_string = NULL;
    char *call_index_string = NULL;
    long inst_index = 0;
    long call_index = 0;
    int ret = 0;

    if (argc < 3)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "Invalid connection\r\n");
        return kStatus_SHELL_Error;
    }
    inst_index_string = argv[1];
    call_index_string = argv[2];

    inst_index = strtol(inst_index_string, NULL, 0);
    call_index = strtol(call_index_string, NULL, 0);

    SHELL_Printf(s_shellHandle, "Terminate call: TBS index %d, call index %d\r\n", (uint8_t)inst_index, (uint8_t)call_index);

    ret = bt_tbs_client_terminate_call(default_conn, (uint8_t)inst_index, (uint8_t)call_index);

    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);

    return kStatus_SHELL_Success;
}
#endif /* CONFIG_BT_TBS_CLIENT_TERMINATE_CALL */

#if (defined(CONFIG_BT_TBS_CLIENT_JOIN_CALLS) && (CONFIG_BT_TBS_CLIENT_JOIN_CALLS > 0))
static shell_status_t call_join(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    char *inst_index_string = NULL;
    long inst_index = 0;
    uint8_t call_index_list[CONFIG_BT_TBS_CLIENT_MAX_CALLS];
    int ret = 0;

    if (argc < 3)
    {
        return kStatus_SHELL_Error;
    }

    if (NULL == default_conn)
    {
        SHELL_Printf(s_shellHandle, "Invalid connection\r\n");
        return kStatus_SHELL_Error;
    }

    if (argc > (CONFIG_BT_TBS_CLIENT_MAX_CALLS + 2))
    {
        SHELL_Printf(s_shellHandle, "Supported max call count is %d\r\n", CONFIG_BT_TBS_CLIENT_MAX_CALLS);
        return kStatus_SHELL_Error;
    }

    inst_index_string = argv[1];
    inst_index = strtol(inst_index_string, NULL, 0);

    SHELL_Printf(s_shellHandle, "join calls, TBS index %d, call index list:", (uint8_t)inst_index);
    for (uint32_t index = 2; index < argc; index++)
    {
        call_index_list[index - 2] = (uint8_t)strtol(argv[index], NULL, 0);
        SHELL_Printf(s_shellHandle, "%d", (uint8_t)call_index_list[index - 2]);
        if (index >= (argc-1))
        {
            SHELL_Printf(s_shellHandle, "\r\n");
        }
        else
        {
            SHELL_Printf(s_shellHandle, ",");
        }
    }

    ret = bt_tbs_client_join_calls(default_conn, (uint8_t)inst_index, &call_index_list[0], argc-2);

    SHELL_Printf(s_shellHandle, "Return code %d\r\n", ret);

    return kStatus_SHELL_Success;
}
#endif /* CONFIG_BT_TBS_CLIENT_JOIN_CALLS */

/* VOL Commander definition */
SHELL_COMMAND_DEFINE(vol_set,    "vol_set [0-100]\r\n", vol_set,     1);
SHELL_COMMAND_DEFINE(vol_up,     "vol_up\r\n",          vol_up,      0);
SHELL_COMMAND_DEFINE(vol_down,   "vol_down\r\n",        vol_down,    0);
SHELL_COMMAND_DEFINE(vol_mute,   "vol_mute\r\n",        vol_mute,    0);
SHELL_COMMAND_DEFINE(vol_unmute, "vol_unmute\r\n",      vol_unmute,  0);
/* local call Commander definition */
SHELL_COMMAND_DEFINE(call_discover, "call_discover <subscribe flag>: Discover the TBS server features\r\n",                              call_discover,    1);
SHELL_COMMAND_DEFINE(call_accept,   "call_accept <tbs index> <callIndex>: Accept a incoming call\r\n",                                   call_accept,      2);
SHELL_COMMAND_DEFINE(call_outgoing, "call_outgoing <tbs index> <callee_URI>: Originate a call\r\n",                                      call_outgoing,    2);
SHELL_COMMAND_DEFINE(call_hold,     "call_hold <tbs index> <callIndex>: Hold a active call\r\n",                                         call_hold,        2);
SHELL_COMMAND_DEFINE(call_retrieve, "call_retrieve <tbs index> <callIndex>: Retrieve a active call\r\n",                                 call_retrieve,    2);
SHELL_COMMAND_DEFINE(call_term,     "call_term <tbs index> <callIndex>: Terminate a call\r\n",                                           call_term,        2);
SHELL_COMMAND_DEFINE(call_join,     "call_join <tbs index> <callIndex1> [<callIndex2> <callIndex3> ...]: Join the calls\r\n",            call_join,        SHELL_IGNORE_PARAMETER_COUNT);

int call_client_init(shell_handle_t shellHandle)
{
    int ret = -1;

    s_shellHandle = shellHandle;

    /* VOL setting commander */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_set));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_up));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_down));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_mute));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vol_unmute));
    /* Telephone call client commanders */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_discover));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_accept));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_outgoing));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_hold));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_retrieve));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_term));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(call_join));

    ret = 0;

    bt_tbs_client_register_cb(&s_btTbsClientCb);

    bt_conn_cb_register(&conn_callbacks);

    return ret;
}