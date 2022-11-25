/*
 * Copyright (c) 2010-2012, 2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _ZEPHYR_POLLING_BASE_IRQ_H_
#define _ZEPHYR_POLLING_BASE_IRQ_H_

#include "bt_config.h"

#include "base/gcc.h"

#ifdef __cplusplus
extern "C" {
#endif

__weak unsigned int arch_irq_lock(void)
{
}

__weak void arch_irq_unlock(unsigned int key)
{
}

#define irq_lock()      arch_irq_lock()
#define irq_unlock(key) arch_irq_unlock(key)

#ifdef __cplusplus
}
#endif

#endif /* _ZEPHYR_POLLING_BASE_GCC_H_ */