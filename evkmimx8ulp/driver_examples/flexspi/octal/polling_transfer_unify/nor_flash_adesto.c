/*
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "app.h"
#include "fsl_flexspi.h"
#include "nor_flash.h"

/* For ATXP032 */
const uint32_t ReadLUTCmdSeq_ADESTO[4] = {
    /*  Fast read in Single SDR mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ + 0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                         kFLEXSPI_1PAD,
                                                         SPINOR_OP_READ_FAST,
                                                         kFLEXSPI_Command_RADDR_SDR,
                                                         kFLEXSPI_1PAD,
                                                         SPINOR_ADDRESS_32_BITS),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ + 1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_SDR,
                                                         kFLEXSPI_1PAD,
                                                         SPINOR_DUMMY_CYCLE_NUMBER_0X8,
                                                         kFLEXSPI_Command_READ_SDR,
                                                         kFLEXSPI_1PAD,
                                                         SPINOR_DATA_SIZE_4_BYTES),
};

const uint32_t OctalReadLUTCmdSeq_ADESTO[4] = {
    /*  OPI SDR read */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ + 0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_OP_READ_FAST,
                                                         kFLEXSPI_Command_RADDR_SDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_ADDRESS_32_BITS),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ + 1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_SDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_DUMMY_CYCLE_NUMBER_0X8,
                                                         kFLEXSPI_Command_READ_SDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_DATA_SIZE_4_BYTES),
};

const uint32_t OctalDDRReadLUTCmdSeq_ADESTO[4] = {
    /*  OPI DDR read */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ + 0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_OP_READ_FAST,
                                                         kFLEXSPI_Command_RADDR_DDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_ADDRESS_32_BITS),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ + 1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_DDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_DUMMY_CYCLE_NUMBER_0X8,
                                                         kFLEXSPI_Command_READ_DDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_DATA_SIZE_4_BYTES),
};



const uint32_t LUTOctalMode_ADESTO[CUSTOM_LUT_LENGTH] = {

    /*  OPI DDR fast read */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ + 0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_OP_READ_FAST,
                                                         kFLEXSPI_Command_RADDR_DDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_ADDRESS_32_BITS),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ + 1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_DDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_DUMMY_CYCLE_NUMBER_0X8,
                                                         kFLEXSPI_Command_READ_DDR,
                                                         kFLEXSPI_8PAD,
                                                         SPINOR_DATA_SIZE_4_BYTES),

    /* Read status register - SPI Mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUS_SPI] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                           kFLEXSPI_1PAD,
                                                           SPINOR_OP_RDSR1,
                                                           kFLEXSPI_Command_READ_SDR,
                                                           kFLEXSPI_1PAD,
                                                           SPINOR_DATA_SIZE_4_BYTES),

    /* Default config is for global unprotect entire memory - SDR SPI Mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_CONFIG] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                       kFLEXSPI_1PAD,
                                                       SPINOR_OP_WRSR,
                                                       kFLEXSPI_Command_WRITE_SDR,
                                                       kFLEXSPI_1PAD,
                                                       SPINOR_DATA_SIZE_4_BYTES),

    /* Write Enable - SPI Mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_SPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, SPINOR_OP_WREN, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Read ID with SPI Mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_READID_SPI] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                           kFLEXSPI_1PAD,
                                                           SPINOR_OP_RDID,
                                                           kFLEXSPI_Command_READ_SDR,
                                                           kFLEXSPI_1PAD,
                                                           SPINOR_DATA_SIZE_4_BYTES),

    /*  Write Enable - DDR OPI Mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_DTR_OPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, SPINOR_OP_WREN, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /*  Erase Sector - OPI DDR Mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_ERASESECTOR_DTR_OPI] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                            kFLEXSPI_8PAD,
                                                            SPINOR_OP_BE_4K,
                                                            kFLEXSPI_Command_RADDR_DDR,
                                                            kFLEXSPI_8PAD,
                                                            SPINOR_ADDRESS_32_BITS),

    /*  Erase Chip - OPI Mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_CHIPERASE_DTR_OPI] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, SPINOR_OP_CHIP_ERASE, kFLEXSPI_Command_STOP, kFLEXSPI_8PAD, 0),

    /*  Program - OPI DDR Mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_DTR_OPI]     = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                            kFLEXSPI_8PAD,
                                                            SPINOR_OP_PP,
                                                            kFLEXSPI_Command_RADDR_DDR,
                                                            kFLEXSPI_8PAD,
                                                            SPINOR_ADDRESS_32_BITS),
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_DTR_OPI + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_WRITE_DDR, kFLEXSPI_8PAD, SPINOR_DATA_SIZE_4_BYTES, kFLEXSPI_Command_STOP, kFLEXSPI_8PAD, 0),

    /* Enter OPI mode(Octal Mode) - SPI */
    [4 * NOR_CMD_LUT_SEQ_IDX_ENTEROPI] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                         kFLEXSPI_1PAD,
                                                         SPINOR_OP_ENTER_OCTAL_MODE_0XE8,
                                                         kFLEXSPI_Command_STOP,
                                                         kFLEXSPI_1PAD,
                                                         0),

    /*  Dummy write, do nothing when AHB write command is triggered. */
    [4 * NOR_CMD_LUT_SEQ_IDX_WRITE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x0, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x0),

    /*  Read status register using Octal SDR */
    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUS_STR_OPI]     = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                               kFLEXSPI_8PAD,
                                                               SPINOR_OP_RDSR1,
                                                               kFLEXSPI_Command_DUMMY_SDR,
                                                               kFLEXSPI_8PAD,
                                                               SPINOR_DUMMY_CYCLE_NUMBER_0X4),
    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUS_STR_OPI + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_READ_SDR, kFLEXSPI_8PAD, SPINOR_DATA_SIZE_4_BYTES, kFLEXSPI_Command_STOP, kFLEXSPI_8PAD, 0x0),

    /*  Read status register using Octal DDR */
    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUS_DTR_OPI]     = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR,
                                                               kFLEXSPI_8PAD,
                                                               SPINOR_OP_RDSR1,
                                                               kFLEXSPI_Command_DUMMY_DDR,
                                                               kFLEXSPI_8PAD,
                                                               SPINOR_DUMMY_CYCLE_NUMBER_0X4),
    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUS_DTR_OPI + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_READ_DDR, kFLEXSPI_8PAD, SPINOR_DATA_SIZE_4_BYTES, kFLEXSPI_Command_STOP, kFLEXSPI_8PAD, 0x0),
};
