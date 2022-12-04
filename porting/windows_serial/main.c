#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <windows.h>

#include "windows_driver_serial.h"

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

int open_hci_driver(int argc, const char * argv[])
{
    bt_uart_interface_t * p_interface = NULL;
    uint8_t com_num;
    // accept config from command line
    if (argc > 1)
    {
        // COM number
        com_num = strtol(argv[1], NULL, 0);

        bt_uart_interface_t tmp = {0, 0, 0, 0, 0};
        
        if(argc == 2)
        {

        }
        else if (argc == 7)
        {
            tmp.rate = strtol(argv[2], NULL, 0);
            tmp.databits = strtol(argv[3], NULL, 0);
            tmp.stopbits = strtol(argv[4], NULL, 0);
            tmp.parity = strtol(argv[5], NULL, 0);
            tmp.flowcontrol = strtol(argv[6], NULL, 0);

            p_interface = &tmp;
        }
        else
        {
            printk("Error, input params length error.");
            return -1;
        }
    }
    else
    {
        printk("Error, must input COM number.");
        return -1;
    }

    // Get Input config.
    if(p_interface == NULL)
    {
        p_interface = (bt_uart_interface_t *)bt_chipset_get_uart_interface();
    }
    
    if(p_interface == NULL)
    {
        printk("Error, VID/PID not set.");
        return -1;
    }

    if (serial_open_device(com_num
        , p_interface->rate, p_interface->databits, p_interface->stopbits, p_interface->parity, p_interface->flowcontrol) < 0)
    {
        printk("Error, uart open failed.");
        return -1;
    }

    return 0;
}

int main(int argc, const char * argv[])
{
    int err = 0;

    bt_log_impl_register(bt_log_impl_local_instance());

    if(open_hci_driver(argc, argv) < 0)
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

        extern void bt_hci_h4_polling(void);
        bt_hci_h4_polling();
    }

    return (err);
}