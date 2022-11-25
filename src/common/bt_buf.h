/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _ZEPHYR_POLLING_COMMON_BT_BUF_H_
#define _ZEPHYR_POLLING_COMMON_BT_BUF_H_

#include "bt_config.h"

#include <bluetooth/buf.h>

#ifdef __cplusplus
extern "C" {
#endif

struct bt_dev_raw
{
    /* Registered HCI driver */
    const struct bt_hci_driver *drv;
};

extern struct bt_dev_raw bt_dev_raw;

uint8_t bt_get_h4_type_by_buffer(enum bt_buf_type type);
struct net_buf *bt_buf_get(enum bt_buf_type type);
bool bt_buf_check_poll_empty(enum bt_buf_type type);
bool bt_buf_check_poll_acl_out_empty(void);
struct net_buf *bt_buf_get_host_tx_cmd(void);
struct net_buf *bt_buf_get_host_tx_acl(void);

void clear_net_buf_pool_fixd_lpm(void);

struct net_buf *bt_buf_get_controller_tx_evt(void);
struct net_buf *bt_buf_get_controller_tx_acl(void);
struct net_buf *bt_buf_get_evt(uint8_t evt, bool discardable, k_timeout_t timeout);

uint8_t bt_buf_reserve_size_host_tx_cmd(void);
uint8_t bt_buf_reserve_size_host_tx_acl(void);
uint8_t bt_buf_reserve_size_controller_tx_evt(void);
uint8_t bt_buf_reserve_size_controller_tx_acl(void);

uint8_t bt_check_rx_evt_need_drop(uint8_t *packet);

void bt_buf_pool_init(void);
uint8_t bt_buf_check_allow_sleep(void);

#ifdef __cplusplus
}
#endif

#endif /* _ZEPHYR_POLLING_COMMON_BT_BUF_H_ */