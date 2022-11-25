/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "bt_config.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/crypto.h>

#define BT_DBG_ENABLED  IS_ENABLED(CONFIG_BT_DEBUG_HCI_CORE)
#define LOG_MODULE_NAME bt_crypto
#include "logging/bt_log.h"

#include "drivers/hci_driver.h"
#include "hci_core.h"
#include "common/aes_soft.h"

uint32_t rand_get32(void)
{
    return rand();
}

int rand_get(uint8_t *buf, uint8_t len)
{
    for (int i = 0; i < len; i++)
    {
        buf[i] = rand_get32();
    }
    return 0;
}

void rand_init(unsigned int seed)
{
    srand(seed);
}

uint32_t bt_rand_get32(void)
{
    return rand_get32();
}

int bt_rand(void *buf, size_t len)
{
    return rand_get(buf, len);
}

void bt_rand_init(unsigned int seed)
{
    rand_init(seed);
}

void reverse_byte(const uint8_t *in, uint8_t *out)
{
    for (int i = 0; i < 16; i++)
    {
        out[16 - 1 - i] = in[i];
    }
}
int bt_encrypt_le(const uint8_t key[16], const uint8_t plaintext[16], uint8_t enc_data[16])
{
    BT_DBG("key %s", bt_hex(key, 16));
    BT_DBG("plaintext %s", bt_hex(plaintext, 16));

    // Our asic is negated
    // aes_encrypt(AES_BIG_ENDIAN, key, plaintext, 16);
    // aes_result(enc_data);
    // aesEncrypt(key, 16, plaintext, enc_data, 16);
    struct AES_ctx ctx;

    uint8_t key_tmp[16];
    uint8_t plaintext_tmp[16];
    reverse_byte(key, key_tmp);
    reverse_byte(plaintext, plaintext_tmp);

    AES_init_ctx(&ctx, key_tmp);
    AES_ECB_encrypt(&ctx, plaintext_tmp);

    reverse_byte(plaintext_tmp, enc_data);

    BT_DBG("enc_data %s", bt_hex(enc_data, 16));

    return 0;
}

int bt_encrypt_sk(const uint8_t key[16], const uint8_t plaintext[16], uint8_t enc_data[16])
{
    BT_DBG("sk key %s", bt_hex(key, 16));
    BT_DBG("sk plaintext %s", bt_hex(plaintext, 16));

    // Our asic is negated
    // aes_encrypt(AES_BIG_ENDIAN, key, plaintext, 16);
    // aes_encrypt_ctrl(0);        // little out
    // aes_result(enc_data);

    BT_DBG("sk enc_data %s", bt_hex(enc_data, 16));

    return 0;
}
