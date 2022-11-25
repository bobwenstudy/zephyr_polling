/*
 * Copyright (c) 2015 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_POLLING_BASE_TYPES_H_
#define _ZEPHYR_POLLING_BASE_TYPES_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 32 bits on ILP32 builds, 64 bits on LP64 builds */
typedef unsigned long ulong_t;

/*
 * A type with strong alignment requirements, similar to C11 max_align_t. It can
 * be used to force alignment of data structures allocated on the stack or as
 * return * type for heap allocators.
 */
typedef union
{
    long long thelonglong;
    long double thelongdouble;
    uintmax_t theuintmax_t;
    size_t thesize_t;
    uintptr_t theuintptr_t;
    void *thepvoid;
    void (*thepfunc)(void);
} z_max_align_t;

#ifdef __cplusplus
}
#endif

#endif /* _ZEPHYR_POLLING_BASE_TYPES_H_ */