/* hci_userchan.c - HCI user channel Bluetooth handling */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include "bt_config.h"

#include "base/atomic.h"
#include "base/byteorder.h"

#include <drivers/hci_driver.h>
// #include <bluetooth/hci_raw.h>
#include <bluetooth/l2cap.h>

#include "logging/bt_log.h"
#include "bt_buf.h"
#include "common/timeout.h"
#include "utils/spool.h"

#define H4_CMD 0x01
#define H4_ACL 0x02
#define H4_SCO 0x03
#define H4_EVT 0x04
#define H4_ISO 0x05

SPOOL_DEFINE(evt_pool, CONFIG_BT_BUF_EVT_RX_COUNT, BT_BUF_EVT_RX_SIZE, 8);

SPOOL_DEFINE(hci_cmd_pool, CONFIG_BT_BUF_CMD_TX_COUNT, BT_BUF_CMD_SIZE(CONFIG_BT_BUF_CMD_TX_SIZE),
             8);
#if defined(CONFIG_BT_ISO)
SPOOL_DEFINE(hci_iso_pool, BT_ISO_TX_BUF_COUNT, BT_ISO_TX_MTU, 8);
#endif

#if defined(CONFIG_BT_CONN)
SPOOL_DEFINE(acl_tx_pool, CONFIG_BT_L2CAP_TX_BUF_COUNT, BT_L2CAP_BUF_SIZE(CONFIG_BT_L2CAP_TX_MTU),
             8);
SPOOL_DEFINE(acl_in_pool, CONFIG_BT_BUF_ACL_RX_COUNT, BT_BUF_ACL_SIZE(CONFIG_BT_BUF_ACL_RX_SIZE),
             8);
SPOOL_DEFINE(hci_acl_pool, CONFIG_BT_BUF_ACL_TX_COUNT, BT_BUF_ACL_SIZE(CONFIG_BT_BUF_ACL_TX_SIZE),
             8);
#endif

#if defined(CONFIG_BT_CONN)
#define NUM_COMLETE_EVENT_SIZE                                                                     \
    BT_BUF_SIZE(sizeof(struct bt_hci_evt_hdr) +                                                    \
                sizeof(struct bt_hci_cp_host_num_completed_packets) +                              \
                CONFIG_BT_MAX_CONN * sizeof(struct bt_hci_handle_count))
/* Dedicated pool for HCI_Number_of_Completed_Packets. This event is always
 * consumed synchronously by bt_recv_prio() so a single buffer is enough.
 * Having a dedicated pool for it ensures that exhaustion of the RX pool
 * cannot block the delivery of this priority event.
 */
SPOOL_DEFINE(num_complete_pool, 1, NUM_COMLETE_EVENT_SIZE, 8);

#endif /* CONFIG_BT_CONN */

uint8_t bt_get_h4_type_by_buffer(enum bt_buf_type type)
{

    switch (type)
    {
    case BT_BUF_EVT:
        return H4_EVT;
    case BT_BUF_ACL_IN:
    case BT_BUF_ACL_OUT:
        return H4_ACL;
    case BT_BUF_CMD:
        return H4_CMD;
    default:
        break;
    }
    BT_ERR("Invalid type: %u", type);
    return 0;
}

bool bt_buf_check_poll_acl_out_empty(void)
{
    return bt_buf_check_poll_empty(BT_BUF_ACL_OUT);
}

bool bt_buf_check_poll_empty(enum bt_buf_type type)
{
    struct spool *pool;
    // struct net_buf *buf;

    switch (type)
    {
    case BT_BUF_CMD:
        pool = &hci_cmd_pool;
        break;
    case BT_BUF_EVT:
        pool = &evt_pool;
        break;
#if defined(CONFIG_BT_CONN)
    case BT_BUF_ACL_OUT:
        pool = &acl_tx_pool;
        break;
    case BT_BUF_ACL_IN:
        pool = &acl_in_pool;
        break;
#endif
#if defined(CONFIG_BT_ISO)
    case BT_BUF_ISO_OUT:
        pool = &hci_iso_pool;
        break;
#endif
    default:
        BT_ERR("Invalid type: %u", type);
        return NULL;
    }

    return net_buf_check_empty(pool);
}

struct net_buf *bt_buf_get(enum bt_buf_type type)
{
    struct spool *pool;
    struct net_buf *buf;

    switch (type)
    {
    case BT_BUF_CMD:
        pool = &hci_cmd_pool;
        break;
    case BT_BUF_EVT:
        pool = &evt_pool;
        break;
#if defined(CONFIG_BT_CONN)
    case BT_BUF_ACL_OUT:
        pool = &acl_tx_pool;
        break;
    case BT_BUF_ACL_IN:
        pool = &acl_in_pool;
        break;
#endif
#if defined(CONFIG_BT_ISO)
    case BT_BUF_ISO_OUT:
        pool = &hci_iso_pool;
        break;
#endif
    default:
        BT_ERR("Invalid type: %u", type);
        return NULL;
    }

    buf = net_buf_alloc(pool, K_NO_WAIT);
    if (!buf)
    {
        return buf;
    }

    net_buf_reserve(buf, BT_BUF_RESERVE);
    bt_buf_set_type(buf, type);

    return buf;
}

uint8_t bt_buf_reserve_size(enum bt_buf_type type)
{
    struct spool *pool;
    // struct net_buf *buf;

    switch (type)
    {
    case BT_BUF_CMD:
        pool = &hci_cmd_pool;
        break;
    case BT_BUF_EVT:
        pool = &evt_pool;
        break;
#if defined(CONFIG_BT_CONN)
    case BT_BUF_ACL_OUT:
        pool = &acl_tx_pool;
        break;
    case BT_BUF_ACL_IN:
        pool = &acl_in_pool;
        break;
#endif
#if defined(CONFIG_BT_ISO)
    case BT_BUF_ISO_OUT:
        pool = &hci_iso_pool;
        break;
#endif
    default:
        BT_ERR("Invalid type: %u", type);
        return 0;
    }

    return spool_size(pool);
}

struct net_buf *bt_buf_get_host_tx_cmd(void)
{
    return bt_buf_get(BT_BUF_CMD);
}

struct net_buf *bt_buf_get_host_tx_acl(void)
{
    return bt_buf_get(BT_BUF_ACL_OUT);
}

struct net_buf *bt_buf_get_controller_tx_evt(void)
{
    return bt_buf_get(BT_BUF_EVT);
}

struct net_buf *bt_buf_get_controller_tx_acl(void)
{
    return bt_buf_get(BT_BUF_ACL_IN);
}

struct net_buf *bt_buf_get_rx(enum bt_buf_type type, k_timeout_t timeout)
{
    return bt_buf_get(type);
}

uint8_t bt_buf_reserve_size_host_tx_cmd(void)
{
    return bt_buf_reserve_size(BT_BUF_CMD);
}

uint8_t bt_buf_reserve_size_host_tx_acl(void)
{
    return bt_buf_reserve_size(BT_BUF_ACL_OUT);
}

uint8_t bt_buf_reserve_size_controller_tx_evt(void)
{
    return bt_buf_reserve_size(BT_BUF_EVT);
}

uint8_t bt_buf_reserve_size_controller_tx_acl(void)
{
    return bt_buf_reserve_size(BT_BUF_ACL_IN);
}

uint8_t bt_check_rx_evt_need_drop(uint8_t *packet)
{
    if (bt_buf_reserve_size(BT_BUF_EVT) < 3)
    {
        if (packet[0] == BT_HCI_EVT_LE_META_EVENT && packet[2] == BT_HCI_EVT_LE_ADVERTISING_REPORT)
        {
            return 1;
        }
    }

    return 0;
}

struct net_buf *bt_buf_get_evt(uint8_t evt, bool discardable, k_timeout_t timeout)
{
    switch (evt)
    {

#if defined(CONFIG_BT_CONN)
    case BT_HCI_EVT_NUM_COMPLETED_PACKETS:
    {
        struct net_buf *buf;
        buf = net_buf_alloc(&num_complete_pool, timeout);
        if (buf)
        {
            net_buf_reserve(buf, BT_BUF_RESERVE);
            bt_buf_set_type(buf, BT_BUF_EVT);
        }

        return buf;
    }
#endif /* CONFIG_BT_CONN */
    case BT_HCI_EVT_CMD_COMPLETE:
    case BT_HCI_EVT_CMD_STATUS:
        return bt_buf_get_controller_tx_evt();
    default:
        return bt_buf_get_controller_tx_evt();
    }
}

void bt_buf_pool_init(void)
{
    SPOOL_INIT(evt_pool, CONFIG_BT_BUF_EVT_RX_COUNT, BT_BUF_EVT_RX_SIZE, 8);
    // SPOOL_INIT(hci_rx_pool, BT_BUF_RX_COUNT, BT_BUF_RX_SIZE);
    SPOOL_INIT(hci_cmd_pool, CONFIG_BT_BUF_CMD_TX_COUNT, BT_BUF_CMD_SIZE(CONFIG_BT_BUF_CMD_TX_SIZE),
               8);
#if defined(CONFIG_BT_CONN)
    SPOOL_INIT(acl_tx_pool, CONFIG_BT_L2CAP_TX_BUF_COUNT, BT_L2CAP_BUF_SIZE(CONFIG_BT_L2CAP_TX_MTU),
               8);
    SPOOL_INIT(acl_in_pool, CONFIG_BT_BUF_ACL_RX_COUNT, BT_BUF_ACL_SIZE(CONFIG_BT_BUF_ACL_RX_SIZE),
               8);
    SPOOL_INIT(hci_acl_pool, CONFIG_BT_BUF_ACL_TX_COUNT, BT_BUF_ACL_SIZE(CONFIG_BT_BUF_ACL_TX_SIZE),
               8);
#endif
#if defined(CONFIG_BT_ISO)
    SPOOL_INIT(hci_iso_pool, BT_ISO_TX_BUF_COUNT, BT_ISO_TX_MTU, 8);
#endif

#if defined(CONFIG_BT_CONN)
    SPOOL_INIT(num_complete_pool, 1, NUM_COMLETE_EVENT_SIZE, 8);
#endif
}

uint8_t bt_buf_check_allow_sleep(void)
{
#if defined(CONFIG_BT_CONN)
    if (!spool_check_full(&acl_in_pool))
    {
        return 0;
    }
    if (!spool_check_full(&hci_acl_pool))
    {
        return 0;
    }
    if (!spool_check_full(&acl_tx_pool))
    {
        return 0;
    }
#endif
    if (!spool_check_full(&evt_pool))
    {
        return 0;
    }
    // if(!spool_check_full(&hci_rx_pool))
    // {
    //     return 0;
    // }
    if (!spool_check_full(&hci_cmd_pool))
    {
        return 0;
    }
#if defined(CONFIG_BT_ISO)
    if (!spool_check_full(&hci_iso_pool))
    {
        return 0;
    }
#endif
#if defined(CONFIG_BT_CONN)
    if (!spool_check_full(&num_complete_pool))
    {
        return 0;
    }
#endif

    return 1;
}

#if defined(CONFIG_BT_MONITOR_SLEEP)
void bt_buf_monitor_sleep(void)
{
#if defined(CONFIG_BT_CONN)
    memset(&acl_in_pool, 0, sizeof(struct spool));
    memset(&hci_acl_pool, 0, sizeof(struct spool));
    memset(&acl_tx_pool, 0, sizeof(struct spool));
#endif
    memset(&evt_pool, 0, sizeof(struct spool));
    // memset(&hci_rx_pool, 0, sizeof(struct spool));
    memset(&hci_cmd_pool, 0, sizeof(struct spool));
#if defined(CONFIG_BT_ISO)
    memset(&hci_iso_pool, 0, sizeof(struct spool));
#endif

#if defined(CONFIG_BT_CONN)
    memset(&num_complete_pool, 0, sizeof(struct spool));
#endif
#if 0
    SPOOL_INIT(hci_rx_pool, 0, 0);
    SPOOL_INIT(hci_cmd_pool, 0, 0);
    SPOOL_INIT(hci_acl_pool, 0, 0);
#if defined(CONFIG_BT_ISO)
    SPOOL_INIT(hci_iso_pool, 0, 0);
#endif

    SPOOL_INIT(acl_tx_pool, 0, 0);
#if defined(CONFIG_BT_CONN)
    SPOOL_INIT(num_complete_pool, 0, 0);
#endif
#endif
}
#endif
