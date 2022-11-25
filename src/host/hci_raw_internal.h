/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_POLLING_HOST_HCI_RAW_INTERNAL_H_
#define _ZEPHYR_POLLING_HOST_HCI_RAW_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

struct bt_dev_raw
{
    /* Registered HCI driver */
    const struct bt_hci_driver *drv;
};

extern struct bt_dev_raw bt_dev;

#ifdef __cplusplus
}
#endif

#endif /* _ZEPHYR_POLLING_HOST_HCI_RAW_INTERNAL_H_ */