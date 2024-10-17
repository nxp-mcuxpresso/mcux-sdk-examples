/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if CONFIG_NCP_BLE

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
#define MAX_L2CAP_CHANNEL 1

#define CREDITS			10
#define DATA_MTU		(23 * CREDITS)

#define L2CAP_POLICY_NONE		0x00
#define L2CAP_POLICY_ALLOWLIST		0x01
#define L2CAP_POLICY_16BYTE_KEY		0x02
NET_BUF_POOL_FIXED_DEFINE(data_tx_pool, 1,
			  BT_L2CAP_SDU_BUF_SIZE(DATA_MTU), NULL);
NET_BUF_POOL_FIXED_DEFINE(data_rx_pool, 1, DATA_MTU, NULL);
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
static int l2cap_accept(struct bt_conn *conn, struct bt_l2cap_server *server,
			struct bt_l2cap_chan **chan);
void ble_l2cap_set_recv(uint8_t *data, uint16_t len);
void ble_l2cap_metrics(uint8_t *data, uint16_t len);
void bt_l2cap_register(uint8_t *data, uint16_t len);
void ble_l2cap_connect(uint8_t *data, uint16_t len);
void ble_l2cap_disconnect(uint8_t *data, uint16_t len);
void ble_l2cap_send_data(uint8_t *data, uint16_t len);
int ble_ncp_L2capInit(void);
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

static uint8_t l2cap_policy;

static struct bt_conn *l2cap_allowlist[CONFIG_BT_MAX_CONN];
static uint32_t l2cap_rate;
static uint32_t l2cap_recv_delay_ms;
osa_msgq_handle_t l2cap_recv_fifo;
OSA_MSGQ_HANDLE_DEFINE(l2cap_recv_fifo_handle, CONFIG_BT_MSG_QUEUE_COUNT, sizeof(void*));
static bool metrics;

#define L2CH_CHAN(_chan) CONTAINER_OF(_chan, struct l2ch, ch.chan)
#define L2CH_WORK(_work) CONTAINER_OF(_work, struct l2ch, recv_work)
#define L2CAP_CHAN(_chan) _chan->ch.chan

struct l2ch {
	bool used;
	struct k_work_delayable recv_work;
	struct bt_l2cap_le_chan ch;
};

static struct l2ch l2ch_chan[MAX_L2CAP_CHANNEL];

static struct bt_l2cap_server server = {
	.accept		= l2cap_accept,
};
/*******************************************************************************
 * Code
 ******************************************************************************/
#if 0
/*
 * @brief   Tester App functionality
 */

uint8_t ncp_ble_init_l2cap(void)
{
    return NCP_CMD_RESULT_OK;
}

uint8_t ncp_ble_unregister_l2cap(void)
{
    return NCP_CMD_RESULT_OK;
}

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

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_READ_SUPPORTED_COMMANDS, NCP_CMD_RESULT_OK, (uint8_t *) rp, sizeof(cmds));
}
#endif

static void l2cap_channel_free(struct l2ch *chan)
{
	if (true == chan->used)
	{
		chan->used = false;
	}
}

static int l2cap_recv_metrics(struct bt_l2cap_chan *chan, struct net_buf *buf)
{
	static uint32_t len;
	static uint32_t cycle_stamp;
	uint32_t delta;

	delta = (uint32_t)OSA_TimeGetMsec() - cycle_stamp;
	delta = (uint32_t)(delta * 1000);

	/* if last data rx-ed was greater than 1 second in the past,
	 * reset the metrics.
	 */
	if (delta > 1000000000) {
		len = 0U;
		l2cap_rate = 0U;
		cycle_stamp = (uint32_t)OSA_TimeGetMsec();
	} else {
		len += buf->len;
		l2cap_rate = ((uint64_t)len << 3) * 1000000000U / delta;
	}
        l2cap_rate = l2cap_rate/1000;
	return NCP_CMD_RESULT_OK;
}

static void l2cap_recv_cb(struct k_work *work)
{
	struct l2ch *c = L2CH_WORK(work);
	struct net_buf *buf;

	while ((buf = net_buf_get(l2cap_recv_fifo, osaWaitNone_c))) {
		ncp_d("Confirming reception\n");
		bt_l2cap_chan_recv_complete(&c->ch.chan, buf);
	}
}

static void dump_hex(const void *data, unsigned len)
{
    (void)PRINTF("**** Dump @ %p Len: %d ****\n\r", data, len);

    unsigned int i    = 0;
    const char *data8 = (const char *)data;
    while (i < len)
    {
        (void)PRINTF("%02x ", data8[i++]);
        if (!(i % 16))
        {
            (void)PRINTF("\n\r");
        }
    }

    (void)PRINTF("\n\r******** End Dump *******\n\r");
}

static int l2cap_recv(struct bt_l2cap_chan *chan, struct net_buf *buf)
{
    struct l2ch *l2ch = L2CH_CHAN(chan);
    struct l2cap_reveive_ev ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(chan->conn);

    if (metrics) {
	return l2cap_recv_metrics(chan, buf);
    }

    ncp_d("Incoming data channel %p len %u\n", chan, buf->len);
#if 0
    if (buf->len) {
	dump_hex(buf->data, buf->len);
    }
#endif
    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }
    ev.psm = l2ch->ch.psm;
    memcpy((uint8_t *)ev.data, buf->data, buf->len);
    ev.len = buf->len;
    ncp_d("sizeof(ev) len %d\n", sizeof(ev));
    ble_prepare_status(NCP_EVENT_L2CAP_RECEIVE, NCP_CMD_RESULT_OK, (uint8_t *) &ev,10 + ev.len);

    if (l2cap_recv_delay_ms > 0) {
	/* Submit work only if queue is empty */
	if (0 == OSA_MsgQAvailableMsgs(l2cap_recv_fifo)) {
		ncp_d("Delaying response in %u ms...\n",
	                            l2cap_recv_delay_ms);
		bt_delayed_work_submit(&l2ch->recv_work,
		                    BT_MSEC(l2cap_recv_delay_ms));
	}
	net_buf_put(l2cap_recv_fifo, buf);
	return -EINPROGRESS;
    }

    return NCP_CMD_RESULT_OK;
}

static void l2cap_sent(struct bt_l2cap_chan *chan)
{
    ncp_d("Outgoing data channel %p transmitted\n", chan);
}

static void l2cap_status(struct bt_l2cap_chan *chan, atomic_t *status)
{
    ncp_d("Channel %p status %u\n", chan, (uint32_t)*status);
}

static void l2cap_connected(struct bt_l2cap_chan *chan)
{
    struct l2ch *c = L2CH_CHAN(chan);
    struct l2cap_connect_ev ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(chan->conn);

    bt_delayed_work_init(&c->recv_work, l2cap_recv_cb);
    ncp_d("Channel %p connected\n", chan);
    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }
    ev.psm = c->ch.psm;

    ble_prepare_status(NCP_EVENT_L2CAP_CONNECT, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));

}

static void l2cap_disconnected(struct bt_l2cap_chan *chan)
{
    struct l2ch *l2ch = L2CH_CHAN(chan);
    struct l2cap_connect_ev ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(chan->conn);

    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }
    ev.psm = l2ch->ch.psm;

    ble_prepare_status(NCP_EVENT_L2CAP_DISCONNECT, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));

    ncp_d("Channel %p disconnected\n", chan);
    l2cap_channel_free(l2ch);
}

static struct net_buf *l2cap_alloc_buf(struct bt_l2cap_chan *chan)
{
	/* print if metrics is disabled */
	if (!metrics) {
		ncp_d("Channel %p requires buffer\n", chan);
	}

	return net_buf_alloc(&data_rx_pool, osaWaitForever_c);
}

static const struct bt_l2cap_chan_ops l2cap_ops = {
	.alloc_buf	= l2cap_alloc_buf,
	.recv		= l2cap_recv,
	.sent		= l2cap_sent,
	.status		= l2cap_status,
	.connected	= l2cap_connected,
	.disconnected	= l2cap_disconnected,
};

static struct l2ch * l2cap_channel_create_new(void)
{
	for (int i = 0;i < MAX_L2CAP_CHANNEL;i++)
	{
		if (false == l2ch_chan[i].used)
		{
			l2ch_chan[i].used = true;
			return &l2ch_chan[i];
		}
	}
	return NULL;
}

static struct l2ch * l2cap_channel_lookup_conn(struct bt_conn *conn)
{
	for (int i = 0;i < MAX_L2CAP_CHANNEL;i++)
	{
		if ((true == l2ch_chan[i].used) && (conn == l2ch_chan[i].ch.chan.conn))
		{
			return &l2ch_chan[i];
		}
	}
	return NULL;
}

static void l2cap_allowlist_remove(struct bt_conn *conn, uint8_t reason)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(l2cap_allowlist); i++) {
		if (l2cap_allowlist[i] == conn) {
			bt_conn_unref(l2cap_allowlist[i]);
			l2cap_allowlist[i] = NULL;
		}
	}
}

static struct bt_conn_cb l2cap_conn_callbacks = {
	.disconnected = l2cap_allowlist_remove,
};

static int l2cap_accept_policy(struct bt_conn *conn)
{
	int i;

	if (l2cap_policy == L2CAP_POLICY_16BYTE_KEY) {
#if ((defined(CONFIG_BT_SMP) && ((CONFIG_BT_SMP) > 0U)) || (defined(CONFIG_BT_BREDR) && ((CONFIG_BT_BREDR) > 0U)))
		uint8_t enc_key_size = bt_conn_enc_key_size(conn);

		if (enc_key_size && enc_key_size < BT_ENC_KEY_SIZE_MAX) {
			return -EPERM;
		}
#endif
	} else if (l2cap_policy == L2CAP_POLICY_ALLOWLIST) {
		for (i = 0; i < ARRAY_SIZE(l2cap_allowlist); i++) {
			if (l2cap_allowlist[i] == conn) {
				return 0;
			}
		}

		return -EACCES;
	}

	return 0;
}

static int l2cap_accept(struct bt_conn *conn, struct bt_l2cap_server *server,
			struct bt_l2cap_chan **chan)
{
	struct l2ch *l2cap_channel;
	int err;

	ncp_d("Incoming conn %p\n", conn);

	err = l2cap_accept_policy(conn);
	if (err < 0) {
		return err;
	}

	l2cap_channel = l2cap_channel_create_new();
	if (NULL == l2cap_channel) {
		ncp_d("No channels available\n");
		return -ENOMEM;
	}

	*chan = &l2cap_channel->ch.chan;

	return 0;
}

/*
 * @brief   recv set
 */
void ble_l2cap_set_recv(uint8_t *data, uint16_t len)
{
    const struct l2cap_recv_cmd_tag *cmd = (void *) data;

    l2cap_recv_delay_ms = cmd->l2cap_recv_delay_ms;
    ncp_d("l2cap receive delay: %u ms\n",l2cap_recv_delay_ms);
    ble_prepare_status(NCP_RSP_BLE_L2CAP_RECEIVE, NCP_CMD_RESULT_OK, NULL, 0);
}

/*
 * @brief   CMD METRICS
 */
void ble_l2cap_metrics(uint8_t *data, uint16_t len)
{
    const struct l2cap_metrics_cmd_tag *cmd = (void *) data;
    uint8_t status = NCP_CMD_RESULT_ERROR;

    ncp_d("l2cap rate: %u bps.", l2cap_rate * 1000);
    if (cmd->metrics_flag == true)
    {
        metrics = true;
        status = NCP_CMD_RESULT_OK;
    }
    else if (cmd->metrics_flag == false)
    {
        metrics = false;
        status = NCP_CMD_RESULT_OK;
    }
    else
    {
        status = NCP_CMD_RESULT_ERROR;
    }
    ble_prepare_status(NCP_RSP_BLE_L2CAP_METRICS, status, NULL, 0);
}

/*
 * @brief   Register L2CAP PSM
 */
void bt_l2cap_register(uint8_t *data, uint16_t len)
{
    const struct l2cap_register_psm_cmd_tag *cmd = (void *) data;
    uint8_t status = NCP_CMD_RESULT_ERROR;
    // int err;
    // const char *policy;

    if (server.psm)
    {
        ncp_e("Already registered\n");
        goto sta;
    }

    server.psm = cmd->psm;
    if(cmd->sec_flag == 1)
    {
        server.sec_level = (bt_security_t)cmd->sec_level;
    }

    if(cmd->policy_flag == 1)
    {
	l2cap_policy = L2CAP_POLICY_ALLOWLIST;
    }
    else if (cmd->policy_flag == 2)
    {
	l2cap_policy = L2CAP_POLICY_16BYTE_KEY;
    }
    else
    {
	//do nothing
    }

    if (bt_l2cap_server_register(&server) < 0)
    {
	ncp_e("Unable to register psm\n");
	server.psm = 0U;
    }
    else
    {
	bt_conn_cb_register(&l2cap_conn_callbacks);
        status = NCP_CMD_RESULT_OK;
	ncp_d("L2CAP psm %u sec_level %u registered\n",
			    server.psm, server.sec_level);
    }
sta:
    ble_prepare_status(NCP_RSP_BLE_L2CAP_REGISTER, status, NULL, 0);
}

/*
 * @brief   Create an L2CAP channel
 */
void ble_l2cap_connect(uint8_t *data, uint16_t len)
{
    const struct l2cap_connect_cmd_tag *cmd = (void *) data;
    struct bt_conn *conn;
    uint8_t status = NCP_CMD_RESULT_ERROR;
    int err;
    struct l2ch *l2cap_channel;

    l2cap_channel = l2cap_channel_create_new();
    if (NULL == l2cap_channel) {
         ncp_e("Channel already in use\n");
         goto sta;
    }
    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (conn)
    {
	if (cmd->sec_flag == 1)
        {
#if (defined(CONFIG_BT_L2CAP_DYNAMIC_CHANNEL) && (CONFIG_BT_L2CAP_DYNAMIC_CHANNEL > 0))
            l2cap_channel->ch.required_sec_level = (bt_security_t)cmd->sec;
#endif
	}
	err = bt_l2cap_chan_connect(conn, &l2cap_channel->ch.chan, cmd->psm);
        bt_conn_unref(conn);
        status = err < 0 ? NCP_CMD_RESULT_ERROR : NCP_CMD_RESULT_OK;
    }
sta:
    ble_prepare_status(NCP_RSP_BLE_L2CAP_CONNECT, status, NULL, 0);
}

/*
 * @brief   Close an L2CAP channel
 */
void ble_l2cap_disconnect(uint8_t *data, uint16_t len)
{
    struct bt_conn *conn;
    uint8_t status = NCP_CMD_RESULT_ERROR;
    int err;
    struct l2ch *l2cap_channel;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (conn)
    {
	l2cap_channel = l2cap_channel_lookup_conn(conn);
	if (NULL == l2cap_channel) {
		ncp_e("Channel is not found\n");
                bt_conn_unref(conn);
                goto sta;
	}

	err = bt_l2cap_chan_disconnect(&l2cap_channel->ch.chan);
	if (err) {
		ncp_e("Unable to disconnect: %u\n", -err);
	}
        bt_conn_unref(conn);
        status = err < 0 ? NCP_CMD_RESULT_ERROR : NCP_CMD_RESULT_OK;
    }
sta:
    ble_prepare_status(NCP_RSP_BLE_L2CAP_DISCONNECT, status, NULL, 0);
}

/*
 * @brief   Send data over L2CAP channel
 */
void ble_l2cap_send_data(uint8_t *data, uint16_t len)
{
    const struct l2cap_send_data_cmd_tag *cmd = (void *) data;
    struct bt_conn *conn;
    uint8_t status = NCP_CMD_RESULT_ERROR;
    struct l2ch *l2cap_channel;
    static uint8_t buf_data[DATA_MTU] = { [0 ... (DATA_MTU - 1)] = 0xff };
    int ret, length, count = 1;
    struct net_buf *buf;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (conn)
    {

	count = cmd->times;

	l2cap_channel = l2cap_channel_lookup_conn(conn);
	if (NULL == l2cap_channel) {
	        ncp_e("Channel is not found\n");
                goto unref;
	}

	length = MIN(l2cap_channel->ch.tx.mtu, DATA_MTU - BT_L2CAP_CHAN_SEND_RESERVE);

	while (count--) {
		ncp_d("Rem %d\n", count);
		buf = net_buf_alloc(&data_tx_pool, BT_SECONDS(2));
		if (!buf) {
			if (l2ch_chan[0].ch.state != BT_L2CAP_CONNECTED) {
				ncp_e("Channel disconnected, stopping TX\n");
                                goto unref;
			}
			ncp_e("Allocation timeout, stopping TX\n");
                        goto unref;
		}
		net_buf_reserve(buf, BT_L2CAP_SDU_CHAN_SEND_RESERVE);

		net_buf_add_mem(buf, buf_data, length);
		ret = bt_l2cap_chan_send(&l2cap_channel->ch.chan, buf);
		if (ret < 0) {
			ncp_e("Unable to send: %d\n", -ret);
			net_buf_unref(buf);
		}
                status = ret < 0 ? NCP_CMD_RESULT_ERROR : NCP_CMD_RESULT_OK; 
	}
    }
    else
    {
        LOG_ERR("Unknown connection");
        status = NCP_CMD_RESULT_ERROR;
        goto sta;
    }

unref:
    bt_conn_unref(conn);
sta:
    ble_prepare_status(NCP_RSP_BLE_L2CAP_SEND, status, NULL, 0);
}

#if 0
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

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_LISTEN, NCP_CMD_RESULT_OK, NULL, 0);
    return;

fail:
    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_LISTEN, NCP_CMD_RESULT_ERROR, NULL, 0);
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
        status = NCP_CMD_RESULT_ERROR;
        goto rsp;
    }

    if (cmd->num > CHANNELS)
    {
        status = NCP_CMD_RESULT_ERROR;
        goto unref;
    }

    if (mtu > DATA_MTU)
    {
        status = NCP_CMD_RESULT_ERROR;
        goto unref;
    }

    for (int32_t i = 0; i < cmd->num; i++)
    {
        if (cmd->chan_id[i] > CHANNELS)
        {
            status = NCP_CMD_RESULT_ERROR;
            goto unref;
        }

        reconf_channels[i] = &channels[cmd->chan_id[i]].le.chan;
    }

    err = bt_l2cap_ecred_chan_reconfigure(reconf_channels, mtu);

    if (err)
    {
            status = NCP_CMD_RESULT_ERROR;
            goto unref;
    }

    status = NCP_CMD_RESULT_OK;

unref:
    bt_conn_unref(conn);
rsp:
    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_RECONFIGURE, status, NULL, 0);
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

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_CREDITS, NCP_CMD_RESULT_OK, NULL, 0);
    return;

fail:
    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_CREDITS, NCP_CMD_RESULT_ERROR, NULL, 0);
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
        status = NCP_CMD_RESULT_ERROR;
        goto failed;
    }

    for (int i = 0; i < cmd->count; i++)
    {
        err = bt_eatt_disconnect_one(conn);
        if (err)
        {
            status = NCP_CMD_RESULT_ERROR;
            goto unref;
        }
    }

    status = NCP_CMD_RESULT_OK;

unref:
    bt_conn_unref(conn);
failed:
    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_DISCONNECT_EATT_CHANS, status, NULL, 0);
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

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_EV_CONNECTED, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
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

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_EV_DISCONNECTED, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
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

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_EV_DATA_RECEIVED, NCP_CMD_RESULT_OK, recv_cb_buf, sizeof(*ev) + buf->len);

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

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_L2CAP) << 16) | L2CAP_EV_RECONFIGURED, NCP_CMD_RESULT_OK, (uint8_t *)&ev, sizeof(ev));
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

int ble_ncp_L2capInit(void)
{
    osa_status_t ret;

    for (int i = 0;i < MAX_L2CAP_CHANNEL;i++)
    {
        l2ch_chan[i].ch.chan.ops = &l2cap_ops;
        l2ch_chan[i].ch.rx.mtu = DATA_MTU;
    }
    ret = OSA_MsgQCreate((osa_msgq_handle_t)l2cap_recv_fifo_handle, CONFIG_BT_MSG_QUEUE_COUNT, sizeof(void *));
    if (KOSA_StatusSuccess == ret)
    {
        l2cap_recv_fifo = (osa_msgq_handle_t)l2cap_recv_fifo_handle;
    }
    else
    {
        ncp_e("Message queue create failed (%d)!\n", ret);
    }
    return (int)ret;
}

#endif /* CONFIG_NCP_BLE */