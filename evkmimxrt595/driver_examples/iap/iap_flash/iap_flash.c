/*
 * Copyright 2019,2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <string.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_iap.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_NOR_INSTANCE       1U        /* FLEXSPI0 */
#define NOR_FLASH_OP_START_ADDRESS 0x200000U /* Operation on 2MB offset */
#define NOR_FLASH_OP_SIZE          0x2000U   /* Test 8KB region */

/* Predefined flash option */
#define EXAMPLE_NOR_FLASH_QSPI      0xC0000000U /* QuadSPI flash */
#define EXAMPLE_NOR_FLASH_QSPI_DDR  0xC0100000U /* QuadSPI DDR flash */
#define EXAMPLE_NOR_FLASH_HYPER_1V8 0xC0233001U /* 1.8V Hyper flash */
#define EXAMPLE_NOR_FLASH_HYPER_3V0 0xC0333000U /* 3.0V Hyper flash */
#define EXAMPLE_NOR_FLASH_MXIC_OSPI 0xC0403000U /* Macronix OctalSPI flash */
#define EXAMPLE_NOR_FLASH_MCRN_OSPI 0xC0600000U /* Micron OctalSPI flash */
#define EXAMPLE_NOR_FLASH_ADST_OSPI 0xC0803000U /* Adesto OctalSPI flash */

#define EXAMPLE_NOR_FLASH         EXAMPLE_NOR_FLASH_MXIC_OSPI /* MXIC flash on EVK board */
#define EXAMPLE_NOR_FLASH_OPTION1 0U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * variables
 ******************************************************************************/

static uint32_t mem_writeBuffer[(NOR_FLASH_OP_SIZE + 3) / 4];
static uint32_t mem_readBuffer[(NOR_FLASH_OP_SIZE + 3) / 4];

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    status_t status;
    flexspi_nor_config_t config;
    serial_nor_config_option_t option;
    uint32_t i;
    uint32_t pages, irqMask;
    uint8_t *pReadBuf  = (uint8_t *)mem_readBuffer;
    uint8_t *pWriteBuf = (uint8_t *)mem_writeBuffer;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\r\nIAP flash example started!\r\n");

    option.option0.U = EXAMPLE_NOR_FLASH;
    option.option1.U = EXAMPLE_NOR_FLASH_OPTION1;

    do
    {
#if defined(FLASH_NEED_RESET) && FLASH_NEED_RESET
        status = BOARD_FlexspiInit(EXAMPLE_NOR_INSTANCE, &config, &option);
#else
        status = IAP_FlexspiNorAutoConfig(EXAMPLE_NOR_INSTANCE, &config, &option);
#endif
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Initialization Failed!***\r\n");
            break;
        }
        PRINTF("\r\n***NOR Flash Initialization Success!***\r\n");

        /* Erase sectors */
        irqMask = DisableGlobalIRQ();
        status  = IAP_FlexspiNorErase(EXAMPLE_NOR_INSTANCE, &config, NOR_FLASH_OP_START_ADDRESS, NOR_FLASH_OP_SIZE);
        EnableGlobalIRQ(irqMask);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Erase Failed!***\r\n");
            break;
        }

        status = IAP_FlexspiNorRead(EXAMPLE_NOR_INSTANCE, &config, mem_readBuffer, NOR_FLASH_OP_START_ADDRESS,
                                    NOR_FLASH_OP_SIZE);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Read Failed!***\r\n");
            break;
        }

        for (i = 0; i < NOR_FLASH_OP_SIZE; i++)
        {
            if (pReadBuf[i] != 0xFF)
            {
                PRINTF("\r\n***NOR Flash Erase Check Failed!***\r\n");
                break;
            }
        }

        PRINTF("\r\n***NOR Flash Erase Success!***\r\n");

        /* Program the page data. */
        /* Initialize the write buffers. */
        for (i = 0; i < NOR_FLASH_OP_SIZE; i++)
        {
            pWriteBuf[i] = i & 0xFF;
        }

        pages   = NOR_FLASH_OP_SIZE / config.pageSize;
        irqMask = DisableGlobalIRQ();
        for (i = 0; i < pages; i++)
        {
            status = IAP_FlexspiNorPageProgram(EXAMPLE_NOR_INSTANCE, &config,
                                               NOR_FLASH_OP_START_ADDRESS + i * config.pageSize, mem_writeBuffer);
            if (status != kStatus_Success)
            {
                PRINTF("\r\n***NOR Flash Page %d Program Failed!***\r\n", i);
                break;
            }
        }
        EnableGlobalIRQ(irqMask);

        status = IAP_FlexspiNorRead(EXAMPLE_NOR_INSTANCE, &config, mem_readBuffer, NOR_FLASH_OP_START_ADDRESS,
                                    NOR_FLASH_OP_SIZE);
        if (status != kStatus_Success)
        {
            PRINTF("\r\n***NOR Flash Read Failed!***\r\n");
            break;
        }

        if (memcmp(mem_writeBuffer, mem_readBuffer, NOR_FLASH_OP_SIZE) != 0)
        {
            PRINTF("\r\n***NOR Flash Page %d Read/Write Failed!***\r\n", i);
            break;
        }

        PRINTF("\r\n***NOR Flash All Pages Read/Write Success!***\r\n");
    } while (0);

    while (1)
    {
    }
}
