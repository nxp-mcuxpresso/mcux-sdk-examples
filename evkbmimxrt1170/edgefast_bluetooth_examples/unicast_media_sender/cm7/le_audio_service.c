/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"

#include "le_audio_service.h"

#include <bluetooth/audio/vcp.h>
#include <bluetooth/audio/mcs.h>
#include <bluetooth/audio/mcc.h>
#include <bluetooth/audio/media_proxy.h>

static struct bt_vcp_vol_ctlr *vcs_remote = NULL;
static struct bt_vcp_vol_ctlr *vcs_clients[LE_CONN_COUNT];
static struct bt_conn *vcs_conn[LE_CONN_COUNT];

static int state_cb_pending[LE_CONN_COUNT] = { 0 };

#define IS_STATE_CB_PENDING(i)      (state_cb_pending[i] > 0)
#define STATE_CB_PENDING_SET(i)     state_cb_pending[i] += 1;
#define STATE_CB_PENDING_CLEAR(i)   state_cb_pending[i] -= 1;

static int pre_volume[LE_CONN_COUNT];
static int pre_mute[LE_CONN_COUNT];

static vcs_client_discover_callback_t vcs_client_discover_user_callback;

static void vcs_client_discover_callback(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t vocs_count, uint8_t aics_count)
{
	if (err) {
		PRINTF("\nVCS discover finished callback error: %d\n", err);
	} else {
		PRINTF("\nVCS discover finished\n");
	}
    
    for (int i = 0; i < LE_CONN_COUNT; i++)
    {
        if (vol_ctlr == vcs_clients[i])
        {
            vcs_client_discover_user_callback(vcs_conn[i], err);

            /* read rend state to init pre_volume/mute. */
            int ret = bt_vcp_vol_ctlr_read_state(vol_ctlr);
            if(ret)
            {
                PRINTF("\nVCS read inst %d state fail with ret %d\n", i, ret);
                break;
            }
            else
            {
                STATE_CB_PENDING_SET(i);
            }
        }
    }
}

static void vcs_client_state_callback(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t volume, uint8_t mute)
{
    int index = -1;

    for(int i = 0; i < LE_CONN_COUNT; i++)
    {
        if(vol_ctlr == vcs_clients[i])
        {
            index = i;
        }
    }

    if(index == -1)
    {
        PRINTF("\nVCS inst pointer %p invalid!\n", vol_ctlr);
        return;
    }

    PRINTF("\nVCS inst %d, volume %d, mute %d\n", index, volume, mute);

    if(err)
    {
        STATE_CB_PENDING_CLEAR(index);
        return;
    }

    if(IS_STATE_CB_PENDING(index))
    {
        STATE_CB_PENDING_CLEAR(index);
    }
    else
    {
        /* state callback invoke without pending, so it is a notify from rend.
        notify state change from rend should be sync to another rend. */
        int index2 = (index == 0) ? 1 : 0;

        /* case 1: volume and mute all changed */
        if((volume != pre_volume[index]) && (mute != pre_mute[index]))
        {
            if(mute == BT_VCP_STATE_UNMUTED)
            {
                /* case 1.1: volume up and unmute? */
                if(volume > pre_volume[index])
                {
                    int ret = bt_vcp_vol_ctlr_unmute_vol_up(vcs_clients[index2]);
                    if(ret)
                    {
                        PRINTF("\nVCS set inst %d unmute vol up fail with ret %d\n", index2, ret);
                    }
                }
                /* case 1.2: volume down and unmute? */
                else
                {
                    int ret = bt_vcp_vol_ctlr_unmute_vol_down(vcs_clients[index2]);
                    if(ret)
                    {
                        PRINTF("\nVCS set inst %d unmute vol down fail with ret %d\n", index2, ret);
                    }
                }
            }
            else
            {
                PRINTF("\nVCS volume change and mute can not change at the same time!\n");
            }
        }
        /* case 2: volume changed */
        else if(volume != pre_volume[index])
        {
            int ret = bt_vcp_vol_ctlr_set_vol(vcs_clients[index2], volume);
            if(ret)
            {
                PRINTF("\nVCS set vol to inst %d fail with ret %d\n", index2, ret);
            }
        }
        /* case 3: mute change */
        else if(mute != pre_mute[index])
        {
            if(mute == BT_VCP_STATE_UNMUTED)
            {
                int ret = bt_vcp_vol_ctlr_unmute(vcs_clients[index2]);
                if(ret)
                {
                    PRINTF("\nVCS unmute inst %d fail with ret %d\n", index2, ret);
                }
            }
            else
            {
                int ret = bt_vcp_vol_ctlr_mute(vcs_clients[index2]);
                if(ret)
                {
                    PRINTF("\nVCS mute inst %d fail with ret %d\n", index2, ret);
                }
            }
        }
    }

    /* save latest volume and mute. */
    pre_volume[index] = volume;
    pre_mute[index] = mute;
}

static void vcs_client_flags_callback(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t flags)
{
	if (err) {
		PRINTF("\nVCS flag callback error: %d\n", err);
	}
}

static void vcs_client_operation_callback(struct bt_vcp_vol_ctlr *vol_ctlr, int err)
{
    if(err)
    {
        PRINTF("\nVCS volume operation fail with err %d\n", err);
        return;
    }

    for(int i = 0; i < LE_CONN_COUNT; i++)
    {
        if(vol_ctlr == vcs_clients[i])
        {
            STATE_CB_PENDING_SET(i);
        }
    }
}

int le_audio_vcs_client_init(vcs_client_discover_callback_t callback)
{
    static struct bt_vcp_vol_ctlr_cb vcs_client_callback;

    vcs_client_discover_user_callback = callback;

    vcs_client_callback.discover = vcs_client_discover_callback;
    vcs_client_callback.state    = vcs_client_state_callback;
    vcs_client_callback.flags    = vcs_client_flags_callback;

	vcs_client_callback.vol_down        = vcs_client_operation_callback;
	vcs_client_callback.vol_up          = vcs_client_operation_callback;
	vcs_client_callback.mute            = vcs_client_operation_callback;
	vcs_client_callback.unmute          = vcs_client_operation_callback;
	vcs_client_callback.vol_down_unmute = vcs_client_operation_callback;
	vcs_client_callback.vol_up_unmute   = vcs_client_operation_callback;
	vcs_client_callback.vol_set         = vcs_client_operation_callback;

    return bt_vcp_vol_ctlr_cb_register(&vcs_client_callback);
}

int le_audio_vcs_discover(struct bt_conn *conn, uint8_t channel)
{
    if (channel >= LE_CONN_COUNT)
    {
        return -EINVAL;
    }

    vcs_conn[channel] = conn;

    return bt_vcp_vol_ctlr_discover(conn, &vcs_clients[channel]);
}

int le_audio_vcs_vol_set(uint8_t volume)
{
    /* Volume change to all render. */
    for(int i = 0; i < LE_CONN_COUNT; i++)
    {
        vcs_remote = vcs_clients[i];
        int ret = bt_vcp_vol_ctlr_set_vol(vcs_remote, volume);
        if(ret)
        {
            PRINTF("\nVCS vol set fail for device %d, error %d\n", i, ret);
        }
    }

    return 0;
}

int le_audio_vcs_vol_up(void)
{
    /* Volume change to all render. */
    for(int i = 0; i < LE_CONN_COUNT; i++)
    {
        vcs_remote = vcs_clients[i];
        int ret = bt_vcp_vol_ctlr_vol_up(vcs_remote);
        if(ret)
        {
            PRINTF("\nVCS vol up fail for device %d, error %d\n", i, ret);
        }
    }

    return 0;
}

int le_audio_vcs_vol_down(void)
{
    /* Volume change to all render. */
    for(int i = 0; i < LE_CONN_COUNT; i++)
    {
        vcs_remote = vcs_clients[i];
        int ret = bt_vcp_vol_ctlr_vol_down(vcs_remote);
        if(ret)
        {
            PRINTF("\nVCS vol down fail for device %d, error %d\n", i, ret);
        }
    }

    return 0;
}

int le_audio_vcs_vol_mute(void)
{
    /* Mute all render. */
    for(int i = 0; i < LE_CONN_COUNT; i++)
    {
        vcs_remote = vcs_clients[i];
        int ret = bt_vcp_vol_ctlr_mute(vcs_remote);
        if(ret)
        {
            PRINTF("\nVCS vol mute fail for device %d, error %d\n", i, ret);
        }
    }

    return 0;
}

int le_audio_vcs_vol_unmute(void)
{
    /* Unmute to all render. */
    for(int i = 0; i < LE_CONN_COUNT; i++)
    {
        vcs_remote = vcs_clients[i];
        int ret = bt_vcp_vol_ctlr_unmute(vcs_remote);
        if(ret)
        {
            PRINTF("\nVCS vol unmute fail for device %d, error %d\n", i, ret);
        }
    }

    return 0;
}

#if defined(CONFIG_BT_MCS) && (CONFIG_BT_MCS > 0)
static struct media_player *mcs_player;

static void mcs_local_player_instance_cb(struct media_player *player, int err)
{
    if(err)
    {
        PRINTF("\nMCS instance is registered, err %d\n", err);
        return;
    }

    mcs_player = player;
}

static mcs_server_state_cb_t mcs_server_state_cb;

static void mcs_media_state_recv(struct media_player *player, int err, uint8_t state)
{
    if(err)
    {
        PRINTF("\nMCS state recv, player %p, err %d, state %d\n", player, err, state);
        return;
    }
}

static void mcs_command_recv(struct media_player *player, int err, const struct mpl_cmd_ntf *result)
{
    if(err)
    {
        PRINTF("\nMCS cmd recv, player %p, err %d, req_op %d, res %d\n", player, err, (int)result->requested_opcode, (int)result->result_code);
    }

    if(result->requested_opcode == BT_MCS_OPC_PLAY)
    {
        mcs_server_state_cb(MCS_SERVER_STATE_PLAYING);
    }
    else if(result->requested_opcode == BT_MCS_OPC_PAUSE)
    {
        mcs_server_state_cb(MCS_SERVER_STATE_PAUSED);
    }
    else
    {
        PRINTF("\nMCS opcode %d not support!\n");
    }
}

int le_audio_mcs_server_init(mcs_server_state_cb_t callback)
{
    int ret;
    static struct media_proxy_ctrl_cbs ctrl_cbs;

    ret = media_proxy_pl_init();
    
    if(ret)
    {
        PRINTF("\nMCS server init fail %d\n", ret);
        return ret;
    }
    
    mcs_server_state_cb = callback;

    ctrl_cbs.local_player_instance = mcs_local_player_instance_cb;
    ctrl_cbs.media_state_recv      = mcs_media_state_recv;
    ctrl_cbs.command_recv          = mcs_command_recv;

    return media_proxy_ctrl_register(&ctrl_cbs);
}

#endif /* CONFIG_BT_MCS */

#if defined(CONFIG_BT_MCC) && (CONFIG_BT_MCC > 0)

static mcs_server_discover_cb_t mcs_server_discover_cb;
static struct bt_conn *mcs_server_conn = NULL;

static void mcc_discover_mcs_cb(struct bt_conn *conn, int err)
{
    if(err)
    {
        PRINTF("\nMedia control server is discoverd, %d\n", err);
        return;
    }

    mcs_server_discover_cb(conn);
    mcs_server_conn = conn;
}

static void mcc_send_cmd_cb(struct bt_conn *conn, int err, const struct mpl_cmd *cmd)
{
    if(err)
    {
        PRINTF("\nMedia control command is sent, %d, op %d\n", err, (int)cmd->opcode);
    }
}

static void mcc_cmd_ntf_cb(struct bt_conn *conn, int err, const struct mpl_cmd_ntf *ntf)
{
    if(err)
    {
        PRINTF("\nMedia control command notify, %d, req_op %d, res %d\n", err, (int)ntf->requested_opcode, (int)ntf->result_code);
    }
}

int le_audio_mcs_client_init(mcs_server_discover_cb_t callback)
{
    static struct bt_mcc_cb mcs_client_callback;

    mcs_server_discover_cb = callback;

    mcs_client_callback.discover_mcs = mcc_discover_mcs_cb;
    mcs_client_callback.send_cmd     = mcc_send_cmd_cb;
    mcs_client_callback.cmd_ntf      = mcc_cmd_ntf_cb;

    return bt_mcc_init(&mcs_client_callback);
}

int le_audio_mcs_discover(struct bt_conn *conn)
{
    return bt_mcc_discover_mcs(conn, true);
}

#endif /* CONFIG_BT_MCC */

int le_audio_mcs_play(void)
{
    struct mpl_cmd cmd;
#if defined(CONFIG_BT_MCC) && (CONFIG_BT_MCC > 0)
    cmd.opcode = BT_MCS_OPC_PLAY;
    cmd.use_param = false;
    cmd.param = 0;

    return bt_mcc_send_cmd(mcs_server_conn, &cmd);
#elif defined(CONFIG_BT_MCS) && (CONFIG_BT_MCS > 0)
    cmd.opcode = BT_MCS_OPC_PLAY;
    cmd.use_param = false;
    cmd.param = 0;

    return media_proxy_ctrl_send_command(mcs_player, &cmd);
#endif
}

int le_audio_mcs_pause(void)
{
    struct mpl_cmd cmd;
#if defined(CONFIG_BT_MCC) && (CONFIG_BT_MCC > 0)
    cmd.opcode = BT_MCS_OPC_PAUSE;
    cmd.use_param = false;
    cmd.param = 0;

    return bt_mcc_send_cmd(mcs_server_conn, &cmd);
#elif defined(CONFIG_BT_MCS) && (CONFIG_BT_MCS > 0)
    cmd.opcode = BT_MCS_OPC_PAUSE;
    cmd.use_param = false;
    cmd.param = 0;

    return media_proxy_ctrl_send_command(mcs_player, &cmd);
#endif
}
