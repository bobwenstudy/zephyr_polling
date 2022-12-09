
#ifndef _PLATFORM_INTERFACE_H_
#define _PLATFORM_INTERFACE_H_

#include "common/bt_storage_kv.h"
#include "logging/bt_log_impl.h"
#include "drivers/hci_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

const bt_log_impl_t *bt_log_impl_local_instance(void);
const struct bt_hci_chipset_driver *bt_hci_chipset_impl_local_instance(void);
const struct bt_storage_kv_impl *bt_storage_kv_impl_local_instance(void);
void bt_timer_impl_local_init(void);

typedef void (*reset_callback_t)(void);

int usb_open_device(uint16_t vid, uint16_t pid);
void reset_usb_driver(reset_callback_t callback);

int serial_open_device(int idx, int rate, int databits, int stopbits, int parity, bool flowcontrol);

#ifdef __cplusplus
}
#endif

#endif //_PLATFORM_INTERFACE_H_
