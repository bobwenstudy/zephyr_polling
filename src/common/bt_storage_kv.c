/**
 * @file rpa.c
 * Resolvable Private Address Generation and Resolution
 */

/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "bt_storage_kv.h"

#include <base/types.h>

#define LOG_MODULE_NAME bt_storage_kv
#include "logging/bt_log.h"


static const struct bt_storage_kv_impl *kv_obj;

void bt_storage_kv_init_list(struct bt_storage_kv_header *list, uint16_t list_cnt)
{
    (*kv_obj->init_list)(list, list_cnt);
}

int bt_storage_kv_get(uint16_t key, uint8_t *data, int *len)
{
    return (*kv_obj->get)(key, data, len);
}

void bt_storage_kv_set(uint16_t key, uint8_t *data, int len)
{
    (*kv_obj->set)(key, data, len);
}

void bt_storage_kv_delete(uint16_t key, uint8_t *data, int len)
{
    (*kv_obj->delete)(key, data, len);
}

void bt_storage_kv_register(const struct bt_storage_kv_impl *impl)
{
    kv_obj = impl;
}