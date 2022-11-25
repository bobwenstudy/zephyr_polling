/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_POLLING_SERVICES_HRS_H_
#define _ZEPHYR_POLLING_SERVICES_HRS_H_

/**
 * @brief Heart Rate Service (HRS)
 * @defgroup bt_hrs Heart Rate Service (HRS)
 * @ingroup bluetooth
 * @{
 *
 * [Experimental] Users should note that the APIs can change
 * as a part of ongoing development.
 */

#include "bt_config.h"

#include "base/types.h"

#include "bluetooth/gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Notify heart rate measurement.
 *
 * This will send a GATT notification to all current subscribers.
 *
 *  @param heartrate The heartrate measurement in beats per minute.
 *
 *  @return Zero in case of success and error code in case of error.
 */
int bt_hrs_notify(uint16_t heartrate);

int bt_hrs_set_blsc(uint8_t blsc);

extern struct bt_gatt_service_static hrs_svc;

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* _ZEPHYR_POLLING_SERVICES_HRS_H_ */