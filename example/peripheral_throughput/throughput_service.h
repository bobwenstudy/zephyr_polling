/** @file
 *  @brief throughput Service sample
 */

#ifndef _THROUGHPUT_SERVICE_H_
#define _THROUGHPUT_SERVICE_H_
/*
 * Copyright (c) 2019 Aaron Tsui <aaron.tsui@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef __cplusplus
extern "C" {
#endif

void throughput_svc_init(void);
void throughput_svc_send(void);

extern struct bt_gatt_service_static throughput_svc;

#ifdef __cplusplus
}
#endif

#endif