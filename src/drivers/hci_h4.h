#ifndef _ZEPHYR_POLLING_DRIVERS_HCI_H4_H_
#define _ZEPHYR_POLLING_DRIVERS_HCI_H4_H_

#include "bt_config.h"

#include "base/types.h"

struct bt_hci_h4_driver
{
    /**
     * @brief Open the HCI transport.
     *
     * Opens the HCI transport for operation. This function must not
     * return until the transport is ready for operation, meaning it
     * is safe to start calling the send() handler.
     *
     * If the driver uses its own RX thread, i.e.
     * CONFIG_BT_RECV_IS_RX_THREAD is set, then this
     * function is expected to start that thread.
     *
     * @return 0 on success or negative error number on failure.
     */
    int (*open)(void);

    int (*send)(uint8_t *buf, uint16_t len);

    int (*recv)(uint8_t *buf, uint16_t len);
};

void bt_hci_h4_polling(void);
int hci_h4_init(const struct bt_hci_h4_driver *h4);

#endif /* _ZEPHYR_POLLING_DRIVERS_HCI_H4_H_ */