/*
 * Copyright (c) 2016 Nordic Semiconductor ASA
 * Copyright (c) 2016 Vinayak Kariappa Chettimada
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ZEPHYR_POLLING_UTILS_MEM_H_
#define _ZEPHYR_POLLING_UTILS_MEM_H_

#ifndef MALIGN
/**
 * @brief Force compiler to place memory at-least on a x-byte boundary
 * @details Compiler extension. Supported by GCC and Clang
 */
#define MALIGN(x) __attribute__((aligned(x)))
#endif

#ifndef MROUND
/**
 * @brief Round up to nearest multiple of 4, for unsigned integers
 * @details
 *   The addition of 3 forces x into the next multiple of 4. This is responsible
 *   for the rounding in the the next step, to be Up.
 *   For ANDing of ~3: We observe y & (~3) == (y>>2)<<2, and we recognize
 *   (y>>2) as a floored division, which is almost undone by the left-shift. The
 *   flooring can't be undone so have achieved a rounding.
 *
 *   Examples:
 *    MROUND( 0) =  0
 *    MROUND( 1) =  4
 *    MROUND( 2) =  4
 *    MROUND( 3) =  4
 *    MROUND( 4) =  4
 *    MROUND( 5) =  8
 *    MROUND( 8) =  8
 *    MROUND( 9) = 12
 *    MROUND(13) = 16
 */
#define MROUND(x) (((uint32_t)(x) + 3) & (~((uint32_t)3)))
#endif

void mem_init(void *mem_pool, uint16_t mem_size, uint16_t mem_count, void **mem_head);
void *mem_acquire(void **mem_head);
void mem_release(void *mem, void **mem_head);

uint16_t mem_free_count_get(void *mem_head);
void *mem_get(void *mem_pool, uint16_t mem_size, uint16_t index);
uint16_t mem_index_get(void *mem, void *mem_pool, uint16_t mem_size);

void mem_rcopy(uint8_t *dst, uint8_t const *src, uint16_t len);
uint8_t mem_nz(uint8_t *src, uint16_t len);

uint32_t mem_ut(void);

#endif /* _ZEPHYR_POLLING_UTILS_MEM_H_ */