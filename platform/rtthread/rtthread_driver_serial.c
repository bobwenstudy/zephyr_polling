#include <stdio.h>
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "rtthread_driver_serial.h"
#include "drivers/hci_driver.h"
#include "drivers/hci_h4.h"

#include "logging/bt_log_impl.h"

#include "common/bt_buf.h"

struct hci_trans_h4_uart_config {
    const char *device_name;    /* Uart device name, i.e. "uart1" */
    int parity;                 /* Parity,           i.e. PARITY_NONE */
    int stopbit;                /* Stop bit,         i.e. STOP_BITS_1 */
    int databit;                /* Data bit,         i.e. DATA_BITS_8 */
    uint32_t baudrate;          /* Bautdate,         i.e. BAUD_RATE_115200 */
    int flowcontrol;            /* Flow Control,     i.e. 1 */
};

struct h4_uart_config{
    const char *name;
    int flowcontrol;        // RT-Thread not support CTS/RTS flowcontrol now, default is true.
    struct serial_configure rt_config;
};

static struct h4_uart_config uart_config = {
    .rt_config = RT_SERIAL_CONFIG_DEFAULT,
};

static rt_device_t h4_uart;

static int hci_driver_h4_open(void)
{
    RT_ASSERT(uart_config.name);

    printk("hci_driver_h4_open, uart_config.name: %s\n", uart_config.name);

    h4_uart = rt_device_find(uart_config.name);
//    printk("hci_driver_h4_open, h4_uart: 0x%x\n", h4_uart);
    RT_ASSERT(h4_uart);

    rt_err_t err;

    if ((err = rt_device_open(h4_uart, RT_DEVICE_FLAG_INT_RX))) {
        printk("Open h4_uart error\n");
        return -1;
    }
    if ((err = rt_device_control(h4_uart, RT_DEVICE_CTRL_CONFIG, &uart_config.rt_config))) {
        printk("Control h4_uart error\n");
        return -1;
    }

    return 0;
}

static int hci_driver_h4_send(uint8_t *buf, uint16_t len)
{
    return rt_device_write(h4_uart, 0, buf, len);
}

static int hci_driver_h4_recv(uint8_t *buf, uint16_t len)
{
    return rt_device_read(h4_uart, 0, buf, len);
}

static const struct bt_hci_h4_driver h4_drv = {
        .open = hci_driver_h4_open,
        .send = hci_driver_h4_send,
        .recv = hci_driver_h4_recv,
};

static void hci_driver_h4_init(void)
{
    hci_h4_init(&h4_drv);
}



static char device_name[10];
int uart_init_process(int idx, int rate, int databits, int stopbits, int parity, bool flowcontrol)
{
    printk("uart_init_process idx: %d, rate: %d, databits: %d, stopbits: %d, parity: %d, flowcontrol: %d\n",
           idx, rate, databits, stopbits, parity, flowcontrol);

    rt_sprintf(device_name, "uart%d", idx);

    uart_config.name        = device_name;
    uart_config.flowcontrol = flowcontrol;

    uart_config.rt_config.baud_rate   = rate;
    uart_config.rt_config.data_bits   = databits;
    uart_config.rt_config.stop_bits   = stopbits;
    uart_config.rt_config.parity      = parity;

#if defined(RTTHREAD_VERSION) && RTTHREAD_VERSION > 40003    //< rtthread version larger than v4.0.3
    uart_config.rt_config.flowcontrol = flowcontrol;
#endif

    return 0;
}

int bt_hci_init_serial_device(int idx, int rate, int databits, int stopbits, int parity, bool flowcontrol)
{
    int ret = uart_init_process(idx, rate, databits, stopbits, parity, flowcontrol);
    if (ret < 0)
    {
        return ret;
    }

    hci_driver_h4_init();

    return (0);
}
