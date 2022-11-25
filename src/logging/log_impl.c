#include "log_impl.h"

#include <stdarg.h>

static const log_impl_t *log_implementation;

void log_impl_printf(uint8_t level, const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    (*log_implementation->printf)(level, format, argptr);
    va_end(argptr);
}

void log_impl_packet(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len)
{
    (*log_implementation->packet)(packet_type, in, packet, len);
}

void log_impl_init(void)
{
    (*log_implementation->init)();
}

void log_impl_register(const log_impl_t *log_impl)
{
    log_implementation = log_impl;
    log_impl_init();
}
