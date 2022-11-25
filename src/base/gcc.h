/*
 * Copyright (c) 2010-2012, 2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _ZEPHYR_POLLING_BASE_GCC_H_
#define _ZEPHYR_POLLING_BASE_GCC_H_

#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>

#include "bt_config.h"

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ORDER_BIG_ENDIAN__
#define __ORDER_BIG_ENDIAN__ (1)
#endif

#ifndef __ORDER_LITTLE_ENDIAN__
#define __ORDER_LITTLE_ENDIAN__ (2)
#endif

#define __BYTE_ORDER__  __ORDER_LITTLE_ENDIAN__
#define __CHAR_BIT__    8
#define __SIZEOF_LONG__ 4

#ifndef __fallthrough
#if __GNUC__ >= 7
#define __fallthrough __attribute__((fallthrough))
#else
#define __fallthrough
#endif /* __GNUC__ >= 7 */
#endif

#ifndef __packed
#define __packed __attribute__((__packed__))
#endif
#ifndef __aligned
#define __aligned(x) __attribute__((__aligned__(x)))
#endif
#define __may_alias __attribute__((__may_alias__))
#ifndef __printf_like
#define __printf_like(f, a) __attribute__((format(printf, f, a)))
#endif
#define __used __attribute__((__used__))
#define __deprecated
#ifndef __deprecated
#define __deprecated __attribute__((deprecated))
#endif
#define ARG_UNUSED(x) (void)(x)

#define likely(x)   __builtin_expect((bool)!!(x), true)
#define unlikely(x) __builtin_expect((bool)!!(x), false)

#define popcount(x) __builtin_popcount(x)

#ifndef __no_optimization
#define __no_optimization __attribute__((optimize("-O0")))
#endif

#ifndef __weak
#define __weak __attribute__((__weak__))
#endif
#define __unused __attribute__((__unused__))

#define __builtin_unreachable()                                                                    \
    do                                                                                             \
    {                                                                                              \
        __ASSERT(false, "Unreachable code");                                                       \
    } while (true)

#define CODE_UNREACHABLE __builtin_unreachable()

#define FUNC_NORETURN __attribute__((__noreturn__))

/* Unaligned access */
#define UNALIGNED_GET(p)                                                                           \
    __extension__({                                                                                \
        struct __attribute__((__packed__))                                                         \
        {                                                                                          \
            __typeof__(*(p)) __v;                                                                  \
        } *__p = (__typeof__(__p))(p);                                                             \
        __p->__v;                                                                                  \
    })

/* Double indirection to ensure section names are expanded before
 * stringification
 */
#define __GENERIC_SECTION(segment) __attribute__((section(STRINGIFY(segment))))
#define Z_GENERIC_SECTION(segment) __GENERIC_SECTION(segment)

#define ___in_section(a, b, c)                                                                     \
    __attribute__((section("." Z_STRINGIFY(a) "." Z_STRINGIFY(b) "." Z_STRINGIFY(c))))
#define __in_section(a, b, c) ___in_section(a, b, c)

#define __in_section_unique(seg) ___in_section(seg, __FILE__, __COUNTER__)

#if !defined(_ASMLANGUAGE)

#define __noinit           __in_section_unique(_NOINIT_SECTION_NAME)
#define __irq_vector_table Z_GENERIC_SECTION(_IRQ_VECTOR_TABLE_SECTION_NAME)
#define __sw_isr_table     Z_GENERIC_SECTION(_SW_ISR_TABLE_SECTION_NAME)

#if defined(CONFIG_ARM)
#define __kinetis_flash_config_section __in_section_unique(_KINETIS_FLASH_CONFIG_SECTION_NAME)
#define __ti_ccfg_section              Z_GENERIC_SECTION(_TI_CCFG_SECTION_NAME)
#define __ccm_data_section             Z_GENERIC_SECTION(_CCM_DATA_SECTION_NAME)
#define __ccm_bss_section              Z_GENERIC_SECTION(_CCM_BSS_SECTION_NAME)
#define __ccm_noinit_section           Z_GENERIC_SECTION(_CCM_NOINIT_SECTION_NAME)
#define __dtcm_data_section            Z_GENERIC_SECTION(_DTCM_DATA_SECTION_NAME)
#define __dtcm_bss_section             Z_GENERIC_SECTION(_DTCM_BSS_SECTION_NAME)
#define __dtcm_noinit_section          Z_GENERIC_SECTION(_DTCM_NOINIT_SECTION_NAME)
#define __imx_boot_conf_section        Z_GENERIC_SECTION(_IMX_BOOT_CONF_SECTION_NAME)
#define __imx_boot_data_section        Z_GENERIC_SECTION(_IMX_BOOT_DATA_SECTION_NAME)
#define __imx_boot_ivt_section         Z_GENERIC_SECTION(_IMX_BOOT_IVT_SECTION_NAME)
#define __imx_boot_dcd_section         Z_GENERIC_SECTION(_IMX_BOOT_DCD_SECTION_NAME)
#endif /* CONFIG_ARM */

#if defined(CONFIG_NOCACHE_MEMORY)
#define __nocache __in_section_unique(_NOCACHE_SECTION_NAME)
#else
#define __nocache
#endif /* CONFIG_NOCACHE_MEMORY */

#endif /* !_ASMLANGUAGE */

// For small retention case.
#define __noretention_data_section Z_GENERIC_SECTION(._noretention_area)

#ifdef __cplusplus
}
#endif

#endif /* _ZEPHYR_POLLING_BASE_GCC_H_ */