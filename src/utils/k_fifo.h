/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_POLLING_UTILS_K_FIFO_H_
#define _ZEPHYR_POLLING_UTILS_K_FIFO_H_

#include "k_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @} */

struct k_fifo
{
    struct k_queue _queue;
};

/**
 * @cond INTERNAL_HIDDEN
 */
#define Z_FIFO_INITIALIZER(obj)                                                                    \
    {                                                                                              \
        ._queue = Z_QUEUE_INITIALIZER(obj._queue)                                                  \
    }

/**
 * INTERNAL_HIDDEN @endcond
 */

/**
 * @defgroup fifo_apis FIFO APIs
 * @ingroup kernel_apis
 * @{
 */

/**
 * @brief Initialize a FIFO queue.
 *
 * This routine initializes a FIFO queue, prior to its first use.
 *
 * @param fifo Address of the FIFO queue.
 */
#define k_fifo_init(fifo) ({ k_queue_init(&(fifo)->_queue); })

/**
 * @brief Cancel waiting on a FIFO queue.
 *
 * This routine causes first thread pending on @a fifo, if any, to
 * return from k_fifo_get() call with NULL value (as if timeout
 * expired).
 *
 * @funcprops \isr_ok
 *
 * @param fifo Address of the FIFO queue.
 */
#define k_fifo_cancel_wait(fifo) ({ k_queue_cancel_wait(&(fifo)->_queue); })

/**
 * @brief Add an element to a FIFO queue.
 *
 * This routine adds a data item to @a fifo. A FIFO data item must be
 * aligned on a word boundary, and the first word of the item is reserved
 * for the kernel's use.
 *
 * @funcprops \isr_ok
 *
 * @param fifo Address of the FIFO.
 * @param data Address of the data item.
 */
#define k_fifo_put(fifo, data) ({ k_queue_append(&(fifo)->_queue, data); })

/**
 * @brief Add an element to a FIFO queue.
 *
 * This routine adds a data item to @a fifo. There is an implicit memory
 * allocation to create an additional temporary bookkeeping data structure from
 * the calling thread's resource pool, which is automatically freed when the
 * item is removed. The data itself is not copied.
 *
 * @funcprops \isr_ok
 *
 * @param fifo Address of the FIFO.
 * @param data Address of the data item.
 *
 * @retval 0 on success
 * @retval -ENOMEM if there isn't sufficient RAM in the caller's resource pool
 */
#define k_fifo_alloc_put(fifo, data)                                                               \
    ({                                                                                             \
        int ret = k_queue_alloc_append(&(fifo)->_queue, data);                                     \
        ret;                                                                                       \
    })

/**
 * @brief Atomically add a list of elements to a FIFO.
 *
 * This routine adds a list of data items to @a fifo in one operation.
 * The data items must be in a singly-linked list, with the first word of
 * each data item pointing to the next data item; the list must be
 * NULL-terminated.
 *
 * @funcprops \isr_ok
 *
 * @param fifo Address of the FIFO queue.
 * @param head Pointer to first node in singly-linked list.
 * @param tail Pointer to last node in singly-linked list.
 */
#define k_fifo_put_list(fifo, head, tail) ({ k_queue_append_list(&(fifo)->_queue, head, tail); })

/**
 * @brief Atomically add a list of elements to a FIFO queue.
 *
 * This routine adds a list of data items to @a fifo in one operation.
 * The data items must be in a singly-linked list implemented using a
 * sys_slist_t object. Upon completion, the sys_slist_t object is invalid
 * and must be re-initialized via sys_slist_init().
 *
 * @funcprops \isr_ok
 *
 * @param fifo Address of the FIFO queue.
 * @param list Pointer to sys_slist_t object.
 */
#define k_fifo_put_slist(fifo, list) ({ k_queue_merge_slist(&(fifo)->_queue, list); })

/**
 * @brief Get an element from a FIFO queue.
 *
 * This routine removes a data item from @a fifo in a "first in, first out"
 * manner. The first word of the data item is reserved for the kernel's use.
 *
 * @note @a timeout must be set to K_NO_WAIT if called from ISR.
 *
 * @funcprops \isr_ok
 *
 * @param fifo Address of the FIFO queue.
 * @param timeout Waiting period to obtain a data item,
 *                or one of the special values K_NO_WAIT and K_FOREVER.
 *
 * @return Address of the data item if successful; NULL if returned
 * without waiting, or waiting period timed out.
 */
#define k_fifo_get(fifo, timeout)                                                                  \
    ({                                                                                             \
        void *ret = k_queue_get(&(fifo)->_queue, timeout);                                         \
        ret;                                                                                       \
    })

/**
 * @brief Query a FIFO queue to see if it has data available.
 *
 * Note that the data might be already gone by the time this function returns
 * if other threads is also trying to read from the FIFO.
 *
 * @funcprops \isr_ok
 *
 * @param fifo Address of the FIFO queue.
 *
 * @return Non-zero if the FIFO queue is empty.
 * @return 0 if data is available.
 */
#define k_fifo_is_empty(fifo) k_queue_is_empty(&(fifo)->_queue)

/**
 * @brief Peek element at the head of a FIFO queue.
 *
 * Return element from the head of FIFO queue without removing it. A usecase
 * for this is if elements of the FIFO object are themselves containers. Then
 * on each iteration of processing, a head container will be peeked,
 * and some data processed out of it, and only if the container is empty,
 * it will be completely remove from the FIFO queue.
 *
 * @param fifo Address of the FIFO queue.
 *
 * @return Head element, or NULL if the FIFO queue is empty.
 */
#define k_fifo_peek_head(fifo)                                                                     \
    ({                                                                                             \
        void *ret = k_queue_peek_head(&(fifo)->_queue);                                            \
        ret;                                                                                       \
    })

/**
 * @brief Peek element at the tail of FIFO queue.
 *
 * Return element from the tail of FIFO queue (without removing it). A usecase
 * for this is if elements of the FIFO queue are themselves containers. Then
 * it may be useful to add more data to the last container in a FIFO queue.
 *
 * @param fifo Address of the FIFO queue.
 *
 * @return Tail element, or NULL if a FIFO queue is empty.
 */
#define k_fifo_peek_tail(fifo)                                                                     \
    ({                                                                                             \
        void *ret = k_queue_peek_tail(&(fifo)->_queue);                                            \
        ret;                                                                                       \
    })

/**
 * @brief Statically define and initialize a FIFO queue.
 *
 * The FIFO queue can be accessed outside the module where it is defined using:
 *
 * @code extern struct k_fifo <name>; @endcode
 *
 * @param name Name of the FIFO queue.
 */
#define K_FIFO_DEFINE(name) struct k_fifo name = Z_FIFO_INITIALIZER(name)

/**
 * @brief get a FIFO size.
 *
 * This routine initializes a FIFO queue, prior to its first use.
 *
 * @param fifo Address of the FIFO queue.
 */
#define k_fifo_size(fifo) ({ sys_slist_size(&(fifo)->_queue.data_q); })

#ifdef __cplusplus
}
#endif

#endif /* _ZEPHYR_POLLING_UTILS_K_FIFO_H_ */