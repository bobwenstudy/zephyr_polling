/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdbool.h>
#include <errno.h>

#include "timeout.h"
#include "base/types.h"
#include "base/util.h"

#define BT_DBG_ENABLED  IS_ENABLED(CONFIG_BT_DEBUG_TIMEOUT)
#define LOG_MODULE_NAME timeout
#include "logging/bt_log.h"

static sys_dlist_t timeout_list = SYS_DLIST_STATIC_INIT(&timeout_list);

static uint32_t curr_tick; // unit is 1ms.

void sys_clock_announce(uint32_t ticks)
{
    curr_tick += ticks;
}

uint32_t sys_clock_tick_get(void)
{
    return curr_tick;
}

static struct _timeout *first(void)
{
    sys_dnode_t *t = sys_dlist_peek_head(&timeout_list);

    return t == NULL ? NULL : CONTAINER_OF(t, struct _timeout, node);
}

static struct _timeout *next(struct _timeout *t)
{
    sys_dnode_t *n = sys_dlist_peek_next(&timeout_list, &t->node);

    return n == NULL ? NULL : CONTAINER_OF(n, struct _timeout, node);
}

static void remove_timeout(struct _timeout *t)
{
    sys_dlist_remove(&t->node);
}

void timeout_polling_work(void)
{
    while (first() != NULL && first()->dticks <= sys_clock_tick_get())
    {
        struct _timeout *t = first();
        // BT_DBG("t: %p, fn: %p\n", t, t->fn);
        remove_timeout(t);

        t->fn(t);
    }
}

void dump_timeout_list(void)
{
    struct _timeout *t;
    sys_dnode_t *last = NULL;
    BT_DBG("head: %p, tail: %p\n", timeout_list.head, timeout_list.tail);
    for (t = first(); t != NULL; t = next(t))
    {
        last = &t->node;
        BT_DBG("next: %p\n", t->node.next);
        BT_DBG("t: %p, dticks: %d, tick: %d\n", t, t->dticks, sys_clock_tick_get());
    }

    if (last != sys_dlist_peek_tail(&timeout_list))
    {
        BT_DBG("error. last: %p, tail: %p\n", last->next, sys_dlist_peek_tail(&timeout_list));
    }
}

void z_add_timeout(struct _timeout *to, _timeout_func_t fn, k_timeout_t timeout)
{
    to->fn = fn;

    struct _timeout *t;

    to->dticks = sys_clock_tick_get() + timeout.ticks;

    for (t = first(); t != NULL; t = next(t))
    {
        if (t->dticks > to->dticks)
        {
            sys_dlist_insert(&t->node, &to->node);
            break;
        }
    }

    if (t == NULL)
    {
        sys_dlist_append(&timeout_list, &to->node);
    }
}

int z_abort_timeout(struct _timeout *to)
{
    int ret = -EINVAL;

    if (sys_dnode_is_linked(&to->node))
    {
        remove_timeout(to);
        ret = 0;
    }

    return ret;
}

/* must be locked */
static k_ticks_t timeout_rem(const struct _timeout *timeout)
{
    int diff = timeout->dticks - sys_clock_tick_get();
    return diff < 0 ? 0 : diff;
}

k_ticks_t z_timeout_remaining(const struct _timeout *timeout)
{
    return timeout_rem(timeout);
}

k_ticks_t z_timeout_expires(const struct _timeout *timeout)
{
    return timeout->dticks;
}

int32_t z_get_next_timeout_expiry(void)
{
    while (1)
    {
    }
}

void z_set_timeout_expiry(int32_t ticks, bool is_idle)
{
    while (1)
    {
    }
}

k_ticks_t z_get_recent_timeout_expiry(void)
{
    struct _timeout *timeout = first();
    if (timeout == NULL)
    {
        return 0;
    }
    return timeout->dticks;
}
