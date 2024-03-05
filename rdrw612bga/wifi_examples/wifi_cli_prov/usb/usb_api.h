/** @file usb_api.h
 *
 *  @brief  This file provides the support for USB APIs
 */
/*
 *  Copyright 2020 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USB_API_H_
#define _USB_API_H_
#include <ff.h>

#define USB_SUPPORT_ENABLE 1

void usb_init(void);

int usb_mount();

int usb_file_open(char *test_file_name);

int usb_file_open_by_mode(char *test_file_name, uint8_t mode);

int usb_file_write(uint8_t *data, size_t data_len);

int usb_file_close();

int usb_file_lseek(size_t lseek_size);

int usb_file_size(void);

int usb_file_read(uint8_t *data, size_t data_len);

#endif /* _USB_API_H_ */
