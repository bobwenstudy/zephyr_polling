#include <errno.h>

#include "chipset_csr8510.h"
#include "windows_driver_libusb.h"

#define STATE_POLLING_NONE      0
#define STATE_POLLING_BOOTING   1
#define STATE_POLLING_PREPARING 2

int state;
int step;

void booting_work(void)
{
}

void preparing_work(void)
{
}

void polling_work(void)
{
    switch (state)
    {
    case STATE_POLLING_BOOTING:
        booting_work();
        break;
    case STATE_POLLING_PREPARING:
        preparing_work();
        break;
    }
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

#define BLE_MAC_ADDR                                                                               \
    {                                                                                              \
        {                                                                                          \
            0x11, 0x22, 0x33, 0x44, 0x55                                                           \
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

void boot_start(void)
{
    state = STATE_POLLING_BOOTING;
    step = 1;
    csr_send_cmd_vs_hci_nop_disable();
}

void prepare_start(void)
{
    state = STATE_POLLING_PREPARING;
    // nothing todo, just update state
    bt_hci_set_prepare_ready();
}

void init_work(void)
{
    state = STATE_POLLING_BOOTING;
    step = 0;
}

void reset_callback(void)
{
    bt_hci_set_boot_ready();
}

void event_process(uint8_t event, struct net_buf *buf)
{
    if (event == BT_HCI_EVT_VENDOR)
    {
        switch (step)
        {
        case 1:
            csr_send_cmd_vs_set_public_addr();
            step = 2;
            break;
        case 2:
            csr_send_cmd_vs_warn_reset();

            reset_usb_driver(reset_callback);
            step = 0;
            break;
        }
    }
}

static const struct bt_hci_chipset_driver chipset_drv = {
        init_work, polling_work, boot_start, prepare_start, event_process,
};

// public API
const struct bt_hci_chipset_driver *chipset_local_instance(void)
{
    return &chipset_drv;
}

static const usb_interface_t usb_interface[] = {
    {0x0a12, 0x0001},
};

const usb_interface_t *chipset_get_usb_interface(uint8_t* size)
{
    *size = sizeof(usb_interface)/sizeof(usb_interface[0]);
    return usb_interface;
}

const uart_interface_t *chipset_get_uart_interface(void)
{
    return NULL;
}