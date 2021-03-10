/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "mx25r_flash.h"
#include "fsl_spi.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SECTOR_ADDR                 0
#define SECTOR_SIZE                 4096
#define FLASH_SPI_SSEL              3
#define EXAMPLE_SPI_MASTER          SPI5
#define EXAMPLE_SPI_MASTER_IRQn     FLEXCOMM5_IRQn
#define EXAMPLE_SPI_MASTER_CLK_SRC  kCLOCK_Flexcomm5
#define EXAMPLE_SPI_MASTER_CLK_FREQ CLOCK_GetFlexCommClkFreq(5)
#define EXAMPLE_SPI_SPOL            kSPI_SpolActiveAllLow
#define BUFFER_SIZE 64

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
char g_buffer[BUFFER_SIZE] = {0};
struct mx25r_instance mx25r;
static spi_master_handle_t g_handle;
volatile bool transfer_is_done = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void masterCallback(SPI_Type *base, spi_master_handle_t *handle, status_t status, void *userData)
{
    transfer_is_done = true;
}

int flash_transfer_cb(void *transfer_prv, uint8_t *tx_data, uint8_t *rx_data, size_t dataSize, bool eof)
{
    spi_transfer_t xfer = {0};
    xfer.txData         = tx_data;
    xfer.rxData         = rx_data;
    xfer.dataSize       = dataSize;
    /* terminate frame */
    if (eof)
    {
        xfer.configFlags |= kSPI_FrameAssert;
    }

    transfer_is_done = false;
    /* transfer nonblocking */
    SPI_MasterTransferNonBlocking((SPI_Type *)transfer_prv, &g_handle, &xfer);
    /* until transfer ends */
    while (!transfer_is_done)
    {
    }

    return 0;
}

int flash_init(void)
{
    spi_master_config_t masterConfig = {0};
    SPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.direction    = kSPI_MsbFirst;
    masterConfig.polarity     = kSPI_ClockPolarityActiveHigh;
    masterConfig.phase        = kSPI_ClockPhaseFirstEdge;
    masterConfig.baudRate_Bps = 100000;
    masterConfig.sselNum      = (spi_ssel_t)FLASH_SPI_SSEL;
    masterConfig.sselPol      = (spi_spol_t)EXAMPLE_SPI_SPOL;
    SPI_MasterInit(EXAMPLE_SPI_MASTER, &masterConfig, EXAMPLE_SPI_MASTER_CLK_FREQ);
    SPI_MasterTransferCreateHandle(EXAMPLE_SPI_MASTER, &g_handle, masterCallback, NULL);
    mx25r_init(&mx25r, flash_transfer_cb, EXAMPLE_SPI_MASTER);
    return mx25r_err_ok;
}

mx25r_err_t flash_is_sector_aligned(uint32_t address)
{
    return (address % SECTOR_SIZE) ? mx25r_err_alignement : mx25r_err_ok;
}

bool flash_is_buffer_empty(char *buffer, uint32_t size)
{
    assert(NULL != buffer);
    int i = 0;
    for (; ((i < size) && (buffer[i] == 0xFF)); i++)
    {
    }
    return i == size ? true : false;
}

int flash_read_buffer(uint32_t address, char *buffer, char size)
{
    /* check sector alignement */
    int status = flash_is_sector_aligned(address);
    if (mx25r_err_ok != status)
    {
        return status;
    }
    /* read command */
    status = mx25r_cmd_read(&mx25r, address, (uint8_t *)buffer, size);
    if (mx25r_err_ok != status)
    {
        PRINTF("'mx25r_cmd_read' failed \r\n");
    }
    return status;
}

int flash_erase_sector(uint32_t address)
{
    /* check sector alignement */
    int status = flash_is_sector_aligned(address);
    if (mx25r_err_ok != status)
    {
        return status;
    }
    /* erase command */
    status = mx25r_cmd_sector_erase(&mx25r, address);
    if (mx25r_err_ok != status)
    {
        PRINTF("'mx25r_cmd_sector_erase' failed \r\n");
    }
    return status;
}

int flash_write_buffer(uint32_t address, char *buffer, char size)
{
    /* check sector alignement */
    int status = flash_is_sector_aligned(address);
    if (mx25r_err_ok != status)
    {
        return status;
    }
    /* write command */
    status = mx25r_cmd_write(&mx25r, address, (uint8_t *)buffer, size);
    if (mx25r_err_ok != status)
    {
        PRINTF("'mx25r_cmd_write' failed \r\n");
    }
    return status;
}

void read_string(char *buffer, int size)
{
    int character;
    int i = 0;
    for (; i + 1 < size; i++)
    {
        character = GETCHAR();
        if (('\n' == character) || ('\r' == character))
        {
            break;
        }
        buffer[i] = character;
        PUTCHAR(character);
    }
    buffer[i] = 0;
}

int app(void)
{
    int status;
    if (mx25r_err_ok != flash_init())
    {
        return -1;
    }

    /* read buffer from flash */
    status = flash_read_buffer(SECTOR_ADDR, g_buffer, BUFFER_SIZE);
    if (mx25r_err_ok != status)
    {
        return status;
    }
    /* consider buffer as empty */
    if (flash_is_buffer_empty(g_buffer, BUFFER_SIZE))
    {
        PRINTF("Flash at 0x%x of size %d B is empty \r\n", SECTOR_ADDR, BUFFER_SIZE);
    }
    else
    {
        PRINTF("Flash at 0x%x of size %d B has message '%s' \r\n", SECTOR_ADDR, BUFFER_SIZE, g_buffer);
    }

    /* read user message */
    PRINTF("Write new message (max %d chars) to flash: \r\n", BUFFER_SIZE - 1);
    read_string(g_buffer, BUFFER_SIZE);
    PRINTF("\r\nmessage is: '%s' \r\n", g_buffer);

    if (!flash_is_buffer_empty(g_buffer, BUFFER_SIZE))
    {
        status = flash_erase_sector(SECTOR_ADDR);
        if (mx25r_err_ok != status)
        {
            return status;
        }
    }

    status = flash_write_buffer(0x0, g_buffer, BUFFER_SIZE);
    if (mx25r_err_ok != status)
    {
        return status;
    }

    return 0;
}

int main(void)
{
    /* Init the boards */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to SPI5 */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM5);

    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(kFC5_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();
    PRINTF("*****app*start***** \r\n");
    if (0 == app())
    {
        PRINTF("Write succeed, please restart the board to see the written message \r\n");
    }
    else
    {
        PRINTF("Write failed \r\n");
    }
    PRINTF("*****app*end***** \r\n");
    while (1)
    {
    }
}
