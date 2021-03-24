/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "board.h"
#include "app_flash.h"
#include "fsl_clock.h"
#include "fsl_qspi.h"
#include "fsl_cache.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define QSPI_ALIAS_BASE       0x04000000U
#define QSPI_CLK_FREQ         CLOCK_GetIpFreq(kCLOCK_Qspi)
#define FLASH_PAGE_SIZE       256U
#define FLASH_SECTOR_SIZE     4096U
#define FLASH_SIZE            0x00800000U
#define FLASH_ENABLE_QUAD_CMD 0x40U

static uint32_t lut[FSL_FEATURE_QSPI_LUT_DEPTH] =
    {/* Seq0 :Quad Read */
     /* CMD:        0xEB - Quad Read, Single pad */
     /* ADDR:       0x18 - 24bit address, Quad pads */
     /* DUMMY:      0x06 - 6 clock cyles, Quad pads */
     /* READ:       0x80 - Read 128 bytes, Quad pads */
     /* JUMP_ON_CS: 0 */
     [0] = 0x0A1804EB,
     [1] = 0x1E800E06,
     [2] = 0x2400,

     /* Seq1: Write Enable */
     /* CMD:      0x06 - Write Enable, Single pad */
     [4] = 0x406,

     /* Seq2: Erase All */
     /* CMD:    0x60 - Erase All chip, Single pad */
     [8] = 0x460,

     /* Seq3: Read Status */
     /* CMD:    0x05 - Read Status, single pad */
     /* READ:   0x01 - Read 1 byte */
     [12] = 0x1c010405,

     /* Seq4: Page Program */
     /* CMD:    0x02 - Page Program, Single pad */
     /* ADDR:   0x18 - 24bit address, Single pad */
     /* WRITE:  0x80 - Write 128 bytes at one pass, Single pad */
     [16] = 0x08180402,
     [17] = 0x2080,

     /* Seq5: Write Register */
     /* CMD:    0x01 - Write Status Register, single pad */
     /* WRITE:  0x01 - Write 1 byte of data, single pad */
     [20] = 0x20010401,

     /* Seq6: Read Config Register */
     /* CMD:  0x05 - Read Config register, single pad */
     /* READ: 0x01 - Read 1 byte */
     [24] = 0x1c010405,

     /* Seq7: Erase Sector */
     /* CMD:  0x20 - Sector Erase, single pad */
     /* ADDR: 0x18 - 24 bit address, single pad */
     [28] = 0x08180420,

     /* Seq8: Dummy */
     /* CMD:    0xFF - Dummy command, used to force SPI flash to exit continuous read mode */
     [32] = 0x4FF,

     /* Seq9: Fast Single read */
     /* CMD:        0x0B - Fast Read, Single Pad */
     /* ADDR:       0x18 - 24bit address, Single Pad */
     /* DUMMY:      0x08 - 8 clock cyles, Single Pad */
     /* READ:       0x80 - Read 128 bytes, Single Pad */
     /* JUMP_ON_CS: 0 */
     [36] = 0x0818040B,
     [37] = 0x1C800C08,
     [38] = 0x2400,

     /* Seq10: Fast Dual read */
     /* CMD:        0x3B - Dual Read, Single Pad */
     /* ADDR:       0x18 - 24bit address, Single Pad */
     /* DUMMY:      0x08 - 8 clock cyles, Single Pad */
     /* READ:       0x80 - Read 128 bytes, Dual pads */
     /* JUMP_ON_CS: 0 */
     [40] = 0x0818043B,
     [41] = 0x1D800C08,
     [42] = 0x2400,

     /* Match MISRA rule */
     [63] = 0};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
qspi_flash_config_t single_config = {.flashA1Size       = FLASH_SIZE, /* 8MB */
                                     .flashA2Size       = 0,
                                     .dataHoldTime      = 0,
                                     .CSHoldTime        = 0,
                                     .CSSetupTime       = 0,
                                     .cloumnspace       = 0,
                                     .dataLearnValue    = 0,
                                     .endian            = kQSPI_64LittleEndian,
                                     .enableWordAddress = false};

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Check if serial flash erase or program finished. */
static void check_if_finished(void)
{
    uint32_t val = 0;
    /* Check WIP bit */
    do
    {
        while (QSPI_GetStatusFlags(QuadSPI0) & kQSPI_Busy)
        {
        }
        QSPI_ClearFifo(QuadSPI0, kQSPI_RxFifo);
        QSPI_ExecuteIPCommand(QuadSPI0, 12U);
        while (QSPI_GetStatusFlags(QuadSPI0) & kQSPI_Busy)
        {
        }
        val = *(volatile uint32_t *)(FSL_FEATURE_QSPI_ARDB_BASE);
        /* Clear ARDB area */
        QSPI_ClearErrorFlag(QuadSPI0, kQSPI_RxBufferDrain);
    } while (val & 0x1);
}

/* Write enable command */
static void cmd_write_enable(void)
{
    while (QSPI_GetStatusFlags(QuadSPI0) & kQSPI_Busy)
    {
    }
    QSPI_ExecuteIPCommand(QuadSPI0, 4U);
}

/* Enable Quad mode */
static void enable_quad_mode(void)
{
    uint32_t val[4] = {FLASH_ENABLE_QUAD_CMD, 0, 0, 0};

    while (QSPI_GetStatusFlags(QuadSPI0) & kQSPI_Busy)
    {
    }
    QSPI_SetIPCommandAddress(QuadSPI0, FSL_FEATURE_QSPI_AMBA_BASE);

    /* Clear Tx FIFO */
    QSPI_ClearFifo(QuadSPI0, kQSPI_TxFifo);

    /* Write enable */
    cmd_write_enable();

    /* Write data into TX FIFO, needs to write at least 16 bytes of data */
    QSPI_WriteBlocking(QuadSPI0, val, 16U);

    /* Set seq id, write register */
    QSPI_ExecuteIPCommand(QuadSPI0, 20);

    /* Wait until finished */
    check_if_finished();
}

/*Erase sector */
static void erase_sector(uint32_t addr)
{
    while (QSPI_GetStatusFlags(QuadSPI0) & kQSPI_Busy)
    {
    }
    QSPI_ClearFifo(QuadSPI0, kQSPI_TxFifo);
    QSPI_SetIPCommandAddress(QuadSPI0, addr);
    cmd_write_enable();
    QSPI_ExecuteIPCommand(QuadSPI0, 28U);
    check_if_finished();
}

/* Program page into serial flash using QSPI polling way */
static void program_page(uint32_t dest_addr, uint32_t *src_addr)
{
    uint32_t leftLongWords = 0;

    while (QSPI_GetStatusFlags(QuadSPI0) & kQSPI_Busy)
    {
    }
    QSPI_ClearFifo(QuadSPI0, kQSPI_TxFifo);
    QSPI_SetIPCommandAddress(QuadSPI0, dest_addr);
    cmd_write_enable();
    while (QSPI_GetStatusFlags(QuadSPI0) & kQSPI_Busy)
    {
    }

    /* First write some data into TXFIFO to prevent from underrun */
    QSPI_WriteBlocking(QuadSPI0, src_addr, FSL_FEATURE_QSPI_TXFIFO_DEPTH * sizeof(uint32_t));
    src_addr += FSL_FEATURE_QSPI_TXFIFO_DEPTH;

    /* Start the program */
    QSPI_SetIPCommandSize(QuadSPI0, FLASH_PAGE_SIZE);
    QSPI_ExecuteIPCommand(QuadSPI0, 16U);

    leftLongWords = FLASH_PAGE_SIZE - FSL_FEATURE_QSPI_TXFIFO_DEPTH * sizeof(uint32_t);
    QSPI_WriteBlocking(QuadSPI0, src_addr, leftLongWords);

    /* Wait until flash finished program */
    check_if_finished();
    while (QSPI_GetStatusFlags(QuadSPI0) & (kQSPI_Busy | kQSPI_IPAccess))
    {
    }
}

static void reset_qspi(void)
{
    QSPI_SoftwareReset(QuadSPI0);
    while (QSPI_GetStatusFlags(QuadSPI0) & (kQSPI_Busy | kQSPI_IPAccess))
    {
    }
}

void APP_InitFlash(void)
{
    uint32_t clockSourceFreq = 0;
    qspi_config_t config     = {0};

    /*Get QSPI default settings and configure the qspi */
    QSPI_GetDefaultQspiConfig(&config);

    /*Set AHB buffer size for reading data through AHB bus */
    config.AHBbufferSize[3] = FLASH_PAGE_SIZE;
    clockSourceFreq         = QSPI_CLK_FREQ;
    QSPI_Init(QuadSPI0, &config, clockSourceFreq);

    /* Copy the LUT table */
    memcpy(single_config.lookuptable, lut, sizeof(lut));

    /*According to serial flash feature to configure flash settings */
    QSPI_SetFlashConfig(QuadSPI0, &single_config);

    /* Enable Quad mode for the flash */
    enable_quad_mode();
}

int32_t APP_EraseFlash(uint32_t offset, uint32_t bytes)
{
    uint32_t addr;
    uint32_t startAddr = FSL_FEATURE_QSPI_AMBA_BASE + offset;
    uint32_t endAddr   = startAddr + bytes;

    if ((offset & (FLASH_SECTOR_SIZE - 1)) != 0)
    {
        PRINTF("ERROR: Flash erase offset must be aligned with %d\r\n", FLASH_SECTOR_SIZE);
        return -1;
    }

    if ((bytes & (FLASH_SECTOR_SIZE - 1)) != 0)
    {
        PRINTF("ERROR: Flash erase bytes must be multiple of %d\r\n", FLASH_SECTOR_SIZE);
        return -1;
    }

    if (endAddr > FSL_FEATURE_QSPI_AMBA_BASE + FLASH_SIZE)
    {
        PRINTF("ERROR: Flash erase exceeds flash size 0x%d\r\n", FLASH_SIZE);
        return -1;
    }

    for (addr = startAddr; addr < endAddr; addr += FLASH_SECTOR_SIZE)
    {
        erase_sector(addr);
    }

    reset_qspi();

    /* Invalidate cache to synchronize the new flash data */
    DCACHE_InvalidateByRange(QSPI_ALIAS_BASE + offset, bytes);

    return 0;
}

int32_t APP_WriteFlash(uint32_t *buf, uint32_t offset, uint32_t bytes)
{
    uint32_t addr;
    uint32_t startAddr = FSL_FEATURE_QSPI_AMBA_BASE + offset;
    uint32_t endAddr   = startAddr + bytes;
    uint32_t *pBuf;
    uint32_t *pFlash;

    if (((uint32_t)buf & 3U) != 0)
    {
        PRINTF("ERROR: Flash write buffer must be aligned with 4 bytes\r\n");
        return -1;
    }

    if ((offset & (FLASH_PAGE_SIZE - 1)) != 0)
    {
        PRINTF("ERROR: Flash write offset must be aligned with %d\r\n", FLASH_PAGE_SIZE);
        return -1;
    }

    if ((bytes & (FLASH_PAGE_SIZE - 1)) != 0)
    {
        PRINTF("ERROR: Flash write bytes must be multiple of %d\r\n", FLASH_PAGE_SIZE);
        return -1;
    }

    if (endAddr > FSL_FEATURE_QSPI_AMBA_BASE + FLASH_SIZE)
    {
        PRINTF("ERROR: Flash erase exceeds flash size 0x%d\r\n", FLASH_SIZE);
        return -1;
    }

    pBuf = buf;
    for (addr = startAddr; addr < endAddr; addr += FLASH_PAGE_SIZE)
    {
        program_page(addr, pBuf);
        pBuf += FLASH_PAGE_SIZE / sizeof(uint32_t);
    }

    reset_qspi();

    /* Invalidate cache to synchronize the new flash data */
    DCACHE_InvalidateByRange(QSPI_ALIAS_BASE + offset, bytes);

    pBuf = buf;
    /* Verify the programmed data */
    for (pFlash = (uint32_t *)startAddr; pFlash < (uint32_t *)endAddr; pFlash++)
    {
        if (*pFlash != *pBuf)
        {
            PRINTF(
                "ERROR: Flash write failure:\r\n"
                "  First failure data - [0x%08x]: 0x%08x, [0x%08x]: 0x%08x\r\n",
                (uint32_t)pBuf, *pBuf, (uint32_t)pFlash, *pFlash);
            return -1;
        }
        pBuf++;
    }

    return 0;
}
