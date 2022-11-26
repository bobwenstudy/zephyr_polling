/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "base/types.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <logging/log_impl.h>

static struct bt_gatt_exchange_params mtu_exchange_params;
static uint32_t write_count;
static uint32_t write_len;
static uint32_t write_rate;
struct bt_conn *conn_connected;
uint32_t last_write_rate;
void (*start_scan_func)(void);

static void write_cmd_cb(struct bt_conn *conn, void *user_data)
{
    static uint32_t cycle_stamp;
    uint64_t delta;
    uint16_t len;
    /* Extract the 16-bit data length stored in user_data */
    len = (uint32_t)user_data & 0xFFFF;
    printk("len= %u.\n", len);
}

static void mtu_exchange_cb(struct bt_conn *conn, uint8_t err,
                            struct bt_gatt_exchange_params *params)
{
    printk("%s: MTU exchange %s (%u)\n", __func__, err == 0U ? "successful" : "failed",
           bt_gatt_get_mtu(conn));
}

static int mtu_exchange(struct bt_conn *conn)
{
    int err;

    printk("%s: Current MTU = %u\n", __func__, bt_gatt_get_mtu(conn));

    mtu_exchange_params.func = mtu_exchange_cb;

    printk("%s: Exchange MTU...\n", __func__);
    err = bt_gatt_exchange_mtu(conn, &mtu_exchange_params);
    if (err)
    {
        printk("%s: MTU exchange failed (err %d)", __func__, err);
    }

    return err;
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
    struct bt_conn_info conn_info;
    char addr[BT_ADDR_LE_STR_LEN];
    int err;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (conn_err)
    {
        printk("%s: Failed to connect to %s (%u)\n", __func__, addr, conn_err);
        return;
    }

    err = bt_conn_get_info(conn, &conn_info);
    if (err)
    {
        printk("Failed to get connection info (%d).\n", err);
        return;
    }

    printk("%s: %s role %u\n", __func__, addr, conn_info.role);

    // conn_connected = bt_conn_ref(conn);
    conn_connected = conn;

    (void)mtu_exchange(conn);

#if defined(CONFIG_BT_SMP)
    if (conn_info.role == BT_CONN_ROLE_CENTRAL)
    {
        err = bt_conn_set_security(conn, BT_SECURITY_L2);
        if (err)
        {
            printk("Failed to set security (%d).\n", err);
        }
    }
#endif
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    struct bt_conn_info conn_info;
    char addr[BT_ADDR_LE_STR_LEN];
    int err;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    err = bt_conn_get_info(conn, &conn_info);
    if (err)
    {
        printk("Failed to get connection info (%d).\n", err);
        return;
    }

    printk("%s: %s role %u (reason %u)\n", __func__, addr, conn_info.role, reason);

    conn_connected = NULL;

    // bt_conn_unref(conn);

    if (conn_info.role == BT_CONN_ROLE_CENTRAL)
    {
        start_scan_func();
    }
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
    printk("%s: int (0x%04x, 0x%04x) lat %u to %u\n", __func__, param->interval_min,
           param->interval_max, param->latency, param->timeout);

    return true;
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency,
                             uint16_t timeout)
{
    printk("%s: int 0x%04x lat %u to %u\n", __func__, interval, latency, timeout);
}

#if defined(CONFIG_BT_SMP)
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    printk("%s: to level %u (err %u)\n", __func__, level, err);
}
#endif

struct bt_conn_cb conn_callbacks = {
        .connected = connected,
        .disconnected = disconnected,
        .le_param_req = le_param_req,
        .le_param_updated = le_param_updated,
#if defined(CONFIG_BT_SMP)
        .security_changed = security_changed,
#endif
};

int write_cmd(struct bt_conn *conn)
{
    static uint8_t data[BT_ATT_MAX_ATTRIBUTE_LEN] = {
            0,
    };
    static uint16_t data_len;
    uint16_t data_len_max;
    int err;

    data_len_max = bt_gatt_get_mtu(conn) - 3;
    if (data_len_max > BT_ATT_MAX_ATTRIBUTE_LEN)
    {
        data_len_max = BT_ATT_MAX_ATTRIBUTE_LEN;
    }

#if TEST_FRAGMENTATION_WITH_VARIABLE_LENGTH_DATA
    /* Use incremental length data for every write command */
    /* TODO: Include test case in BabbleSim tests */
    static bool decrement;

    if (decrement)
    {
        data_len--;
        if (data_len <= 1)
        {
            data_len = 1;
            decrement = false;
        }
    }
    else
    {
        data_len++;
        if (data_len >= data_len_max)
        {
            data_len = data_len_max;
            decrement = true;
        }
    }
#else
    /* Use fixed length data for every write command */
    data_len = data_len_max;
#endif

    /* Pass the 16-bit data length value (instead of reference) in
     * user_data so that unique value is pass for each write callback.
     * Using handle 0x0001, we do not care if it is writable, we just want
     * to transmit the data across.
     */
    err = bt_gatt_write_without_response_cb(conn, 0x0001, data, data_len, false, write_cmd_cb,
                                            (void *)((uint32_t)data_len));
    if (err)
    {
        printk("%s: Write cmd failed (%d).\n", __func__, err);
    }

    return err;
}
