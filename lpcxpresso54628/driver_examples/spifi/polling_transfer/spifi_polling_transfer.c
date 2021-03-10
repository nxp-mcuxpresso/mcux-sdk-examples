/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_spifi.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SPIFI        SPIFI0
#define PAGE_SIZE            (256)
#define SECTOR_SIZE          (4096)
#define EXAMPLE_SPI_BAUDRATE (96000000)
#define FLASH_W25Q
#define COMMAND_NUM    (6)
#define READ           (0)
#define PROGRAM_PAGE   (1)
#define GET_STATUS     (2)
#define ERASE_SECTOR   (3)
#define WRITE_ENABLE   (4)
#define WRITE_REGISTER (5)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_buffer[PAGE_SIZE] = {0};

#if defined FLASH_W25Q
spifi_command_t command[COMMAND_NUM] = {
    {PAGE_SIZE, false, kSPIFI_DataInput, 1, kSPIFI_CommandDataQuad, kSPIFI_CommandOpcodeAddrThreeBytes, 0x6B},
    {PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandDataQuad, kSPIFI_CommandOpcodeAddrThreeBytes, 0x32},
    {1, false, kSPIFI_DataInput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x05},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0x20},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x06},
    {1, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x31}};
#define QUAD_MODE_VAL 0x02
#elif defined FLASH_MX25R
spifi_command_t command[COMMAND_NUM] = {
    {PAGE_SIZE, false, kSPIFI_DataInput, 1, kSPIFI_CommandDataQuad, kSPIFI_CommandOpcodeAddrThreeBytes, 0x6B},
    {PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandOpcodeSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0x38},
    {1, false, kSPIFI_DataInput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x05},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0x20},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x06},
    {1, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x01}};
#define QUAD_MODE_VAL 0x40
#else /* Use MT25Q */
spifi_command_t command[COMMAND_NUM] = {
    {PAGE_SIZE, false, kSPIFI_DataInput, 1, kSPIFI_CommandDataQuad, kSPIFI_CommandOpcodeAddrThreeBytes, 0x6B},
    {PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandOpcodeSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0x38},
    {1, false, kSPIFI_DataInput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x05},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0x20},
    {0, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x06},
    {1, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial, kSPIFI_CommandOpcodeOnly, 0x61}};
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
void check_if_finish()
{
    uint8_t val = 0;
    /* Check WIP bit */
    do
    {
        SPIFI_SetCommand(EXAMPLE_SPIFI, &command[GET_STATUS]);
        while ((EXAMPLE_SPIFI->STAT & SPIFI_STAT_INTRQ_MASK) == 0U)
        {
        }
        val = SPIFI_ReadDataByte(EXAMPLE_SPIFI);
    } while (val & 0x1);
}

#if defined QUAD_MODE_VAL
void enable_quad_mode()
{
    /* Write enable */
    SPIFI_SetCommand(EXAMPLE_SPIFI, &command[WRITE_ENABLE]);

    /* Set write register command */
    SPIFI_SetCommand(EXAMPLE_SPIFI, &command[WRITE_REGISTER]);

    SPIFI_WriteDataByte(EXAMPLE_SPIFI, QUAD_MODE_VAL);

    check_if_finish();
}
#endif

int main(void)
{
    spifi_config_t config = {0};
    uint32_t sourceClockFreq;
    uint32_t i = 0, j = 0, data = 0, page = 0, err = 0;
    uint8_t *val = (uint8_t *)FSL_FEATURE_SPIFI_START_ADDR;

    /* Init the boards */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to SPI3 */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM9);

    BOARD_InitPins();
    BOARD_BootClockPLL220M(); /* Boot up FROHF96M for SPIFI to use*/
    BOARD_BootClockPLL220M(); /* Core clock boot to 220Mhz*/
    BOARD_InitDebugConsole();
    PRINTF("SPIFI flash polling example started \r\n");
    /* Set SPIFI clock source */
    CLOCK_AttachClk(kFRO_HF_to_SPIFI_CLK);
    sourceClockFreq = CLOCK_GetFroHfFreq();

    /* Set the clock divider */
    CLOCK_SetClkDiv(kCLOCK_DivSpifiClk, sourceClockFreq / EXAMPLE_SPI_BAUDRATE, false);

    /* Initialize SPIFI */
    SPIFI_GetDefaultConfig(&config);
    SPIFI_Init(EXAMPLE_SPIFI, &config);

#if defined QUAD_MODE_VAL
    /* Enable Quad mode */
    enable_quad_mode();
#endif

    /* Setup memory command */
    SPIFI_SetMemoryCommand(EXAMPLE_SPIFI, &command[READ]);

    /* Set the buffer */
    for (i = 0; i < PAGE_SIZE; i++)
    {
        g_buffer[i] = i;
    }

    /* Reset the SPIFI to switch to command mode */
    SPIFI_ResetCommand(EXAMPLE_SPIFI);
    EnableIRQ(SPIFI0_IRQn);

    /* Write enable */
    SPIFI_SetCommand(EXAMPLE_SPIFI, &command[WRITE_ENABLE]);
    /* Set address */
    SPIFI_SetCommandAddress(EXAMPLE_SPIFI, 0U);
    /* Erase sector */
    SPIFI_SetCommand(EXAMPLE_SPIFI, &command[ERASE_SECTOR]);
    /* Check if finished */
    check_if_finish();

    /* Program page */
    while (page < (SECTOR_SIZE / PAGE_SIZE))
    {
        SPIFI_SetCommand(EXAMPLE_SPIFI, &command[WRITE_ENABLE]);
        SPIFI_SetCommandAddress(EXAMPLE_SPIFI, page * PAGE_SIZE);
        SPIFI_SetCommand(EXAMPLE_SPIFI, &command[PROGRAM_PAGE]);
        for (i = 0; i < PAGE_SIZE; i += 4)
        {
            for (j = 0; j < 4; j++)
            {
                data |= ((uint32_t)(g_buffer[i + j])) << (j * 8);
            }
            SPIFI_WriteData(EXAMPLE_SPIFI, data);
            data = 0;
        }
        page++;
        check_if_finish();
    }

    /* Reset to memory command mode */
    SPIFI_ResetCommand(EXAMPLE_SPIFI);

    SPIFI_SetMemoryCommand(EXAMPLE_SPIFI, &command[READ]);

    for (i = 0; i < SECTOR_SIZE; i++)
    {
        val = (uint8_t *)(FSL_FEATURE_SPIFI_START_ADDR + i);
        if (*val != g_buffer[i % PAGE_SIZE])
        {
            PRINTF("Data error in address 0x%x, the value in memory is 0x%x\r\n", i, *val);
            err++;
        }
    }

    if (err == 0)
    {
        PRINTF("All data written is correct!\r\n");
    }

    PRINTF("SPIFI Polling example Finished!\r\n");

    while (1)
    {
    }
}
