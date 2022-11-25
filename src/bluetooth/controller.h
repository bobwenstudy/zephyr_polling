/** @file
 *  @brief Bluetooth subsystem controller APIs.
 */

/*
 * Copyright (c) 2018 Codecoup
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _ZEPHYR_POLLING_BLUETOOTH_CONTROLLER_H_
#define _ZEPHYR_POLLING_BLUETOOTH_CONTROLLER_H_

/**
 * @brief Bluetooth Controller
 * @defgroup bt_ctrl Bluetooth Controller
 * @ingroup bluetooth
 * @{
 */
#include "bt_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Set public address for controller
 *
 *  Should be called before bt_enable().
 *
 *  @param addr Public address
 */
void bt_ctlr_set_public_addr(const uint8_t *addr);

#ifdef __cplusplus
}
#endif
/**
 * @}
 */

#endif /* _ZEPHYR_POLLING_BLUETOOTH_CONTROLLER_H_ */