/*
 * Copyright 2019-2021,2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_flexspi.h"
#include "fsl_flexspi_edma.h"
#include "nor_flash.h"
#include "app.h"
#include "flexspi_octal_flash_ops.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
spi_nor_flash_state flash_state;
extern flexspi_device_config_t deviceconfig;
#if FLASH_ADESTO_DEVICE_ATXP032
extern const uint32_t LUTOctalMode_ADESTO[CUSTOM_LUT_LENGTH];
extern const uint32_t ReadLUTCmdSeq_ADESTO[4];
extern const uint32_t OctalReadLUTCmdSeq_ADESTO[4];
extern const uint32_t OctalDDRReadLUTCmdSeq_ADESTO[4];
#endif

#if FLASH_GIGADEVICE_DEVICE_GD25LX256
extern const uint32_t LUTOctalMode_GIGADEVICE[CUSTOM_LUT_LENGTH];
extern const uint32_t ReadLUTCmdSeq_GIGADEVICE[4];
extern const uint32_t OctalReadLUTCmdSeq_GIGADEVICE[4];
#endif

#if FLASH_MACRONIX_DEVICE_MX25UM51345G
extern const uint32_t LUTOctalMode_MACRONIX[CUSTOM_LUT_LENGTH];
extern const uint32_t ReadLUTCmdSeq_MACRONIX[4];
extern const uint32_t OctalReadLUTCmdSeq_MACRONIX[4];
#endif

#if !(defined(FLASH_ADESTO_DEVICE_ATXP032) && FLASH_ADESTO_DEVICE_ATXP032) && \
	!(defined(FLASH_GIGADEVICE_DEVICE_GD25LX256) && FLASH_GIGADEVICE_DEVICE_GD25LX256) && \
	!(defined(FLASH_MACRONIX_DEVICE_MX25UM51345G) && FLASH_MACRONIX_DEVICE_MX25UM51345G)
#error "Pls choose one macro from FLASH_ADESTO_DEVICE_ATXP032, FLASH_GIGADEVICE_DEVICE_GD25LX256, FLASH_MACRONIX_DEVICE_MX25UM51345G to define it as 1"
#endif

extern const uint32_t commonLUTOctalMode[CUSTOM_LUT_LENGTH];

extern const uint32_t commonReadLUTCmdSeq[4];
extern const uint32_t commonOctalReadLUTCmdSeq[4];

static volatile bool g_completionFlag = false;
extern edma_handle_t dmaTxHandle;
extern edma_handle_t dmaRxHandle;
static flexspi_edma_handle_t flexspiHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/
void flexspi_nor_disable_cache(flexspi_cache_status_t *cacheStatus)
{
#if (defined __CORTEX_M) && (__CORTEX_M == 7U)
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    /* Disable D cache. */
    if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR))
    {
        SCB_DisableDCache();
        cacheStatus->DCacheEnableFlag = true;
    }
#endif /* __DCACHE_PRESENT */

#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
    /* Disable I cache. */
    if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
    {
        SCB_DisableICache();
        cacheStatus->ICacheEnableFlag = true;
    }
#endif /* __ICACHE_PRESENT */

#elif (defined FSL_FEATURE_SOC_LMEM_COUNT) && (FSL_FEATURE_SOC_LMEM_COUNT != 0U)
    /* Disable code bus cache and system bus cache */
    if (LMEM_PCCCR_ENCACHE_MASK == (LMEM_PCCCR_ENCACHE_MASK & LMEM->PCCCR))
    {
        L1CACHE_DisableCodeCache();
        cacheStatus->codeCacheEnableFlag = true;
    }
    if (LMEM_PSCCR_ENCACHE_MASK == (LMEM_PSCCR_ENCACHE_MASK & LMEM->PSCCR))
    {
        L1CACHE_DisableSystemCache();
        cacheStatus->systemCacheEnableFlag = true;
    }

#elif (defined FSL_FEATURE_SOC_CACHE64_CTRL_COUNT) && (FSL_FEATURE_SOC_CACHE64_CTRL_COUNT != 0U)
    /* Disable cache */
    CACHE64_DisableCache(EXAMPLE_CACHE);
    cacheStatus->CacheEnableFlag = true;
#endif
}

void flexspi_nor_enable_cache(flexspi_cache_status_t cacheStatus)
{
#if (defined __CORTEX_M) && (__CORTEX_M == 7U)
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    if (cacheStatus.DCacheEnableFlag)
    {
        /* Enable D cache. */
        SCB_EnableDCache();
    }
#endif /* __DCACHE_PRESENT */

#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
    if (cacheStatus.ICacheEnableFlag)
    {
        /* Enable I cache. */
        SCB_EnableICache();
    }
#endif /* __ICACHE_PRESENT */

#elif (defined FSL_FEATURE_SOC_LMEM_COUNT) && (FSL_FEATURE_SOC_LMEM_COUNT != 0U)
    if (cacheStatus.codeCacheEnableFlag)
    {
        /* Enable code cache. */
        L1CACHE_EnableCodeCache();
    }

    if (cacheStatus.systemCacheEnableFlag)
    {
        /* Enable system cache. */
        L1CACHE_EnableSystemCache();
    }
#elif (defined FSL_FEATURE_SOC_CACHE64_CTRL_COUNT) && (FSL_FEATURE_SOC_CACHE64_CTRL_COUNT != 0U)
    if (cacheStatus.CacheEnableFlag)
    {
        /* Enable cache. */
        CACHE64_EnableCache(EXAMPLE_CACHE);
    }
#endif
}

static void flexspi_callback(FLEXSPI_Type *base, flexspi_edma_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        g_completionFlag = true;
    }
}

status_t flexspi_nor_exec_op(FLEXSPI_Type *base,
                             uint32_t deviceAddr,
                             flexspi_port_t port,
                             flexspi_command_type_t cmdType,
                             uint8_t seqIndex,
                             uint8_t seqNumber,
                             uint32_t *data,
                             size_t dataSize)
{
    flexspi_transfer_t flashXfer;

    flashXfer.deviceAddress = deviceAddr;
    flashXfer.port          = port;
    flashXfer.cmdType       = cmdType;
    flashXfer.SeqNumber     = seqNumber;
    flashXfer.seqIndex      = seqIndex;
    flashXfer.data          = data;
    flashXfer.dataSize      = dataSize;

    return FLEXSPI_TransferBlocking(base, &flashXfer);
}

status_t flexspi_nor_exec_op_with_edma(FLEXSPI_Type *base,
                             uint32_t deviceAddr,
                             flexspi_port_t port,
                             flexspi_command_type_t cmdType,
                             uint8_t seqIndex,
                             uint8_t seqNumber,
                             uint32_t *data,
                             size_t dataSize)
{
    flexspi_transfer_t flashXfer;

    flashXfer.deviceAddress = deviceAddr;
    flashXfer.port          = port;
    flashXfer.cmdType       = cmdType;
    flashXfer.SeqNumber     = seqNumber;
    flashXfer.seqIndex      = seqIndex;
    flashXfer.data          = data;
    flashXfer.dataSize      = dataSize;

    return FLEXSPI_TransferEDMA(base, &flexspiHandle, &flashXfer);
}

status_t flexspi_nor_write_enable(FLEXSPI_Type *base, uint32_t baseAddr)
{
    status_t status;
    uint8_t seqIdx = NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_SPI;

    if (flash_state.ioMode == SPINOR_OPI_MODE)
    {
        seqIdx = NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_DTR_OPI;
    }
    /* Write enable */
    status = flexspi_nor_exec_op(
        base, baseAddr, FLASH_PORT, kFLEXSPI_Command,
        seqIdx, 1, NULL, 0);

    return status;
}

status_t flexspi_nor_wait_bus_busy(FLEXSPI_Type *base)
{
    /* Wait status ready. */
    bool isBusy;
    uint32_t readValue = 0U;
    status_t status;
    uint8_t seqIdx = NOR_CMD_LUT_SEQ_IDX_READSTATUS_SPI;

    if (flash_state.ioMode == SPINOR_OPI_MODE)
    {
        if (flash_state.opMode == SPINOR_DTR_MODE)
        {
            seqIdx = NOR_CMD_LUT_SEQ_IDX_READSTATUS_DTR_OPI;
        }
        else
        {
            seqIdx = NOR_CMD_LUT_SEQ_IDX_READSTATUS_STR_OPI;
        }
    }

    do
    {
        status = flexspi_nor_exec_op(
            base, 0, FLASH_PORT, kFLEXSPI_Read,
            seqIdx, 1, &readValue, 1);
        if (status != kStatus_Success)
        {
            return status;
        }

        if (readValue & SR_WIP)
        {
            isBusy = true;
        }
        else
        {
            isBusy = false;
        }

    } while (isBusy);

    return status;
}

#if FLASH_ADESTO_DEVICE_ATXP032
status_t flexspi_nor_enable_octal_for_adesto(FLEXSPI_Type *base)
{
    status_t status = kStatus_Fail;
    uint32_t tempLUT[4] = {0U};
    uint32_t readValue  = 0U;
    /* Adesto enable Octal DDR(DTR) mode needs to configures status register byte 2. */
    uint32_t enableDdrMode   = SR2_SDR_DDR_ADESTO | SR2_OME_ADESTO;
    uint32_t globalUnprotect = 0U;
    uint32_t TempOctalReadLUTCmdSeq[4] = {0};
    uint32_t TempOctalDDRReadLUTCmdSeq[4] = {0};

    memcpy(TempOctalReadLUTCmdSeq, OctalReadLUTCmdSeq_ADESTO, sizeof(OctalReadLUTCmdSeq_ADESTO));
    memcpy(TempOctalDDRReadLUTCmdSeq, OctalDDRReadLUTCmdSeq_ADESTO, sizeof(OctalReadLUTCmdSeq_ADESTO));

    /* Write enable */
    status = flexspi_nor_write_enable(base, 0);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);
    if (status != kStatus_Success)
    {
        return status;
    }
    /* Enable octal mode(OPI). */
    status = flexspi_nor_exec_op(base, 0, FLASH_PORT, kFLEXSPI_Command, NOR_CMD_LUT_SEQ_IDX_ENTEROPI, 1,
                                 NULL, 0);
    if (status != kStatus_Success)
    {
        return status;
    }
    else
    {
        /*
         * Update LUT table for octal mode fast read.Make sure that cpu AHB read instruction from nor flash
         * correctly
         */
        FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, TempOctalReadLUTCmdSeq,
                          ARRAY_SIZE(TempOctalReadLUTCmdSeq));
        flash_state.ioMode = SPINOR_OPI_MODE;
        flash_state.opMode = SPINOR_STR_MODE;
    }


    status = flexspi_nor_wait_bus_busy(base);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Check global protect or ont. */
    status = flexspi_nor_exec_op(base, 0, FLASH_PORT, kFLEXSPI_Read, NOR_CMD_LUT_SEQ_IDX_READSTATUS_OPI, 1,
                                 &readValue, 1);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Global protected. */
    if ((readValue & SR_SWP_ALL_SECTORS_PROTECTED) == SR_SWP_ALL_SECTORS_PROTECTED)
    {
        /*
         * Global unprotect entire memory region if it was protected by default, such as Adesto's octal
         * flash.
         */
        tempLUT[0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, SPINOR_OP_WRSR,
                                     kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_8PAD, SPINOR_DATA_SIZE_4_BYTES);
        tempLUT[1] = 0x00U;

        /* Update LUT table. */
        FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_CONFIG, tempLUT,
                          ARRAY_SIZE(tempLUT));

        /* Write enable */
        status = flexspi_nor_write_enable(base, 0);

        if (status != kStatus_Success)
        {
            return status;
        }

        globalUnprotect = readValue & (~SR_SWP_ALL_SECTORS_PROTECTED);
        status = flexspi_nor_exec_op(base, 0, FLASH_PORT, kFLEXSPI_Write, NOR_CMD_LUT_SEQ_IDX_CONFIG, 1,
                                     &globalUnprotect, 1);
        if (status != kStatus_Success)
        {
            return status;
        }

        status = flexspi_nor_wait_bus_busy(base);

        if (status != kStatus_Success)
        {
            return status;
        }

        /* Check unprotect successfully or not. */
        status = flexspi_nor_exec_op(base, 0, FLASH_PORT, kFLEXSPI_Read, NOR_CMD_LUT_SEQ_IDX_READSTATUS_OPI,
                                     1, &readValue, 1);
        if (status != kStatus_Success)
        {
            return status;
        }

        status = flexspi_nor_wait_bus_busy(base);
        if (status != kStatus_Success)
        {
            return status;
        }

        if ((readValue & SR_SWP_ALL_SECTORS_PROTECTED) != SR_SWP_ALL_SECTORS_UNPROTECTED)
        {
            return -1;
        }
    }

    /* Enable DDR mode. */
    /* Update LUT table for configure status register2.*/
    tempLUT[0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, SPINOR_OP_WRSR2_ADESTO,
                                 kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_8PAD,
                                 SPINOR_DATA_SIZE_1_BYTES); /* write status register byte 2 in octal sdr
                                                               mode to enable Octal DDR mode */
    tempLUT[1] = 0x00U;
    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_CONFIG, tempLUT, ARRAY_SIZE(tempLUT));

    /* Set the command instruction of read status register for LUT in octal ddr mode. */
    tempLUT[0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, SPINOR_OP_RDSR1,
                                 kFLEXSPI_Command_DUMMY_DDR, kFLEXSPI_8PAD, SPINOR_DUMMY_CYCLE_NUMBER_0X4);
    tempLUT[1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_DDR, kFLEXSPI_8PAD, SPINOR_DATA_SIZE_4_BYTES,
                                 kFLEXSPI_Command_STOP, kFLEXSPI_8PAD, 0x0);
    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READSTATUS_OPI, tempLUT, ARRAY_SIZE(tempLUT));

    /* Write enable */
    status = flexspi_nor_write_enable(base, 0);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_exec_op(base, 0, FLASH_PORT, kFLEXSPI_Write, NOR_CMD_LUT_SEQ_IDX_CONFIG, 1,
                                 &enableDdrMode, 1);
    if (status != kStatus_Success)
    {
        return status;
    }
    else
    {
        FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, TempOctalDDRReadLUTCmdSeq,
                          ARRAY_SIZE(TempOctalDDRReadLUTCmdSeq));
        flash_state.opMode = SPINOR_DTR_MODE;
    }

    status = flexspi_nor_wait_bus_busy(base, true);
    
    return status;
}
#endif

#if FLASH_GIGADEVICE_DEVICE_GD25LX256
status_t flexspi_nor_enable_octal_for_gigadevice(FLEXSPI_Type *base)
{
    status_t status = kStatus_Fail;
    uint32_t tempLUT[4] = {0};
    uint32_t val        = SPINOR_IO_MODE_OCTAL_DTR_WITH_DQS_0XE7;
    uint32_t TempOctalReadLUTCmdSeq[4];

    memcpy(TempOctalReadLUTCmdSeq, OctalReadLUTCmdSeq_GIGADEVICE, sizeof(OctalReadLUTCmdSeq_GIGADEVICE));
    /* Write enable */
    status = flexspi_nor_write_enable(base, 0);

    memset((void *)tempLUT, 0, sizeof(tempLUT));
    tempLUT[0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINOR_OP_WR_VOLATILE_CFG_0X81,
                                 kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, SPINOR_ADDRESS_32_BITS);
    tempLUT[1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, SPINOR_DATA_SIZE_4_BYTES,
                                 kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x0);
    /* Update LUT table. */
    FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_CONFIG, tempLUT, ARRAY_SIZE(tempLUT));

    /* write address 0(I/O Mode) of volatile configuration registers with SPI Mode to enter Octal DTR with
     * DQS mode */
    status = flexspi_nor_exec_op(base, SPINOR_VOLATILE_CFG_REG_ADDR_IO_MODE_0X0, FLASH_PORT,
                                 kFLEXSPI_Write, NOR_CMD_LUT_SEQ_IDX_CONFIG, 1, &val, 1);
    if (status != kStatus_Success)
    {
        return status;
    }
    else
    {
        flash_state.ioMode = SPINOR_OPI_MODE;
        flash_state.opMode = SPINOR_DTR_MODE;
    }
    /*
     * Update LUT table for octal mode fast read.Make sure that cpu AHB read instruction from nor flash
     * correctly
     */
    FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, TempOctalReadLUTCmdSeq,
                      ARRAY_SIZE(TempOctalReadLUTCmdSeq));

    /* check whether switched to OPI Mode */
    memset((void *)tempLUT, 0, sizeof(tempLUT));
    tempLUT[0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, SPINOR_OP_RD_VOLATILE_CFG_0X85,
                                 kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_8PAD, SPINOR_ADDRESS_32_BITS);
    tempLUT[1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_DDR, kFLEXSPI_8PAD, SPINOR_DUMMY_CYCLE_NUMBER_0X8,
                                 kFLEXSPI_Command_READ_DDR, kFLEXSPI_8PAD, SPINOR_DATA_SIZE_4_BYTES);
    FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, 4 * NOR_CMD_LUT_SEQ_IDX_CONFIG, tempLUT, ARRAY_SIZE(tempLUT));
    /* read address 0(I/O Mode) of volatile configuration registers in OPI Mode */
    status = flexspi_nor_exec_op(EXAMPLE_FLEXSPI, SPINOR_VOLATILE_CFG_REG_ADDR_IO_MODE_0X0,
                                 FLASH_PORT, kFLEXSPI_Read, NOR_CMD_LUT_SEQ_IDX_CONFIG, 1, &val, 1);
    if (status != kStatus_Success)
    {
        return status;
    }
    else
    {
        if (val == SPINOR_IO_MODE_OCTAL_DTR_WITH_DQS_0XE7)
        {
            flash_state.ioMode = SPINOR_OPI_MODE;
        }
    }

    return status;
}
#endif

#if FLASH_MACRONIX_DEVICE_MX25UM51345G
status_t flexspi_nor_enable_octal_for_macronix(FLEXSPI_Type *base)
{
    status_t status = kStatus_Fail;
    /* MXIC's octal flash. */
    uint32_t value = CR2_DTR_OPI_ENABLE_MACRONIX;
    uint32_t TempOctalReadLUTCmdSeq[4];

    memcpy(TempOctalReadLUTCmdSeq, OctalReadLUTCmdSeq_MACRONIX, sizeof(OctalReadLUTCmdSeq_MACRONIX));
    /* Write enable */
    status = flexspi_nor_write_enable(base, 0);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Enable octal mode. */
    status = flexspi_nor_exec_op(base, 0, FLASH_PORT, kFLEXSPI_Write, NOR_CMD_LUT_SEQ_IDX_ENTEROPI, 1,
                                 &value, 1);
    if (status != kStatus_Success)
    {
        return status;
    }

    flash_state.ioMode = SPINOR_OPI_MODE;
    flash_state.opMode = SPINOR_DTR_MODE;
    status = flexspi_nor_wait_bus_busy(base, true);
    if (status == kStatus_Success)
    {
        flash_state.ioMode = SPINOR_OPI_MODE;

        /*
	 * 8DTRD: enter octal DDR and update read LUT entry into 8DTRD. OCTAL DTR(Double Transfer Rate) Read Mode(from MACRONIX's datasheet).
	 */
        FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, TempOctalReadLUTCmdSeq, 4);
    }

    return status;
}
#endif

#if FLASH_MACRONIX_DEVICE_MX25UM51345G
status_t flexspi_nor_enable_octal_for_macronix(FLEXSPI_Type *base)
{
    status_t status = kStatus_Fail;
    /* MXIC's octal flash. */
    uint32_t value = CR2_DTR_OPI_ENABLE_MACRONIX;
    uint32_t TempOctalReadLUTCmdSeq[4];

    memcpy(TempOctalReadLUTCmdSeq, OctalReadLUTCmdSeq_MACRONIX, sizeof(OctalReadLUTCmdSeq_MACRONIX));

    /* Write enable */
    status = flexspi_nor_write_enable(base, 0);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Enable octal mode. */
    status = flexspi_nor_exec_op(base, 0, FLASH_PORT, kFLEXSPI_Write, NOR_CMD_LUT_SEQ_IDX_ENTEROPI, 1,
                                 &value, 1);
    if (status != kStatus_Success)
    {
        return status;
    }

    flash_state.ioMode = SPINOR_OPI_MODE;
    flash_state.opMode = SPINOR_DTR_MODE;
    status = flexspi_nor_wait_bus_busy(base);
    if (status == kStatus_Success)
    {
        /*
         * 8DTRD: enter octal DDR and update read LUT entry into 8DTRD. OCTAL DTR(Double Transfer Rate) Read Mode(from MACRONIX's datasheet).
         */
        FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, TempOctalReadLUTCmdSeq, 4);
    }

    return status;
}
#endif

status_t flexspi_nor_enable_octal_mode(FLEXSPI_Type *base)
{
    status_t status = kStatus_Success;
    /*
     * Copy LUT information from flash region into RAM region, because flash boot mode maybe not same with application's
     * required mode.
     * If yes, doesn't need to memory copy operation; if no, need to memory opeation before flash access failure due to
     * mismatch LUT read command sequence.
     */
#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_cache_status_t cacheStatus;
    flexspi_nor_disable_cache(&cacheStatus);
#endif

    if (flash_state.ioMode != SPINOR_OPI_MODE) /* When flash is not in OPI Mode */
    {
        switch (flash_state.manufacturerId) /* Manufacturer ID */
        {
#if FLASH_ADESTO_DEVICE_ATXP032
            case (SNOR_MFR_ADESTO):
            {
                status = flexspi_nor_enable_octal_for_adesto(base);
                if (status != kStatus_Success)
                {
                    return status;	
                }
            }
            break;
#endif
#if FLASH_GIGADEVICE_DEVICE_GD25LX256
            case (SNOR_MFR_GIGADEVICE):
            {
                status = flexspi_nor_enable_octal_for_gigadevice(base);
                if (status != kStatus_Success)
                {
                    return status;	
                }
            }
            break;
#endif
#if FLASH_MACRONIX_DEVICE_MX25UM51345G
            case (SNOR_MFR_MACRONIX):
            {
                status = flexspi_nor_enable_octal_for_macronix(base);
                if (status != kStatus_Success)
                {
                    return status;	
                }
            }
            break;
#endif
            default:
                assert(false); /* Not support the nor flash */
                break;
        }

    }

    /* Do software reset. */
    FLEXSPI_SoftwareReset(base);

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_nor_enable_cache(cacheStatus);
#endif

    return status;
}

status_t flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address)
{
    status_t status;

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_cache_status_t cacheStatus;
    flexspi_nor_disable_cache(&cacheStatus);
#endif

    /* Write enable */
    status = flexspi_nor_write_enable(base, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    status =
        flexspi_nor_exec_op(base, address, FLASH_PORT, kFLEXSPI_Command, NOR_CMD_LUT_SEQ_IDX_ERASESECTOR_DTR_OPI, 1, NULL, 0);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    /* Do software reset. */
    FLEXSPI_SoftwareReset(base);

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_nor_enable_cache(cacheStatus);
#endif

    return status;
}

status_t flexspi_nor_flash_page_program(FLEXSPI_Type *base, uint32_t dstAddr, const uint32_t *src)
{
    status_t status;

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_cache_status_t cacheStatus;
    flexspi_nor_disable_cache(&cacheStatus);
#endif

    /* Write neable */
    status = flexspi_nor_write_enable(base, dstAddr);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Prepare page program command */
    status = flexspi_nor_exec_op_with_edma(base, dstAddr, FLASH_PORT, kFLEXSPI_Write, NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_DTR_OPI, 1,
                                 (uint32_t *)src, FLASH_PAGE_SIZE);

    if (status != kStatus_Success)
    {
        return status;
    }

    /*  Wait for transfer completed. */
    while (!g_completionFlag)
    {
    }
    g_completionFlag = false;

    status = flexspi_nor_wait_bus_busy(base);

    /* Do software reset. */
    FLEXSPI_SoftwareReset(base);

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_nor_enable_cache(cacheStatus);
#endif

    return status;
}

#if defined(__ICCARM__)
#pragma optimize = none
#endif
__attribute__((weak)) status_t flexspi_nor_get_id(FLEXSPI_Type *base, uint8_t *Id)
{
    /* Read manufacturer ID based on JEP106V spec, max continuation code table is 9, max manufacturer ID starts from
     * 9 + 1. */
    uint8_t id[SPI_NOR_MAX_ID_LEN] = {0x00U};
    status_t status                = kStatus_Fail;
    uint8_t seqIdx                 = NOR_CMD_LUT_SEQ_IDX_READID_SPI;

    if (flash_state.ioMode == SPINOR_OPI_MODE)
    {
        seqIdx = NOR_CMD_LUT_SEQ_IDX_READID_DTR_OPI;
    }

    status = flexspi_nor_exec_op(base, 0, FLASH_PORT, kFLEXSPI_Read, seqIdx, 1, (uint32_t *)id, sizeof(id));

    if (status == kStatus_Success)
    {
        for (uint8_t i = 0x00U; i < sizeof(id); i++)
        {
            if (CFI_CONTINUATION != id[i])
            {
                *Id++ = id[i];
            }
        }
    }

    return status;
}

status_t flexspi_nor_flash_init(FLEXSPI_Type *base, uint8_t *id)
{
    flexspi_config_t config;
    /* To store custom's LUT table in local. */
    uint32_t tempCustomLUT[CUSTOM_LUT_LENGTH] = {0U};
    status_t status;
    uint32_t val = 0;
    uint32_t tempReadLUTCmdSeq[4];

    (void)val; /* drop warning */
    /* memcpy/memset api is running in nor flash, so make sure that copy the data as soon as possible */

    memset((void *)&flash_state, 0 , sizeof(flash_state));
    memcpy(tempReadLUTCmdSeq, commonReadLUTCmdSeq, sizeof(tempReadLUTCmdSeq));
#if defined(FLASH_MACRONIX_DEVICE_MX25UM51345G) && FLASH_MACRONIX_DEVICE_MX25UM51345G
#if defined(FLASH_GIGADEVICE_DEVICE_GD25LX256) && FLASH_GIGADEVICE_DEVICE_GD25LX256
#error "Cannot define two nor flash, MX25UM51345G's fast read command is different with GD25LX256, so pls define FLASH_GIGADEVICE_DEVICE_GD25LX256 as 0"
#endif
#if defined(FLASH_ADESTO_DEVICE_ATXP032) && FLASH_ADESTO_DEVICE_ATXP032
#error "Cannot define two nor flash, MX25UM51345G's fast read command is different with ATXP032, so pls define FLASH_ADESTO_DEVICE_ATXP032 as 0"
#endif
    memcpy(tempReadLUTCmdSeq, ReadLUTCmdSeq_MACRONIX, sizeof(tempReadLUTCmdSeq));
#endif

    /*
     * Copy LUT information from flash region into RAM region, because flash will be reset and back to single mode;
     * In lately time, LUT table assignment maybe failed after flash reset due to LUT read entry is application's
     * required mode(such as octal DDR mode) and flash is being in single SDR mode, they don't matched.
     */
    memcpy(tempCustomLUT, commonLUTOctalMode, sizeof(tempCustomLUT));

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_cache_status_t cacheStatus;
    flexspi_nor_disable_cache(&cacheStatus);
#endif

    /*Get FLEXSPI default settings and configure the flexspi. */
    FLEXSPI_GetDefaultConfig(&config);

    /*Set AHB buffer size for reading data through AHB bus. */
    config.ahbConfig.enableAHBPrefetch = true;
    config.rxSampleClock               = EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK;
#if !(defined(FSL_FEATURE_FLEXSPI_HAS_NO_MCR0_COMBINATIONEN) && FSL_FEATURE_FLEXSPI_HAS_NO_MCR0_COMBINATIONEN)
    config.enableCombination = true;
#endif
    config.ahbConfig.enableAHBBufferable = true;
    config.ahbConfig.enableAHBCachable   = true;
    FLEXSPI_Init(base, &config); /* The FLEXSPI_Init api does flexspi software reset */

    /* Configure flash settings according to serial flash feature. */
    FLEXSPI_SetFlashConfig(base, &deviceconfig, FLASH_PORT);

    /* Update LUT table into a specific mode, such as octal SDR mode or octal DDR mode based on application's
     * requirement. */
    FLEXSPI_UpdateLUT(base, 0, tempCustomLUT, ARRAY_SIZE(tempCustomLUT));

    FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, tempReadLUTCmdSeq,
                      4); /* Make sure that cpu could load instruction from flash correctly */

    /* Do software reset. */
    FLEXSPI_SoftwareReset(base);

#if defined(EXAMPLE_FLASH_RESET_CONFIG)
    EXAMPLE_FLASH_RESET_CONFIG();
#endif

    /* Flash is in SPI Mode after reseting flash */
    flash_state.ioMode = SPINOR_SPI_MODE;
    flash_state.opMode = SPINOR_STR_MODE;
    status = flexspi_nor_get_id(base, id);
    if (status != kStatus_Success)
    {
        return status;
    }
    flash_state.manufacturerId = id[0];

    switch (flash_state.manufacturerId) /* Manufacturer ID */
    {
#if FLASH_ADESTO_DEVICE_ATXP032
        case (SNOR_MFR_ADESTO):
        {
            /*
             * Copy LUT information from flash region into RAM region, because LUT update maybe corrupt read
             * sequence(LUT[0]) and load wrong LUT table from FLASH region.
             */

            memcpy(tempCustomLUT, LUTOctalMode_ADESTO, sizeof(tempCustomLUT));
            memcpy(tempReadLUTCmdSeq, ReadLUTCmdSeq_ADESTO, sizeof(tempReadLUTCmdSeq));
            /*
             * Update LUT table into a specific mode, such as octal SDR mode or octal DDR mode based on application's
             * requirement.
             */

            FLEXSPI_UpdateLUT(base, 0, tempCustomLUT, ARRAY_SIZE(tempCustomLUT));
            if (flash_state.ioMode == SPINOR_SPI_MODE)
            {
                /*
                 * Make sure that cpu could load instruction from flash correctly(cpu will load
                 * instruction when execute the function EXAMPLE_FLASH_RESET_CONFIG).
                 */
                FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, tempReadLUTCmdSeq, 4);
            }
        }
        break;
#endif
#if FLASH_GIGADEVICE_DEVICE_GD25LX256
        case (SNOR_MFR_GIGADEVICE):
        {
            if (flash_state.ioMode == SPINOR_SPI_MODE)
            {
                uint32_t tempLUT[4] = {0};

                /*
                 * Copy LUT information from flash region into RAM region, because LUT update maybe corrupt read
                 * sequence(LUT[0]) and load wrong LUT table from FLASH region.
                 */
                memcpy(tempCustomLUT, LUTOctalMode_GIGADEVICE, sizeof(tempCustomLUT));
                memcpy(tempReadLUTCmdSeq, ReadLUTCmdSeq_GIGADEVICE, sizeof(tempReadLUTCmdSeq));
                /*
                 * Update LUT table into a specific mode, such as octal SDR mode or octal DDR mode based on
                 * application's requirement.
                 */

                FLEXSPI_UpdateLUT(
                    base, 0, tempCustomLUT,
                    ARRAY_SIZE(tempCustomLUT)); /* NOR_CMD_LUT_SEQ_IDX_READ lookup table will be replaced */

                FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, tempReadLUTCmdSeq,
                                  4); /* Make sure that cpu could load instruction from flash correctly */

                memset((void *)tempLUT, 0, sizeof(tempLUT));

                tempLUT[0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINOR_OP_RDFSR,
                                             kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, SPINOR_DATA_SIZE_4_BYTES);
                /* Update LUT table. */
                FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_CONFIG, tempLUT, ARRAY_SIZE(tempLUT));

                /* Read flag status register in SDR SPI Mode */
                status =
                    flexspi_nor_exec_op(base, 0, FLASH_PORT, kFLEXSPI_Read, NOR_CMD_LUT_SEQ_IDX_CONFIG, 1, &val, 1);
                if (status == kStatus_Success)
                {
                    memset((void *)tempLUT, 0, sizeof(tempLUT));
                    /* check address mode(Flag Status Register FS0, ADS), in 4-Byte address mode when ADS = 1 */
                    flash_state.addrMode = val & FSR_ADS_GIGADEVICE;

                    if (flash_state.addrMode != true)
                    {
                        /* change from 3-byte address mode to 4-byte address mode in SDR SPI Mode */
                        tempLUT[0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD,
                                                     SPINOR_OP_ENABLE_4_BYTE_ADDR_MODE_0XB7,
                                                     kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x0);
                        /* Update LUT table. */
                        FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_CONFIG, tempLUT, ARRAY_SIZE(tempLUT));
                        /* Enable 4-Byte Mode in SPI SDR Mode */
                        status = flexspi_nor_exec_op(base, 0, FLASH_PORT, kFLEXSPI_Command, NOR_CMD_LUT_SEQ_IDX_CONFIG, 1,
                                                     NULL, 0);

                        if (status == kStatus_Success)
                        {
                            tempLUT[0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINOR_OP_RDFSR,
                                                         kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, SPINOR_DATA_SIZE_4_BYTES);
                            /* Update LUT table. */
                            FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_CONFIG, tempLUT, ARRAY_SIZE(tempLUT));

                            /* Read flag status register in SDR SPI Mode */
                            status = flexspi_nor_exec_op(base, 0, FLASH_PORT, kFLEXSPI_Read, NOR_CMD_LUT_SEQ_IDX_CONFIG,
                                                         1, &val, 1);
                            if (status != kStatus_Success)
                            {
                                return status;
                            }
                            else
                            {
                                if (val & FSR_ADS_GIGADEVICE)
                                {
                                    flash_state.addrMode = true;
                                }
                            }
                        }
                    }
                }
                else
                {
                    return status;
                }
            }
        }
        break;
#endif
#if FLASH_MACRONIX_DEVICE_MX25UM51345G
        case (SNOR_MFR_MACRONIX):
        {
            /*
             * Copy LUT information from flash region into RAM region, because LUT update maybe corrupt read
             * sequence(LUT[0]) and load wrong LUT table from FLASH region.
             */

            memcpy(tempCustomLUT, LUTOctalMode_MACRONIX, sizeof(tempCustomLUT));
            memcpy(tempReadLUTCmdSeq, ReadLUTCmdSeq_MACRONIX, sizeof(tempReadLUTCmdSeq));
            /*
             * Update LUT table into a specific mode, such as octal SDR mode or octal DDR mode based on application's
             * requirement.
             */

            FLEXSPI_UpdateLUT(base, 0, tempCustomLUT, ARRAY_SIZE(tempCustomLUT));
            if (flash_state.ioMode == SPINOR_SPI_MODE)
            {
                FLEXSPI_UpdateLUT(base, 4 * NOR_CMD_LUT_SEQ_IDX_READ, tempReadLUTCmdSeq,
                                      4); /* Make sure that cpu could load instruction from flash correctly */
            }
        }
        break;
#endif
        default:
	    assert(false); /* not support the nor flash */
            break;
    }

    /* Create handle for flexspi. */
    FLEXSPI_TransferCreateHandleEDMA(base, &flexspiHandle, flexspi_callback, NULL, &dmaTxHandle, &dmaRxHandle);

#if defined(EXAMPLE_INVALIDATE_FLEXSPI_CACHE)
    EXAMPLE_INVALIDATE_FLEXSPI_CACHE();
#endif

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_nor_enable_cache(cacheStatus);
#endif

    return status;
}
