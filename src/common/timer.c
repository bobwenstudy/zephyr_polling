/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "base/types.h"
#include "base/util.h"

#include "timer.h"
#include "utils/slist.h"

#define LOG_MODULE_NAME timer
#include "logging/bt_log.h"

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
void z_timer_expiration_handler(struct _timeout *t)
{
    struct k_timer *timer = CONTAINER_OF(t, struct k_timer, timeout);
    // check if a period timer.
    if (timer->period.ticks != 0)
    {
        k_timeout_t next = timer->period;

        z_add_timeout(&timer->timeout, z_timer_expiration_handler, next);
    }
    /* update timer's status */
    timer->status += 1U;

    // LOG_DBG("timer: %p, expiry_fn: %p, status: %d\n", timer,
    // timer->expiry_fn, timer->status);
    /* invoke timer expiry function */
    if (timer->expiry_fn != NULL)
    {
        timer->expiry_fn(timer);
    }
}

void k_timer_init(struct k_timer *timer, k_timer_expiry_t expiry_fn, k_timer_stop_t stop_fn)
{
    LOG_DBG("timer: %p, expiry_fn: %p\n", timer, expiry_fn);

    timer->expiry_fn = expiry_fn;
    timer->stop_fn = stop_fn;
    timer->status = 0U;

    z_init_timeout(&timer->timeout);

    timer->user_data = NULL;
}

void k_timer_start(struct k_timer *timer, k_timeout_t duration, k_timeout_t period)
{
    (void)z_abort_timeout(&timer->timeout);
    timer->period = period;
    timer->status = 0U;

    z_add_timeout(&timer->timeout, z_timer_expiration_handler, duration);
}

void k_timer_stop(struct k_timer *timer)
{
    LOG_DBG("timer: %p\n", timer);

    bool inactive = (z_abort_timeout(&timer->timeout) != 0);

    if (inactive)
    {
        return;
    }

    if (timer->stop_fn != NULL)
    {
        timer->stop_fn(timer);
    }
}

uint32_t k_timer_status_get(struct k_timer *timer)
{
    uint32_t result = timer->status;

    timer->status = 0U;

    return result;
}

k_ticks_t k_timer_remaining_ticks(struct k_timer *timer)
{
    return z_timeout_remaining(&timer->timeout);
}
