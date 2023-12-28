/*
 * Copyright 2023 NXP
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

#if defined(CONFIG_BT_VCP_VOL_CTLR) && (CONFIG_BT_VCP_VOL_CTLR > 0)
static struct bt_vcp_vol_ctlr *vcs_remote = NULL;
static struct bt_vcp_vol_ctlr *vcs_clients[CONFIG_BT_MAX_CONN];
static struct bt_conn *vcs_conn[CONFIG_BT_MAX_CONN];

#define LOCAL_VOL_CHANGE_VOL_REQ          0x01
#define LOCAL_VOL_CHANGE_MUTE_REQ         0x02

static volatile uint32_t local_vol_change_req[CONFIG_BT_MAX_CONN] = { 0 };

#define LOCAL_VOL_CHANGE_VOL_SET(i)     (local_vol_change_req[i] |= LOCAL_VOL_CHANGE_VOL_REQ)
#define LOCAL_VOL_CHANGE_VOL_CLEAR(i)   (local_vol_change_req[i] &= (~LOCAL_VOL_CHANGE_VOL_REQ))
#define LOCAL_VOL_CHANGE_VOL_IS_SET(i)  (local_vol_change_req[i] & LOCAL_VOL_CHANGE_VOL_REQ)

#define LOCAL_VOL_CHANGE_MUTE_SET(i)     (local_vol_change_req[i] |= LOCAL_VOL_CHANGE_MUTE_REQ)
#define LOCAL_VOL_CHANGE_MUTE_CLEAR(i)   (local_vol_change_req[i] &= (~LOCAL_VOL_CHANGE_MUTE_REQ))
#define LOCAL_VOL_CHANGE_MUTE_IS_SET(i)  (local_vol_change_req[i] & LOCAL_VOL_CHANGE_MUTE_REQ)

static int     pre_volume[CONFIG_BT_MAX_CONN] = { -1 };
static int     pre_mute[CONFIG_BT_MAX_CONN]   = { -1 };

static vcs_client_discover_callback_t vcs_client_discover_user_callback;
#endif /* CONFIG_BT_VCP_VOL_CTLR */

#if defined(CONFIG_BT_VCP_VOL_REND) && (CONFIG_BT_VCP_VOL_REND > 0)
static vcs_server_vol_callback_t vcs_server_vol_callback;

static void vcs_vol_rend_state_callback(int err, uint8_t volume, uint8_t mute)
{
	if (err) {
		PRINTF("\nVCS state callback error: %d\n", err);
		return;
	}
	PRINTF("\nVCS Volume = %d, mute state = %d\r\n", volume, mute);

	vcs_server_vol_callback(volume, mute);
}

static void vcs_vol_rend_flags_callback(int err, uint8_t flags)
{
	if (err) {
		PRINTF("\nVCS flag callback error: %d\n", err);
	}
}

int le_audio_vcs_server_init(vcs_server_vol_callback_t callback)
{
	int ret;
	struct bt_vcp_vol_rend_register_param vcs_param;
	static struct bt_vcp_vol_rend_cb vcs_server_callback;

	vcs_server_callback.state = vcs_vol_rend_state_callback;
	vcs_server_callback.flags = vcs_vol_rend_flags_callback;

	vcs_param.step = 10 * 255 / 100;
	vcs_param.volume = 90 * 255 / 100;
	vcs_param.mute = BT_VCP_STATE_UNMUTED;
	vcs_param.cb = &vcs_server_callback;

	ret = bt_vcp_vol_rend_register(&vcs_param);
	if (ret) {
		return ret;
	}

    vcs_server_vol_callback = callback;

    return 0;
}
#endif /* CONFIG_BT_VCP_VOL_REND */

#if defined(CONFIG_BT_VCP_VOL_CTLR) && (CONFIG_BT_VCP_VOL_CTLR > 0)
static void vcs_client_discover_callback(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t vocs_count, uint8_t aics_count)
{
	if (err) {
		PRINTF("\nVCS discover finished callback error: %d\n", err);
	} else {
		PRINTF("\nVCS discover finished\n");
	}
    
    for (int i = 0; i < CONFIG_BT_MAX_CONN; i++)
    {
        if (vol_ctlr == vcs_clients[i])
        {
            vcs_client_discover_user_callback(vcs_conn[i], err);
        }
    }
}

static void vcs_client_state_callback(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t volume, uint8_t mute)
{
    int index;
    int ret;

    if (err)
    {
        PRINTF("\nVCS state callback error: %d\n", err);
        return;
    }

    /* Get current device index. */
    for (int i = 0; i < CONFIG_BT_MAX_CONN; i++)
    {
        if (vol_ctlr == vcs_clients[i])
        {
            index = i;
            break;
        }
    }

    PRINTF("\nVCS state update from device %d, vol %d, mute %d!\n", index, (int)volume, (int)mute);

    /* Check if this state callback is come from a local req. */
    if ((LOCAL_VOL_CHANGE_VOL_IS_SET(index)) || (LOCAL_VOL_CHANGE_MUTE_IS_SET(index)))
    {
        if(LOCAL_VOL_CHANGE_VOL_IS_SET(index))
        {
            LOCAL_VOL_CHANGE_VOL_CLEAR(index);
        }
        if(LOCAL_VOL_CHANGE_MUTE_IS_SET(index))
        {
            LOCAL_VOL_CHANGE_MUTE_CLEAR(index);
        }

        /* Save volume and mute state. */
        pre_volume[index] = volume;
        pre_mute[index]   = mute;
        return;
    }

    /* Here we need sync the volume/mute to another render,
    but we only handle the case volume/mute not change at the same time, otherwise we will meet the "-EBUSY" error.
    In most case this will not have effect, except for Unmute/Relative Volume Down/Up operation, but here we not implement them. */
    for(int i = 0; i < CONFIG_BT_MAX_CONN; i++)
    {
        /* Skip the current device. */
        if (i == index)
        {
		    /* Save volume and mute state. */
            pre_volume[i] = volume;
            pre_mute[i]   = mute;
            continue;
        }
        /* Sync volume, only if there is a volume change. */
        if(pre_volume[i] != volume)
        {
            pre_volume[i] = volume;
            vcs_remote = vcs_clients[i];
            int ret = bt_vcp_vol_ctlr_set_vol(vcs_remote, volume);
            if(ret)
            {
                PRINTF("\nVCS sync vol for device %d, error %d\n", i, ret);
            }
            else
            {
                LOCAL_VOL_CHANGE_VOL_SET(i);
                PRINTF("\nVCS sync vol for device %d, success!\n", i);
            }
        }
        /* Sync mute/unmute. */
        if(pre_mute[i] != mute)
        {
            pre_mute[i] = mute;
            vcs_remote = vcs_clients[i];
            if(mute)
            {
                ret = bt_vcp_vol_ctlr_mute(vcs_remote);
                if(ret)
                {
                    PRINTF("\nVCS sync mute for device %d, error %d\n", i, ret);
                }
                else
                {
                    LOCAL_VOL_CHANGE_MUTE_SET(i);
                    PRINTF("\nVCS sync mute for device %d, success!\n", i);
                }
            }
            else
            {
                ret = bt_vcp_vol_ctlr_unmute(vcs_remote);
                if(ret)
                {
                    PRINTF("\nVCS sync unmute for device %d, error %d\n", i, ret);
                }
                else
                {
                    LOCAL_VOL_CHANGE_MUTE_SET(i);
                    PRINTF("\nVCS sync unmute for device %d, success!\n", i);
                }
            }
        }
    }
}

static void vcs_client_flags_callback(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t flags)
{
	if (err) {
		PRINTF("\nVCS flag callback error: %d\n", err);
	}
}

int le_audio_vcs_client_init(vcs_client_discover_callback_t callback)
{
    static struct bt_vcp_vol_ctlr_cb vcs_client_callback;

    vcs_client_discover_user_callback = callback;

    vcs_client_callback.discover = vcs_client_discover_callback;
    vcs_client_callback.state    = vcs_client_state_callback;
    vcs_client_callback.flags    = vcs_client_flags_callback;

    return bt_vcp_vol_ctlr_cb_register(&vcs_client_callback);
}

int le_audio_vcs_discover(struct bt_conn *conn, uint8_t channel)
{
    if (channel >= CONFIG_BT_MAX_CONN)
    {
        return -EINVAL;
    }

    vcs_conn[channel] = conn;

    return bt_vcp_vol_ctlr_discover(conn, &vcs_clients[channel]);
}
#endif /* CONFIG_BT_VCP_VOL_CTLR */

int le_audio_vcs_vol_set(uint8_t volume)
{
#if defined(CONFIG_BT_VCP_VOL_CTLR) && (CONFIG_BT_VCP_VOL_CTLR > 0)
    /* Volume change to all render. */
    for(int i = 0; i < CONFIG_BT_MAX_CONN; i++)
    {
        vcs_remote = vcs_clients[i];
        int ret = bt_vcp_vol_ctlr_set_vol(vcs_remote, volume);
        if(ret)
        {
            PRINTF("\nVCS vol set fail for device %d, error %d\n", i, ret);
        }
        else
        {
            LOCAL_VOL_CHANGE_VOL_SET(i);
        }
    }

    return 0;
#elif defined(CONFIG_BT_VCP_VOL_REND) && (CONFIG_BT_VCP_VOL_REND > 0)
    return bt_vcp_vol_rend_set_vol(volume);
#endif
}

int le_audio_vcs_vol_up(void)
{
#if defined(CONFIG_BT_VCP_VOL_CTLR) && (CONFIG_BT_VCP_VOL_CTLR > 0)
    /* Volume change to all render. */
    for(int i = 0; i < CONFIG_BT_MAX_CONN; i++)
    {
        vcs_remote = vcs_clients[i];
        int ret = bt_vcp_vol_ctlr_vol_up(vcs_remote);
        if(ret)
        {
            PRINTF("\nVCS vol up fail for device %d, error %d\n", i, ret);
        }
        else
        {
            LOCAL_VOL_CHANGE_VOL_SET(i);
        }
    }

    return 0;
#elif defined(CONFIG_BT_VCP_VOL_REND) && (CONFIG_BT_VCP_VOL_REND > 0)
    return bt_vcp_vol_rend_vol_up();
#endif
}

int le_audio_vcs_vol_down(void)
{
#if defined(CONFIG_BT_VCP_VOL_CTLR) && (CONFIG_BT_VCP_VOL_CTLR > 0)
    /* Volume change to all render. */
    for(int i = 0; i < CONFIG_BT_MAX_CONN; i++)
    {
        vcs_remote = vcs_clients[i];
        int ret = bt_vcp_vol_ctlr_vol_down(vcs_remote);
        if(ret)
        {
            PRINTF("\nVCS vol down fail for device %d, error %d\n", i, ret);
        }
        else
        {
            LOCAL_VOL_CHANGE_VOL_SET(i);
        }
    }

    return 0;
#elif defined(CONFIG_BT_VCP_VOL_REND) && (CONFIG_BT_VCP_VOL_REND > 0)
    return bt_vcp_vol_rend_vol_down();
#endif
}

int le_audio_vcs_vol_mute(void)
{
#if defined(CONFIG_BT_VCP_VOL_CTLR) && (CONFIG_BT_VCP_VOL_CTLR > 0)
    /* Mute all render. */
    for(int i = 0; i < CONFIG_BT_MAX_CONN; i++)
    {
        vcs_remote = vcs_clients[i];
        int ret = bt_vcp_vol_ctlr_mute(vcs_remote);
        if(ret)
        {
            PRINTF("\nVCS vol mute fail for device %d, error %d\n", i, ret);
        }
        else
        {
            LOCAL_VOL_CHANGE_MUTE_SET(i);
        }
    }

    return 0;
#elif defined(CONFIG_BT_VCP_VOL_REND) && (CONFIG_BT_VCP_VOL_REND > 0)
    return bt_vcp_vol_rend_mute();
#endif
}

int le_audio_vcs_vol_unmute(void)
{
#if defined(CONFIG_BT_VCP_VOL_CTLR) && (CONFIG_BT_VCP_VOL_CTLR > 0)
    /* Unmute to all render. */
    for(int i = 0; i < CONFIG_BT_MAX_CONN; i++)
    {
        vcs_remote = vcs_clients[i];
        int ret = bt_vcp_vol_ctlr_unmute(vcs_remote);
        if(ret)
        {
            PRINTF("\nVCS vol unmute fail for device %d, error %d\n", i, ret);
        }
        else
        {
            LOCAL_VOL_CHANGE_MUTE_SET(i);
        }
    }

    return 0;
#elif defined(CONFIG_BT_VCP_VOL_REND) && (CONFIG_BT_VCP_VOL_REND > 0)
    return bt_vcp_vol_rend_unmute();
#endif
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
