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

#ifndef _ZEPHYR_POLLING_UTILS_TIMER_H_
#define _ZEPHYR_POLLING_UTILS_TIMER_H_

#include "bt_config.h"

#include "base/types.h"
#include "utils/slist.h"
#include "common\timeout.h"

/**
 *@brief Timer_state.
 */
typedef enum
{
    TIMER_STOP,
    TIMER_START,
} TIMER_STATUS;

/**
 * @}
 */

/**
 * @cond INTERNAL_HIDDEN
 */

struct k_timer
{
    /*
     * _timeout structure must be first here if we want to use
     * dynamic timer allocation. timeout.node is used in the double-linked
     * list of free timers
     */
    struct _timeout timeout;

    /* runs in ISR context */
    void (*expiry_fn)(struct k_timer *timer);

    /* runs in the context of the thread that calls k_timer_stop() */
    void (*stop_fn)(struct k_timer *timer);

    /* timer period */
    k_timeout_t period;

    /* timer status */
    uint8_t status;
    /* timer expire count */
    uint16_t expire_cnt;

    /* user-specific data, also used to support legacy features */
    void *user_data;
};

#define Z_TIMER_INITIALIZER(obj, expiry, stop)                                                     \
    {                                                                                              \
        .expiry_fn = expiry, .status = 0, .user_data = 0,                                          \
    }

/**
 * INTERNAL_HIDDEN @endcond
 */

/**
 * @defgroup timer_apis Timer APIs
 * @ingroup kernel_apis
 * @{
 */

/**
 * @typedef k_timer_expiry_t
 * @brief Timer expiry function type.
 *
 * A timer's expiry function is executed by the system clock interrupt handler
 * each time the timer expires. The expiry function is optional, and is only
 * invoked if the timer has been initialized with one.
 *
 * @param timer     Address of timer.
 */
typedef void (*k_timer_expiry_t)(struct k_timer *timer);

/**
 * @typedef k_timer_stop_t
 * @brief Timer stop function type.
 *
 * A timer's stop function is executed if the timer is stopped prematurely.
 * The function runs in the context of call that stops the timer.  As
 * k_timer_stop() can be invoked from an ISR, the stop function must be
 * callable from interrupt context (isr-ok).
 *
 * The stop function is optional, and is only invoked if the timer has been
 * initialized with one.
 *
 * @param timer     Address of timer.
 */
typedef void (*k_timer_stop_t)(struct k_timer *timer);

/**
 * @brief Statically define and initialize a timer.
 *
 * The timer can be accessed outside the module where it is defined using:
 *
 * @code extern struct k_timer <name>; @endcode
 *
 * @param name Name of the timer variable.
 * @param expiry_fn Function to invoke each time the timer expires.
 * @param stop_fn   Function to invoke if the timer is stopped while running.
 */
#define K_TIMER_DEFINE(name, expiry_fn, stop_fn)                                                   \
    STRUCT_SECTION_ITERABLE(k_timer, name) = Z_TIMER_INITIALIZER(name, expiry_fn, stop_fn)

/**
 * @brief Initialize a timer.
 *
 * This routine initializes a timer, prior to its first use.
 *
 * @param timer     Address of timer.
 * @param expiry_fn Function to invoke each time the timer expires.
 * @param stop_fn   Function to invoke if the timer is stopped while running.
 */
void k_timer_init(struct k_timer *timer, k_timer_expiry_t expiry_fn, k_timer_stop_t stop_fn);

/**
 * @brief Start a timer.
 *
 * This routine starts a timer, and resets its status to zero. The timer
 * begins counting down using the specified duration and period values.
 *
 * Attempting to start a timer that is already running is permitted.
 * The timer's status is reset to zero and the timer begins counting down
 * using the new duration and period values.
 *
 * @param timer     Address of timer.
 * @param duration  Initial timer duration.
 * @param period    Timer period.
 */
void k_timer_start(struct k_timer *timer, k_timeout_t duration, k_timeout_t period);

/**
 * @brief Stop a timer.
 *
 * This routine stops a running timer prematurely. The timer's stop function,
 * if one exists, is invoked by the caller.
 *
 * Attempting to stop a timer that is not running is permitted, but has no
 * effect on the timer.
 *
 * @note The stop handler has to be callable from ISRs if @a k_timer_stop is to
 * be called from ISRs.
 *
 * @funcprops \isr_ok
 *
 * @param timer     Address of timer.
 */
void k_timer_stop(struct k_timer *timer);

/**
 * @brief Read timer status.
 *
 * This routine reads the timer's status, which indicates the number of times
 * it has expired since its status was last read.
 *
 * Calling this routine resets the timer's status to zero.
 *
 * @param timer     Address of timer.
 *
 * @return Timer status.
 */
uint32_t k_timer_status_get(struct k_timer *timer);

/**
 * @brief Associate user-specific data with a timer.
 *
 * This routine records the @a user_data with the @a timer, to be retrieved
 * later.
 *
 * It can be used e.g. in a timer handler shared across multiple subsystems to
 * retrieve data specific to the subsystem this timer is associated with.
 *
 * @param timer     Address of timer.
 * @param user_data User data to associate with the timer.
 */
static inline void k_timer_user_data_set(struct k_timer *timer, void *user_data)
{
    timer->user_data = user_data;
}

/**
 * @brief Retrieve the user-specific data from a timer.
 *
 * @param timer     Address of timer.
 *
 * @return The user data.
 */
static inline void *k_timer_user_data_get(const struct k_timer *timer)
{
    return timer->user_data;
}

/**
 * @brief Get next expiration time of a timer, in system ticks
 *
 * This routine returns the future system uptime reached at the next
 * time of expiration of the timer, in units of system ticks.  If the
 * timer is not running, current system time is returned.
 *
 * @param timer The timer object
 * @return Uptime of expiration, in ticks
 */
static inline k_ticks_t k_timer_expires_ticks(const struct k_timer *timer)
{
    return z_timeout_expires(&timer->timeout);
}

/**
 * @brief Get time remaining before a timer next expires.
 *
 * This routine computes the (approximate) time remaining before a running
 * timer next expires. If the timer is not running, it returns zero.
 *
 * @param timer     Address of timer.
 *
 * @return Remaining time (in milliseconds).
 */
static inline uint32_t k_timer_remaining_get(struct k_timer *timer)
{
    return k_ticks_to_ms_floor32(z_timeout_remaining(&timer->timeout));
}

#endif /* _ZEPHYR_POLLING_UTILS_TIMER_H_ */