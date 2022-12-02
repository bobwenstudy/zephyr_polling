#include <errno.h>

#include "chipset_common.h"

// public API
const struct bt_hci_chipset_driver *bt_hci_chipset_impl_local_instance(void)
{
    return NULL;
}

// For test, you can set your customor setting here.
static const bt_usb_interface_t usb_interface[] = {
    {0, 0},
};

const bt_usb_interface_t *bt_chipset_get_usb_interface(uint8_t* size)
{
    *size = sizeof(usb_interface)/sizeof(usb_interface[0]);
    return usb_interface;
}

// For test, you can set your customor setting here.
static const bt_uart_interface_t uart_interface = {115200, 8, 1, 0, false};
const bt_uart_interface_t *bt_chipset_get_uart_interface(void)
{
    return &uart_interface;
}