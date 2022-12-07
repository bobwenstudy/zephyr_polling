#include <logging/bt_log_impl.h>

void bt_ready(int err)
{
    if (err)
    {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");
}

void app_polling_work(void)
{
    return;
}