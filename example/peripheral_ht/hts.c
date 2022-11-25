/** @file
 *  @brief HTS Service sample
 */

/*
 * Copyright (c) 2020 SixOctets Systems
 * Copyright (c) 2019 Aaron Tsui <aaron.tsui@outlook.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "base/byteorder.h"
#include "base/types.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/uuid.h>
#include <logging/log_impl.h>

static uint8_t simulate_htm;
static uint8_t indicating;
static struct bt_gatt_indicate_params ind_params;

static void htmc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    simulate_htm = (value == BT_GATT_CCC_INDICATE) ? 1 : 0;
}

static void indicate_cb(struct bt_conn *conn, struct bt_gatt_indicate_params *params, uint8_t err)
{
    printk("Indication %s\n", err != 0U ? "fail" : "success");
    indicating = 0U;
}

static void indicate_destroy(struct bt_gatt_indicate_params *params)
{
    printk("Indication complete\n");
    indicating = 0U;
}

/* Health Thermometer Service Declaration */
BT_GATT_SERVICE_DEFINE(hts_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_HTS),
                       BT_GATT_CHARACTERISTIC(BT_UUID_HTS_MEASUREMENT, BT_GATT_CHRC_INDICATE,
                                              BT_GATT_PERM_NONE, NULL, NULL, NULL),
                       BT_GATT_CCC(htmc_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
                       /* more optional Characteristics */
);

void hts_init(void)
{
    printk("no temperature device; using simulated data\n");
}

void hts_indicate(void)
{
    /* Temperature measurements simulation */

    if (simulate_htm)
    {
        static uint8_t htm[5];
        static double temperature = 20U;
        uint32_t mantissa;
        uint8_t exponent;
        int r;

        if (indicating)
        {
            return;
        }

        temperature++;
        if (temperature == 30U)
        {
            temperature = 20U;
        }

        printk("temperature is %gC\n", temperature);

        mantissa = (uint32_t)(temperature * 100);
        exponent = (uint8_t)-2;

        htm[0] = 0; /* temperature in celsius */
        sys_put_le24(mantissa, (uint8_t *)&htm[1]);
        htm[4] = exponent;

        ind_params.attr = &hts_svc.attrs[2];
        ind_params.func = indicate_cb;
        ind_params.destroy = indicate_destroy;
        ind_params.data = &htm;
        ind_params.len = sizeof(htm);

        if (bt_gatt_indicate(NULL, &ind_params) == 0)
        {
            indicating = 1U;
        }
    }
}
