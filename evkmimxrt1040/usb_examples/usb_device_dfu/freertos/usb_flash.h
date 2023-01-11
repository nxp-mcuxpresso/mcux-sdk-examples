/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DFU_FLASH_H__
#define __DFU_FLASH_H__
#include "usb.h"
/* USB DFU config*/
/*! @brief DFU application address and size*/
#define USB_DFU_APP_ADDRESS (0x10000U)
#define USB_DFU_APP_SIZE    (0x8000U)

///*******************************************************************************
// * Definitions
// ******************************************************************************/
typedef enum _dfu_memmroy_status
{
    kStatus_USB_MemmorySuccess = 0U,
    kStatus_USB_MemmoryErrorSecure,
    kStatus_USB_MemmoryErrorErase,
    kStatus_USB_MemmoryErrorEraseVerify,
    kStatus_USB_MemmoryErrorProgram,
    kStatus_USB_MemmoryErrorProgramAddress,
    kStatus_USB_MemmoryErrorProgramVerify,
    kStatus_USB_MemmoryErrorUnknown,
} usb_memmory_status_t;
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief USB memmory initialization function.
 *
 * This function initializes the memmory driver structure and variables.
 *
 * @return A FLASH error or kStatus_FLASH_Success.
 */
extern usb_memmory_status_t USB_MemmoryInit(void);

/*!
 * @brief USB memmory erasing function.
 *
 * This function erases the memmory area from start address to the end.
 *
 * @param address  The start address.
 * @param address  The erase size.
 *
 * @return A FLASH error or kStatus_FLASH_Success.
 */
extern usb_memmory_status_t USB_MemmoryErase(uint32_t address, uint32_t size);

/*!
 * @brief USB memmory programming function.
 *
 * This function program memmory with data at locations passed in through parameters.
 *
 * @param address The start address to be programmed.
 * @param buffer  Pointer to buffer data.
 * @param length  The length of data in byte.
 *
 * @return A FLASH error or kStatus_FLASH_Success.
 */
extern usb_memmory_status_t USB_MemmoryProgram(uint32_t address, uint8_t *buffer, uint32_t length);

#if defined(__cplusplus)
}
#endif
#endif /* __DFU_FLASH_H__ */
