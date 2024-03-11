/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <porting.h>
#include <bluetooth/l2cap.h>
#include <net/buf.h>
#include <sys/util.h>

#include "ncp_glue_ble.h"
#include "fsl_component_log_config.h"
#define LOG_MODULE_NAME bt_l2cap
#include "fsl_component_log.h"
LOG_MODULE_DEFINE(LOG_MODULE_NAME, kLOG_LevelTrace);

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if 0
#define CONTROLLER_INDEX        0
#define DATA_MTU_INITIAL        128
#define DATA_MTU                256
#define DATA_BUF_SIZE           BT_L2CAP_SDU_BUF_SIZE(DATA_MTU)
#define CHANNELS                2
#define SERVERS                 1

typedef struct channel_tag
{
    uint8_t chan_id; /* Internal number that identifies L2CAP channel. */
    struct bt_l2cap_le_chan le;
    bool in_use;
    bool hold_credit;
    struct net_buf *pending_credit;
} channel_t;
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if 0
/* Event callbacks */
static struct net_buf *alloc_buf_cb(struct bt_l2cap_chan *chan);
static void connected_cb(struct bt_l2cap_chan *l2cap_chan);
static void disconnected_cb(struct bt_l2cap_chan *l2cap_chan);
static int recv_cb(struct bt_l2cap_chan *l2cap_chan, struct net_buf *buf);

#if (defined(CONFIG_BT_L2CAP_ECRED) && (CONFIG_BT_L2CAP_ECRED == 1))
static void reconfigured_cb(struct bt_l2cap_chan *l2cap_chan);
#endif /* CONFIG_BT_L2CAP_ECRED */

/* Command handlers */
static void supported_commands(uint8_t *data, uint16_t len);
static void ble_l2cap_connect(uint8_t *data, uint16_t len);
static void disconnect(uint8_t *data, uint16_t len);
static void send_data(uint8_t *data, uint16_t len);
static void ble_l2cap_listen(uint8_t *data, uint16_t len);
static void credits(uint8_t *data, uint16_t len);

#if (defined(CONFIG_BT_L2CAP_ECRED) && (CONFIG_BT_L2CAP_ECRED == 1))
static void reconfigure(uint8_t *data, uint16_t len);
#endif /* CONFIG_BT_L2CAP_ECRED */

#if (defined(CONFIG_BT_EATT) && (CONFIG_BT_EATT == 1))
void disconnect_eatt_chans(uint8_t *data, uint16_t len);
#endif /* CONFIG_BT_EATT */

/* Helpers */
static channel_t *get_free_channel(void);
static int ble_l2cap_accept(struct bt_conn *conn, struct bt_l2cap_server *server, struct bt_l2cap_chan **l2cap_chan);
static bool is_free_psm(uint16_t psm);
static struct bt_l2cap_server *get_free_server(void);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if 0
NET_BUF_POOL_FIXED_DEFINE(data_pool, CHANNELS, DATA_BUF_SIZE, NULL);

static channel_t channels[CHANNELS];
static struct bt_l2cap_server servers[SERVERS];

static bool authorize_flag;
static uint8_t req_keysize;

static uint8_t recv_cb_buf[DATA_BUF_SIZE + sizeof(l2cap_data_received_ev_t)];

static const struct bt_l2cap_chan_ops l2cap_ops = {
    .alloc_buf          = alloc_buf_cb,
    .recv               = recv_cb,
    .connected          = connected_cb,
    .disconnected   = disconnected_cb,
#if (defined(CONFIG_BT_L2CAP_ECRED) && (CONFIG_BT_L2CAP_ECRED == 1))
    .reconfigured   = reconfigured_cb,
#endif /* CONFIG_BT_L2CAP_ECRED */
};
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
#if 0
/*
 * @brief   Tester App functionality
 */

uint8_t ncp_ble_init_l2cap(void)
{
    return NCP_BRIDGE_CMD_RESULT_OK;
}

uint8_t ncp_ble_unregister_l2cap(void)
{
    return NCP_BRIDGE_CMD_RESULT_OK;
}

#ifdef RAW_BT_TESTER
void ncp_ble_handle_l2cap(uint8_t opcode, uint8_t index, uint8_t *data,
             uint16_t len)
{
    switch (opcode) {
    case L2CAP_READ_SUPPORTED_COMMANDS:
        supported_commands(data, len);
        break;
    case L2CAP_CONNECT:
        ble_l2cap_connect(data, len);
        break;
    case L2CAP_DISCONNECT:
        disconnect(data, len);
        break;
    case L2CAP_SEND_DATA:
        send_data(data, len);
        break;
    case L2CAP_LISTEN:
        ble_l2cap_listen(data, len);
        break;
#if (defined(CONFIG_BT_L2CAP_ECRED) && (CONFIG_BT_L2CAP_ECRED == 1))
    case L2CAP_RECONFIGURE:
        reconfigure(data, len);
        break;
#endif /* CONFIG_BT_L2CAP_ECRED */
    case L2CAP_CREDITS:
        credits(data, len);
        break;
#if (defined(CONFIG_BT_EATT) && (CONFIG_BT_EATT == 1))
    case L2CAP_DISCONNECT_EATT_CHANS:
        disconnect_eatt_chans(data, len);
        break;
#endif /* CONFIG_BT_EATT */
    default:
        ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | opcode, index,
                   NCP_BLE_STATUS_UNKNOWN_CMD);
        break;
    }
}
#endif

/*
 * @brief   Commands
 */

/*
 * @brief   Read Supported Commands. Each bit in response is
 *          a flag indicating if command with opcode matching
 *          bit number is supported.
 */
static void supported_commands(uint8_t *data, uint16_t len)
{
    uint8_t cmds[2];
    l2cap_read_supported_commands_rp_t *rp = (void *) cmds;

    (void)memset(cmds, 0, sizeof(cmds));

    ncp_ble_set_bit(cmds, L2CAP_READ_SUPPORTED_COMMANDS);
    ncp_ble_set_bit(cmds, L2CAP_CONNECT);
    ncp_ble_set_bit(cmds, L2CAP_DISCONNECT);
    ncp_ble_set_bit(cmds, L2CAP_LISTEN);
    ncp_ble_set_bit(cmds, L2CAP_SEND_DATA);

#if (defined(CONFIG_BT_L2CAP_ECRED) && (CONFIG_BT_L2CAP_ECRED == 1))
    ncp_ble_set_bit(cmds, L2CAP_RECONFIGURE);
#endif /* CONFIG_BT_L2CAP_ECRED */

    ncp_ble_set_bit(cmds, L2CAP_CREDITS);

#if (defined(CONFIG_BT_EATT) && (CONFIG_BT_EATT == 1))
    ncp_ble_set_bit(cmds, L2CAP_DISCONNECT_EATT_CHANS);
#endif /* CONFIG_BT_EATT */

    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_READ_SUPPORTED_COMMANDS, NCP_BRIDGE_CMD_RESULT_OK, (uint8_t *) rp, sizeof(cmds));
}

/*
 * @brief   Create an L2CAP channel
 */
static void ble_l2cap_connect(uint8_t *data, uint16_t len)
{
    const l2cap_connect_cmd_t *cmd = (void *) data;
    l2cap_connect_rp_t *rp;
    struct bt_conn *conn;
    channel_t *chan = NULL;

#if (defined(CONFIG_BT_L2CAP_ECRED) && (CONFIG_BT_L2CAP_ECRED == 1)) || \
    (defined(CONFIG_BT_L2CAP_DYNAMIC_CHANNEL) && (CONFIG_BT_L2CAP_DYNAMIC_CHANNEL > 0U))
    struct bt_l2cap_chan *allocated_channels[5] = {0};
#endif /* CONFIG_BT_L2CAP_ECRED || CONFIG_BT_L2CAP_DYNAMIC_CHANNEL */

    uint16_t mtu = sys_le16_to_cpu(cmd->mtu);
    uint8_t buf[sizeof(*rp) + CHANNELS];
    bool ecfc = cmd->options & L2CAP_CONNECT_OPT_ECFC;
    int32_t err;

    if (cmd->num == 0 || cmd->num > CHANNELS || mtu > DATA_MTU_INITIAL)
    {
        goto fail;
    }

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (!conn)
    {
        goto fail;
    }

    rp = (void *)buf;

    for (uint8_t i = 0U; i < cmd->num; i++)
    {
        chan = get_free_channel();
        if (!chan)
        {
            goto fail;
        }
        chan->le.chan.ops = &l2cap_ops;
        chan->le.rx.mtu = mtu;
        rp->chan_id[i] = chan->chan_id;

#if (defined(CONFIG_BT_L2CAP_ECRED) && (CONFIG_BT_L2CAP_ECRED == 1)) || \
    (defined(CONFIG_BT_L2CAP_DYNAMIC_CHANNEL) && (CONFIG_BT_L2CAP_DYNAMIC_CHANNEL > 0U))
        allocated_channels[i] = &chan->le.chan;
#endif /* CONFIG_BT_L2CAP_ECRED || CONFIG_BT_L2CAP_DYNAMIC_CHANNEL */

        chan->hold_credit = cmd->options & L2CAP_CONNECT_OPT_HOLD_CREDIT;
    }

    if (cmd->num == 1 && !ecfc)
    {
        err = bt_l2cap_chan_connect(conn, &chan->le.chan, cmd->psm);
        if (err < 0)
        {
            goto fail;
        }
    }
    else if (ecfc)
    {
#if (defined(CONFIG_BT_L2CAP_ECRED) && (CONFIG_BT_L2CAP_ECRED == 1))
        err = bt_l2cap_ecred_chan_connect(conn, allocated_channels,
                                                cmd->psm);
        if (err < 0)
        {
            goto fail;
        }
#else /* CONFIG_BT_L2CAP_ECRED */
        goto fail;
#endif /* CONFIG_BT_L2CAP_ECRED */
    }
    else
    {
        LOG_ERR("Invalid 'num' parameter value");
        goto fail;
    }

    rp->num = cmd->num;

    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) |L2CAP_CONNECT, NCP_BRIDGE_CMD_RESULT_OK, (uint8_t *)rp, sizeof(*rp) + rp->num);

    return;

fail:

#if (defined(CONFIG_BT_L2CAP_DYNAMIC_CHANNEL) && (CONFIG_BT_L2CAP_DYNAMIC_CHANNEL > 0U))
    for (uint8_t i = 0U; i < ARRAY_SIZE(allocated_channels); i++)
    {
        if (allocated_channels[i])
        {
			channels[BT_L2CAP_LE_CHAN(allocated_channels[i])->ident].in_use = false;
        }
    }
#endif /* CONFIG_BT_L2CAP_DYNAMIC_CHANNEL */

    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_CONNECT, NCP_BRIDGE_CMD_RESULT_ERROR, NULL, 0);
}

/*
 * @brief   Close an L2CAP channel
 */
static void disconnect(uint8_t *data, uint16_t len)
{
    const l2cap_disconnect_cmd_t *cmd = (void *) data;
    channel_t *chan = &channels[cmd->chan_id];
    uint8_t status;
    int32_t err;

    err = bt_l2cap_chan_disconnect(&chan->le.chan);
    if (err)
    {
        status = NCP_BRIDGE_CMD_RESULT_ERROR;
        goto rsp;
    }

    status = NCP_BRIDGE_CMD_RESULT_OK;

rsp:
    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_DISCONNECT, status, NULL, 0);
}

/*
 * @brief   Send data over L2CAP channel
 */
static void send_data(uint8_t *data, uint16_t len)
{
    l2cap_send_data_cmd_t *cmd = (void *) data;
    channel_t *chan = &channels[cmd->chan_id];
    struct net_buf *buf;
    int32_t ret;
    uint16_t data_len = sys_le16_to_cpu(cmd->data_len);

    /* Fail if data length exceeds buffer length */
    if (data_len > DATA_MTU)
    {
        goto fail;
    }

    /* Fail if data length exceeds remote's L2CAP SDU */
    if (data_len > chan->le.tx.mtu)
    {
        goto fail;
    }

    buf = net_buf_alloc(&data_pool, osaWaitForever_c);
    if (buf != NULL)
    {
        net_buf_reserve(buf, BT_L2CAP_SDU_CHAN_SEND_RESERVE);

        net_buf_add_mem(buf, cmd->data, data_len);
        ret = bt_l2cap_chan_send(&chan->le.chan, buf);

        if (ret < 0)
        {
            LOG_ERR("Unable to send data: %d", -ret);
            net_buf_unref(buf);
            goto fail;
        }
    }
    else
    {
        goto fail;
    }

    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_SEND_DATA, NCP_BRIDGE_CMD_RESULT_OK, NULL, 0);
    return;

fail:
    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_SEND_DATA, NCP_BRIDGE_CMD_RESULT_ERROR, NULL, 0);
}

/*
 * @brief   Register L2CAP PSM and listen for incoming data
 */
static void ble_l2cap_listen(uint8_t *data, uint16_t len)
{
    const l2cap_listen_cmd_t *cmd = (void *) data;
    struct bt_l2cap_server *server;

    if (!is_free_psm(cmd->psm))
    {
        goto fail;
    }

    if (cmd->psm == 0U)
    {
        goto fail;
    }

    server = get_free_server();
    if (!server)
    {
        goto fail;
    }

    server->accept = ble_l2cap_accept;
    server->psm = cmd->psm;

    if (cmd->response == L2CAP_CONNECTION_RESPONSE_INSUFF_ENC_KEY)
    {
        /* TSPX_psm_encryption_key_size_required */
        req_keysize = 16;
    }
    else if (cmd->response == L2CAP_CONNECTION_RESPONSE_INSUFF_AUTHOR)
    {
        authorize_flag = true;
    }
    else if (cmd->response == L2CAP_CONNECTION_RESPONSE_INSUFF_AUTHEN)
    {
        server->sec_level = BT_SECURITY_L3;
    }

    if (bt_l2cap_server_register(server) < 0)
    {
        server->psm = 0U;
        goto fail;
    }

    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_LISTEN, NCP_BRIDGE_CMD_RESULT_OK, NULL, 0);
    return;

fail:
    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_LISTEN, NCP_BRIDGE_CMD_RESULT_ERROR, NULL, 0);
}

#if (defined(CONFIG_BT_L2CAP_ECRED) && (CONFIG_BT_L2CAP_ECRED == 1))
/*
 * @brief   Reconfigure Enhanced L2CAP Channels
 */
static void reconfigure(uint8_t *data, uint16_t len)
{
    const l2cap_reconfigure_cmd_t *cmd = (void *)data;
    uint16_t mtu = sys_le16_to_cpu(cmd->mtu);
    struct bt_conn *conn;
    uint8_t status;
    int32_t err;

    struct bt_l2cap_chan *reconf_channels[CHANNELS + 1] = {0};

    /* address is first in data */
    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)cmd);
    if (!conn)
    {
        LOG_ERR("Unknown connection");
        status = NCP_BRIDGE_CMD_RESULT_ERROR;
        goto rsp;
    }

    if (cmd->num > CHANNELS)
    {
        status = NCP_BRIDGE_CMD_RESULT_ERROR;
        goto rsp;
    }

    if (mtu > DATA_MTU)
    {
        status = NCP_BRIDGE_CMD_RESULT_ERROR;
        goto rsp;
    }

    for (int32_t i = 0; i < cmd->num; i++)
    {
        if (cmd->chan_id[i] > CHANNELS)
        {
            status = NCP_BRIDGE_CMD_RESULT_ERROR;
            goto rsp;
        }

        reconf_channels[i] = &channels[cmd->chan_id[i]].le.chan;
    }

    err = bt_l2cap_ecred_chan_reconfigure(reconf_channels, mtu);

    if (err)
    {
            status = NCP_BRIDGE_CMD_RESULT_ERROR;
            goto rsp;
    }

    status = NCP_BRIDGE_CMD_RESULT_OK;

rsp:
    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_RECONFIGURE, status, NULL, 0);
}
#endif /* CONFIG_BT_L2CAP_ECRED */

/*
 * @brief   return credits on L2CAP channel
 */
static void credits(uint8_t *data, uint16_t len)
{
    const l2cap_credits_cmd_t *cmd = (void *)data;
    channel_t *chan = &channels[cmd->chan_id];

    if (!chan->in_use)
    {
        goto fail;
    }

    if (chan->pending_credit)
    {
        if (bt_l2cap_chan_recv_complete(&chan->le.chan,
                                        chan->pending_credit) < 0)
        {
            goto fail;
        }

        chan->pending_credit = NULL;
    }

    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_CREDITS, NCP_BRIDGE_CMD_RESULT_OK, NULL, 0);
    return;

fail:
    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_CREDITS, NCP_BRIDGE_CMD_RESULT_ERROR, NULL, 0);
}

#if (defined(CONFIG_BT_EATT) && (CONFIG_BT_EATT == 1))
/*
 * @brief   Disconnect EATT channels
 */
void disconnect_eatt_chans(uint8_t *data, uint16_t len)
{
    const l2cap_disconnect_eatt_chans_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;
    int err;
    int status;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (!conn)
    {
        LOG_ERR("Unknown connection");
        status = NCP_BRIDGE_CMD_RESULT_ERROR;
        goto failed;
    }

    for (int i = 0; i < cmd->count; i++)
    {
        err = bt_eatt_disconnect_one(conn);
        if (err)
        {
            status = NCP_BRIDGE_CMD_RESULT_ERROR;
            goto unref;
        }
    }

    status = NCP_BRIDGE_CMD_RESULT_OK;

unref:
    bt_conn_unref(conn);
failed:
    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_DISCONNECT_EATT_CHANS, status, NULL, 0);
}
#endif /* CONFIG_BT_EATT */

/*
 *    Events
 */

/*
 * @brief   If this callback is provided the channel will use it to allocate
 *          buffers to store incoming data
 */
static struct net_buf *alloc_buf_cb(struct bt_l2cap_chan *chan)
{
    return net_buf_alloc(&data_pool, osaWaitForever_c);
}

/*
 * @brief   This event indicates new L2CAP connection
 */
static void connected_cb(struct bt_l2cap_chan *l2cap_chan)
{
    l2cap_connected_ev_t ev;
    channel_t *chan = CONTAINER_OF(l2cap_chan, channel_t, le);
    struct bt_conn_info info;
    int res;

    (void)memset(&ev, 0, sizeof(ev));

    ev.chan_id = chan->chan_id;

    res = bt_conn_get_info(l2cap_chan->conn, &info);
    if (res == 0)
    {
        switch (info.type)
        {
            case BT_CONN_TYPE_LE:
            {
                ev.mtu_remote = sys_cpu_to_le16(chan->le.tx.mtu);
                ev.mps_remote = sys_cpu_to_le16(chan->le.tx.mps);
                ev.mtu_local = sys_cpu_to_le16(chan->le.rx.mtu);
                ev.mps_local = sys_cpu_to_le16(chan->le.rx.mps);
                ev.address_type = info.le.dst->type;
                memcpy(ev.address, info.le.dst->a.val,
                       sizeof(ev.address));
                break;
            }

            case BT_CONN_TYPE_BR:
            {
                memcpy(ev.address, info.br.dst->val,
                sizeof(ev.address));
                break;
            }
        }
    }

    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_EV_CONNECTED, NCP_BRIDGE_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
}

/*
 * @brief   This event indicates L2CAP disconnection
 */
static void disconnected_cb(struct bt_l2cap_chan *l2cap_chan)
{
    l2cap_disconnected_ev_t ev;
    channel_t *chan = CONTAINER_OF(l2cap_chan, channel_t, le);
    struct bt_conn_info info;
    int res;

    /* release netbuf on premature disconnection */
    if (chan->pending_credit)
    {
        net_buf_unref(chan->pending_credit);
        chan->pending_credit = NULL;
    }

    (void)memset(&ev, 0, sizeof(l2cap_disconnected_ev_t));

    ev.chan_id = chan->chan_id;

    res = bt_conn_get_info(l2cap_chan->conn, &info);
    if (res == 0)
    {
        switch (info.type)
        {
            case BT_CONN_TYPE_LE:
            {
                ev.address_type = info.le.dst->type;
                memcpy(ev.address, info.le.dst->a.val,
                sizeof(ev.address));
                break;
            }

            case BT_CONN_TYPE_BR:
            {
                memcpy(ev.address, info.br.dst->val,
                sizeof(ev.address));
                break;
            }
        }
    }

    chan->in_use = false;

    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_EV_DISCONNECTED, NCP_BRIDGE_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
}

/*
 * @brief   This event forwards data received over L2CAP channel
 */
static int recv_cb(struct bt_l2cap_chan *l2cap_chan, struct net_buf *buf)
{
    int result = 0;

    l2cap_data_received_ev_t *ev = (void *) recv_cb_buf;
    channel_t *chan = CONTAINER_OF(l2cap_chan, channel_t, le);

    ev->chan_id = chan->chan_id;
    ev->data_length = sys_cpu_to_le16(buf->len);
    memcpy(ev->data, buf->data, buf->len);

    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_EV_DATA_RECEIVED, NCP_BRIDGE_CMD_RESULT_OK, recv_cb_buf, sizeof(*ev) + buf->len);

    if (chan->hold_credit && !chan->pending_credit)
    {
        /* no need for extra ref, as when returning EINPROGRESS user
         * becomes owner of the netbuf
         */
        chan->pending_credit = buf;
        result = -EINPROGRESS;
    }

    return result;
}

#if (defined(CONFIG_BT_L2CAP_ECRED) && (CONFIG_BT_L2CAP_ECRED == 1))
/*
 * @brief   This event indicates new that an L2CAP Channel has been reconfigured.
 */
static void reconfigured_cb(struct bt_l2cap_chan *l2cap_chan)
{
    l2cap_reconfigured_ev_t ev;
    channel_t *chan = CONTAINER_OF(l2cap_chan, channel_t, le);

    (void)memset(&ev, 0, sizeof(ev));

    ev.chan_id = chan->chan_id;
    ev.mtu_remote = sys_cpu_to_le16(chan->le.tx.mtu);
    ev.mps_remote = sys_cpu_to_le16(chan->le.tx.mps);
    ev.mtu_local = sys_cpu_to_le16(chan->le.rx.mtu);
    ev.mps_local = sys_cpu_to_le16(chan->le.rx.mps);

    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_EV_RECONFIGURED, NCP_BRIDGE_CMD_RESULT_OK, (uint8_t *)&ev, sizeof(ev));
}
#endif /* CONFIG_BT_L2CAP_ECRED */

/*
 * @brief   Helpers
 */

static channel_t *get_free_channel(void)
{
    channel_t *chan = NULL;

    for (uint8_t i = 0U; i < CHANNELS; i++)
    {
        if (!channels[i].in_use)
        {
            chan = &channels[i];

            (void)memset(chan, 0, sizeof(*chan));
            chan->chan_id = i;

            channels[i].in_use = true;

            break;
        }
    }

    return chan;
}

static int ble_l2cap_accept(struct bt_conn *conn, struct bt_l2cap_server *server, struct bt_l2cap_chan **l2cap_chan)
{
    channel_t *chan;
    int8_t result = 0;
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0))
    if (bt_conn_enc_key_size(conn) < req_keysize)
    {
        result = -EPERM;
    }
    else
#endif
    {
        if (authorize_flag)
        {
            result = -EACCES;
        }
        else
        {
            chan = get_free_channel();

            if (!chan)
            {
                result = -ENOMEM;
            }
            else
            {
                chan->le.chan.ops = &l2cap_ops;
                chan->le.rx.mtu = DATA_MTU_INITIAL;

                *l2cap_chan = &chan->le.chan;
            }
        }
    }

    return result;
}

static bool is_free_psm(uint16_t psm)
{
    bool result = true;

    for (uint8_t i = 0U; i < ARRAY_SIZE(servers); i++)
    {
        if (servers[i].psm == psm)
        {
            result = false;
            break;
        }
    }

    return result;
}

static struct bt_l2cap_server *get_free_server(void)
{
    struct bt_l2cap_server *pFreeServer = NULL;

    for (uint8_t i = 0U; i < SERVERS ; i++)
    {
        if (servers[i].psm == 0)
        {
            pFreeServer = &servers[i];
            break;
        }
    }

    return pFreeServer;
}
#endif
