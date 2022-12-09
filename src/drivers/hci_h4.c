/* h4.c - H:4 UART based Bluetooth driver */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "hci_h4.h"

#include "base/byteorder.h"
#include "base/util.h"
#include "common\timeout.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <drivers/hci_driver.h>

#include "utils\k_fifo.h"

#define BT_DBG_ENABLED  IS_ENABLED(CONFIG_BT_DEBUG_HCI_DRIVER)
#define LOG_MODULE_NAME bt_driver
#include "logging/bt_log.h"

#define H4_NONE 0x00
#define H4_CMD  0x01
#define H4_ACL  0x02
#define H4_SCO  0x03
#define H4_EVT  0x04
#define H4_ISO  0x05

static const struct bt_hci_h4_driver *h4_driver;

static struct
{
    struct net_buf *buf;
    struct k_fifo fifo;

    uint16_t remaining;
    uint16_t discard;

    bool have_hdr;
    bool discardable;

    uint8_t hdr_len;

    uint8_t type;
    union
    {
        struct bt_hci_evt_hdr evt;
        struct bt_hci_acl_hdr acl;
        struct bt_hci_iso_hdr iso;
        uint8_t hdr[4];
    };
} rx;

static struct
{
    uint8_t type;
    struct net_buf *buf;
    struct k_fifo fifo;
} tx;

static inline void h4_get_type(void)
{
    /* Get packet type */
    if (h4_driver->recv(&rx.type, 1) != 1)
    {
        // BT_WARN("Unable to read H:4 packet type");
        rx.type = H4_NONE;
        return;
    }

    switch (rx.type)
    {
    case H4_EVT:
        rx.remaining = sizeof(rx.evt);
        rx.hdr_len = rx.remaining;
        break;
    case H4_ACL:
        rx.remaining = sizeof(rx.acl);
        rx.hdr_len = rx.remaining;
        break;
    case H4_ISO:
        if (IS_ENABLED(CONFIG_BT_ISO))
        {
            rx.remaining = sizeof(rx.iso);
            rx.hdr_len = rx.remaining;
            break;
        }
        __fallthrough;
    default:
        BT_ERR("Unknown H:4 type 0x%02x", rx.type);
        rx.type = H4_NONE;
    }
}

static void h4_read_hdr(void)
{
    int bytes_read = rx.hdr_len - rx.remaining;
    int ret;

    ret = h4_driver->recv(rx.hdr + bytes_read, rx.remaining);
    if (unlikely(ret < 0))
    {
        BT_ERR("Unable to read from UART (ret %d)", ret);
    }
    else
    {
        rx.remaining -= ret;
    }
}

static inline void get_acl_hdr(void)
{
    h4_read_hdr();

    if (!rx.remaining)
    {
        struct bt_hci_acl_hdr *hdr = &rx.acl;

        rx.remaining = sys_le16_to_cpu(hdr->len);
        BT_DBG("Got ACL header. Payload %u bytes", rx.remaining);
        rx.have_hdr = true;
    }
}

static inline void get_iso_hdr(void)
{
    h4_read_hdr();

    if (!rx.remaining)
    {
        struct bt_hci_iso_hdr *hdr = &rx.iso;

        rx.remaining = bt_iso_hdr_len(sys_le16_to_cpu(hdr->len));
        BT_DBG("Got ISO header. Payload %u bytes", rx.remaining);
        rx.have_hdr = true;
    }
}

static inline void get_evt_hdr(void)
{
    struct bt_hci_evt_hdr *hdr = &rx.evt;

    h4_read_hdr();

    if (rx.hdr_len == sizeof(*hdr) && rx.remaining < sizeof(*hdr))
    {
        switch (rx.evt.evt)
        {
        case BT_HCI_EVT_LE_META_EVENT:
            rx.remaining++;
            rx.hdr_len++;
            break;
#if defined(CONFIG_BT_BREDR)
        case BT_HCI_EVT_INQUIRY_RESULT_WITH_RSSI:
        case BT_HCI_EVT_EXTENDED_INQUIRY_RESULT:
            rx.discardable = true;
            break;
#endif
        }
    }

    if (!rx.remaining)
    {
        if (rx.evt.evt == BT_HCI_EVT_LE_META_EVENT &&
            (rx.hdr[sizeof(*hdr)] == BT_HCI_EVT_LE_ADVERTISING_REPORT))
        {
            BT_DBG("Marking adv report as discardable");
            rx.discardable = true;
        }

        rx.remaining = hdr->len - (rx.hdr_len - sizeof(*hdr));
        BT_DBG("Got event header. Payload %u bytes", hdr->len);
        rx.have_hdr = true;
    }
}

static inline void copy_hdr(struct net_buf *buf)
{
    net_buf_add_mem(buf, rx.hdr, rx.hdr_len);
}

static void reset_rx(void)
{
    rx.type = H4_NONE;
    rx.remaining = 0U;
    rx.have_hdr = false;
    rx.hdr_len = 0U;
    rx.discardable = false;
}

static struct net_buf *get_rx(void)
{
    BT_DBG("type 0x%02x, evt 0x%02x", rx.type, rx.evt.evt);

    switch (rx.type)
    {
    case H4_EVT:
        return bt_buf_get_evt(rx.evt.evt, rx.discardable, K_NO_WAIT);
    case H4_ACL:
        return bt_buf_get_rx(BT_BUF_ACL_IN, K_NO_WAIT);
#if defined(CONFIG_BT_ISO)
    case H4_ISO:
        return bt_buf_get_rx(BT_BUF_ISO_IN, K_NO_WAIT);
#endif
    }

    return NULL;
}

static size_t h4_discard(size_t len)
{
    uint8_t buf[33];
    int err;

    err = h4_driver->recv(buf, MIN(len, sizeof(buf)));
    if (unlikely(err < 0))
    {
        BT_ERR("Unable to read from UART (err %d)", err);
        return 0;
    }

    return err;
}

static inline void read_payload(void)
{
    struct net_buf *buf;
    // uint8_t evt_flags;
    int read;

    if (!rx.buf)
    {
        size_t buf_tailroom;

        rx.buf = get_rx();
        if (!rx.buf)
        {
            if (rx.discardable)
            {
                BT_WARN("Discarding event 0x%02x", rx.evt.evt);
                rx.discard = rx.remaining;
                reset_rx();
                return;
            }

            BT_WARN("Failed to allocate, deferring to rx_thread");
            return;
        }

        BT_DBG("Allocated rx.buf %p", rx.buf);

        buf_tailroom = net_buf_tailroom(rx.buf);
        if (buf_tailroom < rx.remaining)
        {
            BT_ERR("Not enough space in buffer %u/%zu", rx.remaining, buf_tailroom);
            rx.discard = rx.remaining;
            reset_rx();
            return;
        }

        copy_hdr(rx.buf);
    }

    read = h4_driver->recv(net_buf_tail(rx.buf), rx.remaining);
    if (unlikely(read < 0))
    {
        BT_ERR("Failed to read UART (err %d)", read);
        return;
    }

    net_buf_add(rx.buf, read);
    rx.remaining -= read;

    BT_DBG("got %d bytes, remaining %u", read, rx.remaining);
    BT_DBG("Payload (len %u): %s", rx.buf->len, bt_hex(rx.buf->data, rx.buf->len));

    if (rx.remaining)
    {
        return;
    }

    buf = rx.buf;
    rx.buf = NULL;

    if (rx.type == H4_EVT)
    {
        // evt_flags = bt_hci_evt_get_flags(rx.evt.evt);
        bt_buf_set_type(buf, BT_BUF_EVT);
    }
    else
    {
        // evt_flags = BT_HCI_EVT_FLAG_RECV;
        bt_buf_set_type(buf, BT_BUF_ACL_IN);
    }

    reset_rx();

    // if (evt_flags & BT_HCI_EVT_FLAG_RECV) {
    // 	BT_DBG("Putting buf %p to rx fifo", buf);
    // 	net_buf_put(&rx.fifo, buf);
    // }

    bt_recv(buf);
}

static inline void read_header(void)
{
    switch (rx.type)
    {
    case H4_NONE:
        h4_get_type();
        return;
    case H4_EVT:
        get_evt_hdr();
        break;
    case H4_ACL:
        get_acl_hdr();
        break;
#if defined(CONFIG_BT_ISO)
    case H4_ISO:
        get_iso_hdr();
        break;
#endif
    default:
        CODE_UNREACHABLE;
        return;
    }

    if (rx.have_hdr && rx.buf)
    {
        if (rx.remaining > net_buf_tailroom(rx.buf))
        {
            BT_ERR("Not enough space in buffer");
            rx.discard = rx.remaining;
            reset_rx();
        }
        else
        {
            copy_hdr(rx.buf);
        }
    }
}

static inline void process_tx(void)
{
    int bytes;

    if (!tx.buf)
    {
        tx.buf = net_buf_get(&tx.fifo, Z_FOREVER);
        if (!tx.buf)
        {
            // BT_ERR("TX interrupt but no pending buffer!");
            // uart_irq_tx_disable(h4_dev);
            return;
        }
    }

    if (!tx.type)
    {
        switch (bt_buf_get_type(tx.buf))
        {
        case BT_BUF_ACL_OUT:
            tx.type = H4_ACL;
            break;
        case BT_BUF_CMD:
            tx.type = H4_CMD;
            break;
        case BT_BUF_ISO_OUT:
#if defined(CONFIG_BT_ISO)
            tx.type = H4_ISO;
            break;
#endif
        default:
            BT_ERR("Unknown buffer type");
            goto done;
        }

        bytes = h4_driver->send(&tx.type, 1);
        if (bytes != 1)
        {
            BT_WARN("Unable to send H:4 type");
            tx.type = H4_NONE;
            return;
        }
    }

    bytes = h4_driver->send(tx.buf->data, tx.buf->len);
    if (unlikely(bytes < 0))
    {
        BT_ERR("Unable to write to UART (err %d)", bytes);
    }
    else
    {
        net_buf_pull(tx.buf, bytes);
    }

    if (tx.buf->len)
    {
        return;
    }

done:
    tx.type = H4_NONE;
    net_buf_unref(tx.buf);
    tx.buf = net_buf_get(&tx.fifo, Z_FOREVER);
}

static inline void process_rx(void)
{
    BT_DBG("remaining %u discard %u have_hdr %u rx.buf %p len %u", rx.remaining, rx.discard,
           rx.have_hdr, rx.buf, rx.buf ? rx.buf->len : 0);

    if (rx.discard)
    {
        rx.discard -= h4_discard(rx.discard);
        return;
    }

    if (rx.have_hdr)
    {
        read_payload();
    }
    else
    {
        read_header();
    }
}

void bt_hci_h4_polling(void)
{
    process_tx();
    process_rx();
}

static int h4_send(struct net_buf *buf)
{
    BT_DBG("buf %p type %u len %u", buf, bt_buf_get_type(buf), buf->len);

    net_buf_put(&tx.fifo, buf);

    return 0;
}

/** Setup the HCI transport, which usually means to reset the Bluetooth IC
 *
 * @param dev The device structure for the bus connecting to the IC
 *
 * @return 0 on success, negative error value on failure
 */
int bt_hci_transport_setup(void)
{
    h4_discard(32);
    return 0;
}

static int h4_open(void)
{
    int ret;

    BT_DBG("");

    ret = bt_hci_transport_setup();
    if (ret < 0)
    {
        return -EIO;
    }

    return 0;
}

#if defined(CONFIG_BT_HCI_SETUP)
static int h4_setup(void)
{
    /* Extern bt_h4_vnd_setup function.
     * This function executes vendor-specific commands sequence to
     * initialize BT Controller before BT Host executes Reset sequence.
     * bt_h4_vnd_setup function must be implemented in vendor-specific HCI
     * extansion module if CONFIG_BT_HCI_SETUP is enabled.
     */
    extern int bt_h4_vnd_setup(const struct device *dev);

    return bt_h4_vnd_setup(h4_dev);
}
#endif

static const struct bt_hci_driver drv = {
        .open = h4_open,
        .send = h4_send,
};

int hci_h4_init(const struct bt_hci_h4_driver *h4)
{
    bt_hci_driver_register(&drv);
    h4_driver = h4;

    k_fifo_init(&rx.fifo);
    k_fifo_init(&tx.fifo);

    return 0;
}
