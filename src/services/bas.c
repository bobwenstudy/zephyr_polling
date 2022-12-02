/** @file
 *  @brief GATT Battery Service
 */

/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "bt_config.h"

#include "base/types.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/uuid.h>

#include <logging/bt_log_impl.h>

#if defined(CONFIG_BT_BAS)
static uint8_t battery_level = 100;

static void blvl_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);

    bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

    printk("BAS Notifications %s\n", notif_enabled ? "enabled" : "disabled");
}

static ssize_t read_blvl(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                         uint16_t len, uint16_t offset)
{
    uint8_t lvl8 = battery_level;

    return bt_gatt_attr_read(conn, attr, buf, len, offset, &lvl8, sizeof(lvl8));
}

BT_GATT_SERVICE_DEFINE(bas_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_BAS),
                       BT_GATT_CHARACTERISTIC(BT_UUID_BAS_BATTERY_LEVEL,
                                              BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                                              BT_GATT_PERM_READ, read_blvl, NULL, &battery_level),
                       BT_GATT_CCC(blvl_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), );

uint8_t bt_bas_get_battery_level(void)
{
    return battery_level;
}

int bt_bas_set_battery_level(uint8_t level)
{
    int rc;

    if (level > 100U)
    {
        return -EINVAL;
    }

    battery_level = level;

    rc = bt_gatt_notify(NULL, &bas_svc.attrs[1], &level, sizeof(level));

    return rc == -ENOTCONN ? 0 : rc;
}
#endif /* defined(CONFIG_BT_BAS) */