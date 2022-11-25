/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief Single-linked list implementation
 *
 * Single-linked list implementation using inline macros/functions.
 * This API is not thread safe, and thus if a list is used across threads,
 * calls to functions must be protected with synchronization primitives.
 */

#ifndef _ZEPHYR_POLLING_UTILS_K_SEM_H_
#define _ZEPHYR_POLLING_UTILS_K_SEM_H_

#include "base/sys_clock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @}
 */

/**
 * @cond INTERNAL_HIDDEN
 */

struct k_sem
{
    unsigned int count;
    unsigned int limit;
};

#define Z_SEM_INITIALIZER(obj, initial_count, count_limit)                                         \
    {                                                                                              \
        .count = initial_count, .limit = count_limit,                                              \
    }

/**
 * INTERNAL_HIDDEN @endcond
 */

/**
 * @defgroup semaphore_apis Semaphore APIs
 * @ingroup kernel_apis
 * @{
 */

/**
 * @brief Maximum limit value allowed for a semaphore.
 *
 * This is intended for use when a semaphore does not have
 * an explicit maximum limit, and instead is just used for
 * counting purposes.
 *
 */
#define K_SEM_MAX_LIMIT UINT_MAX

/**
 * @brief Initialize a semaphore.
 *
 * This routine initializes a semaphore object, prior to its first use.
 *
 * @param sem Address of the semaphore.
 * @param initial_count Initial semaphore count.
 * @param limit Maximum permitted semaphore count.
 *
 * @see K_SEM_MAX_LIMIT
 *
 * @retval 0 Semaphore created successfully
 * @retval -EINVAL Invalid values
 *
 */
static inline int k_sem_init(struct k_sem *sem, unsigned int initial_count, unsigned int limit)
{
    sem->count = initial_count;
    sem->limit = limit;
    return 0;
}

/**
 * @brief Take a semaphore.
 *
 * This routine takes @a sem.
 *
 * @note @a timeout must be set to K_NO_WAIT if called from ISR.
 *
 * @funcprops \isr_ok
 *
 * @param sem Address of the semaphore.
 * @param timeout Waiting period to take the semaphore,
 *                or one of the special values K_NO_WAIT and K_FOREVER.
 *
 * @retval 0 Semaphore taken.
 * @retval -EBUSY Returned without waiting.
 * @retval -EAGAIN Waiting period timed out,
 *			or the semaphore was reset during the waiting period.
 */
static inline int k_sem_take(struct k_sem *sem, k_timeout_t timeout)
{
    if (sem->count > 0)
    {
        sem->count--;
    }
    return 0;
}

/**
 * @brief Give a semaphore.
 *
 * This routine gives @a sem, unless the semaphore is already at its maximum
 * permitted count.
 *
 * @funcprops \isr_ok
 *
 * @param sem Address of the semaphore.
 */
static inline void k_sem_give(struct k_sem *sem)
{
    sem->count += (sem->count != sem->limit) ? 1U : 0U;
}

/**
 * @brief Resets a semaphore's count to zero.
 *
 * This routine sets the count of @a sem to zero.
 * Any outstanding semaphore takes will be aborted
 * with -EAGAIN.
 *
 * @param sem Address of the semaphore.
 */
static inline void k_sem_reset(struct k_sem *sem)
{
    sem->count = 0;
}

/**
 * @brief Get a semaphore's count.
 *
 * This routine returns the current count of @a sem.
 *
 * @param sem Address of the semaphore.
 *
 * @return Current semaphore count.
 */
static inline unsigned int k_sem_count_get(struct k_sem *sem)
{
    return sem->count;
}

/**
 * @brief Statically define and initialize a semaphore.
 *
 * The semaphore can be accessed outside the module where it is defined using:
 *
 * @code extern struct k_sem <name>; @endcode
 *
 * @param name Name of the semaphore.
 * @param initial_count Initial semaphore count.
 * @param count_limit Maximum permitted semaphore count.
 */
#define K_SEM_DEFINE(name, initial_count, count_limit)                                             \
    struct k_sem name = Z_SEM_INITIALIZER(name, initial_count, count_limit);                       \
    BUILD_ASSERT(((count_limit) != 0) && ((initial_count) <= (count_limit)) &&                     \
                 ((count_limit) <= K_SEM_MAX_LIMIT));

#ifdef __cplusplus
}
#endif

#endif /* _ZEPHYR_POLLING_UTILS_K_SEM_H_ */