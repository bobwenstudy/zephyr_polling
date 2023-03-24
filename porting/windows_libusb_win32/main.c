#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <windows.h>

#include "windows_driver_libusb.h"

#include "chipset_interface.h"
#include "platform_interface.h"

#include "base/types.h"
#include "utils/spool.h"
#include <logging/bt_log_impl.h>
#include <drivers/hci_driver.h>
#include "host/hci_core.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

extern void bt_ready(int err);
extern void app_polling_work(void);

int open_hci_driver(int argc, const char *argv[])
{
    bt_usb_interface_t *p_interface = NULL;

    // accept config from command line
    if (argc > 1)
    {
        if (argc != 3)
        {
            printk("Error, input params length error.");
            return -1;
        }
        bt_usb_interface_t tmp = {0, 0};
        tmp.vid = strtol(argv[1], NULL, 0);
        tmp.pid = strtol(argv[2], NULL, 0);
        p_interface = &tmp;
    }

    // Get Input config.
    if (p_interface == NULL)
    {
        p_interface = (bt_usb_interface_t *)bt_chipset_get_usb_interface();
    }

    if (p_interface == NULL)
    {
        printk("Error, VID/PID not set.");
        return -1;
    }

    if (bt_hci_init_usb_device(p_interface->vid, p_interface->pid) < 0)
    {
        printk("Error, usb init failed.");
        return -1;
    }

    return 0;
}

int main(int argc, const char *argv[])
{
    int err = 0;

    bt_log_impl_register(bt_log_impl_local_instance());

    if (open_hci_driver(argc, argv) < 0)
    {
        return -1;
    }
    bt_hci_chipset_driver_register(bt_hci_chipset_impl_local_instance());
    bt_storage_kv_register(bt_storage_kv_impl_local_instance());
    bt_timer_impl_local_init();

    /* Initialize the Bluetooth Subsystem */
    err = bt_enable(bt_ready);

#if defined(CONFIG_BT_MONITOR_SLEEP)
    bt_init_monitor_sleep();
#endif

    while (1)
    {
#if defined(CONFIG_BT_MONITOR_SLEEP)
        if (!bt_check_is_in_sleep())
        {
            bt_polling_work();

            if (bt_is_ready() && bt_check_allow_sleep())
            {
                bt_sleep_prepare_work();
            }
        }
#else
        bt_polling_work();
#endif

        app_polling_work();

        extern void libusb_main_loop(void);
        libusb_main_loop();
    }

    return (err);
}