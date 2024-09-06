/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_xspi.h"
#include "app.h"
#if (defined CACHE_MAINTAIN) && (CACHE_MAINTAIN == 1)
#include "fsl_cache.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 *****************************************************************************/
extern xspi_device_config_t deviceConfig;
extern const uint32_t customLUT[CUSTOM_LUT_LENGTH];
#if defined(EXAMPLE_FLASH_RESET_CONFIG) || defined(FLASH_ADESTO)
extern const uint32_t FastReadSDRLUTCommandSeq[4];
extern const uint32_t OctalReadDDRLUTCommandSeq[4];
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/

status_t xspi_nor_write_enable(XSPI_Type *base, uint32_t baseAddr, bool enableOctal)
{
    xspi_transfer_t flashXfer;
    status_t status;

    /* Write enable */
    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE + baseAddr;
    flashXfer.cmdType         = kXSPI_Command;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.data            = NULL;
    flashXfer.dataSize        = 0UL;
    flashXfer.lockArbitration = false;
    if (enableOctal)
    {
        flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE_OPI;
    }
    else
    {
        flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE;
    }

    status = XSPI_TransferBlocking(base, &flashXfer);

    return status;
}

status_t xspi_nor_wait_bus_busy(XSPI_Type *base, bool enableOctal)
{
    /* Wait status ready. */
    bool isBusy;
    uint32_t readValue;
    status_t status;
    xspi_transfer_t flashXfer;

    flashXfer.deviceAddress = EXAMPLE_XSPI_AMBA_BASE;
    flashXfer.cmdType       = kXSPI_Read;
    flashXfer.data          = &readValue;
    flashXfer.targetGroup   = kXSPI_TargetGroup0;
    if (enableOctal)
    {
        flashXfer.dataSize = 2;
        flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_READ_STATUS_OPI;
    }
    else
    {
        flashXfer.dataSize = 1;
        flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_READ_STATUS;
    }
    flashXfer.lockArbitration = false;

    do
    {
        status = XSPI_TransferBlocking(base, &flashXfer);

        if (status != kStatus_Success)
        {
            return status;
        }
        if (FLASH_BUSY_STATUS_POL)
        {
            if (readValue & (1U << FLASH_BUSY_STATUS_OFFSET))
            {
                isBusy = true;
            }
            else
            {
                isBusy = false;
            }
        }
        else
        {
            if (readValue & (1U << FLASH_BUSY_STATUS_OFFSET))
            {
                isBusy = false;
            }
            else
            {
                isBusy = true;
            }
        }

    } while (isBusy);

    return status;
}

#if defined(FLASH_ENABLE_OCTAL_CMD)
status_t xspi_nor_enable_octal_mode(XSPI_Type *base)
{
    xspi_transfer_t flashXfer;
    status_t status;

    /* Copy LUT information from flash region into RAM region, because flash boot mode maybe not same with application's
       required mode.
       If yes, doesn't need to memory copy operation; if no, need to memory opeation before flash access failure due to
       mismatch LUT read command sequence. */
#if defined(EXAMPLE_FLASH_RESET_CONFIG)
    uint32_t TempOctalReadDDRLUTCommandSeq[4];

    memcpy(TempOctalReadDDRLUTCommandSeq, OctalReadDDRLUTCommandSeq, sizeof(OctalReadDDRLUTCommandSeq));
#endif

#if defined(FLASH_ADESTO) && FLASH_ADESTO
    uint32_t tempLUT[4] = {0};
    uint32_t readValue;

    /* Update LUT table for octal mode. */
    XSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, OctalReadDDRLUTCommandSeq, 4);

    /* Set the command instruction of read status register for LUT in octal sdr mode. */
    tempLUT[0] = XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_8PAD, 0x05, kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 0x04);
    tempLUT[1] = XSPI_LUT_SEQ(kXSPI_Command_READ_SDR, kXSPI_8PAD, 0x04, kXSPI_Command_STOP, kXSPI_8PAD, 0x0);
    /* Update LUT table. */
    XSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ_STATUS_OPI, tempLUT, 4);

    /* Write enable */
    status = xspi_nor_write_enable(base, 0, false);

    /* Enable quad mode. */
    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE;
    flashXfer.cmdType         = kXSPI_Command;
    flashXfer.seqIndex        = NOR_CMD_LUT_SEQ_IDX_ENTER_OPI;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nor_wait_bus_busy(base, true);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check global protect or ont. */
    flashXfer.deviceAddress = EXAMPLE_XSPI_AMBA_BASE;
    flashXfer.cmdType       = kXSPI_Read;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_READ_STATUS_OPI;
    flashXfer.targetGroup   = kXSPI_TargetGroup0;

    flashXfer.data            = &readValue;
    flashXfer.dataSize        = 1;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Global protected. */
    if ((readValue & 0x0CU) == 0x0CU)
    {
        /* Global unprotect entire memory region if it was protected by default, such as Adesto's octal flash. */
        tempLUT[0] = XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_8PAD, 0x01, kXSPI_Command_WRITE_SDR, kXSPI_8PAD, 0x04);
        tempLUT[1] = 0x00U;

        /* Update LUT table. */
        XSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_CONFIG, tempLUT, 4);

        uint32_t globalUnprotect = FLASH_UNPROTECTVALUE;

        /* Write enable */
        status = xspi_nor_write_enable(base, 0, true);

        if (status != kStatus_Success)
        {
            return status;
        }

        flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE;
        flashXfer.cmdType         = kXSPI_Write;
        flashXfer.seqIndex        = NOR_CMD_LUT_SEQ_IDX_CONFIG;
        flashXfer.targetGroup     = kXSPI_TargetGroup0;
        flashXfer.data            = &globalUnprotect;
        flashXfer.dataSize        = 1;
        flashXfer.lockArbitration = false;

        status = XSPI_TransferBlocking(base, &flashXfer);
        if (status != kStatus_Success)
        {
            return status;
        }

        status = xspi_nor_wait_bus_busy(base, true);

        if (status != kStatus_Success)
        {
            return status;
        }

        /* Check unprotect successfully or not. */
        flashXfer.deviceAddress = EXAMPLE_XSPI_AMBA_BASE;
        flashXfer.cmdType       = kXSPI_Read;
        flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_READ_STATUS_OPI;
        flashXfer.targetGroup   = kXSPI_TargetGroup0;

        flashXfer.data            = &readValue;
        flashXfer.dataSize        = 1;
        flashXfer.lockArbitration = false;

        status = XSPI_TransferBlocking(base, &flashXfer);
        if (status != kStatus_Success)
        {
            return status;
        }

        status = xspi_nor_wait_bus_busy(base, true);
        if (status != kStatus_Success)
        {
            return status;
        }

        if ((readValue & 0x0CU) != 0x00U)
        {
            return -1;
        }
    }

    /* Enable DDR mode. */
    /* Update LUT table for configure status register2.*/
    tempLUT[0] = XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_8PAD, 0x31, kXSPI_Command_WRITE_SDR, kXSPI_8PAD, 0x01);
    /* Update LUT table. */
    XSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_CONFIG, tempLUT, 4);

    /* Set the command instruction of read status register for LUT in octal ddr mode. */
    tempLUT[0] = XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_8PAD, 0x05, kXSPI_Command_DUMMY_DDR, kXSPI_8PAD, 0x04);
    tempLUT[1] = XSPI_LUT_SEQ(kXSPI_Command_READ_DDR, kXSPI_8PAD, 0x04, kXSPI_Command_STOP, kXSPI_8PAD, 0x0);
    /* Update LUT table. */
    XSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ_STATUS_OPI, tempLUT, 4);

    /* Adesto enable octal mode needs to configures status register2. */
    uint32_t enableDdrMode = FLASH_ENABLE_OCTAL_DDRMODE;

    /* Write enable */
    status = xspi_nor_write_enable(base, 0, true);
    if (status != kStatus_Success)
    {
        return status;
    }

    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE;
    flashXfer.cmdType         = kXSPI_Write;
    flashXfer.seqIndex        = NOR_CMD_LUT_SEQ_IDX_CONFIG;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.data            = &enableDdrMode;
    flashXfer.dataSize        = 1;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nor_wait_bus_busy(base, true);

#else /* MXIC's octal flash. */

    uint32_t writeValue = FLASH_ENABLE_OCTAL_CMD;

    /* Write enable */
    status = xspi_nor_write_enable(base, 0, false);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Enable quad mode. */
    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE;
    flashXfer.cmdType         = kXSPI_Write;
    flashXfer.seqIndex        = NOR_CMD_LUT_SEQ_IDX_ENTER_OPI;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.data            = &writeValue;
    flashXfer.dataSize        = 1;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nor_wait_bus_busy(base, true);
#endif

#if defined(EXAMPLE_FLASH_RESET_CONFIG)
    /* 8DTRD: enter octal DDR and update read LUT entry into 8DTRD. */
    XSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, TempOctalReadDDRLUTCommandSeq, 4);
#endif

    return status;
}
#endif

status_t xspi_nor_flash_erase_sector(XSPI_Type *base, uint32_t address)
{
    status_t status;
    xspi_transfer_t flashXfer;

    /* Write enable */
    status = xspi_nor_write_enable(base, 0, true);

    if (status != kStatus_Success)
    {
        return status;
    }

    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE + address;
    flashXfer.cmdType         = kXSPI_Command;
    flashXfer.seqIndex        = NOR_CMD_LUT_SEQ_IDX_ERASE_SECTOR;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.lockArbitration = false;
    flashXfer.dataSize        = 0UL;
    flashXfer.data            = NULL;
    status                    = XSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nor_wait_bus_busy(base, true);

    return status;
}

status_t xspi_nor_flash_read(XSPI_Type *base, uint32_t dstAddr, uint32_t *src, uint32_t length)
{
    status_t status;
    xspi_transfer_t flashXfer;

    /* Prepare page program command */
    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE + dstAddr;
    flashXfer.cmdType         = kXSPI_Read;
    flashXfer.seqIndex        = NOR_CMD_LUT_SEQ_IDX_READ;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.data            = (uint32_t *)src;
    flashXfer.dataSize        = length;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nor_wait_bus_busy(base, true);

    return status;
}

status_t xspi_nor_flash_program(XSPI_Type *base, uint32_t dstAddr, uint32_t *src, uint32_t length)
{
    status_t status;
    xspi_transfer_t flashXfer;

    /* Write enable */
    status = xspi_nor_write_enable(base, dstAddr, true);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Prepare page program command */
    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE + dstAddr;
    flashXfer.cmdType         = kXSPI_Write;
    flashXfer.seqIndex        = NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_OCTAL;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.data            = (uint32_t *)src;
    flashXfer.dataSize        = length;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nor_wait_bus_busy(base, true);

    return status;
}

status_t xspi_nor_flash_page_program(XSPI_Type *base, uint32_t dstAddr, uint32_t *src)
{
    status_t status;
    xspi_transfer_t flashXfer;

    /* To make sure external flash be in idle status, added wait for busy before program data for
        an external flash without RWW(read while write) attribute.*/
    status = xspi_nor_wait_bus_busy(base, true);

    if (kStatus_Success != status)
    {
        return status;
    }

    /* Write enable. */
    status = xspi_nor_write_enable(base, dstAddr, true);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Prepare page program command */
    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE + dstAddr;
    flashXfer.cmdType         = kXSPI_Write;
    flashXfer.seqIndex        = NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_OCTAL;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.data            = (uint32_t *)src;
    flashXfer.dataSize        = FLASH_PAGE_SIZE;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nor_wait_bus_busy(base, true);

    return status;
}

status_t xspi_nor_get_vendor_id(XSPI_Type *base, uint8_t *vendorId)
{
    uint32_t temp;
    xspi_transfer_t flashXfer;
    flashXfer.deviceAddress = EXAMPLE_XSPI_AMBA_BASE;
    flashXfer.cmdType       = kXSPI_Read;
    flashXfer.targetGroup   = kXSPI_TargetGroup0;
#if defined(FLASH_ADESTO) && FLASH_ADESTO
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_READ_ID_SPI;
#else
    flashXfer.seqIndex = NOR_CMD_LUT_SEQ_IDX_READ_ID_OPI;
#endif
    flashXfer.data            = &temp;
    flashXfer.dataSize        = sizeof(temp);
    flashXfer.lockArbitration = false;

    status_t status = XSPI_TransferBlocking(base, &flashXfer);

    *vendorId = (uint8_t)(temp & 0xFFUL);

    return status;
}

status_t xspi_nor_erase_chip(XSPI_Type *base)
{
    status_t status;
    xspi_transfer_t flashXfer;

    /* Write enable */
    status = xspi_nor_write_enable(base, 0, true);

    if (status != kStatus_Success)
    {
        return status;
    }

    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE;
    flashXfer.cmdType         = kXSPI_Command;
    flashXfer.seqIndex        = NOR_CMD_LUT_SEQ_IDX_ERASE_CHIP;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = xspi_nor_wait_bus_busy(base, true);

    return status;
}

void xspi_nor_flash_init(XSPI_Type *base)
{
    xspi_config_t config;
    xspi_ahb_access_config_t xspiAhbAccessConfig;
    xspi_ip_access_config_t xspiIpAccessConfig;

    /* To store custom's LUT table in local. */
    uint32_t tempLUT[CUSTOM_LUT_LENGTH] = {0x00U};

    /* Copy LUT information from flash region into RAM region, because LUT update maybe corrupt read sequence(LUT[0])
     * and load wrong LUT table from FLASH region. */
    memcpy(tempLUT, customLUT, sizeof(tempLUT));

    xspi_clock_init();

    config.ptrAhbAccessConfig = &xspiAhbAccessConfig;
    config.ptrIpAccessConfig  = &xspiIpAccessConfig;
    /*Get XSPI default settings and configure the xspi. */
    XSPI_GetDefaultConfig(&config);

    config.byteOrder                                       = kXSPI_64BitLE;
    config.ptrAhbAccessConfig->ahbErrorPayload.highPayload = 0x5A5A5A5AUL;
    config.ptrAhbAccessConfig->ahbErrorPayload.lowPayload  = 0x5A5A5A5AUL;
    config.ptrAhbAccessConfig->ptrAhbWriteConfig         = NULL; /* This demo does not demonstrate AHB write feature.*/
    config.ptrAhbAccessConfig->ARDSeqIndex               = NOR_CMD_LUT_SEQ_IDX_READ;
    config.ptrAhbAccessConfig->enableAHBBufferWriteFlush = true;
    config.ptrAhbAccessConfig->enableAHBPrefetch         = true;

    config.ptrIpAccessConfig->ptrSfpFradConfig               = NULL; /* This demo does not demonstrate SFP feature.*/
    config.ptrIpAccessConfig->ptrSfpMdadConfig               = NULL;
    config.ptrIpAccessConfig->ipAccessTimeoutValue           = 0xFFFFFFFFUL;
    config.ptrIpAccessConfig->sfpArbitrationLockTimeoutValue = 0xFFFFFFUL;
    XSPI_Init(base, &config);

    /* Configure flash settings according to serial flash feature. */
    XSPI_SetDeviceConfig(base, &deviceConfig);

    /*update LUT*/
    XSPI_UpdateLUT(base, 0, tempLUT, CUSTOM_LUT_LENGTH);
}
