#include "base/byteorder.h"
#include "common/timer.h"
#include "host/hci_core.h"

#include "logging/bt_log_impl.h"

#include <stdio.h>
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

// start time.
rt_tick_t last_time;
static void zephyr_polling_timer_timeout(void *args)
{
    rt_tick_t cur_time = rt_tick_get();
    uint32_t diff = (cur_time - last_time) * (1000u / RT_TICK_PER_SECOND);
    sys_clock_announce(diff);
#if defined(CONFIG_BT_MONITOR_SLEEP)
    bt_sleep_wakeup_with_timeout();
#endif

    last_time = cur_time;
}

static struct rt_timer zephyr_polling_timer;
void bt_timer_impl_local_init(void)
{
    last_time = rt_tick_get();

    rt_timer_init(&zephyr_polling_timer
        , "zephyr_polling_timer"
        , zephyr_polling_timer_timeout
        , 0
        , rt_tick_from_millisecond(10)
        , RT_TIMER_FLAG_PERIODIC);

    rt_timer_start(&zephyr_polling_timer);

    sys_clock_announce(0);
}
