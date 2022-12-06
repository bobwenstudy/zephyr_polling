/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

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
#include "common\timer.h"

/* Idle timer */
struct k_timer idle_work;

#include "throughput_service.h"

static const struct bt_data ad[] = {
        BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
        BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_HTS_VAL),
                      BT_UUID_16_ENCODE(BT_UUID_DIS_VAL), BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
};

struct bt_conn *default_conn = NULL;
static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err)
    {
        printk("Connection failed (err 0x%02x)\n", err);
    }
    else
    {
        default_conn = conn;
        printk("Connected\n");
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    printk("Disconnected (reason 0x%02x)\n", reason);
    default_conn = NULL;
}

static struct bt_conn_cb conn_callbacks = {
        .connected = connected,
        .disconnected = disconnected,
};

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
        .cancel = auth_cancel,
};

static void idle_timeout(struct k_timer *work)
{
    if (default_conn != NULL)
    {
        struct bt_conn_info info;
        if (!bt_conn_get_info(default_conn, &info))
        {
            if (info.le.interval > 12)
            {
                struct bt_le_conn_param param;
                int err;

                param.interval_min = 12;
                param.interval_max = 12;
                param.latency = 0;
                param.timeout = 1000;

                err = bt_conn_le_param_update(default_conn, &param);
                if (err)
                {
                    printk("conn update failed (err %d).\n", err);
                }
            }
        }
    }
}

void bt_ready(int err)
{
    if (err)
    {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    throughput_svc_init();

    extern struct bt_gatt_service_static _1_gatt_svc;
    extern struct bt_gatt_service_static _2_gap_svc;

    bt_gatt_service_init(3, _1_gatt_svc, _2_gap_svc, throughput_svc);

    bt_conn_cb_register(&conn_callbacks);
    bt_conn_auth_cb_register(&auth_cb_display);

#if defined(CONFIG_BT_FIXED_PASSKEY)
    bt_passkey_set(1234);
#endif

    k_timer_init(&idle_work, idle_timeout, NULL);
    k_timer_start(&idle_work, K_SECONDS(1), K_SECONDS(1));

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
}
