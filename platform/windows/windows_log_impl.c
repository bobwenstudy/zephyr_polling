#include <stdio.h>

#include <pthread.h>
#include <windows.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "windows_log_impl.h"

#include "base\byteorder.h"
#include "logging\bt_log_impl.h"
#include "logging\bt_log.h"

#define FUNCTION_WINDOWS_LOG_PRINT_IN_WINDOW // becareful, printf need too long time
#define FUNCTION_WINDOWS_LOG_TXT_FILE
#define FUNCTION_WINDOWS_LOG_CFA_FILE

#define LOG_FILE_PATH_MAX_LENGTH (0x400)

/**
 * number of seconds from 1 Jan. 1601 00:00 to 1 Jan 1970 00:00 UTC
 */
#define EPOCH_DIFF 11644473600LL

#define H4_CMD 0x01
#define H4_ACL 0x02
#define H4_SCO 0x03
#define H4_EVT 0x04
#define H4_ISO 0x05

#define LOG_FILE_PRINT_BUFFER_MAX_LENGTH (0x1000)

static pthread_mutex_t print_lock;

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

static void create_file(char *file_name)
{
    FILE *file;
    file = fopen(file_name, "r");

    if (file == NULL)
    {
        file = fopen(file_name, "w");
    }

    //关闭文件
    fclose(file);
}

static void delete_file(char *file_name)
{
    remove(file_name);
}

static void get_log_file_path(char *p_log_path)
{
    char exe_path[LOG_FILE_PATH_MAX_LENGTH];
    GetModuleFileName(NULL, exe_path, LOG_FILE_PATH_MAX_LENGTH);
    *strrchr(exe_path, '\\') = 0;

    sprintf(p_log_path, "%s\\log", exe_path);
}

static void get_log_txt_file_name(char *file_path)
{
    char log_path[LOG_FILE_PATH_MAX_LENGTH];
    // get log path
    get_log_file_path(log_path);
    sprintf(file_path, "%s\\log.txt", log_path);
}

static void get_log_cfa_file_name(char *file_path)
{
    char log_path[LOG_FILE_PATH_MAX_LENGTH];
    // get log path
    get_log_file_path(log_path);
    sprintf(file_path, "%s\\log.cfa", log_path);
}

static void log_printf_dump(uint8_t level, const char *format, va_list argptr)
{
    char log_printf_buffer[LOG_FILE_PRINT_BUFFER_MAX_LENGTH];
    char msg_str[LOG_FILE_PRINT_BUFFER_MAX_LENGTH];
    int len = vsnprintf(msg_str, sizeof(msg_str), format, argptr);

    char timestamp_str[0x100];
    SYSTEMTIME timestamp;
    GetLocalTime(&timestamp);
    snprintf(timestamp_str, sizeof(timestamp_str), "%04u-%02u-%02u %02u:%02u:%02u.%03u",
             timestamp.wYear, timestamp.wMonth, timestamp.wDay, timestamp.wHour, timestamp.wMinute,
             timestamp.wSecond, timestamp.wMilliseconds);

    int total_len = snprintf(log_printf_buffer, sizeof(log_printf_buffer), "[%s] [0x%lx] %s",
                             timestamp_str, GetCurrentThreadId(), msg_str);

    pthread_mutex_lock(&print_lock);
#ifdef FUNCTION_WINDOWS_LOG_TXT_FILE
    char log_txt_file_name[LOG_FILE_PATH_MAX_LENGTH];
    get_log_txt_file_name(log_txt_file_name);
    FILE *file;
    file = fopen(log_txt_file_name, "a");
    fprintf(file, log_printf_buffer);
    fclose(file);
#endif

    // printf("total_len: %d\n", total_len);
#ifndef FUNCTION_WINDOWS_LOG_PRINT_IN_WINDOW
    if (level <= LOG_IMPL_LEVEL_INF)
#endif
    {
        // print in window.
        printf(log_printf_buffer);
    }
    pthread_mutex_unlock(&print_lock);
}

void create_btsnoop_header(uint8_t *buffer, uint32_t ts_usec_high, uint32_t ts_usec_low,
                           uint32_t cumulative_drops, uint8_t packet_type, uint8_t in, uint16_t len)
{
    uint32_t packet_flags = 0;
    if (in)
    {
        packet_flags |= 1;
    }
    switch (packet_type)
    {
    case H4_CMD:
    case H4_EVT:
        packet_flags |= 2;
    default:
        break;
    }
    sys_put_be32(len, buffer + 0);               // Original Length
    sys_put_be32(len, buffer + 4);               // Included Length
    sys_put_be32(packet_flags, buffer + 8);      // Packet Flags
    sys_put_be32(cumulative_drops, buffer + 12); // Cumulativ Drops
    sys_put_be32(ts_usec_high, buffer + 16);     // Timestamp Microseconds High
    sys_put_be32(ts_usec_low, buffer + 20);      // Timestamp Microseconds Low

    buffer[24] = packet_type;
}

static void log_packet_dump(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len)
{
    char log_printf_buffer[LOG_FILE_PRINT_BUFFER_MAX_LENGTH];
    const char *packet_type_str = get_packet_type_str(packet_type, in);
    FILE *file;

    char msg_str[LOG_FILE_PRINT_BUFFER_MAX_LENGTH];
    log_hex_dump(msg_str, sizeof(msg_str), packet, len);

    char timestamp_str[0x100];
    SYSTEMTIME timestamp;
    GetLocalTime(&timestamp);
    snprintf(timestamp_str, sizeof(timestamp_str), "%04u-%02u-%02u %02u:%02u:%02u.%3u",
             timestamp.wYear, timestamp.wMonth, timestamp.wDay, timestamp.wHour, timestamp.wMinute,
             timestamp.wSecond, timestamp.wMilliseconds);

    int total_len = snprintf(log_printf_buffer, sizeof(log_printf_buffer), "[%s] [0x%lx] %s %s\n",
                             timestamp_str, GetCurrentThreadId(), packet_type_str, msg_str);

    pthread_mutex_lock(&print_lock);
#ifdef FUNCTION_WINDOWS_LOG_CFA_FILE
    uint8_t header_btsnoop[25];

    uint32_t tv_sec = 0;
    // uint32_t tv_us = 0;
    uint64_t ts_usec;

    FILETIME file_time;
    ULARGE_INTEGER now_time;
    SystemTimeToFileTime(&timestamp, &file_time);
    now_time.LowPart = file_time.dwLowDateTime;
    now_time.HighPart = file_time.dwHighDateTime;

    ts_usec = 0xdcddb30f2f8000LLU + now_time.QuadPart / 10 - EPOCH_DIFF * 1000000LLU;
    // append packet type to pcap header
    create_btsnoop_header(header_btsnoop, ts_usec >> 32, ts_usec & 0xFFFFFFFF, 0, packet_type, in,
                          len + 1);

    char log_cfa_file_name[LOG_FILE_PATH_MAX_LENGTH];
    get_log_cfa_file_name(log_cfa_file_name);
    // printf("packet len: %d, packet: %s\n", len, msg_str);
    file = fopen(log_cfa_file_name, "ab");
    fwrite(header_btsnoop, sizeof(header_btsnoop), 1, file);
    fwrite(packet, len, 1, file);
    fclose(file);
#endif

#ifdef FUNCTION_WINDOWS_LOG_TXT_FILE
    char log_txt_file_name[LOG_FILE_PATH_MAX_LENGTH];
    get_log_txt_file_name(log_txt_file_name);
    file = fopen(log_txt_file_name, "a");
    fprintf(file, log_printf_buffer);
    fclose(file);
#endif

    // printf("total_len: %d\n", total_len);
#ifdef FUNCTION_WINDOWS_LOG_PRINT_IN_WINDOW
    // print in window.
    printf(log_printf_buffer);
#endif
    pthread_mutex_unlock(&print_lock);
}

static void log_point_dump(uint32_t point)
{
}

static void log_init(void)
{
    char log_path[LOG_FILE_PATH_MAX_LENGTH];
    // get log path
    get_log_file_path(log_path);
    mkdir(log_path);

    pthread_mutex_init(&print_lock, NULL);

#ifdef FUNCTION_WINDOWS_LOG_TXT_FILE
    char log_txt_file_name[LOG_FILE_PATH_MAX_LENGTH];
    get_log_txt_file_name(log_txt_file_name);
    delete_file(log_txt_file_name);
    create_file(log_txt_file_name);
#endif

#ifdef FUNCTION_WINDOWS_LOG_CFA_FILE
    char log_cfa_file_name[LOG_FILE_PATH_MAX_LENGTH];
    get_log_cfa_file_name(log_cfa_file_name);
    delete_file(log_cfa_file_name);
    create_file(log_cfa_file_name);

    // write BTSnoop file header
    const uint8_t file_header[] = {
            // Identification Pattern: "btsnoop\0"
            0x62,
            0x74,
            0x73,
            0x6E,
            0x6F,
            0x6F,
            0x70,
            0x00,
            // Version: 1
            0x00,
            0x00,
            0x00,
            0x01,
            // Datalink Type: 1002 - H4
            0x00,
            0x00,
            0x03,
            0xEA,
    };

    FILE *file;
    file = fopen(log_cfa_file_name, "w");
    fwrite(file_header, sizeof(file_header), 1, file);
    fclose(file);

    // uint8_t packet[] = {0x03, 0x0c, 0x00};
    // log_packet_dump(1, 0, packet, sizeof(packet));
    // log_packet_dump(1, 0, packet, sizeof(packet));
    // log_packet_dump(1, 0, packet, sizeof(packet));
    // log_packet_dump(1, 0, packet, sizeof(packet));
    // log_packet_dump(1, 0, packet, sizeof(packet));
#endif
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
