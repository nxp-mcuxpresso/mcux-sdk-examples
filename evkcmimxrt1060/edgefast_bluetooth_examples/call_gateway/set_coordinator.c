/*
 * Copyright 2024 NXP
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

#include <bluetooth/audio/csip.h>
#include <bluetooth/audio/audio.h>

#include "set_coordinator.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

struct set_coordinator_state {
    struct bt_conn *conn;
    const struct bt_csip_set_coordinator_set_member *member;
    size_t set_count;
    size_t set_scanning_index;
    uint8_t set_scanning_done;
    struct k_work work;
    struct k_work_delayable work_delay;
    bool member_discover;
    bt_addr_le_t *addr;
    bool scanning_peer_addr;
};

#define SET_COORDINATOR_SCAN_TIMEOUT 30 /* s */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);

static void set_coordinator_discover(
	struct bt_conn *conn,
	const struct bt_csip_set_coordinator_set_member *member,
	int err, size_t set_count);
static void set_coordinator_lock_changed(
	struct bt_csip_set_coordinator_csis_inst *inst, bool locked);
static void set_coordinator_lock_set(int err);
static void set_coordinator_ordered_access(
	const struct bt_csip_set_coordinator_set_info *set_info,
	int err, bool locked,
	struct bt_csip_set_coordinator_set_member *member);
static void set_coordinator_release_set(int err);

#if CONFIG_BT_SMP
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);
#endif

static void set_coordinator_scan_delay_work_cb(struct k_work *work);
static void set_coordinator_discover_work_cb(struct k_work *work);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static struct set_coordinator_state coordinator_state[CONFIG_BT_MAX_CONN];

#if (defined(CONFIG_BT_SETTINGS) && ((CONFIG_BT_SETTINGS) > 0U)) && \
    (defined(CONFIG_BT_SMP) && ((CONFIG_BT_SMP) > 0U))
static bt_addr_le_t coordinator_set[CONFIG_BT_MAX_CONN];
#endif

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
#if CONFIG_BT_SMP
    .security_changed = security_changed,
#endif
};

static struct bt_csip_set_coordinator_cb set_coordinator_cb = {
    .discover = set_coordinator_discover,
    .lock_changed = set_coordinator_lock_changed,
    .lock_set = set_coordinator_lock_set,
    .ordered_access = set_coordinator_ordered_access,
    .release_set = set_coordinator_release_set,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

static void disconnect_all_set_coordinator_state(void)
{
#if 0
    for (uint32_t i = 0; i < ARRAY_SIZE(coordinator_state); i++)
    {
        if ((coordinator_state[i].conn != NULL))
        {
            int err;

            err = bt_conn_disconnect(coordinator_state[i].conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
            if (err < 0)
            {
                PRINTF("Fail to disconnect %p (err %d)\n", coordinator_state[i].conn, err);
            }

            if (coordinator_state[i].set_scanning_index > 0) {
                err = bt_le_scan_stop();
                if (err != 0) {
                    PRINTF("Failed to stop scan (err %d)\n", err);
                }
            }
            memset(&coordinator_state[i], 0, sizeof(coordinator_state[i]));
        }
    }
#endif
}

static struct set_coordinator_state *find_set_coordinator_state(struct bt_conn *conn)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(coordinator_state); i++)
    {
        if (coordinator_state[i].conn == conn)
        {
            return &coordinator_state[i];
        }
    }
    return NULL;
}

static struct set_coordinator_state *find_set_coordinator_state_by_address(const bt_addr_le_t *addr)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(coordinator_state); i++)
    {
        if ((coordinator_state[i].conn != NULL))
        {
            if (bt_addr_le_cmp(addr, bt_conn_get_dst(coordinator_state[i].conn)) == 0)
            {
                return &coordinator_state[i];
            }
        }
    }
    return NULL;
}

static struct set_coordinator_state *find_discover_set_coordinator_state(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(coordinator_state); i++)
    {
        if ((coordinator_state[i].conn != NULL) && (coordinator_state[i].set_scanning_index != 0))
        {
            return &coordinator_state[i];
        }
    }
    return NULL;
}

static struct set_coordinator_state *find_scanning_set_coordinator_state(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(coordinator_state); i++)
    {
        if ((coordinator_state[i].conn != NULL) && (coordinator_state[i].scanning_peer_addr))
        {
            return &coordinator_state[i];
        }
    }
    return NULL;
}

static struct set_coordinator_state *create_set_coordinator_state(struct bt_conn *conn)
{
    struct set_coordinator_state *state;

    state = find_set_coordinator_state(conn);
    if (state != NULL)
    {
        return state;
    }

    for (uint32_t i = 0; i < ARRAY_SIZE(coordinator_state); i++)
    {
        if (coordinator_state[i].conn == NULL)
        {
            coordinator_state[i].conn = conn;
            coordinator_state[i].member_discover = false;
            k_work_init(&coordinator_state[i].work, set_coordinator_discover_work_cb);
            k_work_init_delayable(&coordinator_state[i].work_delay, set_coordinator_scan_delay_work_cb);
            coordinator_state[i].addr = NULL;
            coordinator_state[i].scanning_peer_addr = true;
            return &coordinator_state[i];
        }
    }

    return NULL;
}

#if (defined(CONFIG_BT_SETTINGS) && ((CONFIG_BT_SETTINGS) > 0U)) && \
    (defined(CONFIG_BT_SMP) && ((CONFIG_BT_SMP) > 0U))

static int set_coordinator_set(const char *name, size_t len_rd, settings_read_cb read_cb,
		   void *cb_arg)
{

    ssize_t len;

    if (len_rd == sizeof(coordinator_set)) {
        len = read_cb(cb_arg, coordinator_set, sizeof(coordinator_set));

        if (len < 0) {
            PRINTF("Failed to decode value (err %zd)\n", len);
            return len;
        }
    }

	return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(set_coordinator, "set_coordinator", NULL, set_coordinator_set, NULL, NULL);

#endif

struct bond_data
{
    const bt_addr_le_t *addr;
    bool bond;
};

static void bond_cb(const struct bt_bond_info *info, void *user_data)
{
    struct bond_data *data;

    if (NULL == user_data)
    {
        return;
    }

    data = (struct bond_data *)user_data;
    if (bt_addr_le_cmp(&info->addr, data->addr) == 0)
    {
        data->bond = true;
    }
}

static void set_coordinator_scan_delay_work_cb(struct k_work *work)
{
    struct set_coordinator_state *state = CONTAINER_OF(work, struct set_coordinator_state, work_delay);
    int err;

    err = bt_le_scan_stop();
    if (err != 0) {
        PRINTF("Failed to stop scan (err %d)\n", err);
    }

    if (state->conn == NULL)
    {
        PRINTF("Invalid conn pointer of set coordinator %p\n", state);
    }

    PRINTF("Member device cannot be found\n");
}

static void set_coordinator_discover_work_cb(struct k_work *work)
{
    struct set_coordinator_state *state = CONTAINER_OF(work, struct set_coordinator_state, work);
    int err;

    if (state->conn == NULL)
    {
        PRINTF("Invalid conn pointer of set coordinator %p\n", state);
        return;
    }

    err = bt_csip_set_coordinator_discover(state->conn);
    if (err < 0)
    {
        PRINTF("Fail to start set coordinator discover (err %d)\n", err);
        return;
    }
    else
    {
        k_work_schedule(&state->work_delay, BT_SECONDS(SET_COORDINATOR_SCAN_TIMEOUT));
    }
}

static void set_coordinator_scan_devce(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad)
{
    struct set_coordinator_state *state = find_scanning_set_coordinator_state();
    struct bt_conn *default_conn;
    int err;

    if (state == NULL)
    {
        return;
    }

    if (bt_addr_le_cmp(state->addr, BT_ADDR_LE_ANY) == 0)
    {
        state->scanning_peer_addr = false;
        err = bt_le_scan_stop();
        if (err < 0)
        {
            PRINTF("Fail to stop scanning (err %d)\n", err);
        }
        return;
    }

    if (bt_addr_le_cmp(state->addr, addr) == 0)
    {
        err = bt_le_scan_stop();
        if (err != 0) {
            PRINTF("Failed to stop scan (err %d)\n", err);
        }

        /* Send connection request */
        err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
                                BT_LE_CONN_PARAM_DEFAULT,
                                &default_conn);
        if (err)
        {
            PRINTF("Create connection failed (err %d)\n", err);
            err = bt_le_scan_start(BT_LE_SCAN_CODED_ACTIVE, set_coordinator_scan_devce);
            if (err != 0) {
                PRINTF("Failed to start scan (err %d)\n", err);
            }
        }
        else
        {
            bt_conn_unref(default_conn);
            state->scanning_peer_addr = false;
        }
    }
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    struct set_coordinator_state *state;
    bool find = false;
    char addr[BT_ADDR_LE_STR_LEN];
#if (defined(CONFIG_BT_SETTINGS) && ((CONFIG_BT_SETTINGS) > 0U)) && \
    (defined(CONFIG_BT_SMP) && ((CONFIG_BT_SMP) > 0U))
    struct bond_data data;
#endif

    if (err != 0)
    {
        bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
        PRINTF("Fail to connect the peer (err %d)\n", err);
        return;
    }

    (void)find;

    state = create_set_coordinator_state(conn);
    if (state == NULL)
    {
        return;
    }

    state->conn = conn;

#if (defined(CONFIG_BT_SETTINGS) && ((CONFIG_BT_SETTINGS) > 0U)) && \
    (defined(CONFIG_BT_SMP) && ((CONFIG_BT_SMP) > 0U))
    for (uint32_t i = 0; i < ARRAY_SIZE(coordinator_set); i++)
    {
        if (bt_addr_le_cmp(&coordinator_set[i], bt_conn_get_dst(conn)) == 0)
        {
            find = true;
        }
    }

    if (find)
    {
        for (uint32_t i = 0; i < ARRAY_SIZE(coordinator_set); i++)
        {
            if (bt_addr_le_cmp(&coordinator_set[i], BT_ADDR_LE_ANY) == 0)
            {
                continue;
            }

            data.addr = &coordinator_set[i];
            data.bond = false;
            bt_foreach_bond(BT_ID_DEFAULT, bond_cb, &data);

            if (data.bond == false)
            {
                find = false;
            }
        }
    }

    if (find)
    {
        PRINTF("Find members, connecting members\n");

        for (uint32_t i = 0; i < ARRAY_SIZE(coordinator_set); i++)
        {
            if (bt_addr_le_cmp(&coordinator_set[i], bt_conn_get_dst(conn)) == 0)
            {
                continue;
            }

            if (bt_addr_le_cmp(&coordinator_set[i], BT_ADDR_LE_ANY) == 0)
            {
                continue;
            }

            if (find_set_coordinator_state_by_address(&coordinator_set[i]) != NULL)
            {
                continue;
            }

            bt_addr_le_to_str(&coordinator_set[i], addr, sizeof(addr));

            PRINTF("Find members, Start scann the device with address %s\n", addr);
            state->addr = &coordinator_set[i];
            state->scanning_peer_addr = true;
            err = bt_le_scan_start(BT_LE_SCAN_CODED_ACTIVE, set_coordinator_scan_devce);
            if (err != 0) {
                PRINTF("Failed to start scan (err %d)\n", err);
            }
            else
            {
                k_work_schedule(&state->work_delay, BT_SECONDS(SET_COORDINATOR_SCAN_TIMEOUT));
            }
        }
    }

    if (find == false)
#endif
    {
        state->member_discover = true;
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    struct set_coordinator_state *state;

    state = find_set_coordinator_state(conn);
    if (state != NULL)
    {
		if (state->set_scanning_index > 0) {
			int err;

			err = bt_le_scan_stop();
			if (err != 0) {
				PRINTF("Failed to stop scan (err %d)\n", err);
			}
        }
        memset(state, 0, sizeof(*state));
    }
    disconnect_all_set_coordinator_state();
}

#if CONFIG_BT_SMP
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    struct set_coordinator_state *state;

    state = find_set_coordinator_state(conn);
    if (state == NULL)
    {
        return;
    }

    if (state->member_discover)
    {
        state->member_discover = false;
        if (err != 0)
        {
            PRINTF("Fail to member discover\n");
        }else
        {
            PRINTF("Start member discover\n");
            k_work_submit(&state->work);
        }
    }
}
#endif

struct ad_scan
{
    struct set_coordinator_state *state;
    const bt_addr_le_t *addr;
};

static void set_coordinator_scan(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad);

static bool set_member_found(struct bt_data *data, void *user_data)
{
    struct ad_scan *scan = (struct ad_scan *)user_data;

    if (scan->state->set_scanning_index < 1)
    {
		/* Stop parsing */
        return false;
    }

	if (bt_csip_set_coordinator_is_set_member(scan->state->member->insts[scan->state->set_scanning_index - 1].info.set_sirk, data)) {
		const bt_addr_le_t *addr = scan->addr;
		char addr_str[BT_ADDR_LE_STR_LEN];
        struct bt_conn *default_conn;

        k_work_cancel_delayable(&scan->state->work_delay);

		bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
		PRINTF("Found CSIP advertiser with address %s\n", addr_str);

		PRINTF("Found member (%u / %u)\n", scan->state->set_scanning_index, scan->state->set_count);

		if (scan->state->set_scanning_index >= scan->state->set_count) {
			int err;

            err = bt_le_scan_stop();
            if (err != 0) {
                PRINTF("Failed to stop scan (err %d)\n", err);
            }

            /* Send connection request */
            err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
                                    BT_LE_CONN_PARAM_DEFAULT,
                                    &default_conn);
            if (err)
            {
                PRINTF("Create connection failed (err %d)\n", err);
                err = bt_le_scan_start(BT_LE_SCAN_CODED_ACTIVE, set_coordinator_scan);
                if (err != 0) {
                    PRINTF("Failed to start scan (err %d)\n", err);
                }
            }
            else
            {
                bt_conn_unref(default_conn);
                scan->state->set_scanning_index = 0;
                scan->state->set_scanning_done = 1;
            }
		}
        else
        {
            scan->state->set_scanning_index++;
        }

		/* Stop parsing */
		return false;
	}
	/* Continue parsing */
	return true;
}
int set_coordinator_clear(void)
{
    int err = 0;

#if (defined(CONFIG_BT_SETTINGS) && ((CONFIG_BT_SETTINGS) > 0U)) && \
    (defined(CONFIG_BT_SMP) && ((CONFIG_BT_SMP) > 0U))
    memset(coordinator_set, 0, sizeof(coordinator_set));
    err = settings_save_one("set_coordinator", coordinator_set, sizeof(coordinator_set));
#endif

    return err;
}

static int set_coordinator_settings()
{
    int err = 0;

#if (defined(CONFIG_BT_SETTINGS) && ((CONFIG_BT_SETTINGS) > 0U)) && \
    (defined(CONFIG_BT_SMP) && ((CONFIG_BT_SMP) > 0U))
    for (uint32_t i = 0; i < ARRAY_SIZE(coordinator_state); i++)
    {
        if (coordinator_state[i].conn == NULL)
        {
            return -ENOTCONN;
        }
        else
        {
            struct bt_conn *conn = coordinator_state[i].conn;
            struct bond_data data;

            data.addr = bt_conn_get_dst(conn);
            data.bond = false;

            bt_foreach_bond(BT_ID_DEFAULT, bond_cb, &data);

            if (data.bond == false)
            {
                return -ENOTSUP;
            }

            coordinator_set[i] = *data.addr;
        }
    }

    err = settings_save_one("set_coordinator", coordinator_set, sizeof(coordinator_set));
#endif
    return err;
}

static void set_coordinator_scan(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad)
{
    struct set_coordinator_state *state = find_discover_set_coordinator_state();
    struct ad_scan scan;

    if (state == NULL)
    {
        return;
    }
    scan.state = state;
    scan.addr = addr;
    bt_data_parse(ad, set_member_found, (void *)&scan);
}

static void set_coordinator_discover(
	struct bt_conn *conn,
	const struct bt_csip_set_coordinator_set_member *member,
	int err, size_t set_count)
{
    struct set_coordinator_state *state;
    struct set_coordinator_state *idle_state;

    PRINTF("set coordinator discover conn %p member %p count %d (err %d)\n", conn, member, set_count, err);

    state = find_set_coordinator_state(conn);
    if (state == NULL)
    {
        PRINTF("The conn is not found\n");
        return;
    }

    state->member = member;
    state->set_count = set_count;

    for (uint32_t i = 0; i < set_count; i++)
    {
        PRINTF("Member instance %p info:\n", &member->insts[i]);
        PRINTF("    SIRK:");
        for (uint32_t j = 0; j < BT_CSIP_SET_SIRK_SIZE; j++)
        {
            PRINTF(" %02X", member->insts[i].info.set_sirk[j]);
        }
        PRINTF("\n");
        PRINTF("    Set size %d\n", member->insts[i].info.set_size);
        PRINTF("    Rank %d\n", member->insts[i].info.rank);
        PRINTF("    Lockable %d\n", member->insts[i].info.lockable);
    }

    err = set_coordinator_settings();
    if ((err <= 0) && (err != -ENOTCONN))
    {
        PRINTF("Cannot save the set coordinator err %d\n", err);
    }

    if ((state->set_scanning_index == 0) && (set_count > 0))
    {
        idle_state = find_set_coordinator_state(NULL);
        if (idle_state != NULL)
        {
            int err;

            state->set_scanning_index = 1;

            PRINTF("Start scann set coordinator\n");
            err = bt_le_scan_start(BT_LE_SCAN_CODED_ACTIVE, set_coordinator_scan);
            if (err != 0) {
                PRINTF("Failed to start scan (err %d)\n", err);
            }
        }
        else
        {
            k_work_cancel_delayable(&state->work_delay);
        }
    }
}

static void set_coordinator_lock_changed(
	struct bt_csip_set_coordinator_csis_inst *inst, bool locked)
{
    PRINTF("set coordinator %p lock changed %d\n", inst, locked);
}

static void set_coordinator_lock_set(int err)
{
    PRINTF("set coordinator lock set (err %d)\n", err);
}

static void set_coordinator_ordered_access(
	const struct bt_csip_set_coordinator_set_info *set_info,
	int err, bool locked,
	struct bt_csip_set_coordinator_set_member *member)
{
    PRINTF("set coordinator ordered access set_info %p err %d locked %d member %p\n", set_info, err, locked, member);
}

static void set_coordinator_release_set(int err)
{
    PRINTF("set coordinator release set (err %d)\n", err);
}

int set_coordinator_init(void)
{
    int err;
#if (defined(CONFIG_BT_SETTINGS) && ((CONFIG_BT_SETTINGS) > 0U)) && \
    (defined(CONFIG_BT_SMP) && ((CONFIG_BT_SMP) > 0U))
    char addr_str[BT_ADDR_LE_STR_LEN];
#endif

    bt_conn_cb_register(&conn_callbacks);

    err = bt_csip_set_coordinator_register_cb(&set_coordinator_cb);
    if (err < 0)
    {
        return err;
    }

#if (defined(CONFIG_BT_SETTINGS) && ((CONFIG_BT_SETTINGS) > 0U)) && \
    (defined(CONFIG_BT_SMP) && ((CONFIG_BT_SMP) > 0U))
    PRINTF("CSIP List:\n");
    for (uint32_t i = 0; i < ARRAY_SIZE(coordinator_set); i++)
    {
        bt_addr_le_to_str(&coordinator_set[i], addr_str, sizeof(addr_str));
        PRINTF("    [%d] : %s\n", i, addr_str);
    }
#endif

    return 0;
}
