#include <errno.h>

#include "chipset_ats2851.h"

// public API
const struct bt_hci_chipset_driver *bt_hci_chipset_impl_local_instance(void)
{
    return NULL;
}

static const bt_usb_interface_t usb_interface[] = {
    {0x10D7, 0xB012},
};

const bt_usb_interface_t *bt_chipset_get_usb_interface(uint8_t* size)
{
    *size = sizeof(usb_interface)/sizeof(usb_interface[0]);
    return usb_interface;
}

const bt_uart_interface_t *bt_chipset_get_uart_interface(void)
{
    return NULL;
}