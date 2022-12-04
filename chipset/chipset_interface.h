
#ifndef _CHIPSET_INTERFACE_H_
#define _CHIPSET_INTERFACE_H_

#include "drivers/hci_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t vid;
    uint16_t pid;
} bt_usb_interface_t;

typedef struct
{
    int rate;
    int databits;
    int stopbits;
    int parity;
    bool flowcontrol;
} bt_uart_interface_t;

const bt_usb_interface_t *bt_chipset_get_usb_interface(void);
const bt_uart_interface_t *bt_chipset_get_uart_interface(void);

const struct bt_hci_chipset_driver *bt_hci_chipset_impl_local_instance(void);

#ifdef __cplusplus
}
#endif

#endif //_CHIPSET_INTERFACE_H_
