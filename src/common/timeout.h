/*
 * Copyright (c) 2010-2014, Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Macros to abstract toolchain specific capabilities
 *
 * This file contains various macros to abstract compiler capabilities that
 * utilize toolchain specific attributes and/or pragmas.
 */

#ifndef _ZEPHYR_POLLING_UTILS_TIMEOUT_H_
#define _ZEPHYR_POLLING_UTILS_TIMEOUT_H_

#include "bt_config.h"

#include "base/sys_clock.h"
#include "utils/dlist.h"

/**
 * @}
 */

/**
 * @addtogroup clock_apis
 * @{
 */

/**
 * @brief Generate null timeout delay.
 *
 * This macro generates a timeout delay that instructs a kernel API
 * not to wait if the requested operation cannot be performed immediately.
 *
 * @return Timeout delay value.
 */
#define K_NO_WAIT Z_TIMEOUT_NO_WAIT

/**
 * @brief Generate timeout delay from system ticks.
 *
 * This macro generates a timeout delay that instructs a kernel API
 * to wait up to @a t ticks to perform the requested operation.
 *
 * @param t Duration in system ticks.
 *
 * @return Timeout delay value.
 */
#define K_TICKS(t) Z_TIMEOUT_TICKS(t)

/**
 * @brief Generate timeout delay from milliseconds.
 *
 * This macro generates a timeout delay that instructs a kernel API
 * to wait up to @a ms milliseconds to perform the requested operation.
 *
 * @param ms Duration in milliseconds.
 *
 * @return Timeout delay value.
 */
#define K_MSEC(ms) Z_TIMEOUT_MS(ms)

/**
 * @brief Generate timeout delay from seconds.
 *
 * This macro generates a timeout delay that instructs a kernel API
 * to wait up to @a s seconds to perform the requested operation.
 *
 * @param s Duration in seconds.
 *
 * @return Timeout delay value.
 */
#define K_SECONDS(s) K_MSEC((s)*MSEC_PER_SEC)

/**
 * @brief Generate timeout delay from minutes.

 * This macro generates a timeout delay that instructs a kernel API
 * to wait up to @a m minutes to perform the requested operation.
 *
 * @param m Duration in minutes.
 *
 * @return Timeout delay value.
 */
#define K_MINUTES(m) K_SECONDS((m)*60)

/**
 * @brief Generate timeout delay from hours.
 *
 * This macro generates a timeout delay that instructs a kernel API
 * to wait up to @a h hours to perform the requested operation.
 *
 * @param h Duration in hours.
 *
 * @return Timeout delay value.
 */
#define K_HOURS(h) K_MINUTES((h)*60)

/**
 * @brief Generate infinite timeout delay.
 *
 * This macro generates a timeout delay that instructs a kernel API
 * to wait as long as necessary to perform the requested operation.
 *
 * @return Timeout delay value.
 */
#define K_FOREVER Z_FOREVER

#ifdef CONFIG_TIMEOUT_64BIT

/**
 * @brief Generates an absolute/uptime timeout value from system ticks
 *
 * This macro generates a timeout delay that represents an expiration
 * at the absolute uptime value specified, in system ticks.  That is, the
 * timeout will expire immediately after the system uptime reaches the
 * specified tick count.
 *
 * @param t Tick uptime value
 * @return Timeout delay value
 */
#define K_TIMEOUT_ABS_TICKS(t) Z_TIMEOUT_TICKS(Z_TICK_ABS((k_ticks_t)MAX(t, 0)))

/**
 * @brief Generates an absolute/uptime timeout value from milliseconds
 *
 * This macro generates a timeout delay that represents an expiration
 * at the absolute uptime value specified, in milliseconds.  That is,
 * the timeout will expire immediately after the system uptime reaches
 * the specified tick count.
 *
 * @param t Millisecond uptime value
 * @return Timeout delay value
 */
#define K_TIMEOUT_ABS_MS(t) K_TIMEOUT_ABS_TICKS(k_ms_to_ticks_ceil64(t))

/**
 * @brief Generates an absolute/uptime timeout value from microseconds
 *
 * This macro generates a timeout delay that represents an expiration
 * at the absolute uptime value specified, in microseconds.  That is,
 * the timeout will expire immediately after the system uptime reaches
 * the specified time.  Note that timer precision is limited by the
 * system tick rate and not the requested timeout value.
 *
 * @param t Microsecond uptime value
 * @return Timeout delay value
 */
#define K_TIMEOUT_ABS_US(t) K_TIMEOUT_ABS_TICKS(k_us_to_ticks_ceil64(t))

/**
 * @brief Generates an absolute/uptime timeout value from nanoseconds
 *
 * This macro generates a timeout delay that represents an expiration
 * at the absolute uptime value specified, in nanoseconds.  That is,
 * the timeout will expire immediately after the system uptime reaches
 * the specified time.  Note that timer precision is limited by the
 * system tick rate and not the requested timeout value.
 *
 * @param t Nanosecond uptime value
 * @return Timeout delay value
 */
#define K_TIMEOUT_ABS_NS(t) K_TIMEOUT_ABS_TICKS(k_ns_to_ticks_ceil64(t))

/**
 * @brief Generates an absolute/uptime timeout value from system cycles
 *
 * This macro generates a timeout delay that represents an expiration
 * at the absolute uptime value specified, in cycles.  That is, the
 * timeout will expire immediately after the system uptime reaches the
 * specified time.  Note that timer precision is limited by the system
 * tick rate and not the requested timeout value.
 *
 * @param t Cycle uptime value
 * @return Timeout delay value
 */
#define K_TIMEOUT_ABS_CYC(t) K_TIMEOUT_ABS_TICKS(k_cyc_to_ticks_ceil64(t))

#endif

struct _timeout;
typedef void (*_timeout_func_t)(struct _timeout *t);

struct _timeout
{
    sys_dnode_t node;
    _timeout_func_t fn;
#ifdef CONFIG_TIMEOUT_64BIT
    /* Can't use k_ticks_t for header dependency reasons */
    int64_t dticks;
#else
    int32_t dticks;
#endif
};

static inline void z_init_timeout(struct _timeout *to)
{
    sys_dnode_init(&to->node);
}

void z_add_timeout(struct _timeout *to, _timeout_func_t fn, k_timeout_t timeout);

int z_abort_timeout(struct _timeout *to);

static inline bool z_is_inactive_timeout(const struct _timeout *to)
{
    return !sys_dnode_is_linked(&to->node);
}

// static inline void z_init_thread_timeout(struct _thread_base *thread_base)
// {
// 	z_init_timeout(&thread_base->timeout);
// }

// extern void z_thread_timeout(struct _timeout *timeout);

// static inline void z_add_thread_timeout(struct k_thread *thread, k_timeout_t ticks)
// {
// 	z_add_timeout(&thread->base.timeout, z_thread_timeout, ticks);
// }

// static inline int z_abort_thread_timeout(struct k_thread *thread)
// {
// 	return z_abort_timeout(&thread->base.timeout);
// }

int32_t z_get_next_timeout_expiry(void);

void z_set_timeout_expiry(int32_t ticks, bool is_idle);

k_ticks_t z_timeout_remaining(const struct _timeout *timeout);
k_ticks_t z_timeout_expires(const struct _timeout *timeout);

k_ticks_t z_get_recent_timeout_expiry(void);

/**
 * @brief Announce time progress to the kernel
 *
 * Informs the kernel that the specified number of ticks have elapsed
 * since the last call to sys_clock_announce() (or system startup for
 * the first call).  The timer driver is expected to delivery these
 * announcements as close as practical (subject to hardware and
 * latency limitations) to tick boundaries.
 *
 * @param ticks Elapsed time, in ticks
 */
void sys_clock_announce(uint32_t ticks);

void timeout_polling_work(void);

#endif /* _ZEPHYR_POLLING_UTILS_TIMEOUT_H_ */