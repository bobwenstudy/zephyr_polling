/** @file
 *  @brief HTS Service sample
 */

/*
 * Copyright (c) 2020 SixOctets Systems
 * Copyright (c) 2019 Aaron Tsui <aaron.tsui@outlook.com>
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
#include <bluetooth/uuid.h>
#include <logging/bt_log_impl.h>

#include "host/hci_core.h"

extern uint8_t bt_att_get_tx_free_size(void);

enum
{
    TEST_CONFIG_RX_TO_TX = 0x00, // Stop Work
    TEST_CONFIG_RX_TEST,
    TEST_CONFIG_TX_TEST,
};

static uint8_t tx_enable;
static uint8_t test_config;

static uint32_t rx_total_cnt;
static uint32_t tx_total_cnt;

void throughput_svc_send(uint8_t *data, uint8_t len);

static void throughput_tx_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);
    tx_enable = notif_enabled;
}
static ssize_t data_rx(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
                       uint16_t len, uint16_t offset, uint8_t flags)
{
    uint8_t value[400] = {};
    // printk("data_rx(), offset: %d, len: %d, value: %s\n", offset, len, bt_hex_real(buf, len));

    if (offset >= sizeof(value))
    {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if (offset + len > sizeof(value))
    {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    memcpy(value, buf, len);

    // printk("data_rx(), value: %s\n", bt_hex_real(value, len));

    rx_total_cnt += len;

    if (test_config == TEST_CONFIG_RX_TO_TX)
    {
        throughput_svc_send(value, len);
    }

    return len;
}

static ssize_t config_rx(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
                         uint16_t len, uint16_t offset, uint8_t flags)
{
    // char value[20] = {};

    if (offset != 0 || len != 1)
    {
        return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }

    test_config = ((uint8_t *)buf)[0];

    printk("config_rx(), test_config: %d\n", test_config);

    printk("config_rx(), rx_total_cnt: %d, tx_total_cnt: %d\n", rx_total_cnt, tx_total_cnt);

    rx_total_cnt = 0;
    tx_total_cnt = 0;

    return len;
}

/* Throughput Service Declaration */
BT_GATT_SERVICE_DEFINE(throughput_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_DECLARE_16(0xff00)),
                       /* Rx charac */
                       BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(0xff01),
                                              BT_GATT_CHRC_WRITE_WITHOUT_RESP, BT_GATT_PERM_WRITE,
                                              NULL, data_rx, NULL),
                       /* Tx charac */
                       BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(0xff02), BT_GATT_CHRC_NOTIFY,
                                              BT_GATT_PERM_NONE, NULL, NULL, NULL),
                       BT_GATT_CCC(throughput_tx_ccc_cfg_changed,
                                   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

                       /* Config charac */
                       BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(0xff0f), BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_WRITE, NULL, config_rx, NULL), );

void throughput_svc_init(void)
{
    printk("throughput_svc_init()\n");
    tx_enable = 0;
    test_config = 0;
}

void throughput_svc_send_cb(struct bt_conn *conn, void *user_data)
{
    uint8_t reserve_size = bt_att_get_tx_free_size();
    if (reserve_size > 1)
    {
        bt_set_rx_acl_lock(false);
    }
}

void throughput_svc_send(uint8_t *data, uint8_t len)
{
    tx_total_cnt += len;
    // printk("throughput_svc_send(), tx_enable: %d\n", tx_enable);
    if (tx_enable)
    {
        struct bt_gatt_notify_params params;

        memset(&params, 0, sizeof(params));

        params.attr = &throughput_svc.attrs[3];
        params.data = data;
        params.len = len;
        params.func = throughput_svc_send_cb;

        bt_gatt_notify_cb(NULL, &params);

        uint8_t reserve_size = bt_att_get_tx_free_size();
        if (reserve_size <= 1)
        {
            bt_set_rx_acl_lock(true);
        }
    }
}
