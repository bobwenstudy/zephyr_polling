/* keys.c - Bluetooth key handling */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "base/atomic.h"
#include "base/common.h"

#include "keys.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/hci.h>

#define LOG_MODULE_NAME bt_keys
#include "logging/bt_log.h"

#include "common/rpa.h"
#include "gatt_internal.h"
#include "hci_core.h"
#include "smp.h"

#include "common/bt_storage_kv.h"

#if defined(CONFIG_BT_SMP)
static struct bt_keys key_pool[CONFIG_BT_MAX_PAIRED];

#define BT_KEYS_STORAGE_LEN_COMPAT (BT_KEYS_STORAGE_LEN - sizeof(uint32_t))

#if IS_ENABLED(CONFIG_BT_KEYS_OVERWRITE_OLDEST)
static uint32_t aging_counter_val;
static struct bt_keys *last_keys_updated;
#endif /* CONFIG_BT_KEYS_OVERWRITE_OLDEST */

#define BT_SETTINGS_KEY_MAX          (0x10)
#define BT_KEYS_LIST_INFO_MAGIC_INFO (0xaabb)

struct bt_storage_kv_key_list_item
{
    uint8_t id;
    uint8_t index;
    bt_addr_le_t addr;
};

struct bt_storage_kv_key_list_header
{
    uint16_t magic;
    uint16_t cnt;
    struct bt_storage_kv_key_list_item items[BT_SETTINGS_KEY_MAX];
};

void bt_storage_kv_init_key_list_info(struct bt_storage_kv_key_list_header *list_info)
{
    list_info->magic = BT_KEYS_LIST_INFO_MAGIC_INFO;
    list_info->cnt = 0;
}

void bt_storage_kv_get_key_list_info(struct bt_storage_kv_key_list_header *list_info)
{
    int len = sizeof(struct bt_storage_kv_key_list_header);
    int ret = bt_storage_kv_get(KEY_INDEX_LE_KEY_INFO_LIST, (uint8_t *)list_info, &len);
    if ((ret < 0) || (len != sizeof(struct bt_storage_kv_key_list_header)) ||
        (list_info->magic != BT_KEYS_LIST_INFO_MAGIC_INFO))
    {
        bt_storage_kv_init_key_list_info(list_info);
    }
}

int bt_storage_kv_get_key_list_info_pos(struct bt_storage_kv_key_list_header *list_info, uint8_t id,
                                        const bt_addr_le_t *addr)
{
    int pos = -1;
    for (int i = 0; i < list_info->cnt; i++)
    {
        struct bt_storage_kv_key_list_item *item = &list_info->items[i];
        if (id == item->id && !bt_addr_le_cmp(&item->addr, addr))
        {
            pos = i;
            break;
        }
    }

    return pos;
}

int bt_storage_kv_get_key_list_info_index(struct bt_storage_kv_key_list_header *list_info,
                                          uint8_t id, const bt_addr_le_t *addr)
{
    int select_index = -1;
    int pos = bt_storage_kv_get_key_list_info_pos(list_info, id, addr);
    if (pos >= 0)
    {
        select_index = list_info->items[pos].index;
    }

    return select_index;
}

void bt_storage_kv_set_key_list_info_append(struct bt_storage_kv_key_list_header *list_info,
                                            uint8_t id, const bt_addr_le_t *addr, uint8_t index)
{
    // TODO: check the old one?
    uint8_t store_index = list_info->cnt;
    if (store_index >= BT_SETTINGS_KEY_MAX)
    {
        store_index = 0;
    }
    else
    {
        list_info->cnt++;
    }

    struct bt_storage_kv_key_list_item *item = &list_info->items[store_index];
    item->id = id;
    item->index = index;
    bt_addr_le_copy(&item->addr, addr);

    bt_storage_kv_set(KEY_INDEX_LE_KEY_INFO_LIST, (uint8_t *)list_info,
                      sizeof(struct bt_storage_kv_key_list_header));
}

void bt_storage_kv_set_key_list_info_delete(struct bt_storage_kv_key_list_header *list_info,
                                            uint8_t id, const bt_addr_le_t *addr)
{
    // TODO: check the old one?
    int pos = bt_storage_kv_get_key_list_info_pos(list_info, id, addr);
    if (pos > 0)
    {
        list_info->cnt--;
        if (list_info->cnt > 0)
        {
            for (int i = pos; i < list_info->cnt; i++)
            {
                memcpy(&list_info->items[i], &list_info->items[i + 1],
                       sizeof(struct bt_storage_kv_key_list_item));
            }
        }

        bt_storage_kv_set(KEY_INDEX_LE_KEY_INFO_LIST, (uint8_t *)list_info,
                          sizeof(struct bt_storage_kv_key_list_header));
    }
}

void bt_storage_kv_set_key_item(uint8_t index, struct bt_keys *keys)
{
    // TODO: check the old one?
    BT_ASSERT(index < BT_SETTINGS_KEY_MAX);

    bt_storage_kv_set(KEY_INDEX_LE_KEY_INFO_ITEM(index), keys->storage_start, BT_KEYS_STORAGE_LEN);
}

int bt_storage_kv_get_key_item(uint8_t index, struct bt_keys *keys)
{
    // TODO: check the old one?
    BT_ASSERT(index < BT_SETTINGS_KEY_MAX);
    int len = BT_KEYS_STORAGE_LEN;

    // TODO: length judge?
    return bt_storage_kv_get(KEY_INDEX_LE_KEY_INFO_ITEM(index), keys->storage_start, &len);
}

#if defined(CONFIG_BT_SETTINGS)
static void bt_storage_kv_key_store(struct bt_keys *keys)
{
    uint8_t id = keys->id;
    bt_addr_le_t *addr = &keys->addr;
    int select_index = -1;
    struct bt_storage_kv_key_list_header list_info;
    bt_storage_kv_get_key_list_info(&list_info);
    select_index = bt_storage_kv_get_key_list_info_index(&list_info, id, addr);

    if (select_index < 0)
    {
        select_index = 0;
    }

    bt_storage_kv_set_key_list_info_append(&list_info, id, addr, select_index);
    bt_storage_kv_set_key_item(select_index, keys);
}
#endif

__unused
static int bt_storage_kv_key_get(struct bt_keys *keys)
{
    uint8_t id = keys->id;
    bt_addr_le_t *addr = &keys->addr;
    struct bt_storage_kv_key_list_header list_info;
    bt_storage_kv_get_key_list_info(&list_info);
    int select_index = bt_storage_kv_get_key_list_info_index(&list_info, id, addr);

    if (select_index < 0)
    {
        return -1;
    }

    bt_storage_kv_get_key_item(select_index, keys);
    return 0;
}

#if defined(CONFIG_BT_SETTINGS)
static void bt_storage_kv_key_delete(struct bt_keys *keys)
{
    uint8_t id = keys->id;
    bt_addr_le_t *addr = &keys->addr;
    struct bt_storage_kv_key_list_header list_info;
    bt_storage_kv_get_key_list_info(&list_info);
    bt_storage_kv_set_key_list_info_delete(&list_info, id, addr);
}
#endif

#if IS_ENABLED(CONFIG_BT_KEYS_OVERWRITE_OLDEST)
static uint32_t aging_counter_val;
static struct bt_keys *last_keys_updated;

struct key_data
{
    bool in_use;
    uint8_t id;
};

static void find_key_in_use(struct bt_conn *conn, void *data)
{
    struct key_data *kdata = data;
    struct bt_keys *key;

    __ASSERT_NO_MSG(conn != NULL);
    __ASSERT_NO_MSG(data != NULL);

    if (conn->state == BT_CONN_CONNECTED)
    {
        key = bt_keys_find_addr(conn->id, bt_conn_get_dst(conn));
        if (key == NULL)
        {
            return;
        }

        /* Ensure that the reference returned matches the current pool item */
        if (key == &key_pool[kdata->id])
        {
            kdata->in_use = true;
            LOG_DBG("Connected device %s is using key_pool[%d]",
                    bt_addr_le_str(bt_conn_get_dst(conn)), kdata->id);
        }
    }
}

static bool key_is_in_use(uint8_t id)
{
    struct key_data kdata = {false, id};

    bt_conn_foreach(BT_CONN_TYPE_ALL, find_key_in_use, &kdata);

    return kdata.in_use;
}
#endif /* CONFIG_BT_KEYS_OVERWRITE_OLDEST */

struct bt_keys *bt_keys_get_addr(uint8_t id, const bt_addr_le_t *addr)
{
    struct bt_keys *keys;
    int i;
    size_t first_free_slot = ARRAY_SIZE(key_pool);

    __ASSERT_NO_MSG(addr != NULL);

    LOG_DBG("%s", bt_addr_le_str(addr));

    for (i = 0; i < ARRAY_SIZE(key_pool); i++)
    {
        keys = &key_pool[i];

        if (keys->id == id && bt_addr_le_eq(&keys->addr, addr))
        {
            return keys;
        }
        if (first_free_slot == ARRAY_SIZE(key_pool) && bt_addr_le_eq(&keys->addr, BT_ADDR_LE_ANY))
        {
            first_free_slot = i;
        }
    }

#if defined(CONFIG_BT_KEYS_OVERWRITE_OLDEST)
    if (first_free_slot == ARRAY_SIZE(key_pool))
    {
        struct bt_keys *oldest = NULL;
        bt_addr_le_t oldest_addr;

        for (i = 0; i < ARRAY_SIZE(key_pool); i++)
        {
            struct bt_keys *current = &key_pool[i];
            bool key_in_use = key_is_in_use(i);

            if (key_in_use)
            {
                continue;
            }

            if ((oldest == NULL) || (current->aging_counter < oldest->aging_counter))
            {
                oldest = current;
            }
        }

        if (oldest == NULL)
        {
            LOG_DBG("unable to create keys for %s", bt_addr_le_str(addr));
            return NULL;
        }

        /* Use a copy as bt_unpair will clear the oldest key. */
        bt_addr_le_copy(&oldest_addr, &oldest->addr);
        bt_unpair(oldest->id, &oldest_addr);
        if (bt_addr_le_eq(&oldest->addr, BT_ADDR_LE_ANY))
        {
            first_free_slot = oldest - &key_pool[0];
        }
    }

#endif /* CONFIG_BT_KEYS_OVERWRITE_OLDEST */
    if (first_free_slot < ARRAY_SIZE(key_pool))
    {
        keys = &key_pool[first_free_slot];
        keys->id = id;
        bt_addr_le_copy(&keys->addr, addr);
#if defined(CONFIG_BT_KEYS_OVERWRITE_OLDEST)
        keys->aging_counter = ++aging_counter_val;
        last_keys_updated = keys;
#endif /* CONFIG_BT_KEYS_OVERWRITE_OLDEST */
        LOG_DBG("created %p for %s", keys, bt_addr_le_str(addr));
        return keys;
    }

    LOG_DBG("unable to create keys for %s", bt_addr_le_str(addr));

    return NULL;
}

void bt_foreach_bond(uint8_t id, void (*func)(const struct bt_bond_info *info, void *user_data),
                     void *user_data)
{
    int i;

    __ASSERT_NO_MSG(func != NULL);

    for (i = 0; i < ARRAY_SIZE(key_pool); i++)
    {
        struct bt_keys *keys = &key_pool[i];

        if (keys->keys && keys->id == id)
        {
            struct bt_bond_info info;

            bt_addr_le_copy(&info.addr, &keys->addr);
            func(&info, user_data);
        }
    }
}

void bt_keys_foreach_type(enum bt_keys_type type, void (*func)(struct bt_keys *keys, void *data),
                          void *data)
{
    int i;

    __ASSERT_NO_MSG(func != NULL);

    for (i = 0; i < ARRAY_SIZE(key_pool); i++)
    {
        if ((key_pool[i].keys & type))
        {
            func(&key_pool[i], data);
        }
    }
}

struct bt_keys *bt_keys_find(enum bt_keys_type type, uint8_t id, const bt_addr_le_t *addr)
{
    int i;

    __ASSERT_NO_MSG(addr != NULL);

    LOG_DBG("type %d %s", type, bt_addr_le_str(addr));

    for (i = 0; i < ARRAY_SIZE(key_pool); i++)
    {
        if ((key_pool[i].keys & type) && key_pool[i].id == id &&
            bt_addr_le_eq(&key_pool[i].addr, addr))
        {
            return &key_pool[i];
        }
    }

    return NULL;
}

struct bt_keys *bt_keys_get_type(enum bt_keys_type type, uint8_t id, const bt_addr_le_t *addr)
{
    struct bt_keys *keys;

    __ASSERT_NO_MSG(addr != NULL);

    LOG_DBG("type %d %s", type, bt_addr_le_str(addr));

    keys = bt_keys_find(type, id, addr);
    if (keys)
    {
        return keys;
    }

    keys = bt_keys_get_addr(id, addr);
    if (!keys)
    {
        return NULL;
    }

    bt_keys_add_type(keys, type);

    return keys;
}

struct bt_keys *bt_keys_find_irk(uint8_t id, const bt_addr_le_t *addr)
{
    int i;

    __ASSERT_NO_MSG(addr != NULL);

    LOG_DBG("%s", bt_addr_le_str(addr));

    if (!bt_addr_le_is_rpa(addr))
    {
        return NULL;
    }

    for (i = 0; i < ARRAY_SIZE(key_pool); i++)
    {
        if (!(key_pool[i].keys & BT_KEYS_IRK))
        {
            continue;
        }

        if (key_pool[i].id == id && !bt_addr_cmp(&addr->a, &key_pool[i].irk.rpa))
        {
            LOG_DBG("cached RPA %s for %s", bt_addr_str(&key_pool[i].irk.rpa),
                    bt_addr_le_str(&key_pool[i].addr));
            return &key_pool[i];
        }
    }

    for (i = 0; i < ARRAY_SIZE(key_pool); i++)
    {
        if (!(key_pool[i].keys & BT_KEYS_IRK))
        {
            continue;
        }

        if (key_pool[i].id != id)
        {
            continue;
        }

        if (bt_rpa_irk_matches(key_pool[i].irk.val, &addr->a))
        {
            LOG_DBG("RPA %s matches %s", bt_addr_str(&key_pool[i].irk.rpa),
                    bt_addr_le_str(&key_pool[i].addr));

            bt_addr_copy(&key_pool[i].irk.rpa, &addr->a);

            return &key_pool[i];
        }
    }

    LOG_DBG("No IRK for %s", bt_addr_le_str(addr));

    return NULL;
}

struct bt_keys *bt_keys_find_addr(uint8_t id, const bt_addr_le_t *addr)
{
    int i;

    __ASSERT_NO_MSG(addr != NULL);

    LOG_DBG("%s", bt_addr_le_str(addr));

    for (i = 0; i < ARRAY_SIZE(key_pool); i++)
    {
        if (key_pool[i].id == id && bt_addr_le_eq(&key_pool[i].addr, addr))
        {
            return &key_pool[i];
        }
    }

    return NULL;
}

void bt_keys_add_type(struct bt_keys *keys, enum bt_keys_type type)
{
    __ASSERT_NO_MSG(keys != NULL);

    keys->keys |= type;
}

void bt_keys_clear(struct bt_keys *keys)
{
    __ASSERT_NO_MSG(keys != NULL);

    LOG_DBG("%s (keys 0x%04x)", bt_addr_le_str(&keys->addr), keys->keys);

    if (keys->state & BT_KEYS_ID_ADDED)
    {
        bt_id_del(keys);
    }

#if defined(CONFIG_BT_SETTINGS)
    // LOG_DBG("Deleting key %s", key);
    bt_storage_kv_key_delete(keys);
#endif

    (void)memset(keys, 0, sizeof(*keys));
}

#if defined(CONFIG_BT_SETTINGS)
int bt_keys_store(struct bt_keys *keys)
{
    bt_storage_kv_key_store(keys);

    LOG_DBG("Stored keys for %s", bt_addr_le_str(&keys->addr));

    return 0;
}



int bt_keys_loading(void)
{
    char addr_str[BT_ADDR_LE_STR_LEN];
    uint8_t id;
    bt_addr_le_t *addr;
    int index;
    struct bt_keys *keys;
    struct bt_storage_kv_key_list_header list_info;
    bt_storage_kv_get_key_list_info(&list_info);
    LOG_INF("Load key info, total cnt: %d", list_info.cnt);
    for (int i = 0; i < list_info.cnt; i++)
    {
        id = list_info.items[i].id;
        addr = &list_info.items[i].addr;
        index = list_info.items[i].index;
        bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
        LOG_INF("Load list key info, id: %d, addr: %s, index: %d", id, addr_str, index);
        keys = bt_keys_get_addr(id, addr);
        if (!keys)
        {
            LOG_ERR("Failed to allocate keys for %s", bt_addr_le_str(addr));
            return -ENOMEM;
        }
        bt_storage_kv_get_key_item(index, keys);
    }

    return 0;
}

static void id_add(struct bt_keys *keys, void *user_data)
{
    __ASSERT_NO_MSG(keys != NULL);

    bt_id_add(keys);
}
#endif /* CONFIG_BT_SETTINGS */

#if defined(CONFIG_BT_KEYS_OVERWRITE_OLDEST)
void bt_keys_update_usage(uint8_t id, const bt_addr_le_t *addr)
{
    __ASSERT_NO_MSG(addr != NULL);

    struct bt_keys *keys = bt_keys_find_addr(id, addr);

    if (!keys)
    {
        return;
    }

    if (last_keys_updated == keys)
    {
        return;
    }

    keys->aging_counter = ++aging_counter_val;
    last_keys_updated = keys;

    LOG_DBG("Aging counter for %s is set to %u", bt_addr_le_str(addr), keys->aging_counter);

    if (IS_ENABLED(CONFIG_BT_KEYS_SAVE_AGING_COUNTER_ON_PAIRING))
    {
        bt_keys_store(keys);
    }
}

#endif /* CONFIG_BT_KEYS_OVERWRITE_OLDEST */

#if defined(CONFIG_BT_LOG_SNIFFER_INFO)
void bt_keys_show_sniffer_info(struct bt_keys *keys, void *data)
{
    uint8_t ltk[16];

    __ASSERT_NO_MSG(keys != NULL);

    if (keys->keys & BT_KEYS_LTK_P256)
    {
        sys_memcpy_swap(ltk, keys->ltk.val, keys->enc_size);
        LOG_INF("SC LTK: 0x%s", bt_hex(ltk, keys->enc_size));
    }

#if !defined(CONFIG_BT_SMP_SC_PAIR_ONLY)
    if (keys->keys & BT_KEYS_PERIPH_LTK)
    {
        sys_memcpy_swap(ltk, keys->periph_ltk.val, keys->enc_size);
        LOG_INF("Legacy LTK: 0x%s (peripheral)", bt_hex(ltk, keys->enc_size));
    }
#endif /* !CONFIG_BT_SMP_SC_PAIR_ONLY */

    if (keys->keys & BT_KEYS_LTK)
    {
        sys_memcpy_swap(ltk, keys->ltk.val, keys->enc_size);
        LOG_INF("Legacy LTK: 0x%s (central)", bt_hex(ltk, keys->enc_size));
    }
}
#endif /* defined(CONFIG_BT_LOG_SNIFFER_INFO) */
#endif /* defined(CONFIG_BT_SMP) */
