#include "chipset_csr8910_uart.h"
#include "drivers/hci_h4.h"
#include "windows_driver_serial.h"
#include "windows_log_impl.h"
#include "windows_timer_impl.h"

#include <stdarg.h>
#include <stdio.h>

#include <windows.h>

extern void bt_ready(int err);
extern void app_polling_work(void);
int main(void)
{
    int err = 0;
    serial_open_device(9, 921600, 8, 1, 0, 1);
    bt_hci_chipset_driver_register(chipset_csr8910_uart_instance());
    log_impl_register(log_impl_windows_instance());
    timer_impl_windows_init();
    irq_lock_impl_windows_init();

    /* Initialize the Bluetooth Subsystem */
    err = bt_enable(bt_ready);

    printf("Hello World!\n");

    while (1)
    {
        bt_polling_work();
        bt_hci_h4_polling();

        app_polling_work();
    }

    return (0);
}