/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "soc/soc_caps.h"

#if SOC_AES_SUPPORTED

#define ESP_AES_ENCRYPT     1 /**< AES encryption. */
#define ESP_AES_DECRYPT     0 /**< AES decryption. */

#define TEST_AES_MALLOC_CAPS (MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA)

static const uint8_t key_256[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
};

static const uint8_t iv[] = {
    0x10, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09,
    0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
};
#endif /* SOC_AES_SUPPORTED */
