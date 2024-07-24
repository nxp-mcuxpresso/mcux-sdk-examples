/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if CONFIG_NCP_BLE

#include "ncp_ble.h"
#include "ncp_glue_ble.h"
#include <sys/atomic.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/att.h>
#include "service.h"

#include "ncp_adapter.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if 0
#define CONTROLLER_INDEX 0
#define CONTROLLER_NAME "ncp_ble"
#endif

#define BT_LE_AD_DISCOV_MASK (BT_LE_AD_LIMITED | BT_LE_AD_GENERAL)
/*consider ext_adv case*/
#define ADV_BUF_LEN (sizeof(struct gap_device_found_ev) + 2 * 229)

#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))
/* Global to configure the Peer ADV List size */
#define APPL_BLE_PEER_ADV_MAX_REPORT_COUNT   30U
/* BLE Adv Report */
typedef struct
{
    /* BLE ADV Report Event Type */
    uint8_t       event_type;
    /* Device Address */
    bt_addr_le_t  dev_addr;
} APPL_BLE_PEER_ADV_REPORT;
/* BLE Adv list pool*/
static APPL_BLE_PEER_ADV_REPORT appl_hci_le_peer_adv_report[APPL_BLE_PEER_ADV_MAX_REPORT_COUNT];
/* BLE Adv list full flag*/
static uint8_t appl_hci_le_peer_adv_report_list_full;
/* ADV List DS access macros */
#define APPL_HCI_LE_GET_PEER_ADV_ADDR(i)    appl_hci_le_peer_adv_report[(i)].dev_addr
#define APPL_HCI_LE_GET_PEER_ADV_TYPE(i)    appl_hci_le_peer_adv_report[(i)].event_type
/* ADV List DS compare macros */
#define BT_COMPARE_BD_ADDR_AND_TYPE(addr_a,addr_b)\
        ((BT_COMPARE_TYPE((addr_a)->type,(addr_b)->type)) &&\
         (BT_COMPARE_ADDR((addr_a)->a.val,(addr_b)->a.val)))
#define BT_COMPARE_TYPE(type_a,type_b)\
        (((type_a) == (type_b))?true:false)
#define BT_COMPARE_ADDR(addr_a,addr_b)\
        ((0 == memcmp((addr_a), (addr_b), 6))?true:false)
#define BT_BD_ADDR_IS_NON_ZERO(addr)\
        ((0x00U == ((addr)[0U] | (addr)[1U] | (addr)[2U] | (addr)[3U] | (addr)[4U] | (addr)[5U]))?\
        false:true)
#endif //#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void ncp_ble_init_cb(int err);

/* Events */
static void le_connected(struct bt_conn *conn, uint8_t err);
static void le_disconnected(struct bt_conn *conn, uint8_t reason);
static void le_identity_resolved(struct bt_conn *conn, const bt_addr_le_t *rpa,
                 const bt_addr_le_t *identity);
static void le_param_updated(struct bt_conn *conn, uint16_t interval,
                 uint16_t latency, uint16_t timeout);
static void le_security_changed(struct bt_conn *conn, bt_security_t level,
                enum bt_security_err err);
static void le_data_len_updated(struct bt_conn *conn,
			        struct bt_conn_le_data_len_info *info);
static void le_phy_updated(struct bt_conn *conn,
		           struct bt_conn_le_phy_info *info);

void device_found
(
    const bt_addr_le_t *addr,
    int8_t rssi,
    uint8_t evtype,
    struct net_buf_simple *ad
);

/* Command handlers */
#if 0
static void supported_commands(uint8_t *data, uint16_t len);
static void controller_index_list(uint8_t *data,  uint16_t len);
static void controller_info(uint8_t *data, uint16_t len);
static void set_connectable(uint8_t *data, uint16_t len);
static void set_discoverable(uint8_t *data, uint16_t len);
static void set_bondable(uint8_t *data, uint16_t len);
#endif 
void start_advertising(void);
void stop_advertising(const uint8_t *data, uint16_t len);
void start_discovery(const uint8_t *data, uint16_t len);
#if 0
static void start_directed_advertising(const uint8_t *data, uint16_t len);
#endif
void stop_discovery(const uint8_t *data, uint16_t len);
void ble_connect(const uint8_t *data, uint16_t len);
void disconnect(const uint8_t *data, uint16_t len);
#if 0
static void set_io_cap(const uint8_t *data, uint16_t len);
#endif
void pair(const uint8_t *data, uint16_t len);
#if 0
static void unpair(const uint8_t *data, uint16_t len);
static void passkey_entry(const uint8_t *data, uint16_t len);
static void passkey_confirm(const uint8_t *data, uint16_t len);
#endif
void conn_param_update(const uint8_t *data, uint16_t len);
#if 0
static void set_mitm(const uint8_t *data, uint16_t len);
static void set_oob_legacy_data(const uint8_t *data, uint16_t len);
#if ((!defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)) || \
     (defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && (CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY == 0U)))
static void get_oob_sc_local_data(void);
static void set_oob_sc_remote_data(const uint8_t *data, uint16_t len);
#endif /* !defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)) || \
     (defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && (CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY == 0U) */
#endif
void set_filter_list(const uint8_t *data, uint16_t len);

/* Helpers */
static void store_adv
(
    const bt_addr_le_t *addr,
    int8_t rssi,
    struct net_buf_simple *ad
);
static uint8_t get_ad_flags(struct net_buf_simple *ad);
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
#if (defined(CONFIG_BT_SMP_APP_PAIRING_ACCEPT) && (CONFIG_BT_SMP_APP_PAIRING_ACCEPT > 0U))
enum bt_security_err auth_pairing_accept(struct bt_conn *conn,
                     const struct bt_conn_pairing_feat *const feat);
#endif
#endif
static void auth_cancel(struct bt_conn *conn);
#if 0
static void auth_passkey_entry(struct bt_conn *conn);
static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey);
#endif
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey);
static void auth_pairing_complete(struct bt_conn *conn, bool bonded);
void auth_pairing_failed(struct bt_conn *conn, enum bt_security_err reason);
#if 0
static void oob_data_request(struct bt_conn *conn,
                 struct bt_conn_oob_info *oob_info);
#endif

#if (defined(CONFIG_BT_PRIVACY) && ((CONFIG_BT_PRIVACY) > 0U))
static uint8_t read_car_cb
(
    struct bt_conn *conn, uint8_t err,
    struct bt_gatt_read_params *params,
    const void *data,
    uint16_t length
);
#endif

#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))
uint16_t appl_hci_le_is_dev_in_adv_list(bt_addr_le_t  *bd_addr, uint8_t *dev_index);
uint16_t appl_hci_le_find_free_adv_list_inst(uint8_t * free_index);
uint16_t appl_hci_le_init_adv_list(void);
#endif //#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))

extern bool bt_addr_le_is_bonded(uint8_t id, const bt_addr_le_t *addr);
/*******************************************************************************
 * Variables
 ******************************************************************************/
static atomic_t current_settings[1];
struct bt_conn_auth_cb cb;
struct bt_conn_auth_info_cb info_cb;
#if 0
static uint8_t oob_legacy_tk[16] = { 0 };
#endif

#if 0
#if ((!defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)) || \
     (defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && (CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY == 0U)))
static struct bt_le_oob oob_sc_local = { 0 };
static struct bt_le_oob oob_sc_remote = { 0 };
#endif /* ((!defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) ||
           ((defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && (CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY == 0U))))) */
#endif
/* Advertising flags */
static uint8_t ad_flags = BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR;
/* Advertising data */
static uint8_t ad_data[31];
static struct bt_data ad[10] = {
    BT_DATA(BT_DATA_FLAGS, &ad_flags, sizeof(ad_flags)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, 7),
};
static uint8_t adv_data_num = 2;
/* Scanning data */
static uint8_t discovery_flags;
static struct net_buf_simple *adv_buf = NET_BUF_SIMPLE(ADV_BUF_LEN);

/*scanning parameter*/
static struct bt_le_scan_param scan_parameter = {
        .type = BT_LE_SCAN_TYPE_PASSIVE,
        .options = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_WINDOW,
        .timeout = 0,
        .interval_coded = 0,
        .window_coded = 0,
};

#if (defined(CONFIG_BT_PRIVACY) && ((CONFIG_BT_PRIVACY) > 0U))
static struct bt_gatt_read_params read_car_params = {
        .func = read_car_cb,
        .by_uuid.uuid = BT_UUID_CENTRAL_ADDR_RES,
        .by_uuid.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE,
        .by_uuid.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE,
};

static struct {
    bt_addr_le_t addr;
    bool supported;
} cars[CONFIG_BT_MAX_PAIRED];
#endif

static struct bt_conn_cb conn_callbacks = {
    .connected = le_connected,
    .disconnected = le_disconnected,
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
    .identity_resolved = le_identity_resolved,
#endif
    .le_param_updated = le_param_updated,
#if ((defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U)) || (defined(CONFIG_BT_BREDR) && (CONFIG_BT_BREDR > 0U)))
    .security_changed = le_security_changed,
#endif
#if (defined(CONFIG_BT_USER_DATA_LEN_UPDATE) && (CONFIG_BT_USER_DATA_LEN_UPDATE > 0))
    .le_data_len_updated = le_data_len_updated,
#endif
#if (defined(CONFIG_BT_USER_PHY_UPDATE) && (CONFIG_BT_USER_PHY_UPDATE > 0))
    .le_phy_updated = le_phy_updated,
#endif
};

static struct bt_conn_auth_info_cb auth_info_cb = {
	.pairing_failed = auth_pairing_failed,
	.pairing_complete = auth_pairing_complete,
};

static struct bt_conn_auth_cb auth_cb = {

#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
    .passkey_display = auth_passkey_display,
    .passkey_entry = NULL,
    .cancel = auth_cancel,
#endif
 
#if (defined(CONFIG_BT_SMP_APP_PAIRING_ACCEPT) && (CONFIG_BT_SMP_APP_PAIRING_ACCEPT > 0U))
    .pairing_accept = auth_pairing_accept,
#endif
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * @brief   Tester App functionality
 */
uint8_t bt_init(void)
{
    int err;
    (void)memset(&cb, 0, sizeof(cb));
    err = bt_enable(ncp_ble_init_cb);
    if (err < 0)
    {
        ncp_e("Unable to enable Bluetooth: %d", err);
        return NCP_CMD_RESULT_ERROR;
    }

    return NCP_CMD_RESULT_OK;
}

static void ncp_ble_init_cb(int err)
{
    if (err)
    {
        ble_prepare_status(NCP_RSP_BLE_CORE_SUPPORT_CMD, NCP_CMD_RESULT_ERROR, NULL, 0);
        return;
    }

#if (defined(CONFIG_BT_SETTINGS) && (CONFIG_BT_SETTINGS > 0))
    settings_load();
#endif /* CONFIG_BT_SETTINGS */

    bt_conn_auth_cb_register(&auth_cb);

	bt_conn_auth_info_cb_register(&auth_info_cb);

    atomic_clear(current_settings);
    atomic_set_bit(current_settings, GAP_SETTINGS_POWERED);
    atomic_set_bit(current_settings, GAP_SETTINGS_CONNECTABLE);
    atomic_set_bit(current_settings, GAP_SETTINGS_BONDABLE);
    atomic_set_bit(current_settings, GAP_SETTINGS_LE);
#if (defined(CONFIG_BT_PRIVACY) && ((CONFIG_BT_PRIVACY) > 0U))
    atomic_set_bit(current_settings, GAP_SETTINGS_PRIVACY);
#endif /* CONFIG_BT_PRIVACY */

    bt_conn_cb_register(&conn_callbacks);
}

uint8_t ncp_ble_unregister_gap(void)
{
    return NCP_CMD_RESULT_OK;
}

#if 0
/*
 * @brief   Helpers
 */

/*
 * @brief   Read Supported Commands. Each bit in response is
 *          a flag indicating if command with opcode matching
 *          bit number is supported.
 */
static void supported_commands(uint8_t *data, uint16_t len)
{
    uint8_t cmds[4];
    gap_read_supported_commands_rp_t *rp = (void *) &cmds;

    (void)memset(cmds, 0, sizeof(cmds));

    ncp_ble_set_bit(cmds, GAP_READ_SUPPORTED_COMMANDS);
    ncp_ble_set_bit(cmds, GAP_READ_CONTROLLER_INDEX_LIST);
    ncp_ble_set_bit(cmds, GAP_READ_CONTROLLER_INFO);
    ncp_ble_set_bit(cmds, GAP_SET_CONNECTABLE);
    ncp_ble_set_bit(cmds, GAP_SET_DISCOVERABLE);
    ncp_ble_set_bit(cmds, GAP_SET_BONDABLE);
    ncp_ble_set_bit(cmds, GAP_START_ADVERTISING);
    ncp_ble_set_bit(cmds, GAP_START_DIRECTED_ADV);
    ncp_ble_set_bit(cmds, GAP_STOP_ADVERTISING);
    ncp_ble_set_bit(cmds, GAP_START_DISCOVERY);
    ncp_ble_set_bit(cmds, GAP_STOP_DISCOVERY);
    ncp_ble_set_bit(cmds, GAP_CONNECT);
    ncp_ble_set_bit(cmds, GAP_DISCONNECT);
    ncp_ble_set_bit(cmds, GAP_SET_IO_CAP);
    ncp_ble_set_bit(cmds, GAP_PAIR);
    ncp_ble_set_bit(cmds, GAP_PASSKEY_ENTRY);
    ncp_ble_set_bit(cmds, GAP_PASSKEY_CONFIRM);
    ncp_ble_set_bit(cmds, GAP_CONN_PARAM_UPDATE);
    ncp_ble_set_bit(cmds, GAP_SET_MITM);
    ncp_ble_set_bit(cmds, GAP_OOB_LEGACY_SET_DATA);
#if ((!defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)) || \
     (defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && (CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY == 0U)))
    ncp_ble_set_bit(cmds, GAP_OOB_SC_GET_LOCAL_DATA);
    ncp_ble_set_bit(cmds, GAP_OOB_SC_SET_REMOTE_DATA);
#endif

    ncp_ble_set_bit(cmds, GAP_SET_FILTER_LIST);

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_READ_SUPPORTED_COMMANDS, NCP_CMD_RESULT_OK, (uint8_t *) rp, sizeof(cmds));
}

/*
 * @brief   Returns the list of controllers
 */
static void controller_index_list(uint8_t *data,  uint16_t len)
{
    gap_read_controller_index_list_rp_t *rp;
    uint8_t buf[sizeof(*rp) + 1];

    rp = (void *) buf;

    rp->num = 1U;
    rp->index = CONTROLLER_INDEX;

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_READ_CONTROLLER_INDEX_LIST, NCP_CMD_RESULT_OK, (uint8_t *) rp, sizeof(buf));
}

/*
 * @brief   Retrieve the current state and basic
 *          information of a controller.
 */
static void controller_info(uint8_t *data, uint16_t len)
{
    gap_read_controller_info_rp_t rp;
    uint32_t supported_settings;

    (void)memset(&rp, 0, sizeof(rp));

    struct bt_le_oob oob_local = { 0 };

    bt_le_oob_get_local(BT_ID_DEFAULT, &oob_local);
    memcpy(rp.address, &oob_local.addr.a, sizeof(bt_addr_t));

#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
    /* Re-use the oob data read here in get_oob_sc_local_data() */
#if ((!defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)) || \
     (defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && (CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY == 0U)))
    oob_sc_local = oob_local;
#endif /* ((!defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)) || \
           (defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && (CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY == 0U))) */
#endif

    /* If privacy is used, the device uses random type address, otherwise
       static random or public type address is used. */
#if (!defined(CONFIG_BT_PRIVACY) || (CONFIG_BT_PRIVACY == 0U))
    if (oob_local.addr.type == BT_ADDR_LE_RANDOM) {
        atomic_set_bit(current_settings, GAP_SETTINGS_STATIC_ADDRESS);
    }
#endif /* CONFIG_BT_PRIVACY */

    supported_settings = BIT(GAP_SETTINGS_POWERED);
    supported_settings |= BIT(GAP_SETTINGS_CONNECTABLE);
    supported_settings |= BIT(GAP_SETTINGS_BONDABLE);
    supported_settings |= BIT(GAP_SETTINGS_LE);
    supported_settings |= BIT(GAP_SETTINGS_ADVERTISING);

    rp.supported_settings = sys_cpu_to_le32(supported_settings);
    rp.current_settings = sys_cpu_to_le32(current_settings[0]);

    memcpy(rp.name, CONTROLLER_NAME, sizeof(CONTROLLER_NAME));

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_READ_CONTROLLER_INFO, NCP_CMD_RESULT_OK, (uint8_t *) &rp, sizeof(rp));
}

/*
 * @brief   Set the controller to connectable.
 */
static void set_connectable(uint8_t *data, uint16_t len)
{
    const struct gap_set_connectable_cmd *cmd = (void *) data;
    gap_set_connectable_rp_t rp;

    if (cmd->connectable) {
        atomic_set_bit(current_settings, GAP_SETTINGS_CONNECTABLE);
    }
    else
    {
        atomic_clear_bit(current_settings, GAP_SETTINGS_CONNECTABLE);
    }

    rp.current_settings = sys_cpu_to_le32(current_settings[0]);

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_SET_CONNECTABLE, NCP_CMD_RESULT_OK,
            (uint8_t *) &rp, sizeof(rp));
}


/*
 * @brief   Set the controller to discoverable.
 */
static void set_discoverable(uint8_t *data, uint16_t len)
{
    const struct gap_set_discoverable_cmd *cmd = (void *) data;
    gap_set_discoverable_rp_t rp;

    switch (cmd->discoverable)
    {
        case GAP_NON_DISCOVERABLE:
            ad_flags &= ~(BT_LE_AD_GENERAL | BT_LE_AD_LIMITED);
            atomic_clear_bit(current_settings, GAP_SETTINGS_DISCOVERABLE);
        break;

        case GAP_GENERAL_DISCOVERABLE:
            ad_flags &= ~BT_LE_AD_LIMITED;
            ad_flags |= BT_LE_AD_GENERAL;
            atomic_set_bit(current_settings, GAP_SETTINGS_DISCOVERABLE);
        break;

        case GAP_LIMITED_DISCOVERABLE:
            ad_flags &= ~BT_LE_AD_GENERAL;
            ad_flags |= BT_LE_AD_LIMITED;
            atomic_set_bit(current_settings, GAP_SETTINGS_DISCOVERABLE);
        break;

        default:
            ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_SET_DISCOVERABLE, NCP_CMD_RESULT_ERROR, NULL, 0);
        break;
    }

    rp.current_settings = sys_cpu_to_le32(current_settings[0]);

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_SET_DISCOVERABLE, NCP_CMD_RESULT_OK, (uint8_t *) &rp, sizeof(rp));
}

/*
 * @brief   Set the controller to bondable.
 */
static void set_bondable(uint8_t *data, uint16_t len)
{
    const struct gap_set_bondable_cmd *cmd = (void *) data;
    gap_set_bondable_rp_t rp;

    if (cmd->bondable)
    {
        atomic_set_bit(current_settings, GAP_SETTINGS_BONDABLE);
    }
    else
    {
        atomic_clear_bit(current_settings, GAP_SETTINGS_BONDABLE);
    }
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
    bt_set_bondable(cmd->bondable);
#endif

    rp.current_settings = sys_cpu_to_le32(current_settings[0]);

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_SET_BONDABLE, NCP_CMD_RESULT_OK, (uint8_t *) &rp, sizeof(rp));
}
#endif

/*
 * @brief   SThis command is used to start advertising.
 */
void start_advertising(void)
{
    gap_start_advertising_rp_t rp;
    bool adv_conn;
    bool early_return = false;

    if (early_return == false)
    {
        adv_conn = atomic_test_bit(current_settings, GAP_SETTINGS_CONNECTABLE);

        /* NCP_BLE API don't allow to set empty scan response data. */
        if (bt_le_adv_start(adv_conn ? BT_LE_ADV_CONN : BT_LE_ADV_NCONN,
                    ad, adv_data_num, ad, adv_data_num) < 0)
        {
            early_return = true;
            ble_prepare_status(NCP_RSP_BLE_GAP_START_ADV, NCP_CMD_RESULT_ERROR, NULL, 0);
        }

        if (early_return == false)
        {
            atomic_set_bit(current_settings, GAP_SETTINGS_ADVERTISING);
            rp.current_settings = sys_cpu_to_le32(current_settings[0]);

            ble_prepare_status(NCP_RSP_BLE_GAP_START_ADV, NCP_CMD_RESULT_OK, (uint8_t *) &rp, sizeof(rp));
        }
    }
}

/*
 * @brief   This command is used to set adv data.
 */
void set_adv_data(const uint8_t *data, uint16_t len)
{
    const struct gap_set_adv_data_cmd *cmd = (void *) data;
    uint8_t adv_len;
    int i;

    for (i = 0, adv_len = 1U; i < cmd->adv_data_len; adv_len++)
    {
        if (adv_len >= ARRAY_SIZE(ad))
        {
            ncp_d("ad[] Out of memory");
            break;
        }
        ad[adv_len].data_len = cmd->data[i++] - 1;
        ad[adv_len].type = cmd->data[i++];
        memcpy((uint8_t*)&ad_data[i], &cmd->data[i], ad[adv_len].data_len);
        ad[adv_len].data = &ad_data[i];
        i += ad[adv_len].data_len;
    }
    if (i == cmd->adv_data_len)
    {
      adv_data_num = adv_len;
      //ncp_d("adv_data_set_flag %d\r\n", adv_data_set_flag);
      ble_prepare_status(NCP_RSP_BLE_GAP_SET_ADV_DATA, NCP_CMD_RESULT_OK, NULL, 0);
    }
    else
    {
      ble_prepare_status(NCP_RSP_BLE_GAP_SET_ADV_DATA, NCP_CMD_RESULT_ERROR, NULL, 0);
    }
}

/*
 * @brief   This command is used to stop advertising.
 */
void stop_advertising(const uint8_t *data, uint16_t len)
{
    gap_stop_advertising_rp_t rp;
    int err;
    bool early_return = false;

    err = bt_le_adv_stop();
    if (err < 0) {
        ble_prepare_status(NCP_RSP_BLE_GAP_STOP_ADV, NCP_CMD_RESULT_ERROR, NULL, 0);
        ncp_e("Failed to stop advertising: %d", err);
        early_return = true;
    }

    if (early_return == false)
    {
        atomic_clear_bit(current_settings, GAP_SETTINGS_ADVERTISING);
        rp.current_settings = sys_cpu_to_le32(current_settings[0]);

        ble_prepare_status(NCP_RSP_BLE_GAP_STOP_ADV, NCP_CMD_RESULT_OK, (uint8_t *) &rp, sizeof(rp));
    }
}

/*
 * @brief   This command is used to set scan parameter.
 */
void set_scan_parameter(const uint8_t *data, uint16_t len)
{
    const struct gap_set_scan_param_cmd *cmd = (void *) data;

    scan_parameter.options = cmd->options;
    scan_parameter.interval = cmd->interval;
    scan_parameter.window = cmd->window;
    ncp_d("scan_parameter.options %d\r\n", scan_parameter.options);
    ble_prepare_status(NCP_RSP_BLE_GAP_SET_SCAN_PARAM, NCP_CMD_RESULT_OK, NULL, 0);
}

/*
 * @brief   This command is used to start discovery.
 */
void start_discovery(const uint8_t *data, uint16_t len)
{
    const gap_start_discovery_cmd_t *cmd = (void *) data;
    uint8_t status;

    /* only LE scan is supported */
    if (cmd->flags & GAP_DISCOVERY_FLAG_BREDR)
    {
        status = NCP_CMD_RESULT_ERROR;

        ble_prepare_status(NCP_RSP_BLE_GAP_START_SCAN, status, NULL, 0);
    }
    else
    {
#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))
        appl_hci_le_init_adv_list();
#endif //#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))
        scan_parameter.type = cmd->flags & GAP_DISCOVERY_FLAG_LE_ACTIVE_SCAN ?
                     BT_LE_SCAN_TYPE_ACTIVE : BT_LE_SCAN_TYPE_PASSIVE;
        ncp_d("scan_parameter.options %d\r\n", scan_parameter.options);
        if (bt_le_scan_start(&scan_parameter,
                     device_found) < 0)
        {
            status = NCP_CMD_RESULT_ERROR;
            ncp_e("Failed to start scanning");
            ble_prepare_status(NCP_RSP_BLE_GAP_START_SCAN, status, NULL, 0);
        }
        else
        {
            net_buf_simple_init(adv_buf, 0);
            discovery_flags = cmd->flags;
            status = NCP_CMD_RESULT_OK;
            ble_prepare_status(NCP_RSP_BLE_GAP_START_SCAN, status, NULL, 0);
        }
    }
}

#if 0
/*
 * @brief   This command is used to start directed advertising.
 */
static void start_directed_advertising(const uint8_t *data, uint16_t len)
{
    const struct gap_start_directed_adv_cmd *cmd = (void *)data;
    gap_start_directed_adv_rp_t rp;
    struct bt_le_adv_param adv_param;
    uint16_t options = sys_le16_to_cpu(cmd->options);
    const bt_addr_le_t *peer = (bt_addr_le_t *)data;

    adv_param = *BT_LE_ADV_CONN_DIR(peer);

    if (!(options & GAP_START_DIRECTED_ADV_HD))
    {
        adv_param.options |= BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY;
        adv_param.interval_max = BT_GAP_ADV_FAST_INT_MAX_2;
        adv_param.interval_min = BT_GAP_ADV_FAST_INT_MIN_2;
    }

    if (options & GAP_START_DIRECTED_ADV_PEER_RPA) {

#if (defined(CONFIG_BT_PRIVACY) && ((CONFIG_BT_PRIVACY) > 0U))
        /* check if peer supports Central Address Resolution */
        for (int i = 0; i < CONFIG_BT_MAX_PAIRED; i++)
        {
            if (bt_addr_le_cmp(peer, &cars[i].addr) == 0) {
                if (cars[i].supported)
                {
                    adv_param.options |= BT_LE_ADV_OPT_DIR_ADDR_RPA;
                }
            }
        }
#endif
    }

    if (bt_le_adv_start(&adv_param, NULL, 0, NULL, 0) < 0)
    {
        ncp_e("Failed to start advertising");
        goto fail;
    }

    atomic_set_bit(current_settings, GAP_SETTINGS_ADVERTISING);
    rp.current_settings = sys_cpu_to_le32(current_settings[0]);

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_START_DIRECTED_ADV, NCP_CMD_RESULT_OK, (uint8_t *)&rp, sizeof(rp));
    return;
fail:
    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_START_DIRECTED_ADV, NCP_CMD_RESULT_ERROR, NULL, 0);
}
#endif


/*
 * @brief   This command is used to start directed advertising.
 */
void stop_discovery(const uint8_t *data, uint16_t len)
{
    uint8_t status = NCP_CMD_RESULT_OK;
    int err;

    err = bt_le_scan_stop();
    if (err < 0)
    {
        ncp_e("Failed to stop scanning: %d", err);
        status = NCP_CMD_RESULT_ERROR;
    }

    ble_prepare_status(NCP_RSP_BLE_GAP_STOP_SCAN, status, NULL, 0);
}

/*
 * @brief   This command is used to create a Link Layer connection with
 *          the remote device.
 */
void ble_connect(const uint8_t *data, uint16_t len)
{
    const bt_addr_le_t *addr = (const bt_addr_le_t *)data;
    uint8_t status = NCP_CMD_RESULT_OK;
    int err;

    if (bt_addr_le_cmp(addr, BT_ADDR_LE_ANY) != 0) {
        struct bt_conn *conn;

        err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
                    BT_LE_CONN_PARAM_DEFAULT, &conn);
        if (err) {
            ncp_e("Failed to create connection (%d)", err);
            status = NCP_CMD_RESULT_ERROR;
        }
        else
        {
            bt_conn_unref(conn);
        }
    } else {
        err = bt_conn_le_create_auto(BT_CONN_LE_CREATE_CONN,
                         BT_LE_CONN_PARAM_DEFAULT);
        if (err) {
            ncp_e("Failed to create auto connection (%d)", err);
            status = NCP_CMD_RESULT_ERROR;
        }
    }

    ble_prepare_status(NCP_RSP_BLE_GAP_CONNECT, status, NULL, 0);
}

/*
 * @brief   This command is used to terminate an existing connection or
 *          to cancel pending connection attempt.
 */
void disconnect(const uint8_t *data, uint16_t len)
{
    struct bt_conn *conn;
    uint8_t status;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (!conn)
    {
        status = NCP_CMD_RESULT_ERROR;
        ncp_e("Unknown connection");
    }
    else
    {
        if (bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN))
        {
            ncp_e("Failed to disconnect");
            status = NCP_CMD_RESULT_ERROR;
        }
        else
        {
            status = NCP_CMD_RESULT_OK;
            bt_conn_unref(conn);
        }
    }

    ble_prepare_status(NCP_RSP_BLE_GAP_DISCONNECT, status, NULL, 0);
}

#if (defined(CONFIG_BT_USER_DATA_LEN_UPDATE) && (CONFIG_BT_USER_DATA_LEN_UPDATE > 0))
static uint16_t tx_time_calc(uint8_t phy, uint16_t max_len)
{
	/* Access address + header + payload + MIC + CRC */
	uint16_t total_len = 4 + 2 + max_len + 4 + 3;

	switch (phy) {
	case BT_GAP_LE_PHY_1M:
		/* 1 byte preamble, 8 us per byte */
		return 8 * (1 + total_len);
	case BT_GAP_LE_PHY_2M:
		/* 2 byte preamble, 4 us per byte */
		return 4 * (2 + total_len);
	case BT_GAP_LE_PHY_CODED:
		/* S8: Preamble + CI + TERM1 + 64 us per byte + TERM2 */
		return 80 + 16 + 24 + 64 * (total_len) + 24;
	default:
		return NCP_CMD_RESULT_OK;
	}
}

/*
 * @brief   This command is used to set data len.
 */
void set_data_len(const uint8_t *data, uint16_t len)
{
    const struct gap_set_data_len_cmd *cmd = (void *) data;
    struct bt_conn *conn;
    struct bt_conn_le_data_len_param param;
    uint8_t status = NCP_CMD_RESULT_ERROR;
    int err;

    param.tx_max_len = cmd->tx_max_len;
    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (conn)
    {
        if (cmd->time_flag == 1) {
		param.tx_max_time = cmd->tx_max_time;
	} else {
		/* Assume 1M if not able to retrieve PHY */
		uint8_t phy = BT_GAP_LE_PHY_1M;

#if (defined(CONFIG_BT_USER_PHY_UPDATE) && (CONFIG_BT_USER_PHY_UPDATE > 0))
		struct bt_conn_info info;

		err = bt_conn_get_info(conn, &info);
		if (!err) {
			phy = info.le.phy->tx_phy;
		}
#endif
		param.tx_max_time = tx_time_calc(phy, param.tx_max_len);
                if (param.tx_max_time < 328)
                {
                  param.tx_max_time = 328;
                }
		ncp_d("Calculated tx time: %d phy: %d", param.tx_max_time, phy);
	}
        err = bt_conn_le_data_len_update(conn, &param);
        bt_conn_unref(conn);
        status = err < 0 ? NCP_CMD_RESULT_ERROR : NCP_CMD_RESULT_OK;
    }
    ble_prepare_status(NCP_RSP_BLE_GAP_SET_DATA_LEN, status, NULL, 0);
}
#endif

#if (defined(CONFIG_BT_USER_PHY_UPDATE) && (CONFIG_BT_USER_PHY_UPDATE > 0))
/*
 * @brief   This command is used to set phy.
 */
void set_phy(const uint8_t *data, uint16_t len)
{
    const struct gap_set_phy_cmd *cmd = (void *) data;
    struct bt_conn *conn;
    struct bt_conn_le_phy_param param;
    uint8_t status = NCP_CMD_RESULT_ERROR;
    int err;

    param.pref_tx_phy = cmd->pref_tx_phy;
    param.pref_rx_phy = cmd->pref_rx_phy;
    param.options = cmd->options;
    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (conn)
    {
        err = bt_conn_le_phy_update(conn, &param);
        bt_conn_unref(conn);
        status = err < 0 ? NCP_CMD_RESULT_ERROR : NCP_CMD_RESULT_OK;
    }
    ble_prepare_status(NCP_RSP_BLE_GAP_SET_PHY, status, NULL, 0);
}
#endif
#if 0
/*
 * @brief   This command is used to set I/O capabilities
 */
static void set_io_cap(const uint8_t *data, uint16_t len)
{
    const struct gap_set_io_cap_cmd *cmd = (void *) data;
    uint8_t status = NCP_CMD_RESULT_OK;

    /* Reset io cap requirements */
    (void)memset(&cb, 0, sizeof(cb));
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
    bt_conn_auth_cb_register(NULL);
#endif
    switch (cmd->io_cap)
    {
        case GAP_IO_CAP_DISPLAY_ONLY:
            cb.cancel = auth_cancel;
            cb.passkey_display = auth_passkey_display;
        break;

        case GAP_IO_CAP_KEYBOARD_DISPLAY:
            cb.cancel = auth_cancel;
            cb.passkey_display = auth_passkey_display;
            cb.passkey_entry = auth_passkey_entry;
            cb.passkey_confirm = auth_passkey_confirm;
        break;

        case GAP_IO_CAP_NO_INPUT_OUTPUT:
            cb.cancel = auth_cancel;
        break;

        case GAP_IO_CAP_KEYBOARD_ONLY:
            cb.cancel = auth_cancel;
            cb.passkey_entry = auth_passkey_entry;
        break;

        case GAP_IO_CAP_DISPLAY_YESNO:
            cb.cancel = auth_cancel;
            cb.passkey_display = auth_passkey_display;
            cb.passkey_confirm = auth_passkey_confirm;
        break;

        default:
            status = NCP_CMD_RESULT_ERROR;
        break;
    }

    if (status != NCP_CMD_RESULT_ERROR)
    {
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
#if (defined(CONFIG_BT_SMP_APP_PAIRING_ACCEPT) && (CONFIG_BT_SMP_APP_PAIRING_ACCEPT > 0U))
        cb.pairing_accept = auth_pairing_accept;
#endif
        if (bt_conn_auth_cb_register(&cb))
        {
            status = NCP_CMD_RESULT_ERROR;
        }
#endif
    }

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_SET_IO_CAP, status, NULL, 0);
}
#endif

/*
 * @brief   This command is used to initiate security with remote. If
 *          peer is already paired IUT is expected to enable security
 *          (encryption) with peer. If peer is not paired IUT shall
 *          start pairing process.
 */
void pair(const uint8_t *data, uint16_t len)
{
    struct bt_conn *conn;
    uint8_t status;
    int err;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (!conn)
    {
        status = NCP_CMD_RESULT_ERROR;
    }
    else
    {
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
        err = bt_conn_set_security(conn, BT_SECURITY_L2);
        if (err < 0)
        {
            status = NCP_CMD_RESULT_ERROR;
            bt_conn_unref(conn);
        }
        else
        {
            bt_conn_unref(conn);
            status = NCP_CMD_RESULT_OK;
        }
#endif
    }

    ble_prepare_status(NCP_RSP_BLE_GAP_PAIR, status, NULL, 0);
}

#if 0
/*
 * @brief   This command is used to unpair with remote.
 */
static void unpair(const uint8_t *data, uint16_t len)
{
    struct gap_unpair_cmd *cmd = (void *) data;
    struct bt_conn *conn;
    bt_addr_le_t addr;
    uint8_t status;
    int err;

    addr.type = cmd->address_type;
    memcpy(addr.a.val, cmd->address, sizeof(addr.a.val));

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, &addr);
    if (!conn)
    {
        err = bt_unpair(BT_ID_DEFAULT, &addr);
        status = err < 0 ? NCP_CMD_RESULT_ERROR : NCP_CMD_RESULT_OK;
    }
    else
    {
        err = bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
        bt_conn_unref(conn);

        if (err < 0)
        {
            status = NCP_CMD_RESULT_ERROR;
        }
        else
        {
            err = bt_unpair(BT_ID_DEFAULT, &addr);
            status = err < 0 ? NCP_CMD_RESULT_ERROR : NCP_CMD_RESULT_OK;
        }
    }

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_UNPAIR, status, NULL, 0);
}

/*
 * @brief   This command is used to response with passkey for pairing
 *          request.
 */
static void passkey_entry(const uint8_t *data, uint16_t len)
{
    const struct gap_passkey_entry_cmd *cmd = (void *) data;
    struct bt_conn *conn;
    uint8_t status;
    int err;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (!conn)
    {
        status = NCP_CMD_RESULT_ERROR;
    }
    else
    {
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
        err = bt_conn_auth_passkey_entry(conn, sys_le32_to_cpu(cmd->passkey));
        bt_conn_unref(conn);
        status = err < 0 ? NCP_CMD_RESULT_ERROR : NCP_CMD_RESULT_OK;
#endif
    }

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_PASSKEY_ENTRY, status, NULL, 0);
}


/*
 * @brief   This command is used to response for pairing request with
 *          confirmation in accordance with initiator and responder passkey
 */
static void passkey_confirm(const uint8_t *data, uint16_t len)
{
    const struct gap_passkey_confirm_cmd *cmd = (void *) data;
    struct bt_conn *conn;
    uint8_t status;
    int err;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (!conn)
    {
        status = NCP_CMD_RESULT_ERROR;
    }
    else
    {
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
        if (cmd->match)
        {
            err = bt_conn_auth_passkey_confirm(conn);
        }
        else
        {
            err = bt_conn_auth_cancel(conn);
        }
#endif
        bt_conn_unref(conn);
        status = err < 0 ? NCP_CMD_RESULT_ERROR : NCP_CMD_RESULT_OK;
    }

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_PASSKEY_CONFIRM, status, NULL, 0);
}
#endif

/*
 * @brief   This command is used to response for pairing request with
 *          confirmation in accordance with initiator and responder passkey
 */
void conn_param_update(const uint8_t *data, uint16_t len)
{
    const struct gap_conn_param_update_cmd *cmd = (void *) data;
    struct bt_le_conn_param param = {
        .interval_min = sys_le16_to_cpu(cmd->interval_min),
        .interval_max = sys_le16_to_cpu(cmd->interval_max),
        .latency = sys_le16_to_cpu(cmd->latency),
        .timeout = sys_le16_to_cpu(cmd->timeout),
    };
    struct bt_conn *conn;
    uint8_t status = NCP_CMD_RESULT_ERROR;
    int err;

    conn = bt_conn_lookup_addr_le(BT_ID_DEFAULT, (bt_addr_le_t *)data);
    if (conn)
    {
        err = bt_conn_le_param_update(conn, &param);
        bt_conn_unref(conn);
        status = err < 0 ? NCP_CMD_RESULT_ERROR : NCP_CMD_RESULT_OK;
    }

    ble_prepare_status(NCP_RSP_BLE_GAP_CONN_PARAM_UPDATE, status, NULL, 0);
}

#if 0
/*
 * @brief   This command is used to set MITM setting.
 */
static void set_mitm(const uint8_t *data, uint16_t len)
{
    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_SET_MITM, NCP_CMD_RESULT_OK, NULL, 0);
}

/*
 * @brief   This command is used to set legacy OOB data
 */
static void set_oob_legacy_data(const uint8_t *data, uint16_t len)
{
    const struct gap_oob_legacy_set_data_cmd *cmd = (void *) data;

    memcpy(oob_legacy_tk, cmd->oob_data, 16);
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
    bt_le_oob_set_legacy_flag(true);
#endif
    cb.oob_data_request = oob_data_request;

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_OOB_LEGACY_SET_DATA, NCP_CMD_RESULT_OK, NULL, 0);
}
#endif

/*
 *    Events
 */

/*
 * @brief   This event indicates that a device was connected.
 */
static void le_connected(struct bt_conn *conn, uint8_t err)
{
    struct gap_device_connected_ev ev = {0};
    struct bt_conn_info info;

    if (err)
    {
        ncp_e("Failed to connect (err %u)\n", err);
    }else {
        (void)bt_conn_get_info(conn, &info);

        if (info.le.dst != NULL)
        {
            memcpy(ev.address, info.le.dst->a.val, sizeof(ev.address));
            ev.address_type = info.le.dst->type;
        }
        ev.interval = sys_cpu_to_le16(info.le.interval);
        ev.latency = sys_cpu_to_le16(info.le.latency);
        ev.timeout = sys_cpu_to_le16(info.le.timeout);

        // for profile service connect callback
        le_service_connect(conn, err);
        is_create_conn_cmd = true;
        
        ble_prepare_status(NCP_EVENT_DEVICE_CONNECTED, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
    }
}

/*
 * @brief   This event indicates that a device was disconnected.
 */
static void le_disconnected(struct bt_conn *conn, uint8_t reason)
{
    struct gap_device_disconnected_ev ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(conn);

    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }

    // for profile service disconnect callback
    le_service_disconnect(conn, reason);

    ble_prepare_status(NCP_EVENT_DEVICE_DISCONNECT, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
}

/*
 * @brief   This event indicates that the remote Identity Address has been
 *          resolved.
 */
static void le_identity_resolved(struct bt_conn *conn, const bt_addr_le_t *rpa,
                 const bt_addr_le_t *identity)
{
    struct gap_identity_resolved_ev ev;

    ev.address_type = rpa->type;
    memcpy(ev.address, rpa->a.val, sizeof(ev.address));

    ev.identity_address_type = identity->type;
    memcpy(ev.identity_address, identity->a.val,
           sizeof(ev.identity_address));

    ble_prepare_status(NCP_EVENT_IDENITY_RESOLVED, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
}

/*
 * @brief   This event can be sent when the connection parameters have changed
 */
static void le_param_updated(struct bt_conn *conn, uint16_t interval,
                 uint16_t latency, uint16_t timeout)
{
    struct gap_conn_param_update_ev ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(conn);

    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }
    ev.interval = sys_cpu_to_le16(interval);
    ev.latency = sys_cpu_to_le16(latency);
    ev.timeout = sys_cpu_to_le16(timeout);

    if(!is_create_conn_cmd) {
        ble_prepare_status(NCP_EVENT_CONN_PARAM_UPDATE, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
    }
}

/*
 * @brief   This event can be sent when the connection parameters have changed
 */
#if (defined(CONFIG_BT_USER_DATA_LEN_UPDATE) && (CONFIG_BT_USER_DATA_LEN_UPDATE > 0))
static void le_data_len_updated(struct bt_conn *conn,
			        struct bt_conn_le_data_len_info *info)
{
    struct gap_data_len_updated_ev ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(conn);

    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }
    ev.tx_max_len = info->tx_max_len;
    ev.tx_max_time = info->tx_max_time;
    ev.rx_max_len = info->rx_max_len;
    ev.rx_max_time = info->rx_max_time;

    if(!is_create_conn_cmd) {
        ble_prepare_status(NCP_EVENT_DATA_LEN_UPDATED, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
    }
}
#endif

#if (defined(CONFIG_BT_USER_PHY_UPDATE) && (CONFIG_BT_USER_PHY_UPDATE > 0))
static void le_phy_updated(struct bt_conn *conn,
		           struct bt_conn_le_phy_info *info)
{
      struct gap_phy_updated_ev ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(conn);

    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }
    ev.tx_phy = info->tx_phy;
    ev.rx_phy = info->rx_phy;

    if(!is_create_conn_cmd) {
        ble_prepare_status(NCP_EVENT_PHY_UPDATED, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
    }
}
#endif

/*
 * @brief   This event can be sent when the Security Level has changed
 */
static void le_security_changed(struct bt_conn *conn, bt_security_t level,
                enum bt_security_err err)
{
    const bt_addr_le_t *addr = bt_conn_get_dst(conn);
    struct gap_sec_level_changed_ev sec_ev = {0};
    struct gap_bond_lost_ev bond_ev = {0};
    struct bt_conn_info info;
    int res;

    switch (err)
    {
        case BT_SECURITY_ERR_SUCCESS:
            if (addr != NULL)
            {
                memcpy(sec_ev.address, addr->a.val, sizeof(sec_ev.address));
                sec_ev.address_type = addr->type;
            }

            switch (level)
            {
                case BT_SECURITY_L2:
                    sec_ev.sec_level = 2;
                break;

                case BT_SECURITY_L3:
                    sec_ev.sec_level = 3;
                break;

                case BT_SECURITY_L4:
                    sec_ev.sec_level = 4;
                break;

                case BT_SECURITY_L1:
                    sec_ev.sec_level = 1;
                break;

                default:
                case BT_SECURITY_L0:
                    sec_ev.sec_level = 0;
                break;
            }

            if (!is_create_conn_cmd)
            {
                ble_prepare_status(NCP_EVENT_SEC_LEVEL_CHANGED, NCP_CMD_RESULT_OK, (uint8_t *) &sec_ev, sizeof(sec_ev));
            }
        break;

        case BT_SECURITY_ERR_PIN_OR_KEY_MISSING:
            /* for central role this means that peer have no LTK when we
             * started encryption procedure
             *
             * This means bond is lost and we restart pairing to re-bond
             */
            res = bt_conn_get_info(conn, &info);
            if ((res == 0) && (info.role == BT_CONN_ROLE_CENTRAL))
            {
                if (addr != NULL)
                {
                    (void)memcpy(bond_ev.address, addr->a.val, sizeof(bond_ev.address));
                    bond_ev.address_type = addr->type;
                }

                if (!is_create_conn_cmd)
                {
                    ble_prepare_status(NCP_EVENT_BOND_LOST, NCP_CMD_RESULT_OK, (uint8_t *)&bond_ev, sizeof(bond_ev));
                }
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
                (void)bt_conn_set_security(conn, (bt_security_t)(BT_SECURITY_L2 | BT_SECURITY_FORCE_PAIR));
#endif
            }
        break;

        default:
        break;
    }

    // for profile service security changed callback
    le_service_security(conn, level, err);
}

/*
 * @brief   This event indicates that a device was found during device
 *          discovery.
 */
void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t evtype,
             struct net_buf_simple *ad)
{
#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))
    uint8_t index = 0U;
#endif //#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))
    /* if General/Limited Discovery - parse Advertising data to get flags */
    if (!(discovery_flags & GAP_DISCOVERY_FLAG_LE_OBSERVE) &&
        (evtype != BT_GAP_ADV_TYPE_SCAN_RSP))
    {
        uint8_t flags = get_ad_flags(ad);

        /* ignore non-discoverable devices */
        if (!(flags & BT_LE_AD_DISCOV_MASK)) {
            return;
        }

        /* if Limited Discovery - ignore general discoverable devices */
        if ((discovery_flags & GAP_DISCOVERY_FLAG_LIMITED) &&
            !(flags & BT_LE_AD_LIMITED)) {
            return;
        }
    }

    /* attach Scan Response data */
    if (evtype == BT_GAP_ADV_TYPE_SCAN_RSP)
    {
        struct gap_device_found_ev *ev;
        bt_addr_le_t a;

        /* skip if there is no pending advertisement */
        if (!adv_buf->len)
        {
            return;
        }

        ev = (void *) adv_buf->data;
        a.type = ev->address_type;
        memcpy(a.a.val, ev->address, sizeof(a.a.val));

        /*
         * in general, the Scan Response comes right after the
         * Advertisement, but if not if send stored event and ignore
         * this one
         */
        if (bt_addr_le_cmp(addr, &a))
        {
            goto done;
        }

        ev->eir_data_len += ad->len;
        ev->flags |= GAP_DEVICE_FOUND_FLAG_SD;

        memcpy(net_buf_simple_add(adv_buf, ad->len), ad->data, ad->len);

        goto done;
    }

#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))
    /**
     * Validate is this device already exists in the list.
     *  If device is present in the list already, fetch the index at which
     *  the Adv packet is saved in the list.
     */
    if (NCP_CMD_RESULT_OK != appl_hci_le_is_dev_in_adv_list((bt_addr_le_t *)addr, &index))
    {
         /**
          * Device is not present in the list,
          * add the device to the list.
          */
         if (NCP_CMD_RESULT_OK == appl_hci_le_find_free_adv_list_inst(&index))
         {
             /* Save the Event type */
             APPL_HCI_LE_GET_PEER_ADV_TYPE(index) = evtype;
             /* Save the Device address and Type */
             APPL_HCI_LE_GET_PEER_ADV_ADDR(index).type = addr->type;
             memcpy(&(APPL_HCI_LE_GET_PEER_ADV_ADDR(index).a.val), addr->a.val, 6);
             ncp_d("\n%d. BLE ADV Report\n", (index + 1U));
             ncp_d("===================\n");
             ncp_d("Event Type    : 0x%02X\n", APPL_HCI_LE_GET_PEER_ADV_TYPE(index));
             ncp_d("Device Address: ADDR: %02X %02X %02X %02X %02X %02X, TYPE: %02X \n",
                      APPL_HCI_LE_GET_PEER_ADV_ADDR(index).a.val[0U],\
                      APPL_HCI_LE_GET_PEER_ADV_ADDR(index).a.val[1U],\
                      APPL_HCI_LE_GET_PEER_ADV_ADDR(index).a.val[2U],\
                      APPL_HCI_LE_GET_PEER_ADV_ADDR(index).a.val[3U],\
                      APPL_HCI_LE_GET_PEER_ADV_ADDR(index).a.val[4U],\
                      APPL_HCI_LE_GET_PEER_ADV_ADDR(index).a.val[5U],\
                      APPL_HCI_LE_GET_PEER_ADV_ADDR(index).type);
         }
         else
         {   
             /*adv device list full*/
             return;
         }
    }
    else
    {
         /* duplicate adv device*/
         return;
    }
#endif //#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))
    /*
     * if there is another pending advertisement, send it and store the
     * current one
     */
    if (adv_buf->len)
    {
        ble_prepare_status(NCP_EVENT_ADV_REPORT, NCP_CMD_RESULT_OK, adv_buf->data, adv_buf->len);
        net_buf_simple_reset(adv_buf);
    }

    store_adv(addr, rssi, ad);

    /* if Active Scan and scannable event - wait for Scan Response */
    if ((discovery_flags & GAP_DISCOVERY_FLAG_LE_ACTIVE_SCAN) &&
        (evtype == BT_GAP_ADV_TYPE_ADV_IND ||
         evtype == BT_GAP_ADV_TYPE_ADV_SCAN_IND))
    {
        return;
    }

done:
    ble_prepare_status(NCP_EVENT_ADV_REPORT, NCP_CMD_RESULT_OK, adv_buf->data, adv_buf->len);
    net_buf_simple_reset(adv_buf);
}
#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
/*
 *    Helpers
 */
#if (defined(CONFIG_BT_SMP_APP_PAIRING_ACCEPT) && (CONFIG_BT_SMP_APP_PAIRING_ACCEPT > 0U))
enum bt_security_err auth_pairing_accept(struct bt_conn *conn,
                     const struct bt_conn_pairing_feat *const feat)
{
    gap_bond_lost_ev_t ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(conn);

    if ((addr != NULL) && (!bt_addr_le_is_bonded(BT_ID_DEFAULT, addr)))
    {
        return BT_SECURITY_ERR_SUCCESS;
    }

    /* If a peer is already bonded and tries to pair again then it means that
     * the it has lost its bond information.
     */
    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }

    ble_prepare_status(NCP_EVENT_BOND_LOST, NCP_CMD_RESULT_OK, (uint8_t *)&ev, sizeof(ev));

    return BT_SECURITY_ERR_SUCCESS;
}
#endif
#endif

#if (defined(CONFIG_BT_PRIVACY) && ((CONFIG_BT_PRIVACY) > 0U))
static uint8_t read_car_cb
(
    struct bt_conn *conn, uint8_t err,
    struct bt_gatt_read_params *params,
    const void *data,
    uint16_t length
)
{
    struct bt_conn_info info;
    bool supported = false;

    if (!err && data && length == 1) {
        const uint8_t *tmp = data;

        /* only 0 or 1 are valid values */
        if (tmp[0] == 1) {
            supported = true;
        }
    }

    (void)bt_conn_get_info(conn, &info);

    for (int i = 0; i < CONFIG_BT_MAX_PAIRED; i++)
    {
        if ((info.le.dst != NULL) && (bt_addr_le_cmp(info.le.dst, &cars[i].addr) == 0)) {
            cars[i].supported = supported;
            break;
        }
    }

    return BT_GATT_ITER_STOP;
}
#endif

void auth_pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
    gap_bond_pairing_failed_ev_t ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(conn);
    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }
    ev.reason = reason;
    if(!is_create_conn_cmd) {
        ble_prepare_status(NCP_EVENT_PAIRING_FAILED, NCP_CMD_RESULT_OK, (uint8_t *)&ev, sizeof(ev));
    }
}

static uint8_t get_ad_flags(struct net_buf_simple *ad)
{
    uint8_t len;
    uint16_t i;

    /* Parse advertisement to get flags */
    for (i = 0U; i < ad->len; i += len - 1)
    {
        len = ad->data[i++];
        if (!len)
        {
            break;
        }

        /* Check if field length is correct */
        if (len > (ad->len - i) || (ad->len - i) < 1)
        {
            break;
        }

        if (ad->data[i++] == BT_DATA_FLAGS)
        {
            return ad->data[i];
        }
    }

    return 0;
}

static void store_adv
(
    const bt_addr_le_t *addr,
    int8_t rssi,
    struct net_buf_simple *ad
)
{
    struct gap_device_found_ev *ev;

    /* cleanup */
    net_buf_simple_init(adv_buf, 0);

    ev = net_buf_simple_add(adv_buf, sizeof(*ev));

    memcpy(ev->address, addr->a.val, sizeof(ev->address));
    ev->address_type = addr->type;
    ev->rssi = rssi;
    ev->flags = GAP_DEVICE_FOUND_FLAG_AD | GAP_DEVICE_FOUND_FLAG_RSSI;
    ev->eir_data_len = ad->len;
    memcpy(net_buf_simple_add(adv_buf, ad->len), ad->data, ad->len);
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    struct gap_passkey_display_ev ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(conn);

    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }
    ev.passkey = sys_cpu_to_le32(passkey);

    // for profile service auth passkey callback
    le_service_auth_passkey(conn, passkey);

    ble_prepare_status(NCP_EVENT_PASSKEY_DISPLAY, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
}

#if 0
static void auth_passkey_entry(struct bt_conn *conn)
{
    struct gap_passkey_entry_req_ev ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(conn);

    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }

    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_EVENT | GAP_EV_PASSKEY_ENTRY_REQ, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
    struct gap_passkey_confirm_req_ev ev = {0};
    const bt_addr_le_t *addr = bt_conn_get_dst(conn);

    if (addr != NULL)
    {
        memcpy(ev.address, addr->a.val, sizeof(ev.address));
        ev.address_type = addr->type;
    }
    ev.passkey = sys_cpu_to_le32(passkey);

    ble_prepare_status(NCP_RSP_BLE | NCP_CMD_BLE_EVENT | GAP_EV_PASSKEY_CONFIRM_REQ, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(ev));
}
#endif

static void auth_pairing_complete(struct bt_conn *conn, bool bonded)
{
#if (defined(CONFIG_BT_PRIVACY) && ((CONFIG_BT_PRIVACY) > 0U))
    /* Read peer's Central Address Resolution if bonded */
    if (bonded)
    {
        (void)bt_gatt_read(conn, &read_car_params);
    }
#endif
}

static void auth_cancel(struct bt_conn *conn)
{
    // for profile service auth cancel callback
    le_service_auth_cancel(conn);
}

#if 0
static void oob_data_request(struct bt_conn *conn,
                 struct bt_conn_oob_info *oob_info)
{
    struct bt_conn_info info;
    int err = bt_conn_get_info(conn, &info);

    if (err != 0)
    {
        return;
    }

    char addr[BT_ADDR_LE_STR_LEN];

    (void)bt_addr_le_to_str(info.le.dst, addr, sizeof(addr));

    switch (oob_info->type)
    {
#if ((!defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)) || \
     (defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && (CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY == 0U)))
    case BT_CONN_OOB_LE_SC:
    {
        struct bt_le_oob_sc_data *oobd_local =
            oob_info->lesc.oob_config != BT_CONN_OOB_REMOTE_ONLY ?
                      &oob_sc_local.le_sc_data :
                      NULL;

        struct bt_le_oob_sc_data *oobd_remote =
            oob_info->lesc.oob_config != BT_CONN_OOB_LOCAL_ONLY ?
                      &oob_sc_remote.le_sc_data :
                      NULL;

        if (oobd_remote)
        {
            /* Assume that oob_sc_remote
             * corresponds to the currently connected peer
             */
            bt_addr_le_copy(&oob_sc_remote.addr, info.le.remote);
        }

        if (oobd_local &&
            bt_addr_le_cmp(info.le.local, &oob_sc_local.addr)) {
            bt_addr_le_to_str(info.le.local, addr, sizeof(addr));
            bt_conn_auth_cancel(conn);
            return;
        }
#if !(defined(CONFIG_BT_SMP_SC_PAIR_ONLY) && (CONFIG_BT_SMP_SC_PAIR_ONLY > 0U))
        if (oobd_remote != NULL)
        {
            bt_le_oob_set_sc_data(conn, oobd_local, oobd_remote);
        }
#endif
        break;
    }
#endif /* ((!defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)) || \
           (defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && (CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY == 0U))) */

#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
#if !defined(CONFIG_BT_SMP_SC_PAIR_ONLY)
    case BT_CONN_OOB_LE_LEGACY:
        bt_le_oob_set_legacy_tk(conn, oob_legacy_tk);
        break;
#endif /* !defined(CONFIG_BT_SMP_SC_PAIR_ONLY) */
#endif
    default:
        ncp_e("Unhandled OOB type %d", oob_info->type);
        break;
    }
}

#if ((!defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)) || \
     (defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && (CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY == 0U)))
static void get_oob_sc_local_data(void)
{
    cb.oob_data_request = oob_data_request;
    struct gap_oob_sc_get_local_data_rp rp = { 0 };

    memcpy(&rp.conf[0], &oob_sc_local.le_sc_data.c[0], sizeof(rp.conf));
    memcpy(&rp.rand[0], &oob_sc_local.le_sc_data.r[0], sizeof(rp.rand));
    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_OOB_SC_GET_LOCAL_DATA, NCP_CMD_RESULT_OK, (uint8_t *)&rp, sizeof(rp));
}

static void set_oob_sc_remote_data(const uint8_t *data, uint16_t len)
{
    cb.oob_data_request = oob_data_request;
    bt_le_oob_set_sc_flag(true);

    const struct gap_oob_sc_set_remote_data_cmd *cmd = (void *)data;

    /* Note that the .addr field
     * will be set by the oob_data_request callback
     */
    memcpy(&oob_sc_remote.le_sc_data.r[0], &cmd->rand[0],
           sizeof(oob_sc_remote.le_sc_data.r));
    memcpy(&oob_sc_remote.le_sc_data.c[0], &cmd->conf[0],
           sizeof(oob_sc_remote.le_sc_data.c));

    ble_prepare_status(NCP_RSP_BLE | ((uint32_t)(NCP_BLE_SERVICE_ID_GAP) << 16) | GAP_OOB_SC_SET_REMOTE_DATA, NCP_CMD_RESULT_OK, NULL, 0);
}
#endif /* ((!defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY)) || \
           (defined(CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY) && (CONFIG_BT_SMP_OOB_LEGACY_PAIR_ONLY == 0U))) */
#endif

void set_filter_list(const uint8_t *data, uint16_t len)
{
    const struct gap_set_filter_list *cmd = (const void *) data;
    uint8_t status = NCP_CMD_RESULT_OK;
    int err;
#if 0
    if (len < sizeof(*cmd) ||
        len != (sizeof(*cmd) + (cmd->cnt * sizeof(cmd->addr[0])))) {
        status = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        (void)bt_le_filter_accept_list_clear();

        for (int i = 0; i < cmd->cnt; i++) {
            err = bt_le_filter_accept_list_add(&cmd->addr[i]);
            if (err < 0) {
                status = NCP_CMD_RESULT_ERROR;
            }
        }
    }
#endif

    (void)bt_le_filter_accept_list_clear();

    for (int i = 0; i < cmd->cnt; i++) {
        bt_addr_le_t *le_addr = (bt_addr_le_t *) &cmd->addr;
        err = bt_le_filter_accept_list_add(&le_addr[i]);
        if (err < 0) {
            status = NCP_CMD_RESULT_ERROR;
        }
    }

    ble_prepare_status(NCP_RSP_BLE_GAP_SET_FILTER_LIST, status, NULL, 0);
}

#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))
uint16_t appl_hci_le_is_dev_in_adv_list(bt_addr_le_t *bd_addr, uint8_t *dev_index)
{
    uint32_t  index = 0U;
    uint16_t  retval;

    /* NULL Check? */

    retval = NCP_CMD_RESULT_ERROR;

    /* Validate is device is already present in the Adv list */
    for (index = 0U; index < APPL_BLE_PEER_ADV_MAX_REPORT_COUNT; index++)
    {
        if( true ==
           BT_COMPARE_BD_ADDR_AND_TYPE
           (
                bd_addr,
                &APPL_HCI_LE_GET_PEER_ADV_ADDR(index)
           ))
        {
             retval = NCP_CMD_RESULT_OK;
             break;
        }
    }
    /**
     * If Device was found, the Index value would be valid.
     * Else the the index will be APPL_BLE_PEER_ADV_MAX_REPORT_COUNT.
     */
    *(dev_index) = (uint8_t)index;
    return retval;
}

uint16_t appl_hci_le_find_free_adv_list_inst(uint8_t *free_index)
{
    uint32_t       index = 0U;
    uint16_t       retval;
    bt_addr_le_t * t_addr;

    retval = NCP_CMD_RESULT_ERROR;

    if (true != appl_hci_le_peer_adv_report_list_full)
    {
        /* return retval; */
        for (index = 0U; index < APPL_BLE_PEER_ADV_MAX_REPORT_COUNT; index++)
        {
            /* Fetch the Address */
            t_addr = &APPL_HCI_LE_GET_PEER_ADV_ADDR(index);

            /* Check if the BD Address is Non Zero */
            if (true != BT_BD_ADDR_IS_NON_ZERO(t_addr->a.val))
            {
                retval = NCP_CMD_RESULT_OK;
                break;
            }
        }

        /* Store the index */
        *(free_index) = (uint8_t)index;

        if (NCP_CMD_RESULT_OK != retval)
        {
            if (APPL_BLE_PEER_ADV_MAX_REPORT_COUNT == *(free_index))
            {
                /* Device list is fully occupied, needs a reset of the list */
                ncp_e("ADV Report Device list is full!\n");
                ncp_e("Consider Disabling Scanning!\n");

                appl_hci_le_peer_adv_report_list_full = true;
            }
        }
    }
    return retval;
}

uint16_t appl_hci_le_init_adv_list(void)
{
    /* Initialize the fields with default values */
    memset
    (
        appl_hci_le_peer_adv_report,
        0x0,
        sizeof(appl_hci_le_peer_adv_report)
    );
    appl_hci_le_peer_adv_report_list_full = false;
    return NCP_CMD_RESULT_OK;
}
#endif //#if (defined(CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT) && (CONFIG_BLE_ADV_REPORT_BUFFER_LIMIT > 0U))
#endif