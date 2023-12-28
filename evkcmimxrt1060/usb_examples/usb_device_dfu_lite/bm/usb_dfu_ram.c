/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "usb_flash.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief memmory configuration */

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief USB memmory initialization function.
 *
 * This function initializes the memmory driver structure and variables.
 *
 * @return A FLASH error or kStatus_FLASH_Success.
 */
usb_memmory_status_t USB_MemmoryInit(void)
{
    usb_memmory_status_t status = kStatus_USB_MemmorySuccess;

    return status;
}

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
usb_memmory_status_t USB_MemmoryErase(uint32_t address, uint32_t size)
{
    usb_memmory_status_t status = kStatus_USB_MemmorySuccess;
    return status;
}

/*!
 * @brief DFU memmory programming function.
 *
 * This function program memmory with data at locations passed in through parameters.
 *
 * @param address The start address to be programmed.
 * @param buffer  Pointer to buffer data.
 * @param length  The length of data in byte.
 *
 * @return A FLASH error or kStatus_FLASH_Success.
 */
usb_memmory_status_t USB_MemmoryProgram(uint32_t address, uint8_t *buffer, uint32_t length)
{
    usb_memmory_status_t status = kStatus_USB_MemmorySuccess;
    memcpy((void *)address, (void *)buffer, length);
    return status;
}
