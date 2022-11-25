/*
 * Copyright (c) 2017-2021 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_POLLING_HOST_SCAN_H_
#define _ZEPHYR_POLLING_HOST_SCAN_H_

#include "bt_config.h"

#include "hci_core.h"

void bt_scan_reset(void);

bool bt_id_scan_random_addr_check(void);

int bt_le_scan_set_enable(uint8_t enable);

struct bt_le_per_adv_sync *bt_hci_get_per_adv_sync(uint16_t handle);

void bt_periodic_sync_disable(void);

#endif /* _ZEPHYR_POLLING_HOST_SCAN_H_ */