/* hci_ecc.h - HCI ECC emulation */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_POLLING_HOST_HCI_ECC_H_
#define _ZEPHYR_POLLING_HOST_HCI_ECC_H_

void bt_hci_ecc_init(void);
int bt_hci_ecc_send(struct net_buf *buf);
void bt_hci_ecc_supported_commands(uint8_t *supported_commands);

#endif /* _ZEPHYR_POLLING_HOST_HCI_ECC_H_ */