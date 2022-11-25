#ifndef _WINDOWS_DRIVER_LIBUSB_H_
#define _WINDOWS_DRIVER_LIBUSB_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*reset_callback_t)(void);

int usb_open_device(void);
void reset_usb_driver(reset_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif //_WINDOWS_DRIVER_LIBUSB_H_