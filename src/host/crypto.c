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

#include <base/check.h>
#include <base/byteorder.h>

#define LOG_MODULE_NAME bt_crypto
#include "logging/bt_log.h"

#include "drivers/hci_driver.h"
#include "hci_core.h"

#include <tinycrypt/constants.h>
#include <tinycrypt/hmac_prng.h>
#include <tinycrypt/aes.h>
#include <tinycrypt/utils.h>

#if defined(CONFIG_BT_HOST_CRYPTO)

static struct tc_hmac_prng_struct prng;

static int prng_reseed(struct tc_hmac_prng_struct *h)
{
    uint8_t seed[32];
    int64_t extra;
    int ret;

    ret = bt_hci_le_rand(seed, sizeof(seed));
    if (ret)
    {
        return ret;
    }

    // extra = k_uptime_get();

    ret = tc_hmac_prng_reseed(h, seed, sizeof(seed), (uint8_t *)&extra, sizeof(extra));
    if (ret == TC_CRYPTO_FAIL)
    {
        LOG_ERR("Failed to re-seed PRNG");
        return -EIO;
    }

    return 0;
}

int prng_init(void)
{
    uint8_t perso[8];
    int ret;

    ret = bt_hci_le_rand(perso, sizeof(perso));
    if (ret)
    {
        return ret;
    }

    ret = tc_hmac_prng_init(&prng, perso, sizeof(perso));
    if (ret == TC_CRYPTO_FAIL)
    {
        LOG_ERR("Failed to initialize PRNG");
        return -EIO;
    }

    /* re-seed is needed after init */
    return prng_reseed(&prng);
}

static int prng_reseed_new(struct tc_hmac_prng_struct *h, uint8_t* perso)
{
    uint8_t seed[32];
    int64_t extra;
    int ret;

    // temp work.
    memcpy(seed, perso, 8);
    // ret = bt_hci_le_rand(seed, sizeof(seed));
    // if (ret)
    // {
    //     return ret;
    // }

    extra = sys_clock_tick_get();

    ret = tc_hmac_prng_reseed(h, seed, sizeof(seed), (uint8_t *)&extra, sizeof(extra));
    if (ret == TC_CRYPTO_FAIL)
    {
        LOG_ERR("Failed to re-seed PRNG");
        return -EIO;
    }

    return 0;
}

int prng_init_new(uint8_t* perso)
{
    int ret;

    ret = tc_hmac_prng_init(&prng, perso, 8);
    if (ret == TC_CRYPTO_FAIL)
    {
        LOG_ERR("Failed to initialize PRNG");
        return -EIO;
    }

    /* re-seed is needed after init */
    return prng_reseed_new(&prng, perso);
}

#if defined(CONFIG_BT_HOST_CRYPTO_PRNG)
int bt_rand(void *buf, size_t len)
{
    int ret;

    CHECKIF(buf == NULL || len == 0)
    {
        return -EINVAL;
    }

    ret = tc_hmac_prng_generate(buf, len, &prng);
    if (ret == TC_HMAC_PRNG_RESEED_REQ)
    {
        uint8_t perso[8];
        ret = prng_reseed_new(&prng, perso);
        if (ret)
        {
            return ret;
        }

        ret = tc_hmac_prng_generate(buf, len, &prng);
    }

    if (ret == TC_CRYPTO_SUCCESS)
    {
        return 0;
    }

    return -EIO;
}
#else  /* !CONFIG_BT_HOST_CRYPTO_PRNG */
int bt_rand(void *buf, size_t len)
{
    CHECKIF(buf == NULL || len == 0)
    {
        return -EINVAL;
    }

    return bt_hci_le_rand(buf, len);
}
#endif /* CONFIG_BT_HOST_CRYPTO_PRNG */

int bt_encrypt_le(const uint8_t key[16], const uint8_t plaintext[16], uint8_t enc_data[16])
{
    struct tc_aes_key_sched_struct s;
    uint8_t tmp[16];

    CHECKIF(key == NULL || plaintext == NULL || enc_data == NULL)
    {
        return -EINVAL;
    }

    LOG_DBG("key %s", bt_hex(key, 16));
    LOG_DBG("plaintext %s", bt_hex(plaintext, 16));

    sys_memcpy_swap(tmp, key, 16);

    if (tc_aes128_set_encrypt_key(&s, tmp) == TC_CRYPTO_FAIL)
    {
        return -EINVAL;
    }

    sys_memcpy_swap(tmp, plaintext, 16);

    if (tc_aes_encrypt(enc_data, tmp, &s) == TC_CRYPTO_FAIL)
    {
        return -EINVAL;
    }

    sys_mem_swap(enc_data, 16);

    LOG_DBG("enc_data %s", bt_hex(enc_data, 16));

    return 0;
}

int bt_encrypt_be(const uint8_t key[16], const uint8_t plaintext[16], uint8_t enc_data[16])
{
    struct tc_aes_key_sched_struct s;

    CHECKIF(key == NULL || plaintext == NULL || enc_data == NULL)
    {
        return -EINVAL;
    }

    LOG_DBG("key %s", bt_hex(key, 16));
    LOG_DBG("plaintext %s", bt_hex(plaintext, 16));

    if (tc_aes128_set_encrypt_key(&s, key) == TC_CRYPTO_FAIL)
    {
        return -EINVAL;
    }

    if (tc_aes_encrypt(enc_data, plaintext, &s) == TC_CRYPTO_FAIL)
    {
        return -EINVAL;
    }

    LOG_DBG("enc_data %s", bt_hex(enc_data, 16));

    return 0;
}

#endif /* defined(CONFIG_BT_HOST_CRYPTO) */