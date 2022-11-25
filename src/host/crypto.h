/*
 * Copyright (c) 2016-2017 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_POLLING_HOST_CRYPTO_H_
#define _ZEPHYR_POLLING_HOST_CRYPTO_H_

#include "bt_config.h"

int prng_init(void);
void bt_rand_init(unsigned int seed);

#endif /* _ZEPHYR_POLLING_HOST_CRYPTO_H_ */