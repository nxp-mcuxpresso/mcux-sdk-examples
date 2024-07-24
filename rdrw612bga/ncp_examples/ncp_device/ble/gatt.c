/*
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if CONFIG_NCP_BLE

#include "fsl_component_serial_manager.h"
#include "fsl_debug_console.h"


#include "ncp_ble.h"
#include "ncp_glue_ble.h"
#include <sys/atomic.h>

#include <bluetooth/att.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/uuid.h>
#include <bluetooth/hci.h>
#include <bluetooth/att.h>
#include <bluetooth/l2cap.h>

#include <net/buf.h>
#include <bluetooth/buf.h>

#include "fsl_component_log_config.h"
#define LOG_MODULE_NAME bt_gatt
#include "fsl_component_log.h"
LOG_MODULE_DEFINE(LOG_MODULE_NAME, kLOG_LevelTrace);

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! Value of x rounded up to the next multiple of align, which must be a power of 2. */
#define ROUND_UP(x, align)                                   \
    (((unsigned long)(x) + ((unsigned long)(align) - 1)) & \
     ~((unsigned long)(align) - 1))

#define CONTROLLER_INDEX                0
#define MAX_BUFFER_SIZE                 2048
#define MAX_UUID_LEN                    16
#define MAX_NOTIF_DATA                  (MIN(BT_L2CAP_RX_MTU, BT_L2CAP_TX_MTU) - 3)
#define UNUSED_SUBSCRIBE_CCC_HANDLE     0x0000
#define MAX_SUBSCRIPTIONS               2

/* This masks Permission bits from GATT API */
#define GATT_PERM_MASK                  (BT_GATT_PERM_READ | \
                                        BT_GATT_PERM_READ_AUTHEN | \
                                        BT_GATT_PERM_READ_ENCRYPT | \
                                        BT_GATT_PERM_WRITE | \
                                        BT_GATT_PERM_WRITE_AUTHEN | \
                                        BT_GATT_PERM_WRITE_ENCRYPT | \
                                        BT_GATT_PERM_PREPARE_WRITE)
#define GATT_PERM_ENC_READ_MASK         (BT_GATT_PERM_READ_ENCRYPT | \
                                        BT_GATT_PERM_READ_AUTHEN)
#define GATT_PERM_ENC_WRITE_MASK        (BT_GATT_PERM_WRITE_ENCRYPT | \
                                        BT_GATT_PERM_WRITE_AUTHEN)
#define GATT_PERM_READ_AUTHORIZATION    0x40
#define GATT_PERM_WRITE_AUTHORIZATION   0x80

/* bt_gatt_attr_next cannot be used on non-registered services */
#define NEXT_DB_ATTR(attr) (attr + 1)
#define LAST_DB_ATTR (server_db + (attr_count - 1))

#define server_buf_push(_len)   net_buf_push(server_buf, ROUND_UP(_len, 4))
#define server_buf_pull(_len)   net_buf_pull(server_buf, ROUND_UP(_len, 4))

#define DATA_BUF_SIZE (sizeof(gatt_attr_value_changed_ev_t) + BT_ATT_MAX_ATTRIBUTE_LEN)

NET_BUF_POOL_FIXED_DEFINE(data_pool, 1, DATA_BUF_SIZE, NULL);

/*******************************************************************************
 * Types
 ******************************************************************************/
union uuid {
    struct bt_uuid uuid;
    struct bt_uuid_16 u16;
    struct bt_uuid_128 u128;
};

struct add_characteristic {
    uint16_t char_id;
    uint8_t properties;
    uint8_t permissions;
    const struct bt_uuid *uuid;
};

struct gatt_value {
    uint16_t len;
    uint8_t *data;
    uint8_t enc_key_size;
    uint8_t flags[1];
};

enum {
    GATT_VALUE_CCC_FLAG,
    GATT_VALUE_READ_AUTHOR_FLAG,
    GATT_VALUE_WRITE_AUTHOR_FLAG,
};

struct add_descriptor {
    uint16_t desc_id;
    uint8_t permissions;
    const struct bt_uuid *uuid;
};

struct set_value {
    const uint8_t *value;
    uint16_t len;
};

static struct {
    uint16_t len;
    uint8_t buf[MAX_BUFFER_SIZE];
} gatt_buf;

struct get_attrs_foreach_data {
    struct net_buf_simple *buf;
    struct bt_uuid *uuid;
    uint8_t count;
};

struct get_attr_data {
    struct net_buf_simple *buf;
    struct bt_conn *conn;
};

struct net_buf_simple_1 {
    struct net_buf_simple buf;
    uint8_t size;
    uint8_t data[];
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Command handlers */
#if 0
static void supported_commands(uint8_t *data, uint16_t len);
static void add_included(uint8_t *data, uint16_t len);
#endif
void set_value(uint8_t *data, uint16_t len);
#if 0
static void set_enc_key_size(uint8_t *data, uint16_t len);
static void exchange_mtu(uint8_t *data, uint16_t len);
static void disc_all_prim(uint8_t *data, uint16_t len);
#endif
void disc_prim_uuid(uint8_t *data, uint16_t len);
#if 0
static void find_included(uint8_t *data, uint16_t len);
static void disc_all_chrc(uint8_t *data, uint16_t len);
#endif
void disc_chrc_uuid(uint8_t *data, uint16_t len);
#if 0
static void disc_all_desc(uint8_t *data, uint16_t len);
#endif
void read_data(uint8_t *data, uint16_t len);
#if 0
static void read_uuid(uint8_t *data, uint16_t len);
static void read_long(uint8_t *data, uint16_t len);
static void read_multiple(uint8_t *data, uint16_t len, uint8_t opcode);
static void write_without_rsp(uint8_t *data, uint16_t len, uint8_t op, bool sign);
#endif
void write_data(uint8_t *data, uint16_t len);
#if 0
static void write_long(uint8_t *data, uint16_t len);
#endif
void config_subscription(uint8_t *data, uint16_t len, uint16_t op);
#if 0
static void get_attrs(uint8_t *data, uint16_t len);
static void get_attr_val(uint8_t *data, uint16_t len);

#if (defined(CONFIG_BT_GATT_NOTIFY_MULTIPLE) && (CONFIG_BT_GATT_NOTIFY_MULTIPLE > 0))
static void notify_cb(struct bt_conn *conn, void *user_data);
static void notify_mult(uint8_t *data, uint16_t len, uint16_t op);
#endif /* CONFIG_BT_GATT_NOTIFY_MULTIPLE */
#endif

/* Events */
static void attr_value_changed_ev(uint16_t handle, const uint8_t *value, const uint16_t len);
static uint8_t notify_func(struct bt_conn *conn,
                           struct bt_gatt_subscribe_params *params,
                           const void *data, uint16_t length);

/* Helpers */
static int register_service(void);
static struct bt_gatt_attr *gatt_db_add
(
    const struct bt_gatt_attr *pattern,
    size_t user_data_len
);
static uint8_t btp2bt_uuid
(
    const uint8_t *uuid,
    uint8_t len,
    struct bt_uuid *bt_uuid
);
static ssize_t read_value(struct bt_conn *conn, const struct bt_gatt_attr *attr,
              void *buf, uint16_t len, uint16_t offset);
static ssize_t write_value(struct bt_conn *conn,
               const struct bt_gatt_attr *attr, const void *buf,
               uint16_t len, uint16_t offset, uint8_t flags);
static int alloc_included(struct bt_gatt_attr *attr,
                          uint16_t *included_service_id, uint16_t svc_handle);
static uint8_t alloc_value(struct bt_gatt_attr *attr, struct set_value *data);
static uint8_t set_cep_value(struct bt_gatt_attr *attr, const void *value,
                             const uint16_t len);
static void indicate_cb(struct bt_conn *conn,
                        struct bt_gatt_indicate_params *params, uint8_t err);
#if 0
static int set_attr_enc_key_size(const struct bt_gatt_attr *attr,
                                 uint8_t key_size);
static void exchange_func(struct bt_conn *conn, uint8_t err,
                          struct bt_gatt_exchange_params *params);
static uint8_t find_included_cb(struct bt_conn *conn,
                                const struct bt_gatt_attr *attr,
                                struct bt_gatt_discover_params *params);
#endif
static uint8_t disc_chrc_cb(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr,
                            struct bt_gatt_discover_params *params);
static void *gatt_buf_reserve(size_t len);
static void *gatt_buf_add(const void *data, size_t len);
static void discover_destroy(struct bt_gatt_discover_params *params);
static uint8_t disc_prim_cb(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr,
                            struct bt_gatt_discover_params *params);
static void gatt_buf_clear(void);
static void gatt_buf_clear_offset(uint16_t offset);
static uint8_t disc_desc_cb(struct bt_conn *conn,
                                const struct bt_gatt_attr *attr,
                                struct bt_gatt_discover_params *params);
static uint8_t read_cb(struct bt_conn *conn, uint8_t err,
                       struct bt_gatt_read_params *params, const void *data,
                       uint16_t length);
#if 0
static uint8_t read_uuid_cb(struct bt_conn *conn, uint8_t err,
               struct bt_gatt_read_params *params, const void *data,
               uint16_t length);
#endif
static void write_rsp(struct bt_conn *conn, uint8_t err,
                      struct bt_gatt_write_params *params);
#if 0
static void write_long_rsp(struct bt_conn *conn, uint8_t err,
                           struct bt_gatt_write_params *params);
#endif
static int enable_subscription(struct bt_conn *conn, uint16_t ccc_handle,
                               uint16_t value);
static int disable_subscription(struct bt_conn *conn, uint16_t ccc_handle);
static uint8_t discover_func(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr,
                             struct bt_gatt_discover_params *params);
static void discover_complete(struct bt_conn *conn,
                              struct bt_gatt_discover_params *params);
static struct bt_gatt_subscribe_params *find_subscription(uint16_t ccc_handle);
#if 0
static uint8_t get_attrs_rp(const struct bt_gatt_attr *attr, uint16_t handle,
                            void *user_data);
static uint8_t get_attr_val_rp(const struct bt_gatt_attr *attr, uint16_t handle,
                               void *user_data);
static uint8_t err_to_att(int err);
#endif
static uint8_t get_chrc_index(const struct bt_uuid *uuid);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static struct bt_gatt_service server_svcs[SERVER_MAX_SERVICES];
static struct bt_gatt_attr server_db[SERVER_MAX_ATTRIBUTES];
static struct net_buf *server_buf;
NET_BUF_POOL_DEFINE(server_pool, 1, SERVER_BUF_SIZE, 0, NULL);

static uint8_t attr_count = 0;
static uint8_t svc_attr_count = 0;
static uint8_t svc_count = 0;

static bool ccc_added;
static uint8_t ccc_value;

static uint8_t ccc_ev_buf[sizeof(gatt_ccc_cfg_changed_ev_t)];


struct bt_gatt_indicate_params indicate_params;
#if 0
static struct bt_gatt_exchange_params exchange_params;
#endif
static struct bt_gatt_discover_params discover_params;
static uint8_t btp_opcode;
static union uuid uuid;

static struct bt_gatt_write_params write_params;
static uint8_t ev_buf[sizeof(gatt_notification_ev_t) + MAX_NOTIF_DATA];
static struct bt_gatt_subscribe_params subscriptions[MAX_SUBSCRIPTIONS];

static struct bt_gatt_read_params read_params;
#if 0
static struct bt_gatt_read_params read_uuid_params;
static struct bt_gatt_read_params read_long_params;
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

#if (defined(CONFIG_BT_EATT) && (CONFIG_BT_EATT > 0))
static void eatt_connect(uint8_t *data, uint16_t len)
{
	const struct gatt_eatt_connect_cmd *cmd = (void *)data;
	struct bt_conn *conn;
	uint8_t status = NCP_CMD_RESULT_OK;
	int err;

	conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)cmd);
	if (!conn) {
		status = NCP_CMD_RESULT_ERROR;
		goto response;
	}

	err = bt_eatt_connect(conn, cmd->num_channels);
	if (err) {
		status = NCP_CMD_RESULT_ERROR;
	}

	bt_conn_unref(conn);

response:
	ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_EATT_CONNECT, status, NULL, 0);
}
#endif /* CONFIG_BT_EATT */

uint8_t ncp_ble_init_gatt(void)
{
    server_buf = net_buf_alloc(&server_pool, K_NO_WAIT);
    if (!server_buf)
    {
        return NCP_CMD_RESULT_ERROR;
    }

    net_buf_reserve(server_buf, SERVER_BUF_SIZE);

    return NCP_CMD_RESULT_OK;
}

#if 0
uint8_t ncp_ble_unregister_gatt(void)
{
    return NCP_CMD_RESULT_OK;
}

/*
 * @brief   Command handlers
 */

/*
 * @brief   Each bit in response is a flag indicating if command with
 *          opcode matching bit number is supported
 */
static void supported_commands(uint8_t *data, uint16_t len)
{
    uint8_t cmds[5];
    gatt_read_supported_commands_rp_t *rp = (void *) cmds;

    (void)memset(cmds, 0, sizeof(cmds));

    ncp_ble_set_bit(cmds, GATT_READ_SUPPORTED_COMMANDS);
    ncp_ble_set_bit(cmds, GATT_ADD_SERVICE);
    ncp_ble_set_bit(cmds, GATT_ADD_CHARACTERISTIC);
    ncp_ble_set_bit(cmds, GATT_ADD_DESCRIPTOR);
    ncp_ble_set_bit(cmds, GATT_ADD_INCLUDED_SERVICE);
    ncp_ble_set_bit(cmds, GATT_SET_VALUE);
    ncp_ble_set_bit(cmds, GATT_START_SERVER);
    ncp_ble_set_bit(cmds, GATT_SET_ENC_KEY_SIZE);
    ncp_ble_set_bit(cmds, GATT_EXCHANGE_MTU);
    ncp_ble_set_bit(cmds, GATT_DISC_PRIM_UUID);
    ncp_ble_set_bit(cmds, GATT_FIND_INCLUDED);
    ncp_ble_set_bit(cmds, GATT_DISC_ALL_CHRC);
    ncp_ble_set_bit(cmds, GATT_DISC_CHRC_UUID);
    ncp_ble_set_bit(cmds, GATT_DISC_ALL_DESC);
    ncp_ble_set_bit(cmds, GATT_READ);
    ncp_ble_set_bit(cmds, GATT_READ_LONG);
    ncp_ble_set_bit(cmds, GATT_READ_MULTIPLE);
    ncp_ble_set_bit(cmds, GATT_WRITE_WITHOUT_RSP);
    ncp_ble_set_bit(cmds, GATT_SIGNED_WRITE_WITHOUT_RSP);
    ncp_ble_set_bit(cmds, GATT_WRITE);
    ncp_ble_set_bit(cmds, GATT_WRITE_LONG);
    ncp_ble_set_bit(cmds, GATT_CFG_NOTIFY);
    ncp_ble_set_bit(cmds, GATT_CFG_INDICATE);
    ncp_ble_set_bit(cmds, GATT_GET_ATTRIBUTES);
    ncp_ble_set_bit(cmds, GATT_GET_ATTRIBUTE_VALUE);
    ncp_ble_set_bit(cmds, GATT_DISC_ALL_PRIM);
    ncp_ble_set_bit(cmds, GATT_READ_MULTIPLE_VAR);
    ncp_ble_set_bit(cmds, GATT_NOTIFY_MULTIPLE);

#if (defined(CONFIG_BT_EATT) && (CONFIG_BT_EATT > 0))
    ncp_ble_set_bit(cmds, GATT_EATT_CONNECT);
#endif /* CONFIG_BT_EATT */

    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_READ_SUPPORTED_COMMANDS, NCP_CMD_RESULT_OK, (uint8_t *) rp, sizeof(cmds));
}
#endif

/*
 * @brief   This command is used to add new service to GATT Server.
 */
int add_service(const void *cmd, uint16_t cmd_len, void *rsp)
{
    const struct gatt_add_service_cmd *cp = cmd;
    struct gatt_add_service_rp *rp = rsp;
    struct bt_gatt_attr *attr_svc = NULL;
    union uuid uuid;
    size_t uuid_size;

    // if ((cmd_len < sizeof(*cp)) ||
    //     (cmd_len != sizeof(*cp) + cp->uuid_length)) {
    //     return NCP_CMD_RESULT_ERROR;
    // }

    if (btp2bt_uuid(cp->uuid, cp->uuid_length, &uuid.uuid)) {
        return NCP_CMD_RESULT_ERROR;
    }

    uuid_size = uuid.uuid.type == BT_UUID_TYPE_16 ? sizeof(uuid.u16) :
                            sizeof(uuid.u128);

    /* Register last defined service */
    if (svc_attr_count) {
        if (register_service()) {
            return NCP_CMD_RESULT_ERROR;
        }
    }

    svc_count++;

    switch (cp->type) {
    case GATT_SERVICE_PRIMARY:
        attr_svc = gatt_db_add(&(struct bt_gatt_attr)
                       BT_GATT_PRIMARY_SERVICE(&uuid.uuid),
                       uuid_size);
        break;
    case GATT_SERVICE_SECONDARY:
        attr_svc = gatt_db_add(&(struct bt_gatt_attr)
                       BT_GATT_SECONDARY_SERVICE(&uuid.uuid),
                       uuid_size);
        break;
    }

    if (!attr_svc) {
        svc_count--;
        return NCP_CMD_RESULT_ERROR;
    }

    if(rp != NULL) {
        rp->svc_id = sys_cpu_to_le16(attr_svc->handle);
    }

    return NCP_CMD_RESULT_OK;
}

static int alloc_characteristic(struct add_characteristic *ch)
{
    struct bt_gatt_attr *attr_chrc, *attr_value;
    struct bt_gatt_chrc *chrc_data;
    struct gatt_value value;

    /* Add Characteristic Declaration */
    attr_chrc = gatt_db_add(&(struct bt_gatt_attr)
                BT_GATT_ATTRIBUTE(BT_UUID_GATT_CHRC,
                          BT_GATT_PERM_READ,
                          bt_gatt_attr_read_chrc,
                          NULL,
                          (&(struct bt_gatt_chrc){0})),
                sizeof(*chrc_data));

    if (!attr_chrc)
    {
        return -EINVAL;
    }

    (void)memset(&value, 0, sizeof(value));

    if (ch->permissions & GATT_PERM_READ_AUTHORIZATION)
    {
        ncp_ble_set_bit(value.flags, GATT_VALUE_READ_AUTHOR_FLAG);

        /* To maintain backward compatibility, set Read Permission */
        if (!(ch->permissions & GATT_PERM_ENC_READ_MASK))
        {
            ch->permissions |= BT_GATT_PERM_READ;
        }
    }

    if (ch->permissions & GATT_PERM_WRITE_AUTHORIZATION)
    {
        ncp_ble_set_bit(value.flags, GATT_VALUE_WRITE_AUTHOR_FLAG);

        /* To maintain backward compatibility, set Write Permission */
        if (!(ch->permissions & GATT_PERM_ENC_WRITE_MASK))
        {
            ch->permissions |= BT_GATT_PERM_WRITE;
        }
    }

    /* Allow prepare writes */
    ch->permissions |= BT_GATT_PERM_PREPARE_WRITE;

    /* Add Characteristic Value */
    attr_value = gatt_db_add(&(struct bt_gatt_attr)
                 BT_GATT_ATTRIBUTE(ch->uuid,
                    ch->permissions & GATT_PERM_MASK,
                    read_value, write_value, &value),
                    sizeof(value));

    if (!attr_value)
    {
        server_buf_pull(sizeof(*chrc_data));
        /* Characteristic attribute uuid has constant length */
        server_buf_pull(sizeof(uint16_t));
        return -EINVAL;
    }

    chrc_data = attr_chrc->user_data;
    chrc_data->properties = ch->properties;
    chrc_data->uuid = attr_value->uuid;

    ch->char_id = attr_chrc->handle;
    return 0;
}

int add_characteristic(const void *cmd, uint16_t cmd_len, void *rsp)
{
    const gatt_add_characteristic_cmd_t *cp = cmd;
    struct gatt_add_characteristic_rp *rp = rsp;
    struct add_characteristic cmd_data;
    union uuid uuid;

    // if ((cmd_len < sizeof(*cp)) ||
    //     (cmd_len != sizeof(*cp) + cp->uuid_length)) {
    //     return NCP_CMD_RESULT_ERROR;
    // }

    /* Pre-set char_id */
    cmd_data.char_id = 0U;
    cmd_data.permissions = cp->permissions;
    cmd_data.properties = cp->properties;
    cmd_data.uuid = &uuid.uuid;

	if (btp2bt_uuid(cp->uuid, cp->uuid_length, &uuid.uuid)) {
		return NCP_CMD_RESULT_ERROR;
	}

	/* characteristic must be added only sequential */
	if (cp->svc_id) {
		return NCP_CMD_RESULT_ERROR;
	}

	if (alloc_characteristic(&cmd_data)) {
		return NCP_CMD_RESULT_ERROR;
	}

	ccc_added = false;

    if(rp != NULL) {
	    rp->char_id = sys_cpu_to_le16(cmd_data.char_id);
    }

	return NCP_CMD_RESULT_OK;
}

static struct bt_gatt_attr *add_cep(const struct bt_gatt_attr *attr_chrc)
{
    struct bt_gatt_chrc *chrc = attr_chrc->user_data;
    struct bt_gatt_cep cep_value;

    /* Extended Properties bit shall be set */
    if (!(chrc->properties & BT_GATT_CHRC_EXT_PROP))
    {
        return NULL;
    }

    cep_value.properties = 0x0000;

    /* Add CEP descriptor to GATT database */
    return gatt_db_add(&(struct bt_gatt_attr) BT_GATT_CEP(&cep_value),
               sizeof(cep_value));
}

static void ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    struct gatt_ccc_cfg_changed_ev *ev = (void *) ccc_ev_buf;

    //need add a configure changed event
    ev->ccc_value = value;
    if(attr->uuid->type == BT_UUID_TYPE_16)
    {
        uint16_t u16 = sys_cpu_to_le16(BT_UUID_16(attr->uuid)->val);
        ev->uuid_length = 2;
        memcpy(ev->uuid, &u16, ev->uuid_length);
    }
    else
    {
        ev->uuid_length = 16;
        memcpy(ev->uuid, BT_UUID_128(attr->uuid)->val, ev->uuid_length);
    }
    ccc_value = value;

    ble_prepare_status(NCP_EVENT_GATT_CCC_CFG_CHANGED, NCP_CMD_RESULT_OK, ccc_ev_buf, sizeof(*ev));
}

static struct bt_gatt_attr ccc = BT_GATT_CCC(ccc_cfg_changed,
                         BT_GATT_PERM_READ |
                         BT_GATT_PERM_WRITE);

static struct bt_gatt_attr *add_ccc(const struct bt_gatt_attr *attr)
{
    struct bt_gatt_attr *attr_desc;
    struct bt_gatt_chrc *chrc = attr->user_data;
    struct gatt_value *value = NEXT_DB_ATTR(attr)->user_data;

    /* Fail if another CCC already exist on server */
    if (ccc_added)
    {
        return NULL;
    }

    /* Check characteristic properties */
    if (!(chrc->properties &
        (BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_INDICATE)))
    {
        return NULL;
    }

    /* Add CCC descriptor to GATT database */
    attr_desc = gatt_db_add(&ccc, 0);
    if (!attr_desc)
    {
        return NULL;
    }

    ncp_ble_set_bit(value->flags, GATT_VALUE_CCC_FLAG);
    ccc_added = true;

    return attr_desc;
}

static int alloc_descriptor(const struct bt_gatt_attr *attr,
                struct add_descriptor *d)
{
    struct bt_gatt_attr *attr_desc;
    struct gatt_value value;

    if (!bt_uuid_cmp(d->uuid, BT_UUID_GATT_CEP))
    {
        attr_desc = add_cep(attr);
    } else if (!bt_uuid_cmp(d->uuid, BT_UUID_GATT_CCC))
    {
        attr_desc = add_ccc(attr);
    }
    else
    {
        (void)memset(&value, 0, sizeof(value));

        if (d->permissions & GATT_PERM_READ_AUTHORIZATION)
        {
            ncp_ble_set_bit(value.flags,
                       GATT_VALUE_READ_AUTHOR_FLAG);

            /*
             * To maintain backward compatibility,
             * set Read Permission
             */
            if (!(d->permissions & GATT_PERM_ENC_READ_MASK))
            {
                d->permissions |= BT_GATT_PERM_READ;
            }
        }

        if (d->permissions & GATT_PERM_WRITE_AUTHORIZATION)
        {
            ncp_ble_set_bit(value.flags,
                       GATT_VALUE_WRITE_AUTHOR_FLAG);

            /*
             * To maintain backward compatibility,
             * set Write Permission
             */
            if (!(d->permissions & GATT_PERM_ENC_WRITE_MASK))
            {
                d->permissions |= BT_GATT_PERM_WRITE;
            }
        }

        /* Allow prepare writes */
        d->permissions |= BT_GATT_PERM_PREPARE_WRITE;

        attr_desc = gatt_db_add(&(struct bt_gatt_attr)
                    BT_GATT_DESCRIPTOR(d->uuid,
                        d->permissions & GATT_PERM_MASK,
                        read_value, write_value,
                        &value), sizeof(value));
    }

    if (!attr_desc)
    {
        return -EINVAL;
    }

    d->desc_id = attr_desc->handle;
    return 0;
}

static struct bt_gatt_attr *get_base_chrc(struct bt_gatt_attr *attr)
{
    struct bt_gatt_attr *tmp;

    for (tmp = attr; tmp > server_db; tmp--)
    {
        /* Service Declaration cannot precede Descriptor declaration */
        if (!bt_uuid_cmp(tmp->uuid, BT_UUID_GATT_PRIMARY) ||
            !bt_uuid_cmp(tmp->uuid, BT_UUID_GATT_SECONDARY)) {
            break;
        }

        if (!bt_uuid_cmp(tmp->uuid, BT_UUID_GATT_CHRC))
        {
            return tmp;
        }
    }

    return NULL;
}

int add_descriptor(const void *cmd, uint16_t cmd_len, void *rsp)
{
    const gatt_add_descriptor_cmd_t *cp = cmd;
    gatt_add_descriptor_rp_t *rp = rsp;
    struct add_descriptor cmd_data;
    struct bt_gatt_attr *chrc;
    union uuid uuid;

    // if ((cmd_len < sizeof(*cp)) ||
    //     (cmd_len != sizeof(*cp) + cp->uuid_length)) {
    //     return NCP_CMD_RESULT_ERROR;
    // }

    /* Must be declared first svc or at least 3 attrs (svc+char+char val) */
    if (!svc_count || attr_count < 3)
    {
        return NCP_CMD_RESULT_ERROR;
    }

	/* Pre-set desc_id */
	cmd_data.desc_id = 0U;
	cmd_data.permissions = cp->permissions;
	cmd_data.uuid = &uuid.uuid;

	if (btp2bt_uuid(cp->uuid, cp->uuid_length, &uuid.uuid)) {
		return NCP_CMD_RESULT_ERROR;
	}

	/* descriptor can be added only sequential */
	if (cp->char_id) {
		return NCP_CMD_RESULT_ERROR;
	}

	/* Lookup preceding Characteristic Declaration here */
	chrc = get_base_chrc(LAST_DB_ATTR);
	if (!chrc) {
		return NCP_CMD_RESULT_ERROR;
	}

	if (alloc_descriptor(chrc, &cmd_data)) {
		return NCP_CMD_RESULT_ERROR;
	}

    if(rp != NULL) {
	    rp->desc_id = sys_cpu_to_le16(cmd_data.desc_id);
    }

	return NCP_CMD_RESULT_OK;
}

#if 0
static void add_included(uint8_t *data, uint16_t len)
{
    const struct gatt_add_included_service_cmd *cmd = (void *) data;
    gatt_add_included_service_rp_t rp;
    struct bt_gatt_attr *svc;
    uint16_t included_service_id = 0U;

    if (!svc_count || !cmd->svc_id)
    {
        goto fail;
    }

    svc = &server_db[cmd->svc_id - 1];

    /* Fail if attribute stored under requested handle is not a service */
    if (bt_uuid_cmp(svc->uuid, BT_UUID_GATT_PRIMARY) &&
        bt_uuid_cmp(svc->uuid, BT_UUID_GATT_SECONDARY))
    {
        goto fail;
    }

    if (alloc_included(svc, &included_service_id, cmd->svc_id))
    {
        goto fail;
    }

    rp.included_service_id = sys_cpu_to_le16(included_service_id);
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_ADD_INCLUDED_SERVICE, NCP_CMD_RESULT_OK, (uint8_t *) &rp, sizeof(rp));
    return;

fail:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_ADD_INCLUDED_SERVICE, NCP_CMD_RESULT_ERROR, NULL, 0);
}
#endif

void set_value(uint8_t *data, uint16_t len)
{
    const gatt_set_value_cmd_t *cmd = (void *) data;
    struct set_value cmd_data;
    uint8_t status;
    uint8_t attr_index;
    union uuid uuid;

    btp2bt_uuid(cmd->uuid, cmd->uuid_length, &uuid.uuid);

    cmd_data.value = cmd->value;
    cmd_data.len = sys_le16_to_cpu(cmd->len);
    attr_index = get_chrc_index(&uuid.uuid);

    /* set value of local attr, corrected by pre set attr handles */
    status = alloc_value(&server_db[attr_index], &cmd_data);

    ble_prepare_status(NCP_RSP_BLE_GATT_SET_VALUE, status, NULL, 0);
}

int start_server(uint8_t *data, uint16_t len)
{
    gatt_start_server_rp_t rp;

    (void)memset(&rp, 0, sizeof(rp));

    /* Register last defined service */
    if (svc_attr_count)
    {
        if (register_service())
        {
            return NCP_CMD_RESULT_ERROR;
        }
    }

    return NCP_CMD_RESULT_OK;
}

#if 0
static void set_enc_key_size(uint8_t *data, uint16_t len)
{
    const gatt_set_enc_key_size_cmd_t *cmd = (void *) data;
    uint8_t status;

    /* Fail if requested key size is invalid */
    if (cmd->key_size < 0x07 || cmd->key_size > 0x0f)
    {
        status = NCP_CMD_RESULT_ERROR;
        goto fail;
    }

    if (!cmd->attr_id)
    {
        status = set_attr_enc_key_size(LAST_DB_ATTR, cmd->key_size);
    }
    else
    {
        /* set value of local attr, corrected by pre set attr handles */
        status = set_attr_enc_key_size(&server_db[cmd->attr_id -
                           server_db[0].handle],
                           cmd->key_size);
    }

fail:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_SET_ENC_KEY_SIZE, status, NULL, 0);
}

static void exchange_mtu(uint8_t *data, uint16_t len)
{
    struct bt_conn *conn;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (!conn)
    {
        goto fail;
    }

    exchange_params.func = exchange_func;

    if (bt_gatt_exchange_mtu(conn, &exchange_params) < 0)
    {
        bt_conn_unref(conn);

        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_EXCHANGE_MTU, NCP_CMD_RESULT_ERROR, NULL, 0);
}

static void disc_all_prim(uint8_t *data, uint16_t len)
{
    struct bt_conn *conn;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (!conn)
    {
        goto fail_conn;
    }

    discover_params.uuid = NULL;
    discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
    discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
    discover_params.type = BT_GATT_DISCOVER_PRIMARY;
    discover_params.func = disc_prim_cb;

    btp_opcode = GATT_DISC_ALL_PRIM;

    if (bt_gatt_discover(conn, &discover_params) < 0)
    {
        discover_destroy(&discover_params);

        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    bt_conn_unref(conn);

fail_conn:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_DISC_ALL_PRIM, NCP_CMD_RESULT_ERROR, NULL, 0);
}
#endif 

void disc_prim_uuid(uint8_t *data, uint16_t len)
{
    const gatt_disc_prim_uuid_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        goto fail_conn;
    }

    if (btp2bt_uuid(cmd->uuid, cmd->uuid_length, &uuid.uuid))
    {
        goto fail;
    }

    discover_params.uuid = &uuid.uuid;
    discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
    discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
    discover_params.type = BT_GATT_DISCOVER_PRIMARY;
    discover_params.func = disc_prim_cb;

    btp_opcode = GATT_DISC_PRIM_UUID;

    if (bt_gatt_discover(conn, &discover_params) < 0)
    {
        discover_destroy(&discover_params);

        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    bt_conn_unref(conn);

fail_conn:
    ble_prepare_status(NCP_RSP_BLE_GATT_DISC_PRIM, NCP_CMD_RESULT_ERROR, NULL, 0);
}

#if 0
static void find_included(uint8_t *data, uint16_t len)
{
    const struct gatt_find_included_cmd *cmd = (void *) data;
    struct bt_conn *conn;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (!conn)
    {
        goto fail_conn;
    }

    discover_params.start_handle = sys_le16_to_cpu(cmd->start_handle);
    discover_params.end_handle = sys_le16_to_cpu(cmd->end_handle);
    discover_params.type = BT_GATT_DISCOVER_INCLUDE;
    discover_params.func = find_included_cb;

    if (bt_gatt_discover(conn, &discover_params) < 0)
    {
        discover_destroy(&discover_params);

        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    bt_conn_unref(conn);

fail_conn:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_FIND_INCLUDED, NCP_CMD_RESULT_ERROR, NULL, 0);
}

static void disc_all_chrc(uint8_t *data, uint16_t len)
{
    const gatt_disc_all_chrc_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        goto fail_conn;
    }

    discover_params.start_handle = sys_le16_to_cpu(cmd->start_handle);
    discover_params.end_handle = sys_le16_to_cpu(cmd->end_handle);
    discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
    discover_params.func = disc_chrc_cb;

    /* TODO should be handled as user_data via CONTAINER_OF macro */
    btp_opcode = GATT_DISC_ALL_CHRC;

    if (bt_gatt_discover(conn, &discover_params) < 0)
    {
        discover_destroy(&discover_params);

        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    bt_conn_unref(conn);

fail_conn:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_DISC_ALL_CHRC, NCP_CMD_RESULT_ERROR, NULL, 0);
}
#endif

void disc_chrc_uuid(uint8_t *data, uint16_t len)
{
    const gatt_disc_chrc_uuid_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        goto fail_conn;
    }

    if (btp2bt_uuid(cmd->uuid, cmd->uuid_length, &uuid.uuid))
    {
        goto fail;
    }

    discover_params.uuid = &uuid.uuid;
    discover_params.start_handle = sys_le16_to_cpu(cmd->start_handle);
    discover_params.end_handle = sys_le16_to_cpu(cmd->end_handle);
    discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
    discover_params.func = disc_chrc_cb;

    btp_opcode = GATT_DISC_CHRC_UUID;

    if (bt_gatt_discover(conn, &discover_params) < 0)
    {
        discover_destroy(&discover_params);

        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    bt_conn_unref(conn);

fail_conn:
    ble_prepare_status(NCP_RSP_BLE_GATT_DISC_CHRC, NCP_CMD_RESULT_ERROR, NULL, 0);
}

#if 0
static void disc_all_desc(uint8_t *data, uint16_t len)
{
    const gatt_disc_all_desc_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        goto fail_conn;
    }

    discover_params.start_handle = sys_le16_to_cpu(cmd->start_handle);
    discover_params.end_handle = sys_le16_to_cpu(cmd->end_handle);
    discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
    discover_params.func = disc_desc_cb;

    btp_opcode = GATT_DISC_ALL_DESC;

    if (bt_gatt_discover(conn, &discover_params) < 0)
    {
        discover_destroy(&discover_params);

        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    bt_conn_unref(conn);

fail_conn:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | btp_opcode, NCP_CMD_RESULT_ERROR, NULL, 0);
}
#endif

void disc_desc_uuid(uint8_t *data, uint16_t len)
{
    const gatt_disc_desc_uuid_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        goto fail_conn;
    }

    if (btp2bt_uuid(cmd->uuid, cmd->uuid_length, &uuid.uuid))
    {
        goto fail;
    }

    discover_params.uuid = &uuid.uuid;
    discover_params.start_handle = sys_le16_to_cpu(cmd->start_handle);
    discover_params.end_handle = sys_le16_to_cpu(cmd->end_handle);
    discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
    discover_params.func = disc_desc_cb;

    if (bt_gatt_discover(conn, &discover_params) < 0)
    {
        discover_destroy(&discover_params);

        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    bt_conn_unref(conn);

fail_conn:
    ble_prepare_status(NCP_RSP_BLE_GATT_DESC_CHRC, NCP_CMD_RESULT_ERROR, NULL, 0);
}

void read_data(uint8_t *data, uint16_t len)
{
    const gatt_read_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;
    uint16_t offset = gatt_buf.len;
    int res;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        goto fail_conn;
    }

    if (!gatt_buf_reserve(sizeof(struct gatt_read_rp)))
    {
        goto fail;
    }

    read_params.handle_count = 1;
    read_params.single.handle = sys_le16_to_cpu(cmd->handle);
    read_params.single.offset = 0x0000;
    read_params.func = read_cb;

    res = bt_gatt_read(conn, &read_params);

    if (res < 0)
    {
        (void)memset(&read_params, 0, sizeof(read_params));
        gatt_buf_clear_offset(offset);
        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    bt_conn_unref(conn);

fail_conn:
    ble_prepare_status(NCP_RSP_BLE_GATT_READ, NCP_CMD_RESULT_ERROR, NULL, 0);
}

#if 0
static void read_uuid(uint8_t *data, uint16_t len)
{
    const struct gatt_read_uuid_cmd *cmd = (void *) data;
    struct bt_conn *conn;
    uint16_t offset = gatt_buf.len;
    int res;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (!conn) {
        goto fail_conn;
    }

    if (btp2bt_uuid(cmd->uuid, cmd->uuid_length, &uuid.uuid)) {
        goto fail;
    }

    read_uuid_params.by_uuid.uuid = &uuid.uuid;
    read_uuid_params.handle_count = 0;
    read_uuid_params.by_uuid.start_handle = sys_le16_to_cpu(cmd->start_handle);
    read_uuid_params.by_uuid.end_handle = sys_le16_to_cpu(cmd->end_handle);
    read_uuid_params.func = read_uuid_cb;

    btp_opcode = GATT_READ_UUID;
    res = bt_gatt_read(conn, &read_uuid_params);

    if (res < 0)
    {
        (void)memset(&read_uuid_params, 0, sizeof(read_uuid_params));
        gatt_buf_clear_offset(offset);

        goto fail;
    }

    bt_conn_unref(conn);

    return;
fail:
    bt_conn_unref(conn);

fail_conn:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_READ_UUID, NCP_CMD_RESULT_ERROR, NULL, 0);
}

static void read_long(uint8_t *data, uint16_t len)
{
    const gatt_read_long_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;
    uint16_t offset = gatt_buf.len;
    int res;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        goto fail_conn;
    }

    if (!gatt_buf_reserve(sizeof(struct gatt_read_rp)))
    {
        goto fail;
    }

    read_long_params.handle_count = 1;
    read_long_params.single.handle = sys_le16_to_cpu(cmd->handle);
    read_long_params.single.offset = sys_le16_to_cpu(cmd->offset);
    read_long_params.func = read_cb;

    btp_opcode = GATT_READ_LONG;
    res = bt_gatt_read(conn, &read_long_params);

    if (res < 0)
    {
        (void)memset(&read_long_params, 0, sizeof(read_long_params));
        gatt_buf_clear_offset(offset);

        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    bt_conn_unref(conn);

fail_conn:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_READ_LONG, NCP_CMD_RESULT_ERROR, NULL, 0);
}

static void read_multiple(uint8_t *data, uint16_t len, uint8_t opcode)
{
    const gatt_read_multiple_cmd_t *cmd = (void *) data;
    uint16_t handles[255];
    struct bt_conn *conn;
    int i;
    int res;
    struct bt_gatt_read_params read_params;

    for (i = 0; i < cmd->handles_count; i++)
    {
        handles[i] = sys_le16_to_cpu(cmd->handles[i]);
    }

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        goto fail_conn;
    }

    if (!gatt_buf_reserve(sizeof(struct gatt_read_rp)))
    {
        goto fail;
    }

    read_params.func = read_cb;
    read_params.handle_count = i;
    read_params.multiple.handles = handles; /* not used in read func */
    read_params.multiple.variable = (opcode == GATT_READ_MULTIPLE_VAR);
#if (defined(CONFIG_BT_EATT) && (CONFIG_BT_EATT > 0))
    read_params.chan_opt = BT_ATT_CHAN_OPT_NONE;
#endif /* CONFIG_BT_EATT */
    btp_opcode = opcode;
    res = bt_gatt_read(conn, &read_params);

    if (res < 0)
    {
        gatt_buf_clear();
        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    bt_conn_unref(conn);

fail_conn:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_READ_MULTIPLE, NCP_CMD_RESULT_ERROR, NULL, 0);
}

static void write_without_rsp(uint8_t *data, uint16_t len, uint8_t op,
                              bool sign)
{
    const gatt_write_without_rsp_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;
    uint8_t status = NCP_CMD_RESULT_OK;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        status = NCP_CMD_RESULT_ERROR;
        goto rsp;
    }

    if (bt_gatt_write_without_response(conn, sys_le16_to_cpu(cmd->handle),
                       cmd->data,
                       sys_le16_to_cpu(cmd->data_length),
                       sign) < 0)
    {
        status = NCP_CMD_RESULT_ERROR;
    }

    bt_conn_unref(conn);

rsp:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | op, status, NULL, 0);
}
#endif

void write_data(uint8_t *data, uint16_t len)
{
    const gatt_write_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        goto fail;
    }

    write_params.handle = sys_le16_to_cpu(cmd->handle);
    write_params.func = write_rsp;
    write_params.offset = 0U;
    write_params.data = cmd->data;
    write_params.length = sys_le16_to_cpu(cmd->data_length);

    if (bt_gatt_write(conn, &write_params) < 0)
    {
        bt_conn_unref(conn);
        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    ble_prepare_status(NCP_RSP_BLE_GATT_WRITE, NCP_CMD_RESULT_ERROR, NULL, 0);
}

#if 0
static void write_long(uint8_t *data, uint16_t len)
{
    const gatt_write_long_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        goto fail;
    }

    write_params.handle = sys_le16_to_cpu(cmd->handle);
    write_params.func = write_long_rsp;
    write_params.offset = cmd->offset;
    write_params.data = cmd->data;
    write_params.length = sys_le16_to_cpu(cmd->data_length);

    if (bt_gatt_write(conn, &write_params) < 0)
    {
        bt_conn_unref(conn);
        goto fail;
    }

    bt_conn_unref(conn);

    return;

fail:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_WRITE_LONG, NCP_CMD_RESULT_ERROR, NULL, 0);
}
#endif

void config_subscription(uint8_t *data, uint16_t len, uint16_t op)
{
    const gatt_cfg_notify_cmd_t *cmd = (void *) data;
    struct bt_conn *conn;
    uint16_t ccc_handle = sys_le16_to_cpu(cmd->ccc_handle);
    uint8_t status;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);

    if (!conn)
    {
        ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | op, NCP_CMD_RESULT_ERROR, NULL, 0);
        return;
    }

    if (cmd->enable)
    {
        uint16_t value;

        if (op == GATT_CFG_NOTIFY)
        {
            value = BT_GATT_CCC_NOTIFY;
        }
        else
        {
            value = BT_GATT_CCC_INDICATE;
        }

        /* on success response will be sent from callback */
        if (enable_subscription(conn, ccc_handle, value) == 0)
        {
            bt_conn_unref(conn);
            return;
        }

        status = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        if (disable_subscription(conn, ccc_handle) < 0)
        {
            status = NCP_CMD_RESULT_ERROR;
        }
        else
        {
            status = NCP_CMD_RESULT_OK;
        }
    }

    LOG_DBG("Config subscription (op %u) status %u", op, status);

    bt_conn_unref(conn);
    ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | op, status, NULL, 0);
}

#if 0
static void get_attrs(uint8_t *data, uint16_t len)
{
    const gatt_get_attributes_cmd_t *cmd = (void *) data;
    gatt_get_attributes_rp_t *rp;
    struct net_buf_simple *buf = NET_BUF_SIMPLE(NCP_BLE_DATA_MAX_SIZE);
    struct get_attrs_foreach_data foreach;
    uint16_t start_handle, end_handle;
    union uuid uuid;

    start_handle = sys_le16_to_cpu(cmd->start_handle);
    end_handle = sys_le16_to_cpu(cmd->end_handle);

    if (cmd->type_length)
    {
        char uuid_str[BT_UUID_STR_LEN];

        if (btp2bt_uuid(cmd->type, cmd->type_length, &uuid.uuid))
        {
            goto fail;
        }

        bt_uuid_to_str(&uuid.uuid, uuid_str, sizeof(uuid_str));
        LOG_DBG("start 0x%04x end 0x%04x, uuid %s", start_handle,
                end_handle, uuid_str);

        foreach.uuid = &uuid.uuid;
    }
    else
    {
        LOG_DBG("start 0x%04x end 0x%04x", start_handle, end_handle);

        foreach.uuid = NULL;
    }

    net_buf_simple_init(buf, sizeof(*rp));

    foreach.buf = buf;
    foreach.count = 0U;

    bt_gatt_foreach_attr(start_handle, end_handle, get_attrs_rp, &foreach);

    rp = net_buf_simple_push(buf, sizeof(*rp));
    rp->attrs_count = foreach.count;

    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_GET_ATTRIBUTES, NCP_CMD_RESULT_OK, buf->data, buf->len);

    return;

fail:
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_GET_ATTRIBUTES, NCP_CMD_RESULT_ERROR, NULL, 0);
}

static void get_attr_val(uint8_t *data, uint16_t len)
{
    const gatt_get_attribute_value_cmd_t *cmd = (void *) data;
    struct net_buf_simple *buf = NET_BUF_SIMPLE(NCP_BLE_DATA_MAX_SIZE);
    uint16_t handle = sys_le16_to_cpu(cmd->handle);
    struct bt_conn *conn;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)cmd);

    net_buf_simple_init(buf, 0);

    struct get_attr_data cb_data = { .buf = buf, .conn = conn };

    bt_gatt_foreach_attr(handle, handle, get_attr_val_rp, &cb_data);

    if (buf->len)
    {
        ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_GET_ATTRIBUTE_VALUE, NCP_CMD_RESULT_OK, buf->data, buf->len);
    }
    else
    {
        ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_GET_ATTRIBUTE_VALUE, NCP_CMD_RESULT_ERROR, NULL, 0);
    }
}
#endif

/*
 * Helpers
 */
static int register_service(void)
{
    int err;

    server_svcs[svc_count].attrs = server_db +
                       (attr_count - svc_attr_count);
    server_svcs[svc_count].attr_count = svc_attr_count;

    err = bt_gatt_service_register(&server_svcs[svc_count]);
    if (!err)
    {
        /* Service registered, reset the counter */
        svc_attr_count = 0U;
    }

    return err;
}

static struct bt_gatt_attr *gatt_db_add
(
    const struct bt_gatt_attr *pattern,
    size_t user_data_len
)
{
    static struct bt_gatt_attr *attr = server_db;
    const union uuid *u = CONTAINER_OF(pattern->uuid, union uuid, uuid);
    size_t uuid_size = u->uuid.type == BT_UUID_TYPE_16 ? sizeof(u->u16) :
                                 sizeof(u->u128);

    /* Return NULL if database is full */
    if (attr == &server_db[SERVER_MAX_ATTRIBUTES - 1])
    {
        return NULL;
    }

    /* First attribute in db must be service */
    if (!svc_count)
    {
        return NULL;
    }

    memcpy(attr, pattern, sizeof(*attr));

    /* Store the UUID. */
    attr->uuid = server_buf_push(uuid_size);
    memcpy((void *) attr->uuid, (void *)u, uuid_size);

    /* Copy user_data to the buffer. */
    if (user_data_len) {
        attr->user_data = server_buf_push(user_data_len);
        memcpy(attr->user_data, pattern->user_data, user_data_len);
    }

    attr_count++;
    svc_attr_count++;

    return attr++;
}

/* Convert UUID from NCP_BLE command to bt_uuid */
static uint8_t btp2bt_uuid
(
    const uint8_t *uuid,
    uint8_t len,
    struct bt_uuid *bt_uuid
)
{
    uint16_t le16;

    switch (len)
    {
        case 0x02: /* UUID 16 */
            bt_uuid->type = BT_UUID_TYPE_16;
            memcpy(&le16, uuid, sizeof(le16));
            BT_UUID_16(bt_uuid)->val = sys_le16_to_cpu(le16);
        break;

        case 0x10: /* UUID 128*/
            bt_uuid->type = BT_UUID_TYPE_128;
            memcpy(BT_UUID_128(bt_uuid)->val, uuid, 16);
        break;

        default:
            return NCP_CMD_RESULT_ERROR;
    }

    return NCP_CMD_RESULT_OK;
}


static ssize_t read_value(struct bt_conn *conn, const struct bt_gatt_attr *attr,
              void *buf, uint16_t len, uint16_t offset)
{
    const struct gatt_value *value = attr->user_data;

    if (ncp_ble_test_bit(value->flags, GATT_VALUE_READ_AUTHOR_FLAG)) {
        return BT_GATT_ERR(BT_ATT_ERR_AUTHORIZATION);
    }
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0))
    if ((attr->perm & GATT_PERM_ENC_READ_MASK) && (conn != NULL) &&
        (value->enc_key_size > bt_conn_enc_key_size(conn))) {
        return BT_GATT_ERR(BT_ATT_ERR_ENCRYPTION_KEY_SIZE);
    }
#endif
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value->data,
                 value->len);
}

static ssize_t write_value(struct bt_conn *conn,
               const struct bt_gatt_attr *attr, const void *buf,
               uint16_t len, uint16_t offset, uint8_t flags)
{
    struct gatt_value *value = attr->user_data;

    if (ncp_ble_test_bit(value->flags, GATT_VALUE_WRITE_AUTHOR_FLAG))
    {
        return BT_GATT_ERR(BT_ATT_ERR_AUTHORIZATION);
    }
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0))
    if ((attr->perm & GATT_PERM_ENC_WRITE_MASK) &&
        (value->enc_key_size > bt_conn_enc_key_size(conn)))
    {
        return BT_GATT_ERR(BT_ATT_ERR_ENCRYPTION_KEY_SIZE);
    }
#endif

    /* Don't write anything if prepare flag is set */
    if (flags & BT_GATT_WRITE_FLAG_PREPARE)
    {
        return 0;
    }

    if (offset > value->len)
    {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if (offset + len > value->len)
    {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    memcpy(value->data + offset, buf, len);

    /* Maximum attribute value size is 512 bytes */
    __ASSERT_NO_MSG(value->len < 512);

    attr_value_changed_ev(attr->handle, value->data, value->len);

    return len;
}

static void attr_value_changed_ev(uint16_t handle, const uint8_t *value, const uint16_t len)
{
    struct net_buf *buf = net_buf_alloc(&data_pool, osaWaitForever_c);
    if (buf)
    {
        gatt_attr_value_changed_ev_t *ev = (void *) buf->data;

        ev->handle = sys_cpu_to_le16(handle);
        ev->data_length = sys_cpu_to_le16(len);
        memcpy(ev->data, value, len);

        ble_prepare_status(NCP_EVENT_ATTR_VALUE_CHANGED, NCP_CMD_RESULT_OK, (uint8_t*)ev, len + sizeof(gatt_attr_value_changed_ev_t));
        net_buf_unref(buf);
    }
}

static uint8_t get_chrc_index(const struct bt_uuid *uuid)
{
    uint8_t index;

    for(index = 0; index < SERVER_MAX_ATTRIBUTES - 1; index ++)
    {
        struct bt_gatt_attr *attr = &server_db[index];
        if (!bt_uuid_cmp(attr->uuid, uuid))
        {
            break;
        }
    }
    return index;
}

static int alloc_included(struct bt_gatt_attr *attr,
                          uint16_t *included_service_id, uint16_t svc_handle)
{
    struct bt_gatt_attr *attr_incl;

    /*
     * user_data_len is set to 0 to NOT allocate memory in server_buf for
     * user_data, just to assign to it attr pointer.
     */
    attr_incl = gatt_db_add(&(struct bt_gatt_attr)
                BT_GATT_INCLUDE_SERVICE(attr), 0);

    if (!attr_incl) {
        return -EINVAL;
    }

    attr_incl->user_data = attr;

    *included_service_id = attr_incl->handle;
    return 0;
}

static uint8_t alloc_value(struct bt_gatt_attr *attr, struct set_value *data)
{
    struct gatt_value *value;

    /* Value has been already set while adding CCC to the gatt_db */
    if (!bt_uuid_cmp(attr->uuid, BT_UUID_GATT_CCC))
    {
        return NCP_CMD_RESULT_OK;
    }

    /* Set CEP value */
    if (!bt_uuid_cmp(attr->uuid, BT_UUID_GATT_CEP))
    {
        return set_cep_value(attr, data->value, data->len);
    }

    value = attr->user_data;

    /* Check if attribute value has been already set */
    if (!value->len)
    {
        value->data = server_buf_push(data->len);
        value->len = data->len;
    }

    /* Fail if value length doesn't match  */
    if (value->len != data->len)
    {
        return NCP_CMD_RESULT_ERROR;
    }

    memcpy(value->data, data->value, value->len);

    if (ncp_ble_test_bit(value->flags, GATT_VALUE_CCC_FLAG) && ccc_value)
    {
        if (ccc_value == BT_GATT_CCC_NOTIFY)
        {
            bt_gatt_notify(NULL, attr, value->data, value->len);
        }
        else
        {
            indicate_params.attr = attr;
            indicate_params.data = value->data;
            indicate_params.len = value->len;
            indicate_params.func = indicate_cb;
            indicate_params.destroy = NULL;

            (void)bt_gatt_indicate(NULL, &indicate_params);
        }
    }

    return NCP_CMD_RESULT_OK;
}

static uint8_t set_cep_value(struct bt_gatt_attr *attr, const void *value,
                             const uint16_t len)
{
    struct bt_gatt_cep *cep_value = attr->user_data;
    uint16_t properties;

    if (len != sizeof(properties))
    {
        return NCP_CMD_RESULT_ERROR;
    }

    memcpy(&properties, value, len);
    cep_value->properties = sys_le16_to_cpu(properties);

    return NCP_CMD_RESULT_OK;
}

static void indicate_cb(struct bt_conn *conn,
                        struct bt_gatt_indicate_params *params, uint8_t err)
{
    if (err != 0U)
    {
        LOG_ERR("Indication fail");
    }
    else
    {
        LOG_DBG("Indication success");
    }
}

#if 0
static int set_attr_enc_key_size(const struct bt_gatt_attr *attr,
                                 uint8_t key_size)
{
    struct gatt_value *value;

    /* Fail if requested attribute is a service */
    if (!bt_uuid_cmp(attr->uuid, BT_UUID_GATT_PRIMARY) ||
        !bt_uuid_cmp(attr->uuid, BT_UUID_GATT_SECONDARY) ||
        !bt_uuid_cmp(attr->uuid, BT_UUID_GATT_INCLUDE))
    {
        return -EINVAL;
    }

    /* Fail if permissions are not set */
    if (!(attr->perm & (GATT_PERM_ENC_READ_MASK |
                GATT_PERM_ENC_WRITE_MASK)))
    {
        return -EINVAL;
    }

    value = attr->user_data;
    value->enc_key_size = key_size;

    return 0;
}

static void exchange_func(struct bt_conn *conn, uint8_t err,
                          struct bt_gatt_exchange_params *params)
{
    if (err)
    {
        ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_EXCHANGE_MTU, NCP_CMD_RESULT_ERROR, NULL, 0);

        return;
    }

    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_EXCHANGE_MTU, NCP_CMD_RESULT_OK, NULL, 0);
}
#endif

static void *gatt_buf_reserve(size_t len)
{
    return gatt_buf_add(NULL, len);
}

static void *gatt_buf_add(const void *data, size_t len)
{
    void *ptr = gatt_buf.buf + gatt_buf.len;

    if ((len + gatt_buf.len) > MAX_BUFFER_SIZE)
    {
        return NULL;
    }

    if (data)
    {
        memcpy(ptr, data, len);
    }
    else
    {
        (void)memset(ptr, 0, len);
    }

    gatt_buf.len += len;

    LOG_DBG("%d/%d used", gatt_buf.len, MAX_BUFFER_SIZE);

    return ptr;
}

static void discover_destroy(struct bt_gatt_discover_params *params)
{
    (void)memset(params, 0, sizeof(*params));
    gatt_buf_clear();
}

static uint8_t disc_prim_cb(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr,
                            struct bt_gatt_discover_params *params)
{
    struct bt_gatt_service_val *data;
    struct gatt_service *service;
    uint8_t uuid_length;
    static uint8_t services_count = 0;

    if (!attr)
    {
        /* copy data for the response from the gatt buffer */
        uint8_t data_len = gatt_buf.len;
        struct gatt_disc_prim_rp *rp = gatt_buf_reserve(1 + data_len);
        if (rp != NULL)
        {
            rp->services_count = services_count;
            memcpy(rp->services, gatt_buf.buf, data_len);

            ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | btp_opcode, NCP_CMD_RESULT_OK, (uint8_t*)rp, data_len + 1);
        }
        discover_destroy(params);
        services_count = 0;
        return BT_GATT_ITER_STOP;
    }

    data = attr->user_data;

    uuid_length = data->uuid->type == BT_UUID_TYPE_16 ? 2 : 16;

    /* save service information for building the response */
    service = gatt_buf_reserve(sizeof(*service) - SERVER_MAX_UUID_LEN + uuid_length);
    if (!service) {

        ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | btp_opcode, NCP_CMD_RESULT_ERROR, NULL, 0);
        discover_destroy(params);
        return BT_GATT_ITER_STOP;
    }

    service->start_handle = sys_cpu_to_le16(attr->handle);
    service->end_handle = sys_cpu_to_le16(data->end_handle);
    service->uuid_length = uuid_length;

    if (data->uuid->type == BT_UUID_TYPE_16) {
        uint16_t u16 = sys_cpu_to_le16(BT_UUID_16(data->uuid)->val);

        memcpy(service->uuid, &u16, uuid_length);
    } else {
        memcpy(service->uuid, BT_UUID_128(data->uuid)->val,
               uuid_length);
    }

    /* update service count */
    services_count++;

    return BT_GATT_ITER_CONTINUE;
}

static void gatt_buf_clear(void)
{
    (void)memset(&gatt_buf, 0, sizeof(gatt_buf));
}

static void gatt_buf_clear_offset(uint16_t offset)
{
    (void)memset((gatt_buf.buf + offset) , 0, sizeof(gatt_buf.buf) - offset);
    gatt_buf.len = offset;
}

#if 0
static uint8_t find_included_cb(struct bt_conn *conn,
                                const struct bt_gatt_attr *attr,
                                struct bt_gatt_discover_params *params)
{
    struct bt_gatt_include *data;
    gatt_included_t *included;
    uint8_t uuid_length;
    static uint8_t services_count = 0;

    if (!attr)
    {
        /* copy data for the response from the gatt buffer */
        uint8_t data_len = gatt_buf.len;
        gatt_find_included_rp_t *rp = gatt_buf_reserve(1 + data_len);
        if (rp != NULL)
        {
            rp->services_count = services_count;
            memcpy(rp->included, gatt_buf.buf, data_len);

            ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_FIND_INCLUDED, NCP_CMD_RESULT_OK, (uint8_t*)rp, data_len + 1);
        }
        discover_destroy(params);
        services_count = 0;
        return BT_GATT_ITER_STOP;
    }

    data = attr->user_data;

    uuid_length = data->uuid->type == BT_UUID_TYPE_16 ? 2 : 16;

    included = gatt_buf_reserve(sizeof(*included) - SERVER_MAX_UUID_LEN + uuid_length);

    if (!included)
    {
        ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_FIND_INCLUDED, NCP_CMD_RESULT_ERROR, NULL, 0);
        discover_destroy(params);
        return BT_GATT_ITER_STOP;
    }

    included->included_handle = attr->handle;
    included->service.start_handle = sys_cpu_to_le16(data->start_handle);
    included->service.end_handle = sys_cpu_to_le16(data->end_handle);
    included->service.uuid_length = uuid_length;

    if (data->uuid->type == BT_UUID_TYPE_16)
    {
        uint16_t u16 = sys_cpu_to_le16(BT_UUID_16(data->uuid)->val);

        memcpy(included->service.uuid, &u16, uuid_length);
    }
    else
    {
        memcpy(included->service.uuid, BT_UUID_128(data->uuid)->val,
               uuid_length);
    }

    /* update service count */
    services_count++;

    return BT_GATT_ITER_CONTINUE;
}
#endif

static uint8_t disc_chrc_cb(struct bt_conn *conn,
                            const struct bt_gatt_attr *attr,
                            struct bt_gatt_discover_params *params)
{
    struct bt_gatt_chrc *data;
    gatt_characteristic_t *chrc;
    uint8_t uuid_length;
    static uint8_t characteristics_count = 0;

    if (!attr)
    {
        /* copy data for the response from the gatt buffer */
        uint16_t data_len = gatt_buf.len;
        gatt_disc_chrc_rp_t *rp = gatt_buf_reserve(1 + data_len);
        if (rp != NULL)
        {
            rp->characteristics_count = characteristics_count;
            memcpy(rp->characteristics, gatt_buf.buf, data_len);

            ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | btp_opcode, NCP_CMD_RESULT_OK, (uint8_t*)rp, data_len + 1);
        }
        discover_destroy(params);
        characteristics_count = 0;
        return BT_GATT_ITER_STOP;
    }

    data = attr->user_data;

    uuid_length = data->uuid->type == BT_UUID_TYPE_16 ? 2 : 16;

    /* save characteristic information for building the response */
    chrc = gatt_buf_reserve(sizeof(*chrc) - SERVER_MAX_UUID_LEN + uuid_length);

    if (!chrc)
    {
        ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | btp_opcode, NCP_CMD_RESULT_ERROR, NULL, 0);
        discover_destroy(params);
        return BT_GATT_ITER_STOP;
    }

    chrc->characteristic_handle = sys_cpu_to_le16(attr->handle);
    chrc->properties = data->properties;
    chrc->value_handle = sys_cpu_to_le16(attr->handle + 1);
    chrc->uuid_length = uuid_length;

    if (data->uuid->type == BT_UUID_TYPE_16)
    {
        uint16_t u16 = sys_cpu_to_le16(BT_UUID_16(data->uuid)->val);

        memcpy(chrc->uuid, &u16, uuid_length);
    }
    else
    {
        memcpy(chrc->uuid, BT_UUID_128(data->uuid)->val, uuid_length);
    }

    /* update characteristics count */
    characteristics_count++;

    return BT_GATT_ITER_CONTINUE;
}

static uint8_t disc_desc_cb(struct bt_conn *conn,
                                const struct bt_gatt_attr *attr,
                                struct bt_gatt_discover_params *params)
{
    gatt_descriptor_t *descriptor;
    uint8_t uuid_length;
    static uint8_t descriptors_count = 0;

    if (!attr)
    {
        /* copy data for the response from the gatt buffer */
        uint16_t data_len = gatt_buf.len;
        gatt_disc_all_desc_rp_t *rp = gatt_buf_reserve(1 + data_len);
        if (rp != NULL)
        {
            rp->descriptors_count = descriptors_count;
            memcpy(rp->descriptors, gatt_buf.buf, data_len);

            ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | btp_opcode, NCP_CMD_RESULT_OK, (uint8_t*)rp, data_len + 1);
        }
        discover_destroy(params);
        descriptors_count = 0;
        return BT_GATT_ITER_STOP;
    }

    uuid_length = attr->uuid->type == BT_UUID_TYPE_16 ? 2 : 16;

     /* save descriptor information for building the response */
    descriptor = gatt_buf_reserve(sizeof(*descriptor) - SERVER_MAX_UUID_LEN + uuid_length);

    if (!descriptor)
    {
        ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | btp_opcode, NCP_CMD_RESULT_ERROR, NULL, 0);
        discover_destroy(params);
        return BT_GATT_ITER_STOP;
    }

    descriptor->descriptor_handle = sys_cpu_to_le16(attr->handle);
    descriptor->uuid_length = uuid_length;

    if (attr->uuid->type == BT_UUID_TYPE_16)
    {
        uint16_t u16 = sys_cpu_to_le16(BT_UUID_16(attr->uuid)->val);

        memcpy(descriptor->uuid, &u16, uuid_length);
    }
    else
    {
        memcpy(descriptor->uuid, BT_UUID_128(attr->uuid)->val,
               uuid_length);
    }

    /* update descriptors count */
    descriptors_count++;

    return BT_GATT_ITER_CONTINUE;
}

static void read_destroy(struct bt_gatt_read_params *params)
{
    (void)memset(params, 0, sizeof(*params));
    gatt_buf_clear();
}

static uint8_t read_cb(struct bt_conn *conn, uint8_t err,
                       struct bt_gatt_read_params *params, const void *data,
                       uint16_t length)
{
    gatt_read_rp_t *rp = (void *) gatt_buf.buf;

    /* Respond to the Lower Tester with ATT Error received */
    if (err)
    {
        rp->att_response = err;
    }

    /* read complete */
    if (!data)
    {
        ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | btp_opcode, NCP_CMD_RESULT_OK, gatt_buf.buf, gatt_buf.len);
        gatt_buf_clear();
        return BT_GATT_ITER_STOP;
    }

    if (!gatt_buf_add(data, length))
    {
        ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | btp_opcode, NCP_CMD_RESULT_ERROR, NULL, 0);
        gatt_buf_clear();
        return BT_GATT_ITER_STOP;
    }

    rp->data_length += length;

    return BT_GATT_ITER_CONTINUE;
}

static uint8_t read_uuid_cb(struct bt_conn *conn, uint8_t err,
               struct bt_gatt_read_params *params, const void *data,
               uint16_t length)
{

    gatt_char_value_t *value;
    static uint8_t values_count = 0;
    static uint8_t att_response = BT_ATT_ERR_SUCCESS;

    /* Respond to the Lower Tester with ATT Error received */
    if ((err) && (err != BT_ATT_ERR_UNLIKELY)) {
                att_response = err;
    }

    /* read complete */
    if (!data) {
        /* copy data for the response from the gatt buffer */
        uint8_t data_len = gatt_buf.len;
        struct gatt_read_uuid_rp *rp = gatt_buf_reserve(2 + data_len);
        if (rp != NULL)
        {
            rp->values_count = values_count;
            rp->att_response = att_response;
            memcpy(rp->values, gatt_buf.buf, data_len);

            ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | btp_opcode, NCP_CMD_RESULT_OK, (uint8_t*)rp, data_len + 2);
        }
        read_destroy(params);
        values_count = 0;
        return BT_GATT_ITER_STOP;
    }

        /* save value information for building the response */
        value = gatt_buf_reserve(sizeof(*value) - SERVER_MAX_UUID_LEN + length);
        if (!value) {
            ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | btp_opcode, NCP_CMD_RESULT_ERROR, NULL, 0);
            read_destroy(params);
            return BT_GATT_ITER_STOP;
    }

    value->handle = params->by_uuid.start_handle;
    value->data_len = length;

    memcpy(value->data, data, length);

        /* update values count */
    values_count++;

    return BT_GATT_ITER_CONTINUE;
}

static void write_rsp(struct bt_conn *conn, uint8_t err,
                      struct bt_gatt_write_params *params)
{
    ble_prepare_status(NCP_RSP_BLE_GATT_WRITE, NCP_CMD_RESULT_OK, &err, sizeof(err));
}

#if 0
static void write_long_rsp(struct bt_conn *conn, uint8_t err,
                           struct bt_gatt_write_params *params)
{
    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | GATT_WRITE_LONG, NCP_CMD_RESULT_OK, &err, sizeof(err));
}
#endif

static int enable_subscription(struct bt_conn *conn, uint16_t ccc_handle,
                               uint16_t value)
{
    struct bt_gatt_subscribe_params *subscription;

    /* find unused subscription */
    subscription = find_subscription(UNUSED_SUBSCRIBE_CCC_HANDLE);

    if (!subscription)
    {
        return -ENOMEM;
    }

    /* if discovery is busy fail */
    if (discover_params.start_handle)
    {
        return -EBUSY;
    }

    /* Discover Characteristic Value this CCC Descriptor refers to */
    discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
    discover_params.end_handle = ccc_handle;
    discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
    discover_params.func = discover_func;

    subscription->ccc_handle = ccc_handle;
    subscription->value = value;
    subscription->notify = notify_func;

#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0))
    /* require security level from time of subscription */
#if (defined(CONFIG_AUTO_PTS_DEFALUT_MIN_SEC_L2_ENABLE) && (CONFIG_AUTO_PTS_DEFALUT_MIN_SEC_L2_ENABLE > 0))
	/* GAP/SEC/SEM/BV-56-C 
	   GAP/SEC/SEM/BV-62-C
	   GAP/SEC/SEM/BV-63-C
	   GAP/SEC/SEM/BV-64-C
	   GAP/SEC/SEM/BV-65-C
	   GAP/SEC/SEM/BV-66-C
	   GAP/SEC/SEM/BV-67-C */
#if (defined(CONFIG_BT_SMP_SC_ONLY) && (CONFIG_BT_SMP_SC_ONLY == 1)) || \
    (defined(CONFIG_BT_SMP_SC_PAIR_ONLY) && (CONFIG_BT_SMP_SC_PAIR_ONLY == 1))
    subscription->min_security = BT_SECURITY_L4;
#elif (defined(CONFIG_BT_SMP_ENFORCE_MITM) && (CONFIG_BT_SMP_ENFORCE_MITM == 1))
    subscription->min_security = BT_SECURITY_L3;
#elif (defined(CONFIG_BT_SMP_ENFORCE_MITM) && (CONFIG_BT_SMP_ENFORCE_MITM == 0))
    subscription->min_security = BT_SECURITY_L2;
#endif
#else
    subscription->min_security = bt_conn_get_security(conn);
#endif /* CONFIG_AUTO_PTS_DEFALUT_MIN_SEC_L2_ENABLE > 0 */
#endif /* CONFIG_BT_SMP > 0 */

    return bt_gatt_discover(conn, &discover_params);
}

static int disable_subscription(struct bt_conn *conn, uint16_t ccc_handle)
{
    struct bt_gatt_subscribe_params *subscription;

    /* Fail if CCC handle doesn't match */
    subscription = find_subscription(ccc_handle);

    if (!subscription)
    {
        LOG_ERR("CCC handle doesn't match");
        return -EINVAL;
    }

    if (bt_gatt_unsubscribe(conn, subscription) < 0)
    {
        return -EBUSY;
    }

    (void)memset(subscription, 0, sizeof(*subscription));

    return 0;
}

static uint8_t notify_func(struct bt_conn *conn,
                           struct bt_gatt_subscribe_params *params,
                           const void *data, uint16_t length)
{
    struct gatt_notification_ev *ev = (void *) ev_buf;
    const bt_addr_le_t *addr;

    if (!conn || !data)
    {
        LOG_DBG("Unsubscribed");
        (void)memset(params, 0, sizeof(*params));
        return BT_GATT_ITER_STOP;
    }

    addr = bt_conn_get_dst(conn);
    ev->type = (uint8_t)params->value;
    ev->handle = sys_cpu_to_le16(params->value_handle);

    length = MIN(length, MAX_NOTIF_DATA);

    ev->data_length = sys_cpu_to_le16(length);
    memcpy(ev->data, data, length);
    if (addr != NULL)
    {
        memcpy(ev->address, addr->a.val, sizeof(ev->address));
        ev->address_type = addr->type;
    }

    ble_prepare_status(NCP_EVENT_GATT_NOTIFICATION, NCP_CMD_RESULT_OK, ev_buf, sizeof(*ev) + length);

    return BT_GATT_ITER_CONTINUE;
}

static void discover_complete(struct bt_conn *conn,
                              struct bt_gatt_discover_params *params)
{
    struct bt_gatt_subscribe_params *subscription;
    uint8_t op, status;

    subscription = find_subscription(discover_params.end_handle);
    __ASSERT_NO_MSG(subscription);

    /* If no value handle it means that chrc has not been found */
    if (!subscription->value_handle)
    {
        status = NCP_CMD_RESULT_ERROR;
        goto fail;
    }

    if (bt_gatt_subscribe(conn, subscription) < 0)
    {
        status = NCP_CMD_RESULT_ERROR;
        goto fail;
    }

    status = NCP_CMD_RESULT_OK;

fail:
    op = subscription->value == BT_GATT_CCC_NOTIFY ? GATT_CFG_NOTIFY :
                                GATT_CFG_INDICATE;

    if (status == NCP_CMD_RESULT_ERROR)
    {
        (void)memset(subscription, 0, sizeof(*subscription));
    }

    ble_prepare_status(NCP_CMD_BLE | NCP_CMD_BLE_GATT | NCP_MSG_TYPE_RESP | op, status, NULL, 0);

    (void)memset(params, 0, sizeof(*params));
}

static uint8_t discover_func(struct bt_conn *conn,
                             const struct bt_gatt_attr *attr,
                             struct bt_gatt_discover_params *params)
{
    struct bt_gatt_subscribe_params *subscription;

    if (!attr)
    {
        discover_complete(conn, params);
        return BT_GATT_ITER_STOP;
    }

    subscription = find_subscription(discover_params.end_handle);
    __ASSERT_NO_MSG(subscription);

    /* Characteristic Value Handle is the next handle beyond declaration */
    subscription->value_handle = attr->handle + 1;

    /*
     * Continue characteristic discovery to get last characteristic
     * preceding this CCC descriptor
     */
    return BT_GATT_ITER_CONTINUE;
}

static struct bt_gatt_subscribe_params *find_subscription(uint16_t ccc_handle)
{
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++)
    {
        if (subscriptions[i].ccc_handle == ccc_handle)
        {
            return &subscriptions[i];
        }
    }

    return NULL;
}

#if 0
static uint8_t get_attrs_rp(const struct bt_gatt_attr *attr, uint16_t handle,
                void *user_data)
{
    struct get_attrs_foreach_data *foreach = user_data;
    struct gatt_attr *gatt_attr;

    if (foreach->uuid && bt_uuid_cmp(foreach->uuid, attr->uuid))
    {

        return BT_GATT_ITER_CONTINUE;
    }

    gatt_attr = net_buf_simple_add(foreach->buf, sizeof(*gatt_attr));
    gatt_attr->handle = sys_cpu_to_le16(handle);
    gatt_attr->permission = attr->perm;

    if (attr->uuid->type == BT_UUID_TYPE_16)
    {
        gatt_attr->type_length = 2U;
        net_buf_simple_add_le16(foreach->buf,
                                BT_UUID_16(attr->uuid)->val);
    }
    else
    {
        gatt_attr->type_length = 16U;
        net_buf_simple_add_mem(foreach->buf,
                               BT_UUID_128(attr->uuid)->val,
                               gatt_attr->type_length);
    }

    foreach->count++;

    return BT_GATT_ITER_CONTINUE;
}

static uint8_t err_to_att(int err)
{
    if (err < 0 && err >= -0xff)
    {
        return -err;
    }

    return BT_ATT_ERR_UNLIKELY;
}

static uint8_t get_attr_val_rp(const struct bt_gatt_attr *attr, uint16_t handle,
                               void *user_data)
{
    struct get_attr_data *u_data = user_data;
    struct net_buf_simple *buf = u_data->buf;
    struct bt_conn *conn = u_data->conn;
    gatt_get_attribute_value_rp_t *rp;
    ssize_t read, to_read;

    rp = net_buf_simple_add(buf, sizeof(*rp));
    rp->value_length = 0x0000;
    rp->att_response = 0x00;

    do
    {
        to_read = net_buf_simple_tailroom(buf);

        if (!attr->read)
        {
            rp->att_response = BT_ATT_ERR_READ_NOT_PERMITTED;
            break;
        }

        read = attr->read(conn, attr, buf->data + buf->len, to_read,
                          rp->value_length);
        if (read < 0)
        {
            rp->att_response = err_to_att(read);
            break;
        }

        rp->value_length += read;

        net_buf_simple_add(buf, read);
    } while (read == to_read);

    return BT_GATT_ITER_STOP;
}

#if (defined(CONFIG_BT_GATT_NOTIFY_MULTIPLE) && (CONFIG_BT_GATT_NOTIFY_MULTIPLE > 0))
static void notify_cb(struct bt_conn *conn, void *user_data)
{
    LOG_DBG("Nofication sent");
}

static void notify_mult(uint8_t *data, uint16_t len, uint16_t op)
{
    const struct gatt_cfg_notify_mult_cmd *cmd = (void *) data;
    struct bt_gatt_notify_params params[CONFIG_BT_L2CAP_TX_BUF_COUNT];
    struct bt_conn *conn;
    const size_t min_cnt = 1U;
    int err = 0;
    uint8_t status = NCP_CMD_RESULT_OK;
    uint16_t attr_data_len = 0;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (!conn) {
        status = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        if (cmd->cnt < min_cnt || cmd->cnt > CONFIG_BT_L2CAP_TX_BUF_COUNT) {
            LOG_ERR("Invalid count value %d (range %zu to %zu)",
                    cmd->cnt, min_cnt, CONFIG_BT_L2CAP_TX_BUF_COUNT);

            status = NCP_CMD_RESULT_ERROR;
        }
        else
        {
            (void)memset(params, 0, sizeof(params));

            for (uint16_t i = 0U; i < cmd->cnt; i++) {
                struct bt_gatt_attr attr = server_db[cmd->attr_id[i] -
                    server_db[0].handle];

                attr_data_len = strtoul(attr.user_data, NULL, 16);
                params[i].uuid = 0;
                params[i].attr = &attr;
                params[i].data = &attr.user_data;
                params[i].len = attr_data_len;
                params[i].func = notify_cb;
                params[i].user_data = NULL;
            }

            err = bt_gatt_notify_multiple(conn, cmd->cnt, params);
            if (err != 0) {
                LOG_ERR("bt_gatt_notify_multiple failed: %d", err);
                status = NCP_CMD_RESULT_ERROR;
            } else {
                LOG_DBG("Send %u notifications", cmd->cnt);
            }
        }
    }

    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_GATT | op, status, NULL, 0);
}
#endif /* CONFIG_BT_GATT_NOTIFY_MULTIPLE */
#endif
#endif /* CONFIG_NCP_BLE */