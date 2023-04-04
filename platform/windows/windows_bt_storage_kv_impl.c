#include <stdio.h>

#include <pthread.h>
#include <windows.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "windows_bt_storage_kv_impl.h"

#include "base\byteorder.h"
#include "base\util.h"
#include "logging\bt_log_impl.h"

#define FILE_INFO_HEADER "bt_storage_kv_info"

#define STORAGE_FILE_PATH_MAX_LENGTH (0x400)

#define LOG_FILE_PRINT_BUFFER_MAX_LENGTH (0x1000)

static void get_storage_file_path(char *log_path)
{
    char exe_path[STORAGE_FILE_PATH_MAX_LENGTH];
    GetModuleFileName(NULL, exe_path, STORAGE_FILE_PATH_MAX_LENGTH);
    *strrchr(exe_path, '\\') = 0;

    sprintf(log_path, "%s\\storage", exe_path);
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

static void write_to_file(char *file_name, uint8_t *data, uint16_t len)
{
    FILE *file;
    file = fopen(file_name, "wb");
    fwrite(data, len, 1, file);
    fclose(file);
}

static unsigned long get_file_size(const char *filename)
{
    unsigned long size;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printk("ERROR: Open file %s failed.\n", filename);
        return 0;
    }
    fseek(fp, SEEK_SET, SEEK_END);
    size = ftell(fp);
    fclose(fp);

    return size;
}

static void get_storage_file(char *name, uint16_t key)
{
    char storage_path[STORAGE_FILE_PATH_MAX_LENGTH];
    // get log path
    get_storage_file_path(storage_path);

    sprintf(name, "%s\\%s_%04X.kv", storage_path, FILE_INFO_HEADER, key);
}

static void init_list(struct bt_storage_kv_header *list, uint16_t list_cnt)
{
    // TODO: Do nothing.
}

static int get(uint16_t key, uint8_t *data, int *len)
{
    char file_name[STORAGE_FILE_PATH_MAX_LENGTH];
    printk("get: key: 0x%x\n", key);
    get_storage_file(file_name, key);
    unsigned long file_size = get_file_size(file_name);
    // only get length.
    if (data == NULL)
    {
        *len = file_size;
        return 0;
    }
    FILE *file;
    file = fopen(file_name, "rb");
    if (file == NULL)
    {
        *len = -1;
        return -1;
    }
    else
    {
        fread(data, MIN(*len, file_size), 1, file);
    }

    fclose(file);
    return 0;
}

static void set(uint16_t key, uint8_t *data, int len)
{
    char file_name[STORAGE_FILE_PATH_MAX_LENGTH];
    printk("set: key: 0x%x, len: %d\n", key, len);
    get_storage_file(file_name, key);

    // TODO: check last data right or not.
    delete_file(file_name);
    create_file(file_name);

    write_to_file(file_name, data, len);
}

static void delete(uint16_t key, uint8_t *data, int len)
{
    char file_name[STORAGE_FILE_PATH_MAX_LENGTH];
    printk("delete: key: 0x%x\n", key);
    get_storage_file(file_name, key);

    // TODO: check last data right or not.
    delete_file(file_name);
}

static const struct bt_storage_kv_impl kv_impl = {
        init_list,
        get,
        set,
        delete,
};

const struct bt_storage_kv_impl *bt_storage_kv_impl_local_instance(void)
{
    char storage_path[STORAGE_FILE_PATH_MAX_LENGTH];
    // get log path
    get_storage_file_path(storage_path);
    mkdir(storage_path);

    return &kv_impl;
}
