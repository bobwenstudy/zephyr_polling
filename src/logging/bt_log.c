/* log.c - logging helpers */

/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Helper for printk parameters to convert from binary to hex.
 * We declare multiple buffers so the helper can be used multiple times
 * in a single printk call.
 */

#include <stddef.h>

#include "bt_config.h"

#include "base/common.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <bluetooth/hci.h>

#include <stdarg.h>

typedef int (*out_func_t)(int c, void *ctx);

enum pad_type
{
    PAD_NONE,
    PAD_ZERO_BEFORE,
    PAD_SPACE_BEFORE,
    PAD_SPACE_AFTER,
};

#ifdef CONFIG_PRINTK64
typedef uint64_t printk_val_t;
#else
typedef uint32_t printk_val_t;
#endif

/* Maximum number of digits in a printed decimal value (hex is always
 * less, obviously).  Funny formula produces 10 max digits for 32 bit,
 * 21 for 64.
 */
#define DIGITS_BUFLEN (11U * (sizeof(printk_val_t) / 4U) - 1U)

struct str_context
{
    char *str;
    int max;
    int count;
};

const char *bt_hex_real(const void *buf, size_t len)
{
    static const char hex[] = "0123456789abcdef";
    static char str[129] __noretention_data_section;
    const uint8_t *b = buf;
    size_t i;

    len = MIN(len, (sizeof(str) - 1) / 2);

    for (i = 0; i < len; i++)
    {
        str[i * 2] = hex[b[i] >> 4];
        str[i * 2 + 1] = hex[b[i] & 0xf];
    }

    str[i * 2] = '\0';

    return str;
}

const char *bt_addr_str_real(const bt_addr_t *addr)
{
    static char str[BT_ADDR_STR_LEN] __noretention_data_section;

    bt_addr_to_str(addr, str, sizeof(str));

    return str;
}

const char *bt_addr_le_str_real(const bt_addr_le_t *addr)
{
    static char str[BT_ADDR_LE_STR_LEN] __noretention_data_section;

    bt_addr_le_to_str(addr, str, sizeof(str));

    return str;
}

const char *bt_uuid_str_real(const struct bt_uuid *uuid)
{
    static char str[BT_UUID_STR_LEN] __noretention_data_section;

    bt_uuid_to_str(uuid, str, sizeof(str));

    return str;
}

static void print_digits(out_func_t out, void *ctx, printk_val_t num, unsigned int base,
                         bool pad_before, char pad_char, int min_width)
{
    char buf[DIGITS_BUFLEN];
    unsigned int i;

    /* Print it backwards into the end of the buffer, low digits first */
    for (i = DIGITS_BUFLEN - 1U; num != 0U; i--)
    {
        buf[i] = "0123456789abcdef"[num % base];
        num /= base;
    }

    if (i == DIGITS_BUFLEN - 1U)
    {
        buf[i] = '0';
    }
    else
    {
        i++;
    }

    int pad = MAX(min_width - (int)(DIGITS_BUFLEN - i), 0);

    for (/**/; pad > 0 && pad_before; pad--)
    {
        out(pad_char, ctx);
    }
    for (/**/; i < DIGITS_BUFLEN; i++)
    {
        out(buf[i], ctx);
    }
    for (/**/; pad > 0; pad--)
    {
        out(pad_char, ctx);
    }
}

static void print_hex(out_func_t out, void *ctx, printk_val_t num, enum pad_type padding,
                      int min_width)
{
    print_digits(out, ctx, num, 16U, padding != PAD_SPACE_AFTER,
                 padding == PAD_ZERO_BEFORE ? '0' : ' ', min_width);
}

static void print_dec(out_func_t out, void *ctx, printk_val_t num, enum pad_type padding,
                      int min_width)
{
    print_digits(out, ctx, num, 10U, padding != PAD_SPACE_AFTER,
                 padding == PAD_ZERO_BEFORE ? '0' : ' ', min_width);
}

static bool ok64(out_func_t out, void *ctx, long long val)
{
    if (sizeof(printk_val_t) < 8U && val != (long)val)
    {
        out('E', ctx);
        out('R', ctx);
        out('R', ctx);
        return false;
    }
    return true;
}

static bool negative(printk_val_t val)
{
    const printk_val_t hibit = ~(((printk_val_t)~1) >> 1U);

    return (val & hibit) != 0U;
}

/**
 * @brief Printk internals
 *
 * See printk() for description.
 * @param fmt Format string
 * @param ap Variable parameters
 *
 * @return N/A
 */
void z_vprintk(out_func_t out, void *ctx, const char *fmt, va_list ap)
{
    int might_format = 0; /* 1 if encountered a '%' */
    enum pad_type padding = PAD_NONE;
    int min_width = -1;
    char length_mod = 0;

    /* fmt has already been adjusted if needed */

    while (*fmt)
    {
        if (!might_format)
        {
            if (*fmt != '%')
            {
                out((int)*fmt, ctx);
            }
            else
            {
                might_format = 1;
                min_width = -1;
                padding = PAD_NONE;
                length_mod = 0;
            }
        }
        else
        {
            switch (*fmt)
            {
            case '-':
                padding = PAD_SPACE_AFTER;
                goto still_might_format;
            case '0':
                if (min_width < 0 && padding == PAD_NONE)
                {
                    padding = PAD_ZERO_BEFORE;
                    goto still_might_format;
                }
                __fallthrough;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (min_width < 0)
                {
                    min_width = *fmt - '0';
                }
                else
                {
                    min_width = 10 * min_width + *fmt - '0';
                }

                if (padding == PAD_NONE)
                {
                    padding = PAD_SPACE_BEFORE;
                }
                goto still_might_format;
            case 'h':
            case 'l':
            case 'z':
                if (*fmt == 'h' && length_mod == 'h')
                {
                    length_mod = 'H';
                }
                else if (*fmt == 'l' && length_mod == 'l')
                {
                    length_mod = 'L';
                }
                else if (length_mod == 0)
                {
                    length_mod = *fmt;
                }
                else
                {
                    out((int)'%', ctx);
                    out((int)*fmt, ctx);
                    break;
                }
                goto still_might_format;
            case 'd':
            case 'i':
            case 'u':
            {
                printk_val_t d;

                if (length_mod == 'z')
                {
                    d = va_arg(ap, ssize_t);
                }
                else if (length_mod == 'l')
                {
                    d = va_arg(ap, long);
                }
                else if (length_mod == 'L')
                {
                    long long lld = va_arg(ap, long long);
                    if (!ok64(out, ctx, lld))
                    {
                        break;
                    }
                    d = (printk_val_t)lld;
                }
                else if (*fmt == 'u')
                {
                    d = va_arg(ap, unsigned int);
                }
                else
                {
                    d = va_arg(ap, int);
                }

                if (*fmt != 'u' && negative(d))
                {
                    out((int)'-', ctx);
                    d = -d;
                    min_width--;
                }
                print_dec(out, ctx, d, padding, min_width);
                break;
            }
            case 'p':
                out('0', ctx);
                out('x', ctx);
                /* left-pad pointers with zeros */
                padding = PAD_ZERO_BEFORE;
                min_width = sizeof(void *) * 2U;
                __fallthrough;
            case 'x':
            case 'X':
            {
                printk_val_t x;

                if (*fmt == 'p')
                {
                    x = (uintptr_t)va_arg(ap, void *);
                }
                else if (length_mod == 'l')
                {
                    x = va_arg(ap, unsigned long);
                }
                else if (length_mod == 'L')
                {
                    x = va_arg(ap, unsigned long long);
                }
                else
                {
                    x = va_arg(ap, unsigned int);
                }

                print_hex(out, ctx, x, padding, min_width);
                break;
            }
            case 's':
            {
                char *s = va_arg(ap, char *);
                char *start = s;

                while (*s)
                {
                    out((int)(*s++), ctx);
                }

                if (padding == PAD_SPACE_AFTER)
                {
                    int remaining = min_width - (s - start);
                    while (remaining-- > 0)
                    {
                        out(' ', ctx);
                    }
                }
                break;
            }
            case 'c':
            {
                int c = va_arg(ap, int);

                out(c, ctx);
                break;
            }
            case '%':
            {
                out((int)'%', ctx);
                break;
            }
            default:
                out((int)'%', ctx);
                out((int)*fmt, ctx);
                break;
            }
            might_format = 0;
        }
    still_might_format:
        ++fmt;
    }
}

static int str_out(int c, struct str_context *ctx)
{
    if (ctx->str == NULL || ctx->count >= ctx->max)
    {
        ctx->count++;
        return c;
    }

    if (ctx->count == ctx->max - 1)
    {
        ctx->str[ctx->count++] = '\0';
    }
    else
    {
        ctx->str[ctx->count++] = c;
    }

    return c;
}

int vsnprintk(char *str, size_t size, const char *fmt, va_list ap)
{
    struct str_context ctx = {str, size, 0};

    z_vprintk((out_func_t)str_out, &ctx, fmt, ap);

    if (ctx.count < ctx.max)
    {
        str[ctx.count] = '\0';
    }

    return ctx.count;
}

int snprintk(char *str, size_t size, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vsnprintk(str, size, fmt, ap);
    va_end(ap);

    return ret;
}

void log_hex_dump(char *str, size_t out_len, const void *buf, size_t len)
{
    static const char hex[] = "0123456789ABCDEF";
    const uint8_t *b = buf;
    size_t i;

    len = MIN(len, (out_len - 1) / 3);

    for (i = 0; i < len; i++)
    {
        str[i * 3] = hex[b[i] >> 4];
        str[i * 3 + 1] = hex[b[i] & 0xf];
        if (i != len - 1)
        {
            str[i * 3 + 2] = ' ';
        }
    }

    str[i * 3 - 1] = '\0';
}
