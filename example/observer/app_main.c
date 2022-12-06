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

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad)
{
    char addr_str[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    printk("Device found: %s (RSSI %d)\n", addr_str, rssi);
}

void bt_ready(int err)
{
    struct bt_le_scan_param scan_param = {
            .type = BT_LE_SCAN_TYPE_PASSIVE,
            .options = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
            .interval = BT_GAP_SCAN_FAST_INTERVAL,
            .window = BT_GAP_SCAN_FAST_WINDOW,
    };

    if (err)
    {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    printk("Starting Observer\n");

    err = bt_le_scan_start(&scan_param, device_found);
    printk("\n");
    if (err)
    {
        printk("Starting scanning failed (err %d)\n", err);
        return;
    }
}

void app_polling_work(void)
{
}
