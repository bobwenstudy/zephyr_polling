/** @file
 *  @brief Bluetooth subsystem logging helpers.
 */

/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_POLLING_LOGGING_BT_LOG_H_
#define _ZEPHYR_POLLING_LOGGING_BT_LOG_H_

#include "bt_config.h"
#include "bt_log_impl.h"

#include "base/__assert.h"
#include "base/types.h"
#include "base/printk.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_BT_DEBUG
#define LOG_LEVEL CONFIG_BT_LOG_LEVEL
#else
#define LOG_LEVEL LOG_IMPL_LEVEL_NONE
#endif

// #define LOG_LEVEL LOG_IMPL_LEVEL_DBG

#define LOG_DBG(fmt, ...)  LOG_IMPL_DBG(LOG_MODULE_NAME, LOG_LEVEL, fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)  LOG_IMPL_ERR(LOG_MODULE_NAME, LOG_LEVEL, fmt, ##__VA_ARGS__)
#define LOG_WRN(fmt, ...) LOG_IMPL_WRN(LOG_MODULE_NAME, LOG_LEVEL, fmt, ##__VA_ARGS__)
#define LOG_INF(fmt, ...) LOG_IMPL_INF(LOG_MODULE_NAME, LOG_LEVEL, fmt, ##__VA_ARGS__)

#define BT_PACKET_DUMP(_packet_type, _in, _packet, _len)                                           \
    LOG_PACKET_DUMP(_packet_type, _in, _packet, _len)

#if defined(CONFIG_BT_ASSERT_VERBOSE)
#define BT_ASSERT_PRINT(test)         __ASSERT_LOC(test)
#define BT_ASSERT_PRINT_MSG(fmt, ...) __ASSERT_MSG_INFO(fmt, ##__VA_ARGS__)
#else
#define BT_ASSERT_PRINT(test)
#define BT_ASSERT_PRINT_MSG(fmt, ...)
#endif /* CONFIG_BT_ASSERT_VERBOSE */

#if defined(CONFIG_BT_ASSERT_PANIC)
#define BT_ASSERT_DIE() while (1)
#else
#define BT_ASSERT_DIE() while (1)
#endif /* CONFIG_BT_ASSERT_PANIC */

#if defined(CONFIG_BT_ASSERT)
#define BT_ASSERT(cond)                                                                            \
    do                                                                                             \
    {                                                                                              \
        if (!(cond))                                                                               \
        {                                                                                          \
            BT_ASSERT_PRINT(cond);                                                                 \
            BT_ASSERT_DIE();                                                                       \
        }                                                                                          \
    } while (0)

#define BT_ASSERT_MSG(cond, fmt, ...)                                                              \
    do                                                                                             \
    {                                                                                              \
        if (!(cond))                                                                               \
        {                                                                                          \
            BT_ASSERT_PRINT(cond);                                                                 \
            BT_ASSERT_PRINT_MSG(fmt, ##__VA_ARGS__);                                               \
            BT_ASSERT_DIE();                                                                       \
        }                                                                                          \
    } while (0)
#else
#define BT_ASSERT(cond)               __ASSERT_NO_MSG(cond)
#define BT_ASSERT_MSG(cond, msg, ...) __ASSERT(cond, msg, ##__VA_ARGS__)
#endif /* CONFIG_BT_ASSERT*/

#define BT_HEXDUMP_DBG(_data, _length, _str) LOG_HEXDUMP_DBG((const uint8_t *)_data, _length, _str)

/* NOTE: These helper functions always encodes into the same buffer storage.
 * It is the responsibility of the user of this function to copy the information
 * in this string if needed.
 *
 * NOTE: These functions are not thread-safe!
 */
const char *bt_hex_real(const void *buf, size_t len);
const char *bt_addr_str_real(const bt_addr_t *addr);
const char *bt_addr_le_str_real(const bt_addr_le_t *addr);
const char *bt_uuid_str_real(const struct bt_uuid *uuid);

/* NOTE: log_strdup does not guarantee a duplication of the string.
 * It is therefore still the responsibility of the user to handle the
 * restrictions in the underlying function call.
 */
#define bt_hex(buf, len)     log_strdup(bt_hex_real(buf, len))
#define bt_addr_str(addr)    log_strdup(bt_addr_str_real(addr))
#define bt_addr_le_str(addr) log_strdup(bt_addr_le_str_real(addr))
#define bt_uuid_str(uuid)    log_strdup(bt_uuid_str_real(uuid))

// extern int snprintk(char *str, size_t size, const char *fmt, ...);
void log_hex_dump(char *str, size_t out_len, const void *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _ZEPHYR_POLLING_LOGGING_BT_LOG_H_ */