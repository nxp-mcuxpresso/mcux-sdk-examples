/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"

#if defined(FSL_FEATURE_SOC_CAAM_COUNT) && (FSL_FEATURE_SOC_CAAM_COUNT > 0)
#include "fsl_caam.h"
#elif defined(FSL_FEATURE_SOC_DCP_COUNT) && (FSL_FEATURE_SOC_DCP_COUNT > 0)
#include "fsl_dcp.h"
#else
#error "The platform does not support DCP or CAAM."
#endif

#include "fsl_debug_console.h"
#include "mflash_file.h"

#define MAX_FILE_SIZE                   (400U)
#define CIPHER_KEY_SIZE_BITS            (128U)
#define CIPHER_KEY_SIZE_BYTES           (CIPHER_KEY_SIZE_BITS / 8)

AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t in_buffer[CIPHER_KEY_SIZE_BYTES], 64);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t out_buffer[CIPHER_KEY_SIZE_BYTES], 64);

#ifdef FSL_FEATURE_SOC_CAAM_COUNT

/*! @brief CAAM job ring interface 0 in system memory. */
AT_NONCACHEABLE_SECTION(static caam_job_ring_interface_t s_jrif0);

/*! @brief 16 bytes key for CBC method: "ultrapassword123". */
static const uint8_t s_CbcKey128[] = {0x75, 0x6c, 0x74, 0x72, 0x61, 0x70, 0x61, 0x73,
                                      0x73, 0x77, 0x6f, 0x72, 0x64, 0x31, 0x32, 0x33};

/*! @brief Initialization vector for CBC method: 16 bytes: "mysecretpassword". */
static const uint8_t s_CbcIv[CAAM_AES_BLOCK_SIZE] = {0x6d, 0x79, 0x73, 0x65, 0x63, 0x72, 0x65, 0x74,
                                                     0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64};

static caam_handle_t caamHandle = {
        .jobRing = kCAAM_JobRing0
    };

/*!
 * @brief Encrypts and decrypts in AES CBC mode.
 */
static status_t do_crypto(uint8_t *data, uint8_t *output, uint32_t *data_len, bool encrypt)
{
    uint8_t iv_temp[CAAM_AES_BLOCK_SIZE];
    uint32_t size = 0;
    status_t status;

    memcpy(iv_temp, s_CbcIv, CAAM_AES_BLOCK_SIZE);
    while(size < *data_len) {
        memset(in_buffer, 0, CIPHER_KEY_SIZE_BYTES);
        memcpy(in_buffer, data, (*data_len - size) >= CIPHER_KEY_SIZE_BYTES ?
                                CIPHER_KEY_SIZE_BYTES : (*data_len - size));

        if (encrypt) {
            status = CAAM_AES_EncryptCbc(CAAM, &caamHandle, in_buffer, out_buffer, CIPHER_KEY_SIZE_BYTES,
                                         iv_temp, s_CbcKey128, sizeof(s_CbcKey128));
            memcpy(iv_temp, out_buffer, CIPHER_KEY_SIZE_BYTES);
        } else {
            status = CAAM_AES_DecryptCbc(CAAM, &caamHandle, in_buffer, out_buffer, CIPHER_KEY_SIZE_BYTES,
                                         iv_temp, s_CbcKey128, sizeof(s_CbcKey128));
            memcpy(iv_temp, in_buffer, CIPHER_KEY_SIZE_BYTES);
        }

        if (status != kStatus_Success) {
            PRINTF("do_crypto error!\r\n");
            return status;
        }

        memcpy(output, out_buffer, CIPHER_KEY_SIZE_BYTES);
        data   += CIPHER_KEY_SIZE_BYTES;
        output += CIPHER_KEY_SIZE_BYTES;
        size   += CIPHER_KEY_SIZE_BYTES;
    }
    *data_len = size;

    return status;
}

static status_t init_caam(void)
{
    caam_config_t caamConfig;

    /* Get default configuration. */
    CAAM_GetDefaultConfig(&caamConfig);

    /* setup memory for job ring interfaces. Can be in system memory or CAAM's secure memory. */
    caamConfig.jobRingInterface[0] = &s_jrif0;

    /* Init CAAM driver, including CAAM's internal RNG */
    return CAAM_Init(CAAM, &caamConfig);
}

uint32_t get_seed(void)
{
    uint32_t random_val = 0;

    CAAM_RNG_GetRandomData(CAAM, &caamHandle, kCAAM_RngStateHandle0,
                           (uint8_t *)&random_val, 4, kCAAM_RngDataAny, NULL);

    return random_val;
}
#else

typedef enum _dcp_otp_key_select
{
    kDCP_OTPMKKeyLow  = 1U, /* Use [127:0] from snvs key as dcp key */
    kDCP_OTPMKKeyHigh = 2U, /* Use [255:128] from snvs key as dcp key */
    kDCP_OCOTPKeyLow  = 3U, /* Use [127:0] from ocotp key as dcp key */
    kDCP_OCOTPKeyHigh = 4U  /* Use [255:128] from ocotp key as dcp key */
} dcp_otp_key_select;

static status_t DCP_OTPKeySelect(dcp_otp_key_select keySelect)
{
    status_t retval = kStatus_Success;
    if (keySelect == kDCP_OTPMKKeyLow)
    {
        IOMUXC_GPR->GPR3 &= ~(1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 &= ~(1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
    }

    else if (keySelect == kDCP_OTPMKKeyHigh)
    {
        IOMUXC_GPR->GPR3 |= (1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 &= ~(1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
    }

    else if (keySelect == kDCP_OCOTPKeyLow)
    {
        IOMUXC_GPR->GPR3 &= ~(1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 |= (1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
    }

    else if (keySelect == kDCP_OCOTPKeyHigh)
    {
        IOMUXC_GPR->GPR3 |= (1 << IOMUXC_GPR_GPR3_DCP_KEY_SEL_SHIFT);
        IOMUXC_GPR->GPR10 |= (1 << IOMUXC_GPR_GPR10_DCPKEY_OCOTP_OR_KEYMUX_SHIFT);
    }

    else
    {
        retval = kStatus_InvalidArgument;
    }
    return retval;
}

/* When doing encryption, the size of output will be a mutiple of CIPHER_KEY_SIZE_BYTES. */
static status_t do_crypto(uint8_t *data, uint8_t *output, uint32_t *data_len, bool encrypt)
{
    status_t status;
    dcp_handle_t m_handle;
    uint8_t *start;
    uint8_t *end;
    uint8_t *dst;
    size_t copy_size;

    m_handle.channel    = kDCP_Channel0;
    m_handle.swapConfig = kDCP_NoSwap;
    m_handle.keySlot    = kDCP_OtpKey;

    status = DCP_AES_SetKey(DCP, &m_handle, NULL, 0);
    if (status != kStatus_Success)
    {
        return status;
    }

    start = data;
    end = data + *data_len;
    dst = output;

    *data_len = 0;
    while (start < end)
    {
        copy_size = MIN(CIPHER_KEY_SIZE_BYTES, end - start);

        memset(in_buffer, 0, sizeof(in_buffer));

        memcpy(in_buffer, start, copy_size);

        if (encrypt)
        {
            status = DCP_AES_EncryptEcb(DCP, &m_handle,
                                        in_buffer, out_buffer,
                                        CIPHER_KEY_SIZE_BYTES);
        }
        else
        {
            status = DCP_AES_DecryptEcb(DCP, &m_handle,
                                        in_buffer, out_buffer,
                                        CIPHER_KEY_SIZE_BYTES);
        }

        if (status != kStatus_Success)
        {
            return status;
        }

        memcpy(dst, out_buffer, sizeof(out_buffer));

        start += copy_size;
        dst += copy_size;
        *data_len += CIPHER_KEY_SIZE_BYTES;
    }

    return kStatus_Success;
}

static status_t init_dcp(void)
{
    dcp_config_t dcpConfig;
    status_t status;

    DCP_GetDefaultConfig(&dcpConfig);

    /* Set OTP key type in IOMUX registers before initializing DCP. */
    /* Software reset of DCP must be issued after changing the OTP key type. */
    status = DCP_OTPKeySelect(kDCP_OTPMKKeyLow);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Reset and initialize DCP */
    DCP_Init(DCP, &dcpConfig);

    return kStatus_Success;
}
#endif /* FSL_FEATURE_SOC_CAAM_COUNT */

status_t secure_storage_init(char *filename)
{
    status_t status;
    mflash_file_t file_table[] =
    {
        {
            .path = filename,
            .max_size = 200
        },
        {0}
    };

#ifdef FSL_FEATURE_SOC_CAAM_COUNT
    status = init_caam();
#else
    status = init_dcp();
#endif
    if (status != kStatus_Success)
    {
        return status;
    }

    status = mflash_init(file_table, 1);
    if (status != kStatus_Success)
    {
        PRINTF("[!] ERROR in mflash_init!");
        return status;
    }

    return kStatus_Success;
}

status_t secure_save_file(char *filename, uint8_t *data, uint32_t data_len)
{
    status_t status;
    uint8_t output[MAX_FILE_SIZE];
    uint32_t size;

    if ((filename == NULL) || (strlen(filename) > 63) ||
        (data == NULL) || (data_len <= 0) || (data_len > MAX_FILE_SIZE))
    {
        return kStatus_InvalidArgument;
    }

    size = data_len;
    status = do_crypto(data, output, &size, true);
    if (kStatus_Success != status)
    {
        return status;
    }

    status = mflash_file_save(filename, output, size);
    if (kStatus_Success != status)
    {
        return status;
    }

    return kStatus_Success;
}

status_t secure_read_file(char *filename, uint8_t *data, uint32_t *data_len)
{
    status_t status;
    uint8_t *ciphertext;

    if ((filename == NULL) || (strlen(filename) > 63) ||
        (data == NULL))
    {
        return kStatus_InvalidArgument;
    }

    status = mflash_file_mmap(filename, &ciphertext, data_len);
    if (kStatus_Success != status)
    {
        return status;
    }

    status = do_crypto(ciphertext, data, data_len, false);
    if (kStatus_Success != status)
    {
        return status;
    }

    return kStatus_Success;
}
