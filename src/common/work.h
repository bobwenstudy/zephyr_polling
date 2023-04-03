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

#ifndef _ZEPHYR_POLLING_UTILS_WORK_H_
#define _ZEPHYR_POLLING_UTILS_WORK_H_

#include "bt_config.h"

#include "base/types.h"
#include "utils/slist.h"
#include "common/timeout.h"

/**
 * @cond INTERNAL_HIDDEN
 */

struct k_work;
struct k_work_delayable;
struct k_work_sync;

struct k_work_sync
{
    // union {
    // 	struct z_work_flusher flusher;
    // 	struct z_work_canceller canceller;
    // };
    void *none;
};

/** @brief The signature for a work item handler function.
 *
 * The function will be invoked by the thread animating a work queue.
 *
 * @param work the work item that provided the handler.
 */
typedef void (*k_work_handler_t)(struct k_work *work);

/** @brief A structure used to submit work after a delay. */
struct k_work
{
    /* The work item. */
    struct _timeout timeout;

    /* The function to be invoked by the work queue thread. */
    k_work_handler_t handler;
};

void z_work_expiration_handler(struct _timeout *t);

/** @brief Initialize a (non-delayable) work structure.
 *
 * This must be invoked before submitting a work structure for the first time.
 * It need not be invoked again on the same work structure.  It can be
 * re-invoked to change the associated handler, but this must be done when the
 * work item is idle.
 *
 * @funcprops \isr_ok
 *
 * @param work the work structure to be initialized.
 *
 * @param handler the handler to be invoked by the work item.
 */
static inline void k_work_init(struct k_work *work, k_work_handler_t handler)
{
    work->handler = handler;
    z_init_timeout(&work->timeout);
}

/** @brief Busy state flags from the work item.
 *
 * A zero return value indicates the work item appears to be idle.
 *
 * @note This is a live snapshot of state, which may change before the result
 * is checked.  Use locks where appropriate.
 *
 * @funcprops \isr_ok
 *
 * @param work pointer to the work item.
 *
 * @return a mask of flags K_WORK_DELAYED, K_WORK_QUEUED,
 * K_WORK_RUNNING, and K_WORK_CANCELING.
 */
static inline int k_work_busy_get(const struct k_work *work)
{
    while (1)
        ;
    return 0;
}

/** @brief Test whether a work item is currently pending.
 *
 * Wrapper to determine whether a work item is in a non-idle dstate.
 *
 * @note This is a live snapshot of state, which may change before the result
 * is checked.  Use locks where appropriate.
 *
 * @funcprops \isr_ok
 *
 * @param work pointer to the work item.
 *
 * @return true if and only if k_work_busy_get() returns a non-zero value.
 */
static inline bool k_work_is_pending(const struct k_work *work)
{
    while (1)
        ;
    return false;
}

/** @brief Submit a work item to the system queue.
 *
 * @funcprops \isr_ok
 *
 * @param work pointer to the work item.
 *
 * @return as with k_work_submit_to_queue().
 */
static inline int k_work_submit(struct k_work *work)
{
    (void)z_abort_timeout(&work->timeout);
    z_add_timeout(&work->timeout, z_work_expiration_handler, K_NO_WAIT);
    return 0;
}

/** @brief Wait for last-submitted instance to complete.
 *
 * Resubmissions may occur while waiting, including chained submissions (from
 * within the handler).
 *
 * @note Be careful of caller and work queue thread relative priority.  If
 * this function sleeps it will not return until the work queue thread
 * completes the tasks that allow this thread to resume.
 *
 * @note Behavior is undefined if this function is invoked on @p work from a
 * work queue running @p work.
 *
 * @param work pointer to the work item.
 *
 * @param sync pointer to an opaque item containing state related to the
 * pending cancellation.  The object must persist until the call returns, and
 * be accessible from both the caller thread and the work queue thread.  The
 * object must not be used for any other flush or cancel operation until this
 * one completes.  On architectures with CONFIG_KERNEL_COHERENCE the object
 * must be allocated in coherent memory.
 *
 * @retval true if call had to wait for completion
 * @retval false if work was already idle
 */
static inline bool k_work_flush(struct k_work *work, struct k_work_sync *sync)
{
    while (1)
        ;
    return false;
}

/** @brief Cancel a work item.
 *
 * This attempts to prevent a pending (non-delayable) work item from being
 * processed by removing it from the work queue.  If the item is being
 * processed, the work item will continue to be processed, but resubmissions
 * are rejected until cancellation completes.
 *
 * If this returns zero cancellation is complete, otherwise something
 * (probably a work queue thread) is still referencing the item.
 *
 * See also k_work_cancel_sync().
 *
 * @funcprops \isr_ok
 *
 * @param work pointer to the work item.
 *
 * @return the k_work_busy_get() status indicating the state of the item after all
 * cancellation steps performed by this call are completed.
 */
static inline int k_work_cancel(struct k_work *work)
{
    return z_abort_timeout(&work->timeout);
}

/** @brief A structure used to submit work after a delay. */
struct k_work_delayable
{
    /* The work item. */
    struct k_work work;
};

/** @brief Cancel delayable work.
 *
 * Similar to k_work_cancel() but for delayable work.  If the work is
 * scheduled or submitted it is canceled.  This function does not wait for the
 * cancellation to complete.
 *
 * @note The work may still be running when this returns.  Use
 * k_work_flush_delayable() or k_work_cancel_delayable_sync() to ensure it is
 * not running.
 *
 * @note Canceling delayable work does not prevent rescheduling it.  It does
 * prevent submitting it until the cancellation completes.
 *
 * @funcprops \isr_ok
 *
 * @param dwork pointer to the delayable work item.
 *
 * @return the k_work_delayable_busy_get() status indicating the state of the
 * item after all cancellation steps performed by this call are completed.
 */
static inline int k_work_cancel_delayable(struct k_work_delayable *dwork)
{
    return z_abort_timeout(&dwork->work.timeout);
}

/** @brief Reschedule a work item to the system work queue after a
 * delay.
 *
 * This is a thin wrapper around k_work_reschedule_for_queue(), with all the
 * API characteristics of that function.
 *
 * @param dwork pointer to the delayable work item.
 *
 * @param delay the time to wait before submitting the work item.
 *
 * @return as with k_work_reschedule_for_queue().
 */
static inline int k_work_reschedule(struct k_work_delayable *dwork, k_timeout_t delay)
{
    (void)z_abort_timeout(&dwork->work.timeout);
    z_add_timeout(&dwork->work.timeout, z_work_expiration_handler, delay);
    return 0;
}

/** @brief Initialize a delayable work structure.
 *
 * This must be invoked before scheduling a delayable work structure for the
 * first time.  It need not be invoked again on the same work structure.  It
 * can be re-invoked to change the associated handler, but this must be done
 * when the work item is idle.
 *
 * @funcprops \isr_ok
 *
 * @param dwork the delayable work structure to be initialized.
 *
 * @param handler the handler to be invoked by the work item.
 */
static inline void k_work_init_delayable(struct k_work_delayable *dwork, k_work_handler_t handler)
{
    k_work_init(&dwork->work, handler);
}

/**
 * @brief Get the parent delayable work structure from a work pointer.
 *
 * This function is necessary when a @c k_work_handler_t function is passed to
 * k_work_schedule_for_queue() and the handler needs to access data from the
 * container of the containing `k_work_delayable`.
 *
 * @param work Address passed to the work handler
 *
 * @return Address of the containing @c k_work_delayable structure.
 */
static inline struct k_work_delayable *k_work_delayable_from_work(struct k_work *work)
{
    return (struct k_work_delayable *)work;
}

/** @brief Get the number of ticks until a scheduled delayable work will be
 * submitted.
 *
 * @note This is a live snapshot of state, which may change before the result
 * can be inspected.  Use locks where appropriate.
 *
 * @funcprops \isr_ok
 *
 * @param dwork pointer to the delayable work item.
 *
 * @return the number of ticks until the timer that will schedule the work
 * item will expire, or zero if the item is not scheduled.
 */
static inline uint32_t k_work_delayable_remaining_get(const struct k_work_delayable *dwork)
{
    return k_ticks_to_ms_floor32(z_timeout_remaining(&dwork->work.timeout));
}

/** @brief Submit an idle work item to the system work queue after a
 * delay.
 *
 * This is a thin wrapper around k_work_schedule_for_queue(), with all the API
 * characteristics of that function.
 *
 * @param dwork pointer to the delayable work item.
 *
 * @param delay the time to wait before submitting the work item.  If @c
 * K_NO_WAIT this is equivalent to k_work_submit_to_queue().
 *
 * @return as with k_work_schedule_for_queue().
 */
static inline int k_work_schedule(struct k_work_delayable *dwork, k_timeout_t delay)
{
    return k_work_reschedule(dwork, delay);
}


/**
 * @addtogroup workqueue_apis
 * @{
 */

#define Z_WORK_INITIALIZER(work_handler) { \
	.handler = work_handler, \
}

/**
 * @brief Initialize a statically-defined work item.
 *
 * This macro can be used to initialize a statically-defined workqueue work
 * item, prior to its first use. For example,
 *
 * @code static K_WORK_DEFINE(<work>, <work_handler>); @endcode
 *
 * @param work Symbol name for work item object
 * @param work_handler Function to invoke each time work item is processed.
 */
#define K_WORK_DEFINE(work, work_handler) \
	struct k_work work = Z_WORK_INITIALIZER(work_handler)


#endif /* _ZEPHYR_POLLING_UTILS_WORK_H_ */