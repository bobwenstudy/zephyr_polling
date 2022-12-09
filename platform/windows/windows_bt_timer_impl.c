#include "base\byteorder.h"
#include "common\timer.h"
#include "host\hci_core.h"

#include <stdio.h>

#include <pthread.h>
#include <windows.h>

// start time.
static ULARGE_INTEGER last_time;

uint32_t timer_get_delay_time_ms(void)
{
    FILETIME file_time;
    SYSTEMTIME system_time;
    ULARGE_INTEGER now_time;
    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    now_time.LowPart = file_time.dwLowDateTime;
    now_time.HighPart = file_time.dwHighDateTime;
    uint32_t time_ms = (uint32_t)((now_time.QuadPart - last_time.QuadPart) / 10000);

    last_time.LowPart = now_time.LowPart;
    last_time.HighPart = now_time.HighPart;

    // printf("timer_get_delay_time_ms: %u\n", time_ms);
    return time_ms;
}

pthread_t timer_thread;
static int timer_process_loop(void *args)
{
    // printk("timer_process_loop\n");

    while (1)
    {
        sys_clock_announce(timer_get_delay_time_ms());
#if defined(CONFIG_BT_MONITOR_SLEEP)
        bt_sleep_wakeup_with_timeout();
#endif
        Sleep(1);

        // if(!is_enable)
        // {
        //     break;
        // }
    }
    return 0;
}

void bt_timer_impl_local_init(void)
{
    FILETIME file_time;
    SYSTEMTIME system_time;
    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    last_time.LowPart = file_time.dwLowDateTime;
    last_time.HighPart = file_time.dwHighDateTime;

    pthread_create(&timer_thread, NULL, (void *)timer_process_loop, NULL);

    sys_clock_announce(0);
}
