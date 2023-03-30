/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#ifndef CRED_FLASH_STORAGE_H
#define CRED_FLASH_STORAGE_H

uint32_t init_flash_storage(char *filename);

uint32_t save_wifi_credentials(char *filename, char *ssid, char *passphrase);

uint32_t get_saved_wifi_credentials(char *filename, char *ssid, char *passphrase);

uint32_t reset_saved_wifi_credentials(char *filename);

#endif
