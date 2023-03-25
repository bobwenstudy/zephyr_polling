#include <rtthread.h>
#include <rthw.h>

#include "rtthread_bt_log_impl.h"

#include "base/byteorder.h"
#include "logging/bt_log_impl.h"
#include "logging/bt_log.h"


#define H4_CMD 0x01
#define H4_ACL 0x02
#define H4_SCO 0x03
#define H4_EVT 0x04
#define H4_ISO 0x05

#define LOG_FILE_PRINT_BUFFER_MAX_LENGTH (0x100)

__unused
static const char *get_packet_type_str(uint8_t packet_type, uint8_t in)
{
    switch (packet_type)
    {
    case H4_CMD:
        return ("CMD => ");
        break;
    case H4_EVT:
        return ("EVT <= ");
        break;
    case H4_ACL:
        if (in)
        {
            return ("ACL <= ");
        }
        else
        {
            return ("ACL => ");
        }
        break;
    case H4_SCO:
        if (in)
        {
            return ("SCO <= ");
        }
        else
        {
            return ("SCO => ");
        }
        break;
    case H4_ISO:
        if (in)
        {
            return ("ISO <= ");
        }
        else
        {
            return ("ISO => ");
        }
        break;
    default:
        return "";
    }
}

static void log_printf_dump(uint8_t level, const char *format, va_list argptr)
{
    char log_printf_buffer[LOG_FILE_PRINT_BUFFER_MAX_LENGTH];

    rt_vsnprintf(log_printf_buffer, sizeof(log_printf_buffer), format,
                             argptr);
    
    rt_kprintf(log_printf_buffer);
}

static void log_packet_dump(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len)
{
#if 0
    char log_printf_buffer[LOG_FILE_PRINT_BUFFER_MAX_LENGTH];
    const char *packet_type_str = get_packet_type_str(packet_type, in);

    char msg_str[LOG_FILE_PRINT_BUFFER_MAX_LENGTH];
    log_hex_dump(msg_str, sizeof(msg_str), packet, len);

    rt_snprintf(log_printf_buffer, sizeof(log_printf_buffer), "%s %s\n",
                             packet_type_str, msg_str);

    rt_kprintf(log_printf_buffer);
#endif
}

static void log_point_dump(uint32_t point)
{
}

static void log_init(void)
{

}

static const bt_log_impl_t log_impl = {
        log_init,
        log_packet_dump,
        log_printf_dump,
        log_point_dump,
};

const bt_log_impl_t *bt_log_impl_local_instance(void)
{
    return &log_impl;
}
