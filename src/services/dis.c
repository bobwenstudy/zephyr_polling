/** @file
 *  @brief GATT Device Information Service
 */

/*
 * Copyright (c) 2019 Demant
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "bt_config.h"

#include "base/types.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/uuid.h>

#define LOG_MODULE_NAME dis
#include "logging/bt_log.h"

#if defined(CONFIG_BT_DIS)

#if CONFIG_BT_DIS_PNP
struct dis_pnp
{
    uint8_t pnp_vid_src;
    uint16_t pnp_vid;
    uint16_t pnp_pid;
    uint16_t pnp_ver;
} __packed;

static struct dis_pnp dis_pnp_id = {
        .pnp_vid_src = CONFIG_BT_DIS_PNP_VID_SRC,
        .pnp_vid = CONFIG_BT_DIS_PNP_VID,
        .pnp_pid = CONFIG_BT_DIS_PNP_PID,
        .pnp_ver = CONFIG_BT_DIS_PNP_VER,
};
#endif

static uint8_t dis_model[CONFIG_BT_DIS_STR_MAX] = CONFIG_BT_DIS_MODEL;
static uint8_t dis_manuf[CONFIG_BT_DIS_STR_MAX] = CONFIG_BT_DIS_MANUF;
#if defined(CONFIG_BT_DIS_SERIAL_NUMBER)
static uint8_t dis_serial_number[CONFIG_BT_DIS_STR_MAX] = CONFIG_BT_DIS_SERIAL_NUMBER_STR;
#endif
#if defined(CONFIG_BT_DIS_FW_REV)
static uint8_t dis_fw_rev[CONFIG_BT_DIS_STR_MAX] = CONFIG_BT_DIS_FW_REV_STR;
#endif
#if defined(CONFIG_BT_DIS_HW_REV)
static uint8_t dis_hw_rev[CONFIG_BT_DIS_STR_MAX] = CONFIG_BT_DIS_HW_REV_STR;
#endif
#if defined(CONFIG_BT_DIS_SW_REV)
static uint8_t dis_sw_rev[CONFIG_BT_DIS_STR_MAX] = CONFIG_BT_DIS_SW_REV_STR;
#endif

#define BT_DIS_MODEL_REF             dis_model
#define BT_DIS_MANUF_REF             dis_manuf
#define BT_DIS_SERIAL_NUMBER_STR_REF dis_serial_number
#define BT_DIS_FW_REV_STR_REF        dis_fw_rev
#define BT_DIS_HW_REV_STR_REF        dis_hw_rev
#define BT_DIS_SW_REV_STR_REF        dis_sw_rev

static ssize_t read_str(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                        uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data,
                             strlen(attr->user_data));
}

#if CONFIG_BT_DIS_PNP
static ssize_t read_pnp_id(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                           uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &dis_pnp_id, sizeof(dis_pnp_id));
}
#endif

/* Device Information Service Declaration */
BT_GATT_SERVICE_DEFINE(
        dis_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_DIS),

        BT_GATT_CHARACTERISTIC(BT_UUID_DIS_MODEL_NUMBER, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
                               read_str, NULL, BT_DIS_MODEL_REF),
        BT_GATT_CHARACTERISTIC(BT_UUID_DIS_MANUFACTURER_NAME, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
                               read_str, NULL, BT_DIS_MANUF_REF),
#if CONFIG_BT_DIS_PNP
        BT_GATT_CHARACTERISTIC(BT_UUID_DIS_PNP_ID, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
                               read_pnp_id, NULL, &dis_pnp_id),
#endif

#if defined(CONFIG_BT_DIS_SERIAL_NUMBER)
        BT_GATT_CHARACTERISTIC(BT_UUID_DIS_SERIAL_NUMBER, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
                               read_str, NULL, BT_DIS_SERIAL_NUMBER_STR_REF),
#endif
#if defined(CONFIG_BT_DIS_FW_REV)
        BT_GATT_CHARACTERISTIC(BT_UUID_DIS_FIRMWARE_REVISION, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
                               read_str, NULL, BT_DIS_FW_REV_STR_REF),
#endif
#if defined(CONFIG_BT_DIS_HW_REV)
        BT_GATT_CHARACTERISTIC(BT_UUID_DIS_HARDWARE_REVISION, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
                               read_str, NULL, BT_DIS_HW_REV_STR_REF),
#endif
#if defined(CONFIG_BT_DIS_SW_REV)
        BT_GATT_CHARACTERISTIC(BT_UUID_DIS_SOFTWARE_REVISION, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
                               read_str, NULL, BT_DIS_SW_REV_STR_REF),
#endif

);
#endif
