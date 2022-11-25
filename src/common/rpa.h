/* rpa.h - Bluetooth Resolvable Private Addresses (RPA) generation and
 * resolution
 */

/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_POLLING_COMMON_RPA_H_
#define _ZEPHYR_POLLING_COMMON_RPA_H_

#include "bt_config.h"

#include <bluetooth/addr.h>

bool bt_rpa_irk_matches(const uint8_t irk[16], const bt_addr_t *addr);
int bt_rpa_create(const uint8_t irk[16], bt_addr_t *rpa);

#endif /* _ZEPHYR_POLLING_COMMON_RPA_H_ */