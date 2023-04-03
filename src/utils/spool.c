/*
 * Copyright (c) 2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "spool.h"
#include "common/net_buf.h"

#include <logging/bt_log_impl.h>

uint8_t spool_is_empty(struct spool *pool)
{
    return pool->rptr == pool->wptr;
}

uint8_t spool_init(struct spool *pool, uint8_t num, void **buf, uint8_t *storage_buf,
                   uint16_t data_size, uint8_t user_data_size)
{
    pool->buf = buf;
    pool->num = num + 1;
    pool->wptr = num;
    pool->rptr = 0;
    pool->data_size = data_size;
    pool->user_data_size = user_data_size;
    pool->__buf = storage_buf;

    for (int i = 0; i < num; i++)
    {
        buf[i] = (uint8_t *)(storage_buf +
                             ((data_size + sizeof(struct net_buf) + user_data_size) + 3) / 4 * 4 *
                                     i);
    }

    return 0;
}


uint8_t spool_get_id(struct spool *pool, struct net_buf *buf)
{
    for (int i = 0; i < pool->num; i++)
    {
        if(buf == (uint8_t *)(pool->__buf +
                             ((pool->data_size + sizeof(struct net_buf) + pool->user_data_size) + 3) / 4 * 4 *
                                     i))
                                     {
                                        return i;
                                     }
    }

    return 0;
}

uint8_t spool_size(struct spool *pool)
{
    uint8_t num = pool->num;
    uint8_t rptr = pool->rptr;
    uint8_t wptr = pool->wptr;
    return wptr >= rptr ? wptr - rptr : num - (rptr - wptr);
}

uint8_t spool_enqueue(struct spool *pool, void *val)
{
    uint8_t num = pool->num;
    uint8_t wptr = pool->wptr;
    uint8_t rptr = pool->rptr;

    uint8_t nwptr = (wptr == num - 1) ? 0 : wptr + 1;
    if (nwptr == rptr)
        return false; // full

    // printk("pool: %p, wptr: %d, nwptr: %d, rptr: %d, val: %x\n", pool, wptr, nwptr, rptr,
    // val);
    pool->buf[wptr] = val;
    pool->wptr = nwptr;
    return true;
}

void *spool_dequeue(struct spool *pool)
{
    uint8_t num = pool->num;
    uint8_t wptr = pool->wptr;
    uint8_t rptr = pool->rptr;
    // printk("pool: %p, wptr: %d, rptr: %d\n", pool, wptr, rptr);
    void *val;
    if (wptr == rptr)
        return NULL; // full
    val = pool->buf[rptr];
    rptr = (rptr == num - 1) ? 0 : rptr + 1;
    pool->rptr = rptr;
    // printk("pool: %p, wptr: %d, rptr: %d, val: %x\n", pool, wptr, rptr, val);
    return val;
}

void *spool_dequeue_peek(struct spool *pool)
{
    // uint8_t num = pool->num;
    uint8_t wptr = pool->wptr;
    uint8_t rptr = pool->rptr;
    void *val;
    if (wptr == rptr)
        return NULL; // full
    val = pool->buf[rptr];
    return val;
}

uint8_t spool_check_full(struct spool *pool)
{
    return spool_size(pool) == pool->num - 1;
}
