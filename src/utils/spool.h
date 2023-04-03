#ifndef _ZEPHYR_POLLING_UTILS_SPOOL_H_
#define _ZEPHYR_POLLING_UTILS_SPOOL_H_

#include "base/types.h"

struct spool
{
    uint8_t num;  /* Number of buffers */
    uint8_t rptr; /* Read. Read index */
    uint8_t wptr; /* Write. Write index */

    /* Size of user data allocated to this pool */
    uint8_t user_data_size;

    uint16_t data_size; /* data size */
    void **buf;         /* point ptr buffer */
    void *__buf;         /* point store buffer */
};
/* Alignment needed for various parts of the buffer definition */
#define __spool_buf_align __aligned(sizeof(void *))

/**
 * @def SPOOL_DEFINE
 * @brief Define a new pool for buffers based on fixed-size data
 *
 * @param _name      Name of the pool variable.
 * @param _count     Number of buffers in the pool.
 * @param _data_size Maximum data payload per buffer.
 * @param _ud_size   User data space to reserve per buffer.
 */
// ptr need reserve one.
#define SPOOL_DEFINE(_name, _num, _data_size, _ud_size)                                            \
    static struct spool _name __noretention_data_section;                                          \
    static void *spool_ptr_##_name[_num + 1] __noretention_data_section;                           \
    static uint8_t                                                                                 \
            spool_storage_##_name[_num][(_data_size + sizeof(struct net_buf) + _ud_size + 3) / 4 * \
                                        4] __spool_buf_align __noretention_data_section;

#define SPOOL_INIT(_name, _num, _data_size, _ud_size)                                              \
    spool_init(&_name, _num, spool_ptr_##_name, (void *)spool_storage_##_name, _data_size, _ud_size)

uint8_t spool_init(struct spool *pool, uint8_t num, void **buf, uint8_t *storage_buf,
                   uint16_t data_size, uint8_t user_data_size);
uint8_t spool_size(struct spool *pool);
uint8_t spool_enqueue(struct spool *pool, void *val);
void *spool_dequeue(struct spool *pool);
void *spool_dequeue_peek(struct spool *pool);
uint8_t spool_check_full(struct spool *pool);

#endif /* _ZEPHYR_POLLING_UTILS_SPOOL_H_ */