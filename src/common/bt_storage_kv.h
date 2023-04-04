#ifndef _ZEPHYR_POLLING_COMMON_bt_storage_kv_H_
#define _ZEPHYR_POLLING_COMMON_bt_storage_kv_H_

#include "bt_config.h"

#include "base/types.h"

enum bt_storage_kv_keys
{
    KEY_INDEX_NULL = 0x0000,

    KEY_INDEX_LE_ID_ADDR_LIST = 0x0001,
    KEY_INDEX_LE_ID_IRK_LIST = 0x0002,
    KEY_INDEX_LE_ID_NAME = 0x0003,
    KEY_INDEX_LE_ID_APPEARANCE = 0x0004,

    KEY_INDEX_LE_KEY_INFO_LIST = 0x0100,
    KEY_INDEX_LE_KEY_INFO_ITEM_BASE = 0x0110,
};

#define KEY_INDEX_LE_KEY_INFO_ITEM(__x) (KEY_INDEX_LE_KEY_INFO_ITEM_BASE + (__x))

struct bt_storage_kv_header
{
    uint16_t key;
    uint16_t length; // zero length means length not same.
};

struct bt_storage_kv_impl
{
    void (*init_list)(struct bt_storage_kv_header *list, uint16_t list_cnt);

    int (*get)(uint16_t key, uint8_t *data, int *len);

    void (*set)(uint16_t key, uint8_t *data, int len);

    void (*delete)(uint16_t key, uint8_t *data, int len);
};

void bt_storage_kv_init_list(struct bt_storage_kv_header *list, uint16_t list_cnt);
int bt_storage_kv_get(uint16_t key, uint8_t *data, int *len);
void bt_storage_kv_set(uint16_t key, uint8_t *data, int len);
void bt_storage_kv_delete(uint16_t key, uint8_t *data, int len);
void bt_storage_kv_register(const struct bt_storage_kv_impl *impl);

/**
 * Function used to read the data from the settings storage in
 * h_set handler implementations.
 *
 * @param[in] cb_arg  arguments for the read function. Appropriate cb_arg is
 *                    transferred to h_set handler implementation by
 *                    the backend.
 * @param[out] data  the destination buffer
 * @param[in] len    length of read
 *
 * @return positive: Number of bytes read, 0: key-value pair is deleted.
 *                   On error returns -ERRNO code.
 */
typedef ssize_t (*settings_read_cb)(void *cb_arg, void *data, size_t len);

#endif /* _ZEPHYR_POLLING_COMMON_bt_storage_kv_H_ */