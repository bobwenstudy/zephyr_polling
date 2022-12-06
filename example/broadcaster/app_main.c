#include <stddef.h>
#include <stdio.h>

#include "base/types.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <drivers/hci_driver.h>
#include <logging/bt_log_impl.h>
#include "common\timer.h"

static uint8_t mfg_data[] = {0xff, 0xff, 0x00};

static const struct bt_data ad[] = {
        BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, 3),
};

struct k_timer adv_work_timer;

void start_adv_process(void)
{
    int err;
    printk("Sending advertising data: 0x%02X\n", mfg_data[2]);

    /* Start advertising */
    err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err)
    {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }
}

void stop_adv_process(void)
{
    int err;

    err = bt_le_adv_stop();
    if (err)
    {
        printk("Advertising failed to stop (err %d)\n", err);
        return;
    }

    mfg_data[2]++;
}

uint8_t current_work_state;
void adv_work_timer_expiry(struct k_timer *timer)
{
    current_work_state++;
    printk("adv_work_timer_expiry, current_work_state: %d\n", current_work_state);

    if (current_work_state % 0x02 == 0)
    {
        start_adv_process();
    }
    else
    {
        stop_adv_process();
    }
}

void bt_ready(int err)
{
    char addr_s[BT_ADDR_LE_STR_LEN];
    bt_addr_le_t addr = {0};
    size_t count = 1;

    if (err)
    {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    k_timer_init(&adv_work_timer, adv_work_timer_expiry, NULL);
    k_timer_start(&adv_work_timer, K_SECONDS(1), K_SECONDS(1));
}

void app_polling_work(void)
{
    return;
}