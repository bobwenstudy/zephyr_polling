#include <stdio.h>
#include <pthread.h>

#include "windows_driver_libusb.h"
#include "drivers/hci_driver.h"
#include "lib/lusb0_usb.h"

#include "logging/log_impl.h"

#include "common/bt_buf.h"
#include "host/hci_core.h"

// Device vendor and product id.
#define MY_VID 0x0a12
#define MY_PID 0x0001

// Device configuration and interface id.
#define MY_CONFIG 1
#define MY_INTF   0

#define END_POINT_SCO_R    (131)
#define END_POINT_SCO_W    (3)
#define END_POINT_ACL_R    (130)
#define END_POINT_ACL_W    (2)
#define END_POINT_CMD_CTRL (0)
#define END_POINT_EVT_INTR (129) //(129)

static volatile bool is_enable;
static volatile bool is_in_reset;
static volatile bool is_ready_work; // avoid last work have pendding packet.

static struct k_fifo tx_queue;
static pthread_mutex_t tx_lock;
static struct k_fifo rx_queue;
static pthread_mutex_t rx_lock;

usb_dev_handle *usb_dev;
usb_dev_handle *open_dev(void)
{
    struct usb_bus *bus;
    struct usb_device *dev;

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == MY_VID && dev->descriptor.idProduct == MY_PID)
            {
                return usb_open(dev);
            }
        }
    }
    return NULL;
}

static void display_devices(void)
{
    struct usb_bus *bus;
    struct usb_device *dev;

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            printk("idVendor: 0x%x, idProduct: 0x%x\n", dev->descriptor.idVendor,
                   dev->descriptor.idProduct);
        }
    }
}

static struct net_buf *pop_tx_queue(void)
{
    pthread_mutex_lock(&tx_lock);
    struct net_buf *buf = net_buf_get(&tx_queue, Z_FOREVER);
    pthread_mutex_unlock(&tx_lock);

    return buf;
}

static void push_tx_queue(struct net_buf *buf)
{
    pthread_mutex_lock(&tx_lock);
    net_buf_put(&tx_queue, buf);
    pthread_mutex_unlock(&tx_lock);
}

static struct net_buf *pop_rx_queue(void)
{
    pthread_mutex_lock(&rx_lock);
    struct net_buf *buf = net_buf_get(&rx_queue, Z_FOREVER);
    pthread_mutex_unlock(&rx_lock);

    return buf;
}

static void push_rx_queue(struct net_buf *buf)
{
    pthread_mutex_lock(&rx_lock);
    net_buf_put(&rx_queue, buf);
    pthread_mutex_unlock(&rx_lock);
}

pthread_t usb_tx_thread;
static int tx_process_loop(void *args)
{
    printk("tx_process_loop\n");
    struct net_buf *buf;
    int ret = 0;

    while (1)
    {
        if (!is_enable)
        {
            break;
        }

        if (!is_ready_work)
        {
            continue;
        }

        if (!k_fifo_is_empty(&tx_queue))
        {
            buf = pop_tx_queue();

            byte type = bt_buf_get_type(buf);

            if (type == BT_BUF_CMD)
            {
                ret = usb_control_msg(usb_dev, 0x20, 0, /* set/get test */
                                      0,                /* test type    */
                                      0,                /* interface id */
                                      (char *)buf->data, buf->len, 1000);
            }
            else if (type == BT_BUF_ACL_OUT)
            {
                ret = usb_interrupt_write(usb_dev, END_POINT_ACL_W, (char *)buf->data, buf->len,
                                          1000);
            }

            if (ret < 0)
            {
                printk("error tx:\n%s\n", usb_strerror());
            }
            else
            {
                // printk("success: tx %d bytes\n", ret);
            }

            net_buf_unref(buf);
        }
    }
    return 0;
}

pthread_t usb_rx_evt_thread;
static int rx_evt_process_loop(void *args)
{
    printk("rx_evt_process_loop\n");
    uint8_t tmp[1024];
    int ret;
    while (1)
    {
        if (!is_enable)
        {
            break;
        }

        int reserve_size = bt_buf_reserve_size_controller_tx_evt();
        if (reserve_size == 0
#if defined(CONFIG_BT_MONITOR_SLEEP)
            && !bt_check_is_in_sleep()
#endif
        )
        {
            printk("rx_evt_process_loop(), reserve buff not enough.\n");
            Sleep(10);
            continue;
        }
        ret = usb_interrupt_read(usb_dev, END_POINT_EVT_INTR, (char *)tmp, sizeof(tmp), 1000);
        if (ret < 0)
        {
            is_ready_work = true;
            // printk("read data failed, %s", usb_strerror());
            if (is_in_reset)
            {
                printk("error reading:\n%s\n", usb_strerror());
                Sleep(100);
            }
        }
        else
        {
            if (!is_ready_work)
            {
                printk("warning, rx last packet.\n");
                continue;
            }
            // printk("success: bulk read %d bytes\n", ret);
            // printk("data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", tmp[0], tmp[1], tmp[2],
            // tmp[3], tmp[4], tmp[5]);
#if defined(CONFIG_BT_MONITOR_SLEEP)
            bt_sleep_wakeup_work_start();
#endif
            if (bt_check_rx_evt_need_drop(tmp))
            {
                printk("rx_evt_process_loop(), no reserve buff, drop adv.\n");
            }
            else
            {
                struct net_buf *buf;
                buf = bt_buf_get_controller_tx_evt();

                if (buf)
                {
                    net_buf_add_mem(buf, tmp, ret);
                    push_rx_queue(buf);
                }
                else
                {
                    while (1)
                    {
                        printk("rx_evt_process_loop(), no reserve buff\n");
                        Sleep(1000);
                    }
                }
            }
#if defined(CONFIG_BT_MONITOR_SLEEP)
            bt_sleep_wakeup_work_end();
#endif
        }
    }
    printk("rx_evt_process_loop end\n");

    return 0;
}

pthread_t usb_rx_acl_thread;
static int rx_acl_process_loop(void *args)
{
    printk("rx_acl_process_loop\n");

    uint8_t tmp[1024];
    int ret;
    while (1)
    {
        if (!is_enable)
        {
            break;
        }

        int reserve_size = bt_buf_reserve_size_controller_tx_acl();
        if (reserve_size == 0
#if defined(CONFIG_BT_MONITOR_SLEEP)
            && !bt_check_is_in_sleep()
#endif
        )
        {
            printk("rx_acl_process_loop(), reserve buff not enough.\n");
            Sleep(10);
            continue;
        }
        ret = usb_interrupt_read(usb_dev, END_POINT_ACL_R, (char *)tmp, sizeof(tmp), 1000);
        if (ret < 0)
        {
            if (is_in_reset)
            {
                printk("acl error reading:\n%s\n", usb_strerror());
                Sleep(100);
            }
        }
        else
        {
#if defined(CONFIG_BT_MONITOR_SLEEP)
            bt_sleep_wakeup_work_start();
#endif
            // printk("acl success: bulk read %d bytes\n", ret);
            // printk("acl data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", tmp[0], tmp[1],
            // tmp[2], tmp[3], tmp[4], tmp[5]);

            struct net_buf *buf;
            buf = bt_buf_get_controller_tx_acl();

            if (buf)
            {
                net_buf_add_mem(buf, tmp, ret);
                push_rx_queue(buf);
            }
            else
            {
                while (1)
                {
                    printk("rx_acl_process_loop(), no reserve buff\n");
                    Sleep(1000);
                }
            }
#if defined(CONFIG_BT_MONITOR_SLEEP)
            bt_sleep_wakeup_work_end();
#endif
        }
    }
    printk("rx_acl_process_loop end\n");

    return 0;
}

static int hci_driver_open(void)
{
    printk("hci_driver_open()\n");

    return 0;
}

static int hci_driver_send(struct net_buf *buf)
{
    push_tx_queue(buf);

    return 0;
}

static const struct bt_hci_driver drv = {
        .open = hci_driver_open,
        .send = hci_driver_send,
};

static void hci_driver_init(void)
{
    bt_hci_driver_register(&drv);
}

int usb_open_process(void)
{
    usb_init();         /* initialize the library */
    usb_find_busses();  /* find all busses */
    usb_find_devices(); /* find all connected devices */

    display_devices();
    usb_dev = open_dev();

    if (usb_set_configuration(usb_dev, MY_CONFIG) < 0)
    {
        printk("error setting config #%d: %s\n", MY_CONFIG, usb_strerror());
        usb_close(usb_dev);
        return -1;
    }
    else
    {
        printk("success: set configuration #%d\n", MY_CONFIG);
    }

    if (usb_claim_interface(usb_dev, 0) < 0)
    {
        printk("error claiming interface #%d:\n%s\n", MY_INTF, usb_strerror());
        usb_close(usb_dev);
        return -1;
    }
    else
    {
        printk("success: claim_interface #%d\n", MY_INTF);
    }

    is_enable = true;
    is_ready_work = false;
    is_in_reset = false;

    pthread_mutex_init(&tx_lock, NULL);
    pthread_mutex_init(&rx_lock, NULL);

    pthread_create(&usb_tx_thread, NULL, (void *)tx_process_loop, NULL);
    // pthread_join(usb_tx_thread, NULL);

    pthread_create(&usb_rx_evt_thread, NULL, (void *)rx_evt_process_loop, NULL);
    // pthread_join(usb_rx_evt_thread, NULL);

    pthread_create(&usb_rx_acl_thread, NULL, (void *)rx_acl_process_loop, NULL);
    // pthread_join(usb_rx_acl_thread, NULL);

    k_fifo_init(&tx_queue);
    k_fifo_init(&rx_queue);

    return 0;
}

int usb_open_device(void)
{
    int ret = usb_open_process();
    if (ret < 0)
    {
        return ret;
    }

    hci_driver_init();

    return (0);
}

reset_callback_t local_callback;

static int reset_driver_process(void *args)
{
    printk("reset_driver_process, wait usb reboot.\n");
    Sleep(5000);
    printk("reset_driver_process, usb reboot ready.\n");

    is_enable = false;
    is_in_reset = true;

    // wait thread close.
    pthread_join(usb_tx_thread, NULL);
    pthread_join(usb_rx_evt_thread, NULL);
    pthread_join(usb_rx_acl_thread, NULL);

    int ret = usb_open_process();
    if (ret < 0)
    {
        return ret;
    }

    local_callback();

    return 0;
}

void reset_usb_driver(reset_callback_t callback)
{
    local_callback = callback;

    pthread_t reset_thread;
    pthread_create(&reset_thread, NULL, (void *)reset_driver_process, NULL);
}

void libusb_main_loop(void)
{
    if (k_fifo_is_empty(&rx_queue))
    {
        return;
    }
    struct net_buf *buf = pop_rx_queue();
    if (buf)
    {
        bt_recv(buf);
    }
}
