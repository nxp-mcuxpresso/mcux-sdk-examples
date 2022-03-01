/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "usb.h"
#include "fsl_mmc.h"
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
/* State in Card driver. */
mmc_card_t g_mmc;
mmc_card_t *usbDeviceMscMmc;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief disk configration function.
 *
 * This function configure the disk.
 */
void BOARD_USB_Disk_Config(uint32_t usbPriorty)
{
    BOARD_MMC_Config(&g_mmc, (usbPriorty - 1U));
}

/*!
 * @brief device msc disk init function.
 *
 * This function initialize the disk.
 * @return kStatus_USB_Success or error.
 */
uint8_t USB_DeviceMscDiskStorageInit(void)
{
    usb_status_t error = kStatus_USB_Success;
    usbDeviceMscMmc    = &g_mmc;

    /* Init mmc. */
    if (MMC_Init(usbDeviceMscMmc))
    {
        PRINTF("\n MMC init failed \n");
        error = kStatus_USB_Error;
    }

    return error;
}

/*!
 * @brief Writes data blocks to the disk.
 *
 */
status_t USB_Disk_WriteBlocks(const uint8_t *buffer, uint32_t startBlock, uint32_t blockCount)
{
    return MMC_WriteBlocks(usbDeviceMscMmc, buffer, startBlock, blockCount);
}

/*!
 * @brief read data blocks from the disk.
 *
 */
status_t USB_Disk_ReadBlocks(uint8_t *buffer, uint32_t startBlock, uint32_t blockCount)
{
    return MMC_ReadBlocks(usbDeviceMscMmc, buffer, startBlock, blockCount);
}

/*!
 * @brief get block size.
 *
 */
uint32_t USB_Disk_GetBlockSize()
{
    return usbDeviceMscMmc->blockSize;
}

/*!
 * @brief get block count.
 *
 */
uint32_t USB_Disk_GetBlockCount()
{
    return usbDeviceMscMmc->userPartitionBlocks;
}
