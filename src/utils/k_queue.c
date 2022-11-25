/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief dynamic-size QUEUE object.
 */

#include "k_queue.h"

#include "base/__assert.h"
#include "base/common.h"
#include "base/check.h"

void *z_queue_node_peek(sys_snode_t *node, bool needs_free)
{
    return node;
}

void k_queue_init(struct k_queue *queue)
{
    sys_slist_init(&queue->data_q);
}

static inline void handle_poll_events(struct k_queue *queue, uint32_t state)
{
}

void k_queue_cancel_wait(struct k_queue *queue)
{
}

static int32_t queue_insert(struct k_queue *queue, void *prev, void *data, bool alloc,
                            bool is_append)
{
    if (is_append)
    {
        prev = sys_slist_peek_tail(&queue->data_q);
    }

    if (alloc)
    {
        __ASSERT(alloc, "Not support.");
    }

    sys_slist_insert(&queue->data_q, prev, data);

    return 0;
}

void k_queue_insert(struct k_queue *queue, void *prev, void *data)
{
    (void)queue_insert(queue, prev, data, false, false);
}

void k_queue_append(struct k_queue *queue, void *data)
{
    (void)queue_insert(queue, NULL, data, false, true);
}

void k_queue_prepend(struct k_queue *queue, void *data)
{
    (void)queue_insert(queue, NULL, data, false, false);
}

int32_t k_queue_alloc_append(struct k_queue *queue, void *data)
{
    int32_t ret = queue_insert(queue, NULL, data, true, true);

    return ret;
}

int32_t k_queue_alloc_prepend(struct k_queue *queue, void *data)
{
    int32_t ret = queue_insert(queue, NULL, data, true, false);

    return ret;
}

int k_queue_append_list(struct k_queue *queue, void *head, void *tail)
{
    sys_slist_append_list(&queue->data_q, head, tail);

    return 0;
}

int k_queue_merge_slist(struct k_queue *queue, sys_slist_t *list)
{
    sys_slist_merge_slist(&queue->data_q, list);

    return 0;
}

void *k_queue_get(struct k_queue *queue, k_timeout_t timeout)
{
    return sys_slist_get(&queue->data_q);
}

bool k_queue_remove(struct k_queue *queue, void *data)
{
    bool ret = sys_slist_find_and_remove(&queue->data_q, (sys_snode_t *)data);

    return ret;
}

bool k_queue_unique_append(struct k_queue *queue, void *data)
{
    sys_snode_t *test;

    SYS_SLIST_FOR_EACH_NODE (&queue->data_q, test)
    {
        if (test == (sys_snode_t *)data)
        {
            return false;
        }
    }

    k_queue_append(queue, data);

    return true;
}

void *k_queue_peek_head(struct k_queue *queue)
{
    void *ret = z_queue_node_peek(sys_slist_peek_head(&queue->data_q), false);

    return ret;
}

void *k_queue_peek_tail(struct k_queue *queue)
{
    void *ret = z_queue_node_peek(sys_slist_peek_tail(&queue->data_q), false);

    return ret;
}
