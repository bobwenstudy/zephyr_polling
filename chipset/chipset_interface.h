
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
} usb_interface_t;

typedef struct
{
    int rate;
    int databits;
    int stopbits;
    int parity;
    bool flowcontrol;
} uart_interface_t;

const usb_interface_t *chipset_get_usb_interface(uint8_t* size);
const uart_interface_t *chipset_get_uart_interface(void);

const struct bt_hci_chipset_driver *chipset_local_instance(void);

#ifdef __cplusplus
}
#endif

#endif //_CHIPSET_INTERFACE_H_
