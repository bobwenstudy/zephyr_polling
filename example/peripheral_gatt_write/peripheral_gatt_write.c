/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "services/bas.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "base/byteorder.h"
#include "base/types.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <logging/bt_log_impl.h>

extern int mtu_exchange(struct bt_conn *conn);
extern int write_cmd(struct bt_conn *conn);
extern struct bt_conn *conn_connected;
extern uint32_t last_write_rate;
extern struct bt_conn_cb conn_callbacks;

static const struct bt_data ad[] = {
        BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
};

__unused
static void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
    printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

#if defined(CONFIG_BT_SMP)
static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_callbacks = {
        .cancel = auth_cancel,
};
#endif /* CONFIG_BT_SMP */

// static struct bt_gatt_cb gatt_callbacks = {
// 	.att_mtu_updated = mtu_updated
// };

static uint32_t count;
void bt_ready(int err)
{
    if (err)
    {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    extern struct bt_gatt_service_static _1_gatt_svc;
    extern struct bt_gatt_service_static _2_gap_svc;

    bt_gatt_service_init(2, _1_gatt_svc, _2_gap_svc);

    // bt_gatt_cb_register(&gatt_callbacks);
    bt_conn_cb_register(&conn_callbacks);

    conn_connected = NULL;
    last_write_rate = 0U;

    count = 1;

#if defined(CONFIG_BT_SMP)
    (void)bt_conn_auth_cb_register(&auth_callbacks);
#endif /* CONFIG_BT_SMP */

#if defined(CONFIG_BT_FIXED_PASSKEY)
    bt_passkey_set(1234);
#endif

    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err)
    {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }

    printk("Advertising successfully started\n");
}

void app_polling_work(void)
{
    struct bt_conn *conn = conn_connected;

    if (conn)
    {
        if (count)
        {
            count--;
            (void)write_cmd(conn);
        }
    }
}
