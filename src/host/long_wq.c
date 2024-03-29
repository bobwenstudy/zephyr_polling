/* long_work.c - Workqueue intended for long-running operations. */

/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <common/work.h>

// K_THREAD_STACK_DEFINE(bt_lw_stack_area, CONFIG_BT_LONG_WQ_STACK_SIZE);
// static struct k_work_q bt_long_wq;

int bt_long_wq_schedule(struct k_work_delayable *dwork, k_timeout_t timeout)
{
    return k_work_schedule(dwork, timeout);
    // return k_work_schedule_for_queue(&bt_long_wq, dwork, timeout);
}

int bt_long_wq_reschedule(struct k_work_delayable *dwork, k_timeout_t timeout)
{
    return k_work_reschedule(dwork, timeout);
    // return k_work_reschedule_for_queue(&bt_long_wq, dwork, timeout);
}

int bt_long_wq_submit(struct k_work *work)
{
    return k_work_submit(work);
    // return k_work_submit_to_queue(&bt_long_wq, work);
}

// static int long_wq_init(const struct device *d)
// {
//     ARG_UNUSED(d);

//     const struct k_work_queue_config cfg = {.name = "BT_LW_WQ"};

//     k_work_queue_init(&bt_long_wq);

//     k_work_queue_start(&bt_long_wq, bt_lw_stack_area, K_THREAD_STACK_SIZEOF(bt_lw_stack_area),
//                        CONFIG_BT_LONG_WQ_PRIO, &cfg);

//     return 0;
// }

// SYS_INIT(long_wq_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
