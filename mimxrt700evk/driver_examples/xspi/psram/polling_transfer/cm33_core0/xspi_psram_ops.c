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

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t xspi_hyper_ram_ipcommand_write_data(XSPI_Type *base, uint32_t address, uint32_t *buffer, uint32_t length)
{
    xspi_transfer_t flashXfer;
    status_t status;

    /* Write data */
    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE + address;
    flashXfer.cmdType         = kXSPI_Write;
    flashXfer.seqIndex        = HYPERRAM_CMD_LUT_SEQ_IDX_BURST_WRITE;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.data            = buffer;
    flashXfer.dataSize        = length;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);

    return status;
}

status_t xspi_hyper_ram_ipcommand_read_data(XSPI_Type *base, uint32_t address, uint32_t *buffer, uint32_t length)
{
    xspi_transfer_t flashXfer;
    status_t status;

    /* Read data */
    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE + address;
    flashXfer.cmdType         = kXSPI_Read;
    flashXfer.seqIndex        = HYPERRAM_CMD_LUT_SEQ_IDX_BURST_READ;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.data            = buffer;
    flashXfer.dataSize        = length;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);

    return status;
}

static status_t xspi_hyper_ram_write_mcr(XSPI_Type *base, uint32_t regAddr, uint8_t *mrVal)
{
    xspi_transfer_t flashXfer;
    status_t status;

    /* Write data */
    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE + regAddr;
    flashXfer.cmdType         = kXSPI_Write;
    flashXfer.seqIndex        = HYPERRAM_CMD_LUT_SEQ_IDX_REG_WRITE;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.data            = (uint32_t *)mrVal;
    flashXfer.dataSize        = 4;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);

    return status;
}

static status_t xspi_hyper_ram_get_mcr(XSPI_Type *base, uint32_t regAddr, uint8_t *mrVal)
{
    xspi_transfer_t flashXfer;
    status_t status;

    /* Read data */
    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE + regAddr;
    flashXfer.cmdType         = kXSPI_Read;
    flashXfer.seqIndex        = HYPERRAM_CMD_LUT_SEQ_IDX_REG_READ;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.data            = (uint32_t *)mrVal;
    flashXfer.dataSize        = 4;
    flashXfer.lockArbitration = false;

    status = XSPI_TransferBlocking(base, &flashXfer);

    return status;
}

void xspi_hyper_ram_ahbcommand_write_data(XSPI_Type *base, uint32_t address, uint32_t *buffer, uint32_t length)
{
    uint32_t *startAddr = (uint32_t *)(EXAMPLE_XSPI_AMBA_BASE + address);
    memcpy(startAddr, buffer, length);
}

void xspi_hyper_ram_ahbcommand_read_data(XSPI_Type *base, uint32_t address, uint32_t *buffer, uint32_t length)
{
    uint32_t *startAddr = (uint32_t *)(EXAMPLE_XSPI_AMBA_BASE + address);
    memcpy(buffer, startAddr, length);
}

status_t xspi_hyper_ram_reset(XSPI_Type *base)
{
    xspi_transfer_t flashXfer;
    status_t status;

    /* Write data */
    flashXfer.deviceAddress   = EXAMPLE_XSPI_AMBA_BASE;
    flashXfer.cmdType         = kXSPI_Command;
    flashXfer.seqIndex        = HYPERRAM_CMD_LUT_SEQ_IDX_RESET;
    flashXfer.targetGroup     = kXSPI_TargetGroup0;
    flashXfer.lockArbitration = false;
    flashXfer.data            = NULL;
    flashXfer.dataSize        = 0U;

    status = XSPI_TransferBlocking(base, &flashXfer);

    if (status == kStatus_Success)
    {
        /* for loop of 50000 is about 1ms (@200 MHz CPU) */
        for (uint32_t i = 2000000U; i > 0; i--)
        {
            __NOP();
        }
    }
    return status;
}

#if defined(XSPI_ENABLE_VARIABLE_LATENCY) && (XSPI_ENABLE_VARIABLE_LATENCY)
status_t xspi_hyperram_optimize_latency(XSPI_Type *base)
{
    uint16_t cr0Register[2] = {0x0U, 0x0U};

    (void)xspi_hyper_ram_get_mcr(base, (1U << 11), (uint8_t *)cr0Register);

    cr0Register[1] &= ~(1U << 3U);
    cr0Register[0] &= ~(0x07 << 4); /* Clear drive strength */
    cr0Register[0] |= (0x03 << 4);  /* 46Ohms */

    (void)xspi_hyper_ram_write_mcr(base, (1U << 11), (uint8_t *)cr0Register);
    
    (void)xspi_hyper_ram_get_mcr(base, (1U << 11), (uint8_t *)cr0Register);
    if ((cr0Register[1] & 0x8U) != 0U)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}
#endif

#if defined(ENABLE_CKN) && (ENABLE_CKN)
/* For W958DkNBKX. */
status_t xspi_hyperram_enable_clkn(XSPI_Type *base)
{
    uint16_t cr1Register[2] = {0x0, 0x0};

    /* Enable CLKn. */
    (void)xspi_hyper_ram_get_mcr(base, ((1U << 11) | (1U << 0)), (uint8_t *)cr1Register);
    cr1Register[1] &= ~(0x01 << 6); /* enable ckn */
    (void)xspi_hyper_ram_write_mcr(base, ((1U << 12) | (1U << 0)), (uint8_t *)cr1Register);
    
    (void)xspi_hyper_ram_get_mcr(base, ((1U << 11) | (1U << 0)), (uint8_t *)cr1Register);
    if ((cr1Register[1] & 0x40U) != 0U)
    {
        return kStatus_Fail;   
    }
    
    return kStatus_Success;
}
#endif /* ENABLE_CKN */

void xspi_hyper_ram_init(XSPI_Type *base)
{
    xspi_ip_access_config_t hyperRamIpAccessConfig;
    xspi_ahb_access_config_t hyperRamAhbAccessConfig;
    xspi_ahb_write_config_t hyperRamAhbWriteConfig = {
        .AWRSeqIndex        = HYPERRAM_CMD_LUT_SEQ_IDX_BURST_WRITE,
        .blockRead          = false,
        .blockSequenceWrite = false,
    };

    xspi_config_t config;

    /* To store custom's LUT table in local. */
    uint32_t tempLUT[CUSTOM_LUT_LENGTH] = {0x00U};

    /* Copy LUT information from flash region into RAM region, because LUT update maybe corrupt read sequence(LUT[0])
     * and load wrong LUT table from FLASH region. */
    memcpy(tempLUT, customLUT, sizeof(tempLUT));

    xspi_clock_init();

    config.ptrAhbAccessConfig = &hyperRamAhbAccessConfig;
    config.ptrIpAccessConfig  = &hyperRamIpAccessConfig;

    /* Get XSPI default settings and configure the xspi. */
    XSPI_GetDefaultConfig(&config);

    config.byteOrder                                       = kXSPI_64BitLE;
    config.ptrAhbAccessConfig->ahbErrorPayload.highPayload = 0x5A5A5A5AUL;
    config.ptrAhbAccessConfig->ahbErrorPayload.lowPayload  = 0x5A5A5A5AUL;
    config.ptrAhbAccessConfig->ARDSeqIndex                 = HYPERRAM_CMD_LUT_SEQ_IDX_BURST_READ;
    config.ptrAhbAccessConfig->enableAHBBufferWriteFlush   = true;
    config.ptrAhbAccessConfig->enableAHBPrefetch           = true;
    config.ptrAhbAccessConfig->ptrAhbWriteConfig           = &hyperRamAhbWriteConfig;

    config.ptrIpAccessConfig->ipAccessTimeoutValue           = 0xFFFFFFFFUL;
    config.ptrIpAccessConfig->ptrSfpFradConfig               = NULL;
    config.ptrIpAccessConfig->ptrSfpMdadConfig               = NULL;
    config.ptrIpAccessConfig->sfpArbitrationLockTimeoutValue = 0xFFFFFFUL;

    XSPI_Init(base, &config);

    /* Configure flash settings according to serial flash feature. */
    XSPI_SetDeviceConfig(base, &deviceConfig);

    /* Update LUT table. */
    XSPI_UpdateLUT(base, 0, tempLUT, CUSTOM_LUT_LENGTH);

#if defined(ENABLE_CKN) && (ENABLE_CKN)
    XSPI_UpdateDeviceAddrMode(base, kXSPI_DeviceByteAddressable);
    while(xspi_hyperram_enable_clkn(base) != kStatus_Success)
    {};
    XSPI_UpdateDeviceAddrMode(base, kXSPI_Device4ByteAddressable);
#endif /* ENABLE_CKN */
    
#if defined(XSPI_ENABLE_VARIABLE_LATENCY) && (XSPI_ENABLE_VARIABLE_LATENCY)
    XSPI_UpdateDeviceAddrMode(base, kXSPI_DeviceByteAddressable);
    while(xspi_hyperram_optimize_latency(base) != kStatus_Success)
    {}
    XSPI_UpdateDeviceAddrMode(base, kXSPI_Device4ByteAddressable);
#endif
}
