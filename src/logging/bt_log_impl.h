#ifndef _ZEPHYR_POLLING_LOGGING_LOG_IMPL_H_
#define _ZEPHYR_POLLING_LOGGING_LOG_IMPL_H_

#include <stddef.h>

#include "bt_config.h"

#include "base/types.h"
#include "base/common.h"

#define FUNCTION_CONTROL_DEBUG_ENABLE

#define LOG_IMPL_LEVEL_NONE 0U
#define LOG_IMPL_LEVEL_ERR  1U
#define LOG_IMPL_LEVEL_WRN  2U
#define LOG_IMPL_LEVEL_INF  3U
#define LOG_IMPL_LEVEL_DBG  4U

static inline char z_log_minimal_level_to_char(int level)
{
    switch (level)
    {
    case LOG_IMPL_LEVEL_ERR:
        return 'E';
    case LOG_IMPL_LEVEL_WRN:
        return 'W';
    case LOG_IMPL_LEVEL_INF:
        return 'I';
    case LOG_IMPL_LEVEL_DBG:
        return 'D';
    default:
        return '?';
    }
}

extern void bt_log_impl_printf(uint8_t _level, const char *format, ...);
extern void bt_log_impl_packet(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len);
extern void bt_log_impl_init(void);

#define LOG_IMPL_TO_PRINTK(_fun, _line, _level, _name, fmt, ...)                                   \
    do                                                                                             \
    {                                                                                              \
        bt_log_impl_printf(_level,                                                                    \
                        "%c: "                                                                     \
                        "(%s)"                                                                     \
                        "%s():%d: " fmt "\n",                                                      \
                        z_log_minimal_level_to_char(_level), #_name, _fun, _line, ##__VA_ARGS__);  \
    } while (false);

#ifdef FUNCTION_CONTROL_DEBUG_ENABLE
#define __LOG_IMPL(_level, _name, _level_thod, ...)                                                \
    if (_level <= _level_thod)                                                                     \
    {                                                                                              \
        LOG_IMPL_TO_PRINTK(__func__, __LINE__, _level, _name, __VA_ARGS__);                        \
    }

#define __LOG_IMPL_RAW(_level, _fmt, ...)                                                          \
    do                                                                                             \
    {                                                                                              \
        bt_log_impl_printf(_level, _fmt, ##__VA_ARGS__);                                              \
    } while (false);

#define __PACKET_IMPL(_packet_type, _in, _packet, _len)                                            \
    do                                                                                             \
    {                                                                                              \
        bt_log_impl_packet(_packet_type, _in, _packet, _len);                                         \
    } while (false)
#define __LOG_INIT_IMPL()                                                                          \
    do                                                                                             \
    {                                                                                              \
        bt_log_impl_init(_packet_type, _in, _packet, _len);                                           \
    } while (false)
#else
#define __LOG_IMPL(_level, _name, _level_thod, ...)
#define __LOG_IMPL_RAW(_level, _fmt, ...)
#define __PACKET_IMPL(_packet_type, _in, _packet, _len)
#define __LOG_INIT_IMPL()
#endif

static inline char *log_strdup(const char *str)
{
    return (char *)str;
}

/**
 * @brief Logger API
 * @defgroup log_api Logging API
 * @ingroup logger
 * @{
 */

/**
 * @brief Writes an ERROR level message to the log.
 *
 * @details It's meant to report severe errors, such as those from which it's
 * not possible to recover.
 *
 * @param ... A string optionally containing printk valid conversion specifier,
 * followed by as many values as specifiers.
 */
#define LOG_IMPL_ERR(...) __LOG_IMPL(LOG_IMPL_LEVEL_ERR, __VA_ARGS__)

/**
 * @brief Writes a WARNING level message to the log.
 *
 * @details It's meant to register messages related to unusual situations that
 * are not necessarily errors.
 *
 * @param ... A string optionally containing printk valid conversion specifier,
 * followed by as many values as specifiers.
 */
#define LOG_IMPL_WRN(...) __LOG_IMPL(LOG_IMPL_LEVEL_WRN, __VA_ARGS__)

/**
 * @brief Writes an INFO level message to the log.
 *
 * @details It's meant to write generic user oriented messages.
 *
 * @param ... A string optionally containing printk valid conversion specifier,
 * followed by as many values as specifiers.
 */
#define LOG_IMPL_INF(...) __LOG_IMPL(LOG_IMPL_LEVEL_INF, __VA_ARGS__)

/**
 * @brief Writes a DEBUG level message to the log.
 *
 * @details It's meant to write developer oriented information.
 *
 * @param ... A string optionally containing printk valid conversion specifier,
 * followed by as many values as specifiers.
 */
#define LOG_IMPL_DBG(...) __LOG_IMPL(LOG_IMPL_LEVEL_DBG, __VA_ARGS__)

#define LOG_INIT() __LOG_INIT_IMPL()

#define LOG_PACKET_DUMP(_packet_type, _in, _packet, _len)                                          \
    __PACKET_IMPL(_packet_type, _in, _packet, _len)

#define printk(fmt, ...) __LOG_IMPL_RAW(LOG_IMPL_LEVEL_INF, fmt, ##__VA_ARGS__)

typedef struct
{
    // init work
    void (*init)(void);
    // log packet
    void (*packet)(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len);
    // log message
    void (*printf)(uint8_t level, const char *format, va_list argptr);
    // log point
    void (*point)(uint32_t val);
} bt_log_impl_t;

/**
 * @brief Init Logger
 * @param log_impl - platform-specific implementation
 */
void bt_log_impl_register(const bt_log_impl_t *log_impl);

#endif /* _ZEPHYR_POLLING_LOGGING_LOG_IMPL_H_ */