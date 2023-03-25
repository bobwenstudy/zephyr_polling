#include <errno.h>

#include "chipset_csr8910.h"
#include "common\timer.h"

#define STATE_POLLING_NONE      0
#define STATE_POLLING_BOOTING   1
#define STATE_POLLING_PREPARING 2

struct k_timer csr_sync_timer;
struct k_timer csr_warn_reset_timer;

int state;
int step;

static int csr_send_cmd_vs_set_ana_freq_to_26mhz(void)
{
    uint8_t data[] = {0xc2, 0x02, 0x00, 0x09, 0x00, 0x01, 0x00, 0x03, 0x70, 0x00,
                      0x00, 0xfe, 0x01, 0x01, 0x00, 0x08, 0x00, 0x90, 0x65};
    struct net_buf *buf;

    buf = bt_hci_cmd_create(0xfc00, sizeof(data));
    if (!buf)
    {
        return -ENOBUFS;
    }
    net_buf_add_mem(buf, data, sizeof(data));

    return bt_hci_cmd_send(0xfc00, buf);
}

static int csr_send_cmd_vs_set_ana_ftrim_0x24_for_csr8811(void)
{
    uint8_t data[] = {0xc2, 0x02, 0x00, 0x09, 0x00, 0x04, 0x00, 0x03, 0x70, 0x00,
                      0x00, 0xf6, 0x01, 0x01, 0x00, 0x08, 0x00, 0x24, 0x00};
    struct net_buf *buf;

    buf = bt_hci_cmd_create(0xfc00, sizeof(data));
    if (!buf)
    {
        return -ENOBUFS;
    }
    net_buf_add_mem(buf, data, sizeof(data));

    return bt_hci_cmd_send(0xfc00, buf);
}

static int csr_send_cmd_vs_hci_nop_disable(void)
{
    uint8_t data[] = {0xc2, 0x02, 0x00, 0x09, 0x00, 0x02, 0x00, 0x03, 0x70, 0x00,
                      0x00, 0xf2, 0x00, 0x01, 0x00, 0x08, 0x00, 0x01, 0x00};
    struct net_buf *buf;

    buf = bt_hci_cmd_create(0xfc00, sizeof(data));
    if (!buf)
    {
        return -ENOBUFS;
    }
    net_buf_add_mem(buf, data, sizeof(data));

    return bt_hci_cmd_send(0xfc00, buf);
}

static int csr_send_cmd_vs_enable_rts_cts_for_bcsp(void)
{
    uint8_t data[] = {0xc2, 0x02, 0x00, 0x09, 0x00, 0x01, 0x00, 0x03, 0x70, 0x00,
                      0x00, 0xbf, 0x01, 0x01, 0x00, 0x08, 0x00, 0x0e, 0x08};
    struct net_buf *buf;

    buf = bt_hci_cmd_create(0xfc00, sizeof(data));
    if (!buf)
    {
        return -ENOBUFS;
    }
    net_buf_add_mem(buf, data, sizeof(data));

    return bt_hci_cmd_send(0xfc00, buf);
}

__unused
static int csr_send_cmd_vs_set_uart_baudrate_115200(void)
{
    uint8_t data[] = {0xc2, 0x02, 0x00, 0x0a, 0x00, 0x02, 0x00, 0x03, 0x70, 0x00, 0x00,
                      0xea, 0x01, 0x02, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0xc2};
    struct net_buf *buf;

    buf = bt_hci_cmd_create(0xfc00, sizeof(data));
    if (!buf)
    {
        return -ENOBUFS;
    }
    net_buf_add_mem(buf, data, sizeof(data));

    return bt_hci_cmd_send(0xfc00, buf);
}

static int csr_send_cmd_vs_set_uart_baudrate_921600(void)
{
    uint8_t data[] = {0xc2, 0x02, 0x00, 0x0a, 0x00, 0x0d, 0x00, 0x03, 0x70, 0x00, 0x00,
                      0xea, 0x01, 0x02, 0x00, 0x08, 0x00, 0x0e, 0x00, 0x00, 0x10};
    struct net_buf *buf;

    buf = bt_hci_cmd_create(0xfc00, sizeof(data));
    if (!buf)
    {
        return -ENOBUFS;
    }
    net_buf_add_mem(buf, data, sizeof(data));

    return bt_hci_cmd_send(0xfc00, buf);
}

#define BLE_MAC_ADDR                                                                               \
    {                                                                                              \
        {                                                                                          \
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66                                                     \
        }                                                                                          \
    }

static int csr_send_cmd_vs_set_public_addr(void)
{
    uint8_t data[] = {0xc2, 0x02, 0x00, 0x0c, 0x00, 0x08, 0x00, 0x03, 0x70, 0x00, 0x00, 0x01, 0x00,
                      0x04, 0x00, 0x08, 0x00, 0xf3, 0x00, 0xf5, 0xf4, 0xf0, 0x00, 0xf2, 0xf1};
    struct net_buf *buf;
    bt_addr_t addr = BLE_MAC_ADDR;

    // addr.val[0] = (BLE_MAC_ADDR)&0xff;
    // addr.val[1] = (BLE_MAC_ADDR >> 8) & 0xff;
    // addr.val[2] = (BLE_MAC_ADDR >> 16) & 0xff;
    // addr.val[3] = (BLE_MAC_ADDR >> 24) & 0xff;
    // addr.val[4] = (BLE_MAC_ADDR >> 32) & 0xff;
    // addr.val[5] = (BLE_MAC_ADDR >> 40) & 0xff;

    data[17] = addr.val[3];
    data[19] = addr.val[5];
    data[20] = addr.val[4];
    data[21] = addr.val[2];
    data[23] = addr.val[1];
    data[24] = addr.val[0];

    buf = bt_hci_cmd_create(0xfc00, sizeof(data));
    if (!buf)
    {
        return -ENOBUFS;
    }
    net_buf_add_mem(buf, data, sizeof(data));

    return bt_hci_cmd_send(0xfc00, buf);
}

static int csr_send_cmd_vs_warn_reset(void)
{
    uint8_t data[] = {0xc2, 0x02, 0x00, 0x09, 0x00, 0x09, 0x00, 0x02, 0x40, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    struct net_buf *buf;

    buf = bt_hci_cmd_create(0xfc00, sizeof(data));
    if (!buf)
    {
        return -ENOBUFS;
    }
    net_buf_add_mem(buf, data, sizeof(data));

    return bt_hci_cmd_send(0xfc00, buf);
}

static int csr_send_cmd_hci_reset_for_baudrate_sync(void)
{
    return bt_hci_cmd_send(BT_HCI_OP_RESET, NULL);
}

void boot_start(void)
{
    state = STATE_POLLING_BOOTING;

    step = 1;
    k_timer_start(&csr_sync_timer, K_MSEC(1000), Z_TIMEOUT_NO_WAIT);
    csr_send_cmd_hci_reset_for_baudrate_sync();
}

void prepare_start(void)
{
    state = STATE_POLLING_PREPARING;
    // nothing todo, just update state
    bt_hci_set_prepare_ready();
}

void hci_uart_sync_timeout(struct k_timer *timer)
{
    printf("hci_uart_sync_timeout()\n");
    k_timer_start(&csr_sync_timer, K_MSEC(1000), Z_TIMEOUT_NO_WAIT);
    csr_send_cmd_hci_reset_for_baudrate_sync();
}

void csr_reset_callback(struct k_timer *timer)
{
    printf("csr_reset_callback()\n");
    k_timer_stop(timer);

    k_timer_start(&csr_sync_timer, K_MSEC(1000), Z_TIMEOUT_NO_WAIT);
    csr_send_cmd_hci_reset_for_baudrate_sync();
}

void init_work(void)
{
    k_timer_init(&csr_sync_timer, hci_uart_sync_timeout, NULL);
    k_timer_init(&csr_warn_reset_timer, csr_reset_callback, NULL);

    state = STATE_POLLING_BOOTING;
    step = 0;
}

void event_process(uint8_t event, struct net_buf *buf)
{
    if ((event == BT_HCI_EVT_VENDOR) ||
        (event == BT_HCI_EVT_CMD_COMPLETE && (step == 1 || step == 20)))
    {
        switch (step)
        {
        case 1:
            k_timer_stop(&csr_sync_timer);
            csr_send_cmd_vs_set_ana_freq_to_26mhz();
            step = 2;
            break;
        case 2:
            csr_send_cmd_vs_set_ana_ftrim_0x24_for_csr8811();
            step = 3;
            break;
        case 3:
            csr_send_cmd_vs_hci_nop_disable();
            step = 4;
            break;
        case 4:
            csr_send_cmd_vs_enable_rts_cts_for_bcsp();
            step = 5;
            break;
        case 5:
            csr_send_cmd_vs_set_uart_baudrate_921600();
            step = 6;
            break;
        case 6:
            csr_send_cmd_vs_hci_nop_disable();
            step = 7;
            break;
        case 7:
            csr_send_cmd_vs_set_public_addr();
            step = 8;
            break;
        case 8:
            csr_send_cmd_vs_warn_reset();

            k_timer_start(&csr_warn_reset_timer, K_MSEC(1000), Z_TIMEOUT_NO_WAIT);
            step = 20;
            break;

        case 20:
            bt_hci_set_boot_ready();
            k_timer_stop(&csr_sync_timer);
            step = 0;
            break;
        }
    }
}

static const struct bt_hci_chipset_driver chipset_drv = {
        init_work, boot_start, prepare_start, event_process,
};

// public API
const struct bt_hci_chipset_driver *bt_hci_chipset_impl_local_instance(void)
{
    return &chipset_drv;
}

const bt_usb_interface_t *bt_chipset_get_usb_interface(void)
{
    return NULL;
}

static const bt_uart_interface_t uart_interface = {921600, 8, 1, 0, true};
const bt_uart_interface_t *bt_chipset_get_uart_interface(void)
{
    return &uart_interface;
}
