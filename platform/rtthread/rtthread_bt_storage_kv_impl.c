#include <stdio.h>

#include "rtthread_bt_storage_kv_impl.h"

#include "base/byteorder.h"
#include "base/util.h"
#include "logging/bt_log_impl.h"


static void init_list(struct bt_storage_kv_header *list, uint16_t list_cnt)
{
    // TODO: Do nothing.
}

static int get(uint16_t key, uint8_t *data, uint16_t *len)
{
    printk("get: key: 0x%x\n", key);

    return 0;
}

static void set(uint16_t key, uint8_t *data, uint16_t len)
{
    printk("set: key: 0x%x, len: %d\n", key, len);
}

static void delete (uint16_t key, uint8_t *data, uint16_t len)
{
    printk("delete: key: 0x%x\n", key);
}

static const struct bt_storage_kv_impl kv_impl = {
        init_list,
        get,
        set,
        delete,
};

const struct bt_storage_kv_impl *bt_storage_kv_impl_local_instance(void)
{
    return &kv_impl;
}
