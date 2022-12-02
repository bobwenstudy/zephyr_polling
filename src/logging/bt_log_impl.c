#include "bt_log_impl.h"

#include <stdarg.h>

static const bt_log_impl_t *bt_log_implementation;

void bt_log_impl_printf(uint8_t level, const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    (*bt_log_implementation->printf)(level, format, argptr);
    va_end(argptr);
}

void bt_log_impl_packet(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len)
{
    (*bt_log_implementation->packet)(packet_type, in, packet, len);
}

void bt_log_impl_init(void)
{
    (*bt_log_implementation->init)();
}

void bt_log_impl_register(const bt_log_impl_t *log_impl)
{
    bt_log_implementation = log_impl;
    bt_log_impl_init();
}
