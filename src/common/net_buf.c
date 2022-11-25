/* buf.c - Buffer management */

/*
 * Copyright (c) 2015-2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "base/byteorder.h"
#include "net_buf.h"

#define BT_DBG_ENABLED  IS_ENABLED(CONFIG_BT_DEBUG_NET_BUF)
#define LOG_MODULE_NAME net_buf
#include "logging/bt_log.h"

#if defined(CONFIG_BT_DEBUG)
#define NET_BUF_DBG(fmt, ...)  BT_DBG(fmt, ##__VA_ARGS__)
#define NET_BUF_ERR(fmt, ...)  BT_ERR(fmt, ##__VA_ARGS__)
#define NET_BUF_WARN(fmt, ...) BT_WARN(fmt, ##__VA_ARGS__)
#define NET_BUF_INFO(fmt, ...) BT_INFO(fmt, ##__VA_ARGS__)
#else
#define NET_BUF_DBG(fmt, ...)
#define NET_BUF_ERR(fmt, ...)
#define NET_BUF_WARN(fmt, ...)
#define NET_BUF_INFO(fmt, ...)
#endif

#define NET_BUF_ASSERT(cond, ...) BT_ASSERT(cond, ##__VA_ARGS__)

#if CONFIG_NET_BUF_WARN_ALLOC_INTERVAL > 0
#define WARN_ALLOC_INTERVAL (CONFIG_NET_BUF_WARN_ALLOC_INTERVAL)
#else
#define WARN_ALLOC_INTERVAL 0
#endif

void net_buf_reset(struct net_buf *buf)
{
    __ASSERT_NO_MSG(buf->flags == 0U);
    __ASSERT_NO_MSG(buf->frags == NULL);

    net_buf_simple_reset(&buf->b);
}

#if defined(CONFIG_BT_DEBUG)
struct net_buf *net_buf_alloc_debug(struct spool *pool, const char *func, int line)
#else
struct net_buf *net_buf_alloc(struct spool *pool, k_timeout_t timeout)
#endif
{
    struct net_buf *buf;

    NET_BUF_DBG("%s():%d: pool %p", func, line, pool);

    buf = (struct net_buf *)spool_dequeue(pool);
    NET_BUF_DBG("buf %p", buf);
    if (buf == NULL)
    {
        return NULL;
    }

    buf->pool_id = pool;
    buf->__buf = buf->user_data + 4; // default user size is point.

    buf->ref = 1U;
    buf->flags = 0U;
    buf->frags = NULL;
    buf->size = pool->data_size;
    net_buf_reset(buf);

    return buf;
}

struct net_buf *net_buf_get(struct k_fifo *fifo, k_timeout_t timeout)
{
    struct net_buf *buf, *frag;

    NET_BUF_DBG("test_head: %p, tail: %p", fifo->_queue.data_q.head, fifo->_queue.data_q.tail);
    buf = (struct net_buf *)k_fifo_get(fifo, timeout);
    if (!buf)
    {
        return NULL;
    }
    NET_BUF_DBG("test_head: %p, tail: %p, buf: %p, node: %p", fifo->_queue.data_q.head,
                fifo->_queue.data_q.tail, buf, buf->node.next);

    // NET_BUF_DBG("%s():%d: buf %p fifo %p", func, line, buf, fifo);

    /* Get any fragments belonging to this buffer */
    for (frag = buf; (frag->flags & NET_BUF_FRAGS); frag = frag->frags)
    {
        frag->frags = (struct net_buf *)k_fifo_get(fifo, K_NO_WAIT);
        __ASSERT_NO_MSG(frag->frags);

        /* The fragments flag is only for FIFO-internal usage */
        frag->flags &= ~NET_BUF_FRAGS;
    }

    /* Mark the end of the fragment list */
    frag->frags = NULL;

    return buf;
}

void net_buf_simple_init_with_data(struct net_buf_simple *buf, void *data, size_t size)
{
    buf->__buf = data;
    buf->data = data;
    buf->size = size;
    buf->len = size;
}
/*note :The buffer is not expected to contain any data when this API is called ,Ԥ��reserve size room
 */
void net_buf_simple_reserve(struct net_buf_simple *buf, size_t reserve)
{
    __ASSERT_NO_MSG(buf);
    __ASSERT_NO_MSG(buf->len == 0U);
    NET_BUF_DBG("buf_simple %p reserve %zu", buf, reserve);

    buf->data = buf->__buf + reserve;
}

void net_buf_slist_put(sys_slist_t *list, struct net_buf *buf)
{
    struct net_buf *tail;

    __ASSERT_NO_MSG(list);
    __ASSERT_NO_MSG(buf);

    for (tail = buf; tail->frags; tail = tail->frags)
    {
        tail->flags |= NET_BUF_FRAGS;
        NET_BUF_DBG("buf %p ref %u pool_id %p frags %p", buf, buf->ref, buf->pool_id, buf->frags);
    }
    NET_BUF_DBG("test_flag %d", buf->flags);

    sys_slist_append_list(list, &buf->node, &tail->node);
}

struct net_buf *net_buf_slist_get(sys_slist_t *list)
{
    struct net_buf *buf, *frag;

    __ASSERT_NO_MSG(list);

    buf = (void *)sys_slist_get(list);

    if (!buf)
    {
        return NULL;
    }
    NET_BUF_DBG("test_flag %d", buf->flags);

    /* Get any fragments belonging to this buffer */
    for (frag = buf; (frag->flags & NET_BUF_FRAGS); frag = frag->frags)
    {
        frag->frags = (void *)sys_slist_get(list);

        __ASSERT_NO_MSG(frag->frags);

        /* The fragments flag is only for list-internal usage */
        frag->flags &= ~NET_BUF_FRAGS;
    }

    /* Mark the end of the fragment list */
    frag->frags = NULL;

    return buf;
}

void net_buf_put(struct k_fifo *fifo, struct net_buf *buf)
{
    struct net_buf *tail;

    __ASSERT_NO_MSG(fifo);
    __ASSERT_NO_MSG(buf);

    for (tail = buf; tail->frags; tail = tail->frags)
    {
        tail->flags |= NET_BUF_FRAGS;
        NET_BUF_DBG("buf %p ref %u pool_id %p frags %p", buf, buf->ref, buf->pool_id, buf->frags);
    }

    NET_BUF_DBG("test_head: %p, tail: %p, buf: %p, node: %p, tail: %p", fifo->_queue.data_q.head,
                fifo->_queue.data_q.tail, buf, buf->node.next, tail);
    k_fifo_put_list(fifo, buf, tail);
    NET_BUF_DBG("test_head: %p, tail: %p, buf: %p, node: %p, tail: %p", fifo->_queue.data_q.head,
                fifo->_queue.data_q.tail, buf, buf->node.next, tail);
}

#if defined(CONFIG_BT_DEBUG)
void net_buf_unref_debug(struct net_buf *buf, const char *func, int line)
#else
void net_buf_unref(struct net_buf *buf)
#endif
{
    __ASSERT_NO_MSG(buf);

    while (buf)
    {
        struct net_buf *frags = buf->frags;
        struct spool *pool = buf->pool_id;

#if defined(CONFIG_BT_DEBUG)
        if (!buf->ref)
        {
            NET_BUF_ERR("%s():%d: buf %p double free", func, line, buf);
            return;
        }
        NET_BUF_DBG("%s():%d: buf %p", func, line, buf);
#endif
        NET_BUF_DBG("buf %p ref %u pool_id %p frags %p", buf, buf->ref, buf->pool_id, buf->frags);
        NET_BUF_DBG("test_flag %d", buf->flags);

        if (--buf->ref > 0)
        {
            return;
        }

        buf->data = NULL;
        buf->frags = NULL;

        if (pool)
        {
            spool_enqueue(pool, buf);
        }

        buf = frags;
    }
}
/*Increment the reference count of a buffer.*/
struct net_buf *net_buf_ref(struct net_buf *buf)
{
    __ASSERT_NO_MSG(buf);

    NET_BUF_DBG("net_buf_ref, buf %p (old) ref %u pool_id %p", buf, buf->ref, buf->pool_id);
    NET_BUF_DBG("test_flag %d", buf->flags);
    buf->ref++;
    return buf;
}

struct net_buf *net_buf_clone(struct net_buf *buf, k_timeout_t timeout)
{
    // TODO: Think need support or not.
    return NULL;
}

struct net_buf *net_buf_frag_last(struct net_buf *buf)
{
    __ASSERT_NO_MSG(buf);

    while (buf->frags)
    {
        buf = buf->frags;
    }

    return buf;
}
/*���µ�fragƬ�μ��븸Ƭ�κ�Ļ�����Ƭ���б���*/
void net_buf_frag_insert(struct net_buf *parent, struct net_buf *frag)
{
    __ASSERT_NO_MSG(parent);
    __ASSERT_NO_MSG(frag);

    if (parent->frags)
    {
        net_buf_frag_last(frag)->frags = parent->frags;
    }
    /* Take ownership of the fragment reference */
    parent->frags = frag;
}

struct net_buf *net_buf_frag_add(struct net_buf *head, struct net_buf *frag)
{
    __ASSERT_NO_MSG(frag);

    if (!head)
    {
        return net_buf_ref(frag);
    }

    net_buf_frag_insert(net_buf_frag_last(head), frag);

    return head;
}

#if defined(CONFIG_BT_DEBUG)
struct net_buf *net_buf_frag_del_debug(struct net_buf *parent, struct net_buf *frag,
                                       const char *func, int line)
#else
struct net_buf *net_buf_frag_del(struct net_buf *parent, struct net_buf *frag)
#endif
{
    struct net_buf *next_frag;

    __ASSERT_NO_MSG(frag);

    if (parent)
    {
        __ASSERT_NO_MSG(parent->frags);
        __ASSERT_NO_MSG(parent->frags == frag);
        parent->frags = frag->frags;
    }

    next_frag = frag->frags;

    frag->frags = NULL;

#if defined(CONFIG_BT_DEBUG)
    net_buf_unref_debug(frag, func, line);
#else
    net_buf_unref(frag);
#endif

    return next_frag;
}

size_t net_buf_linearize(void *dst, size_t dst_len, struct net_buf *src, size_t offset, size_t len)
{
    struct net_buf *frag;
    size_t to_copy;
    size_t copied;

    len = MIN(len, dst_len);

    frag = src;

    /* find the right fragment to start copying from */
    while (frag && offset >= frag->len)
    {
        offset -= frag->len;
        frag = frag->frags;
    }

    /* traverse the fragment chain until len bytes are copied */
    copied = 0;
    while (frag && len > 0)
    {
        to_copy = MIN(len, frag->len - offset);
        memcpy((uint8_t *)dst + copied, frag->data + offset, to_copy);

        copied += to_copy;

        /* to_copy is always <= len */
        len -= to_copy;
        frag = frag->frags;

        /* after the first iteration, this value will be 0 */
        offset = 0;
    }

    return copied;
}

/* This helper routine will append multiple bytes, if there is no place for
 * the data in current fragment then create new fragment and add it to
 * the buffer. It assumes that the buffer has at least one fragment.
 */
size_t net_buf_append_bytes(struct net_buf *buf, size_t len, const void *value, k_timeout_t timeout,
                            net_buf_allocator_cb allocate_cb, void *user_data)
{
    struct net_buf *frag = net_buf_frag_last(buf);
    size_t added_len = 0;
    const uint8_t *value8 = value;

    do
    {
        uint16_t count = MIN(len, net_buf_tailroom(frag));

        net_buf_add_mem(frag, value8, count);
        len -= count;
        added_len += count;
        value8 += count;

        if (len == 0)
        {
            return added_len;
        }

        if (allocate_cb)
        {
            frag = allocate_cb(timeout, user_data);
        }
        else
        {
            struct spool *pool;

            /* Allocate from the original pool if no callback has
             * been provided.
             */
            pool = buf->pool_id;
            frag = net_buf_alloc(pool, timeout);
        }

        if (!frag)
        {
            return added_len;
        }

        net_buf_frag_add(buf, frag);
    } while (1);

    /* Unreachable */
    return 0;
}

#define NET_BUF_SIMPLE_DBG(fmt, ...)  NET_BUF_DBG(fmt, ##__VA_ARGS__)
#define NET_BUF_SIMPLE_ERR(fmt, ...)  NET_BUF_ERR(fmt, ##__VA_ARGS__)
#define NET_BUF_SIMPLE_WARN(fmt, ...) NET_BUF_WARN(fmt, ##__VA_ARGS__)
#define NET_BUF_SIMPLE_INFO(fmt, ...) NET_BUF_INFO(fmt, ##__VA_ARGS__)

void net_buf_simple_clone(const struct net_buf_simple *original, struct net_buf_simple *clone)
{
    memcpy(clone, original, sizeof(struct net_buf_simple));
}

void *net_buf_simple_add(struct net_buf_simple *buf, size_t len)
{
    uint8_t *tail = net_buf_simple_tail(buf);

    NET_BUF_SIMPLE_DBG("net_buf_simple_add, buf %p len %zu", buf, len);

    __ASSERT_NO_MSG(net_buf_simple_tailroom(buf) >= len);

    buf->len += len;
    return tail;
}

void *net_buf_simple_add_mem(struct net_buf_simple *buf, const void *mem, size_t len)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_add_mem, buf %p len %zu", buf, len);

    return memcpy(net_buf_simple_add(buf, len), mem, len);
}

uint8_t *net_buf_simple_add_u8(struct net_buf_simple *buf, uint8_t val)
{
    uint8_t *u8;

    NET_BUF_SIMPLE_DBG("net_buf_simple_add_u8, buf %p val 0x%02x", buf, val);

    u8 = net_buf_simple_add(buf, 1);
    *u8 = val;

    return u8;
}

void net_buf_simple_add_le16(struct net_buf_simple *buf, uint16_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_add_le16, buf %p val %u", buf, val);

    sys_put_le16(val, net_buf_simple_add(buf, sizeof(val)));
}

void net_buf_simple_add_be16(struct net_buf_simple *buf, uint16_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_add_be16, buf %p val %u", buf, val);

    sys_put_be16(val, net_buf_simple_add(buf, sizeof(val)));
}

void net_buf_simple_add_le24(struct net_buf_simple *buf, uint32_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_add_le24, buf %p val %u", buf, val);

    sys_put_le24(val, net_buf_simple_add(buf, 3));
}

void net_buf_simple_add_be24(struct net_buf_simple *buf, uint32_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_add_be24, buf %p val %u", buf, val);

    sys_put_be24(val, net_buf_simple_add(buf, 3));
}

void net_buf_simple_add_le32(struct net_buf_simple *buf, uint32_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_add_le32, buf %p val %u", buf, val);

    sys_put_le32(val, net_buf_simple_add(buf, sizeof(val)));
}

void net_buf_simple_add_be32(struct net_buf_simple *buf, uint32_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_add_be32, buf %p val %u", buf, val);

    sys_put_be32(val, net_buf_simple_add(buf, sizeof(val)));
}

void net_buf_simple_add_le48(struct net_buf_simple *buf, uint64_t val)
{
    // NET_BUF_SIMPLE_DBG("net_buf_simple_add_le48, buf %p val %" PRIu64, buf, val);

    sys_put_le48(val, net_buf_simple_add(buf, 6));
}

void net_buf_simple_add_be48(struct net_buf_simple *buf, uint64_t val)
{
    // NET_BUF_SIMPLE_DBG("net_buf_simple_add_be48, buf %p val %" PRIu64, buf, val);

    sys_put_be48(val, net_buf_simple_add(buf, 6));
}

void net_buf_simple_add_le64(struct net_buf_simple *buf, uint64_t val)
{
    // NET_BUF_SIMPLE_DBG("net_buf_simple_add_le64, buf %p val %" PRIu64, buf, val);

    sys_put_le64(val, net_buf_simple_add(buf, sizeof(val)));
}

void net_buf_simple_add_be64(struct net_buf_simple *buf, uint64_t val)
{
    // NET_BUF_SIMPLE_DBG("net_buf_simple_add_be64, buf %p val %" PRIu64, buf, val);

    sys_put_be64(val, net_buf_simple_add(buf, sizeof(val)));
}
/*ͷ���޸�����ָ��ͻ����������Կ��ǻ�������ͷ�ĸ������ݡ�*/
void *net_buf_simple_push(struct net_buf_simple *buf, size_t len)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_push, buf %p len %zu", buf, len);

    __ASSERT_NO_MSG(net_buf_simple_headroom(buf) >= len);

    buf->data -= len;
    buf->len += len;
    return buf->data;
}

void net_buf_simple_push_le16(struct net_buf_simple *buf, uint16_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_push_le16, buf %p val %u", buf, val);

    sys_put_le16(val, net_buf_simple_push(buf, sizeof(val)));
}

void net_buf_simple_push_be16(struct net_buf_simple *buf, uint16_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_push_be16, buf %p val %u", buf, val);

    sys_put_be16(val, net_buf_simple_push(buf, sizeof(val)));
}

void net_buf_simple_push_u8(struct net_buf_simple *buf, uint8_t val)
{
    uint8_t *data = net_buf_simple_push(buf, 1);

    *data = val;
}

void net_buf_simple_push_le24(struct net_buf_simple *buf, uint32_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_push_le24, buf %p val %u", buf, val);

    sys_put_le24(val, net_buf_simple_push(buf, 3));
}

void net_buf_simple_push_be24(struct net_buf_simple *buf, uint32_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_push_be24, buf %p val %u", buf, val);

    sys_put_be24(val, net_buf_simple_push(buf, 3));
}

void net_buf_simple_push_le32(struct net_buf_simple *buf, uint32_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_push_le32, buf %p val %u", buf, val);

    sys_put_le32(val, net_buf_simple_push(buf, sizeof(val)));
}

void net_buf_simple_push_be32(struct net_buf_simple *buf, uint32_t val)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_push_be32, buf %p val %u", buf, val);

    sys_put_be32(val, net_buf_simple_push(buf, sizeof(val)));
}

void net_buf_simple_push_le48(struct net_buf_simple *buf, uint64_t val)
{
    // NET_BUF_SIMPLE_DBG("net_buf_simple_push_le48, buf %p val %" PRIu64, buf, val);

    sys_put_le48(val, net_buf_simple_push(buf, 6));
}

void net_buf_simple_push_be48(struct net_buf_simple *buf, uint64_t val)
{
    // NET_BUF_SIMPLE_DBG("net_buf_simple_push_be48, buf %p val %" PRIu64, buf, val);

    sys_put_be48(val, net_buf_simple_push(buf, 6));
}

void net_buf_simple_push_le64(struct net_buf_simple *buf, uint64_t val)
{
    // NET_BUF_SIMPLE_DBG("net_buf_simple_push_le64, buf %p val %" PRIu64, buf, val);

    sys_put_le64(val, net_buf_simple_push(buf, sizeof(val)));
}

void net_buf_simple_push_be64(struct net_buf_simple *buf, uint64_t val)
{
    // NET_BUF_SIMPLE_DBG("net_buf_simple_push_be64, buf %p val %" PRIu64, buf, val);

    sys_put_be64(val, net_buf_simple_push(buf, sizeof(val)));
}
/*���ж��ó���len>buf-len?
 * buf->len��ȥ�ó��ĳ��ȣ������޸ĺ��ָ��*/
void *net_buf_simple_pull(struct net_buf_simple *buf, size_t len)
{
    NET_BUF_SIMPLE_DBG("net_buf_simple_pull, buf %p len %zu", buf, len);

    __ASSERT_NO_MSG(buf->len >= len);

    buf->len -= len;
    return buf->data += len;
}

void *net_buf_simple_pull_mem(struct net_buf_simple *buf, size_t len)
{
    void *data = buf->data;

    NET_BUF_SIMPLE_DBG("net_buf_simple_pull_mem, buf %p len %zu", buf, len);

    __ASSERT_NO_MSG(buf->len >= len);

    buf->len -= len;
    buf->data += len;

    return data;
}

uint8_t net_buf_simple_pull_u8(struct net_buf_simple *buf)
{
    uint8_t val;

    val = buf->data[0];
    net_buf_simple_pull(buf, 1);

    return val;
}

uint16_t net_buf_simple_pull_le16(struct net_buf_simple *buf)
{
    uint16_t val;

    val = UNALIGNED_GET((uint16_t *)buf->data);
    net_buf_simple_pull(buf, sizeof(val));

    return sys_le16_to_cpu(val);
}

uint16_t net_buf_simple_pull_be16(struct net_buf_simple *buf)
{
    uint16_t val;

    val = UNALIGNED_GET((uint16_t *)buf->data);
    net_buf_simple_pull(buf, sizeof(val));

    return sys_be16_to_cpu(val);
}

uint32_t net_buf_simple_pull_le24(struct net_buf_simple *buf)
{
    struct uint24
    {
        uint32_t u24 : 24;
    } __packed val;

    val = UNALIGNED_GET((struct uint24 *)buf->data);
    net_buf_simple_pull(buf, sizeof(val));

    return sys_le24_to_cpu(val.u24);
}

uint32_t net_buf_simple_pull_be24(struct net_buf_simple *buf)
{
    struct uint24
    {
        uint32_t u24 : 24;
    } __packed val;

    val = UNALIGNED_GET((struct uint24 *)buf->data);
    net_buf_simple_pull(buf, sizeof(val));

    return sys_be24_to_cpu(val.u24);
}

uint32_t net_buf_simple_pull_le32(struct net_buf_simple *buf)
{
    uint32_t val;

    val = UNALIGNED_GET((uint32_t *)buf->data);
    net_buf_simple_pull(buf, sizeof(val));

    return sys_le32_to_cpu(val);
}

uint32_t net_buf_simple_pull_be32(struct net_buf_simple *buf)
{
    uint32_t val;

    val = UNALIGNED_GET((uint32_t *)buf->data);
    net_buf_simple_pull(buf, sizeof(val));

    return sys_be32_to_cpu(val);
}

uint64_t net_buf_simple_pull_le48(struct net_buf_simple *buf)
{
    struct uint48
    {
        uint64_t u48 : 48;
    } __packed val;

    val = UNALIGNED_GET((struct uint48 *)buf->data);
    net_buf_simple_pull(buf, sizeof(val));

    return sys_le48_to_cpu(val.u48);
}

uint64_t net_buf_simple_pull_be48(struct net_buf_simple *buf)
{
    struct uint48
    {
        uint64_t u48 : 48;
    } __packed val;

    val = UNALIGNED_GET((struct uint48 *)buf->data);
    net_buf_simple_pull(buf, sizeof(val));

    return sys_be48_to_cpu(val.u48);
}

uint64_t net_buf_simple_pull_le64(struct net_buf_simple *buf)
{
    uint64_t val;

    val = UNALIGNED_GET((uint64_t *)buf->data);
    net_buf_simple_pull(buf, sizeof(val));

    return sys_le64_to_cpu(val);
}

uint64_t net_buf_simple_pull_be64(struct net_buf_simple *buf)
{
    uint64_t val;

    val = UNALIGNED_GET((uint64_t *)buf->data);
    net_buf_simple_pull(buf, sizeof(val));

    return sys_be64_to_cpu(val);
}

size_t net_buf_simple_headroom(struct net_buf_simple *buf)
{
    return buf->data - buf->__buf;
}

size_t net_buf_simple_tailroom(struct net_buf_simple *buf)
{
    return buf->size - net_buf_simple_headroom(buf) - buf->len;
}

bool net_buf_check_empty(struct spool *pool)
{
    // uint64_t end = 1; // z_timeout_end_calc(timeout);
    // struct net_buf *buf;

    __ASSERT_NO_MSG(pool);

    return spool_size(pool);
}
