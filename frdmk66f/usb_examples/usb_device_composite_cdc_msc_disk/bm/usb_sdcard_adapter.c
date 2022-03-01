/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "usb.h"
#include "fsl_sd.h"
#include "sdmmc_config.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* State in Disk driver. */
sd_card_t g_sd;

sd_card_t *usbDeviceMscSdcard;

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_USB_Disk_Config(uint8_t usbPriorty)
{
    BOARD_SD_Config(&g_sd, NULL, (usbPriorty - 1U), NULL);
}

/*!
 * @brief device msc card init function.
 *
 * This function initialize the card.
 * @return kStatus_USB_Success or error.
 */
uint8_t USB_DeviceMscDiskStorageInit(void)
{
    usb_status_t error = kStatus_USB_Success;
    usbDeviceMscSdcard = &g_sd;

    /* Init card. */
    if (SD_Init(usbDeviceMscSdcard))
    {
        PRINTF("\n SD card init failed \n");
        error = kStatus_USB_Error;
    }

    return error;
}

status_t USB_Disk_WriteBlocks(const uint8_t *buffer, uint32_t startBlock, uint32_t blockCount)
{
    return SD_WriteBlocks(usbDeviceMscSdcard, buffer, startBlock, blockCount);
}

status_t USB_Disk_ReadBlocks(uint8_t *buffer, uint32_t startBlock, uint32_t blockCount)
{
    return SD_ReadBlocks(usbDeviceMscSdcard, buffer, startBlock, blockCount);
}

uint32_t USB_Disk_GetBlockSize()
{
    return usbDeviceMscSdcard->blockSize;
}

uint32_t USB_Disk_GetBlockCount()
{
    return usbDeviceMscSdcard->blockCount;
}
