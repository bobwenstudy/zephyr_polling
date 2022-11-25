/*
 * Copyright (c) 1997-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "work.h"

/**
 * @brief Handle expiration of a kernel timer object.
 *
 * @param t  Timeout used by the timer.
 */
void z_work_expiration_handler(struct _timeout *t)
{
    struct k_work *work = CONTAINER_OF(t, struct k_work, timeout);

    /* invoke timer expiry function */
    if (work->handler != NULL)
    {
        work->handler(work);
    }
}
