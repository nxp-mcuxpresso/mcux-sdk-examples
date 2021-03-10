/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_eeprom.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EEPROM_SOURCE_CLOCK kCLOCK_BusClk
#define EEPROM_CLK_FREQ     CLOCK_GetFreq(kCLOCK_BusClk)
#define EXAMPLE_EEPROM      EEPROM

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#define FSL_FEATURE_EEPROM_PAGE_SIZE (FSL_FEATURE_EEPROM_SIZE / FSL_FEATURE_EEPROM_PAGE_COUNT)
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t data[FSL_FEATURE_EEPROM_PAGE_SIZE / 4];
    uint32_t pageNum = FSL_FEATURE_EEPROM_PAGE_COUNT;
    uint32_t i = 0, j = 0, err = 0;
    uint32_t eeprom_data = 0xFFFFFFFFU;
    eeprom_config_t config;
    uint32_t sourceClock_Hz = 0;

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL220M();
    BOARD_InitDebugConsole();

    PRINTF("EEPROM example begins...\r\n");

    /* Init EEPROM */
    EEPROM_GetDefaultConfig(&config);
    sourceClock_Hz = EEPROM_CLK_FREQ;
    EEPROM_Init(EXAMPLE_EEPROM, &config, sourceClock_Hz);

    /* Prepare page data to write */
    for (i = 0; i < FSL_FEATURE_EEPROM_PAGE_SIZE / 4; i++)
    {
        data[i] = i;
    }

    /* Write all data into eeprom */
    for (i = 0; i < pageNum - 1; i++)
    {
        EEPROM_WritePage(EXAMPLE_EEPROM, i, data);
        for (j = 0; j < FSL_FEATURE_EEPROM_PAGE_SIZE / 4; j++)
        {
            eeprom_data = *((uint32_t *)(FSL_FEATURE_EEPROM_BASE_ADDRESS + i * FSL_FEATURE_EEPROM_PAGE_SIZE + j * 4));
            if (eeprom_data != data[j])
            {
                err++;
                PRINTF("Page %d offset %d is wrong, data is %x \r\n", i, (j * 4), eeprom_data);
            }
        }
        PRINTF("Page %d program finished!\r\n", i);
    }

    if (err == 0)
    {
        PRINTF("All data is correct! EEPROM example succeed!\r\n");
    }

    while (1)
    {
    }
}
