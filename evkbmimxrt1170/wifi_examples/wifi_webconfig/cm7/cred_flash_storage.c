/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "cred_flash_storage.h"
#include "webconfig.h"
#include "fsl_debug_console.h"
#include "mflash_file.h"
#include "wpl.h"

#define FILE_HEADER "wifi_credentials:"

static uint32_t save_file(char *filename, char *data, uint32_t data_len)
{
    if ((filename == NULL) || (strlen(filename) > 63) || (data == NULL) || (data_len <= 0))
    {
        return 1;
    }

    /* Write the data to file. */
    if (kStatus_Success != mflash_file_save(filename, (uint8_t *)data, data_len))
    {
        PRINTF("[!] mflash_save_file failed\r\n");
        __BKPT(0);
        return 1;
    }
    else
    {
        PRINTF("[i] mflash_save_file success\r\n");
    }

    return 0;
}

uint32_t init_flash_storage(char *filename)
{
    /* Flash structure */
    mflash_file_t file_table[] = {{.path = filename, .max_size = 200}, {0}};

    if (mflash_init(file_table, 1) != kStatus_Success)
    {
        PRINTF("[!] ERROR in mflash_init!");
        __BKPT(0);
        return 1;
    }
    return 0;
}

uint32_t save_wifi_credentials(char *filename, char *ssid, char *passphrase, char *security)
{
    if (filename == NULL || (strlen(filename) > 63))
    {
        return 1;
    }

    if (strlen(ssid) > WPL_WIFI_SSID_LENGTH)
    {
        PRINTF("[!] SSID is too long. It can only be %d characters but is %d characters.\n", WPL_WIFI_SSID_LENGTH,
               strlen(ssid));
        return 1;
    }

    if (strlen(passphrase) > WPL_WIFI_PASSWORD_LENGTH)
    {
        PRINTF("[!] Password is too long. It can only be %d characters but is %d characters.\n",
               WPL_WIFI_PASSWORD_LENGTH, strlen(passphrase));
        return 1;
    }
    
    if (strlen(security) > WIFI_SECURITY_LENGTH)
    {
        PRINTF("[!] Security string is too long.\n");
        return 1;
    }

    char credentials_buf[sizeof(FILE_HEADER) + WPL_WIFI_SSID_LENGTH + WPL_WIFI_PASSWORD_LENGTH + WIFI_SECURITY_LENGTH + 4];
    uint32_t data_len;

    strcpy(credentials_buf, FILE_HEADER);
    strcat(credentials_buf, ssid);
    strcat(credentials_buf, "\n");
    strcat(credentials_buf, passphrase);
    strcat(credentials_buf, "\n");
    strcat(credentials_buf, security);
    strcat(credentials_buf, "\n");

    data_len = strlen(credentials_buf) + 1; // Need to also store \0

    if (save_file(filename, credentials_buf, data_len))
    {
        return 1;
    }

    return 0;
}

uint32_t get_saved_wifi_credentials(char *filename, char *ssid, char *passphrase, char *security)
{
    uint8_t *credentials_buf;
    uint32_t data_len = 0;
    status_t status;
    ssid[0]       = '\0';
    passphrase[0] = '\0';
    security[0] = '\0';

    if (filename == NULL || (strlen(filename) > 63))
    {
        return 1;
    }

    status = mflash_file_mmap(filename, &credentials_buf, &data_len);
    if (status == kStatus_Success)
    {
        if ((data_len > sizeof(FILE_HEADER)) &&
            (strncmp((char *)credentials_buf, FILE_HEADER, strlen(FILE_HEADER)) == 0))
        {
            credentials_buf += strlen(FILE_HEADER);
            uint32_t pos = 0;

            while (*credentials_buf != '\n' && pos <= WPL_WIFI_SSID_LENGTH)
            {
                ssid[pos] = *credentials_buf;
                credentials_buf++;
                pos++;
            };
            ssid[pos] = '\0';
            credentials_buf++;
            pos = 0;

            while (*credentials_buf != '\n' && pos <= WPL_WIFI_PASSWORD_LENGTH)
            {
                passphrase[pos] = *credentials_buf;
                pos++;
                credentials_buf++;
            };
            passphrase[pos] = '\0';
            credentials_buf++;
            pos = 0;
            
            while (*credentials_buf != '\n' && pos <= WIFI_SECURITY_LENGTH)
            {
                security[pos] = *credentials_buf;
                pos++;
                credentials_buf++;
            };
            security[pos] = '\0';
            
        }

        return 0;
    }
    return 1;
}

uint32_t reset_saved_wifi_credentials(char *filename)
{
    if (filename == NULL || (strlen(filename) > 63))
    {
        return 1;
    }
    return save_file(filename, "", 1);
}
