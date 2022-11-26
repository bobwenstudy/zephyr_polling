/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "hog.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "base/types.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <logging/log_impl.h>

#include "services/bas.h"
#include "services/dis.h"

static const struct bt_data ad[] = {
        BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
        BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_HIDS_VAL),
                      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err)
    {
        printk("Failed to connect to %s (%u)\n", addr, err);
        return;
    }

    printk("Connected %s\n", addr);

    if (bt_conn_set_security(conn, BT_SECURITY_L2))
    {
        printk("Failed to set security\n");
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Disconnected from %s (reason 0x%02x)\n", addr, reason);
}

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (!err)
    {
        printk("Security changed: %s level %u\n", addr, level);
    }
    else
    {
        printk("Security failed: %s level %u err %d\n", addr, level, err);
    }
}

static struct bt_conn_cb conn_callbacks = {
        .connected = connected,
        .disconnected = disconnected,
        .security_changed = security_changed,
};

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
        .passkey_display = auth_passkey_display,
        .passkey_entry = NULL,
        .cancel = auth_cancel,
};

int is_bt_ready_work = 0;
void bt_ready(int err)
{
    if (err)
    {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    is_bt_ready_work = 1;
    printk("Bluetooth initialized\n");

    extern struct bt_gatt_service_static _1_gatt_svc;
    extern struct bt_gatt_service_static _2_gap_svc;
    extern struct bt_gatt_service_static hog_svc;
    bt_gatt_service_init(5, _1_gatt_svc, _2_gap_svc, dis_svc, bas_svc, hog_svc);

    hog_init();

    bt_conn_cb_register(&conn_callbacks);
    bt_conn_auth_cb_register(&auth_cb_display);

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

int test_cnt = 0;
void app_polling_work(void)
{
    if (!is_bt_ready_work)
    {
        return;
    }

    if (test_cnt++ < 200000)
    {
        return;
    }
    test_cnt = 0;
}
