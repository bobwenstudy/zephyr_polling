
#ifndef _CHIPSET_CSR8910_UART_H_
#define _CHIPSET_CSR8910_UART_H_

#include "drivers/hci_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

const struct bt_hci_chipset_driver *chipset_csr8910_uart_instance(void);

#ifdef __cplusplus
}
#endif

#endif //_CHIPSET_CSR8910_UART_H_
