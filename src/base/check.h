/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_POLLING_BASE_CHECK_H_
#define _ZEPHYR_POLLING_BASE_CHECK_H_

#include "bt_config.h"

#include "base/__assert.h"

#if defined(CONFIG_ASSERT_ON_ERRORS)
#define CHECKIF(expr)                                                                              \
    __ASSERT_NO_MSG(!(expr));                                                                      \
    if (0)
#elif defined(CONFIG_NO_RUNTIME_CHECKS)
#define CHECKIF(...) if (0)
#else
#define CHECKIF(expr) if (expr)
#endif

#endif /* _ZEPHYR_POLLING_BASE_CHECK_H_ */