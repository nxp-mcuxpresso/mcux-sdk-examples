/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_flexspi_nor_flash.h"
#include "fsl_debug_console.h"

#include "fsl_common.h"
#include "pin_mux.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FlexSpiInstance           0U
#define EXAMPLE_FLEXSPI_AMBA_BASE FlexSPI0_AMBA_BASE
#define FLASH_SIZE                0x02000000UL /* 32MBytes */
#define FLASH_PAGE_SIZE           256UL        /* 256Bytes */
#define FLASH_SECTOR_SIZE         0x1000UL     /* 4KBytes */
#define FLASH_BLOCK_SIZE          0x10000UL    /* 64KBytes */
#define BUFFER_LEN FLASH_PAGE_SIZE

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void error_trap(void);
void app_finalize(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief config serial NOR option  */
static serial_nor_config_option_t option = {
    .option0.U = 0xc0000001U,
    .option1.U = 0U,
};

/*! @brief FLEXSPI NOR flash driver Structure */
static flexspi_nor_config_t norConfig;
/*! @brief Buffer for program */
static uint8_t s_buffer[BUFFER_LEN];
/*! @brief Buffer for readback */
static uint8_t s_buffer_rbc[BUFFER_LEN];

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * @brief Gets called when an error occurs.
 *
 * @details Print error message and trap forever.
 */
void error_trap(void)
{
    PRINTF("\r\n\r\n\r\n\t---- HALTED DUE TO FLEXSPI NOR ERROR! ----");
    while (1)
    {
    }
}

/*
 * @brief Gets called when the app is complete.
 *
 * @details Print finshed message and trap forever.
 */
void app_finalize(void)
{
    /* Print finished message. */
    PRINTF("\r\n End of FLEXSPI NOR Example! \r\n");
    while (1)
    {
    }
}

int main(void)
{
    status_t status;
    uint32_t i = 0U;
    uint32_t serialNorAddress; /* Address of the serial nor device location */
    uint32_t AHBNorAddress;    /* Access the serial nor flash via AHB bus */
    uint32_t serialNorTotalSize;
    uint32_t serialNorSectorSize;
    uint32_t serialNorPageSize;

    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("\r\n FLEXSPI NOR example started!\r\n");
    /* Clean up FLEXSPI NOR flash driver Structure */
    memset(&norConfig, 0U, sizeof(flexspi_nor_config_t));
    /* Setup FLEXSPI NOR Configuration Block */
    status = FLEXSPI_NorFlash_GetConfig(FlexSpiInstance, &norConfig, &option);
    if (status == kStatus_Success)
    {
        PRINTF("\r\n Successfully get FLEXSPI NOR configuration block\r\n ");
    }
    else
    {
        PRINTF("\r\n Get FLEXSPI NOR configuration block failure!\r\n");
        error_trap();
    }

    /* Initializes the FLEXSPI module for the other FLEXSPI APIs */
    status = FLEXSPI_NorFlash_Init(FlexSpiInstance, &norConfig);
    if (status == kStatus_Success)
    {
        PRINTF("\r\n Successfully init FLEXSPI serial NOR flash\r\n ");
    }
    else
    {
        PRINTF("\r\n Erase sector failure !\r\n");
        error_trap();
    }

    serialNorTotalSize  = norConfig.memConfig.sflashA1Size;
    serialNorSectorSize = norConfig.sectorSize;
    serialNorPageSize   = norConfig.pageSize;

    /* Print serial NOR flash information */
    PRINTF("\r\n Serial NOR flash Information: ");
    PRINTF("\r\n Serial NOR flash size:\t%d KB, Hex: (0x%x)", (serialNorTotalSize / 1024U), serialNorTotalSize);
    PRINTF("\r\n Serial NOR flash sector size:\t%d KB, Hex: (0x%x) ", (serialNorSectorSize / 1024U),
           serialNorSectorSize);
    PRINTF("\r\n Serial NOR flash page size:\t%d B, Hex: (0x%x)\r\n", serialNorPageSize, serialNorPageSize);

/*
 * SECTOR_INDEX_FROM_END = 1 means the last sector,
 * SECTOR_INDEX_FROM_END = 2 means (the last sector - 1) ...
 */
#ifndef SECTOR_INDEX_FROM_END
#define SECTOR_INDEX_FROM_END 1U
#endif
    /* Erase a sector from target device dest address */
    serialNorAddress = serialNorTotalSize - (SECTOR_INDEX_FROM_END * serialNorSectorSize);
    AHBNorAddress    = EXAMPLE_FLEXSPI_AMBA_BASE + serialNorAddress;

    /* Erase one sector. */
    PRINTF("\r\n Erasing serial NOR flash over FLEXSPI");
    status = FLEXSPI_NorFlash_Erase(FlexSpiInstance, &norConfig, serialNorAddress, serialNorSectorSize);
    if (status == kStatus_Success)
    {
        /* Print message for user. */
        PRINTF("\r\n Successfully erased one sector of NOR flash device 0x%x -> 0x%x\r\n", serialNorAddress,
               (serialNorAddress + serialNorSectorSize));
    }
    else
    {
        PRINTF("\r\n Erase sector failure!\r\n");
        error_trap();
    }

    PRINTF("\r\n Program a buffer to a page of NOR flash\r\n");
    /* Prepare user buffer. */
    for (i = 0; i < BUFFER_LEN; i++)
    {
        s_buffer[i] = i;
    }

    /* Program user buffer into FLEXSPI NOR flash */
    status = FLEXSPI_NorFlash_ProgramPage(FlexSpiInstance, &norConfig, serialNorAddress, (const uint32_t *)s_buffer);
    if (status != kStatus_Success)
    {
        PRINTF("\r\n Page program failure!\r\n");
        error_trap();
    }

    /* Verify programming by reading back from FLEXSPI AMBA memory directly */
    PRINTF("\r\n Access serial NOR flash through AHB\r\n ");
    memcpy(s_buffer_rbc, (void *)(AHBNorAddress), sizeof(s_buffer_rbc));
    if (memcmp(s_buffer_rbc, s_buffer, sizeof(s_buffer)) == 0)
    {
        PRINTF("\r\n Successfully programmed and verified location FLEXSPI AMBA memory 0x%x -> 0x%x \r\n",
               (AHBNorAddress), (AHBNorAddress + sizeof(s_buffer)));
    }
    else
    {
        PRINTF("\r\n Program data -  read out data value incorrect!\r\n ");
        error_trap();
    }

    /* Erase the context we have progeammed before*/
    status = FLEXSPI_NorFlash_Erase(FlexSpiInstance, &norConfig, serialNorAddress, serialNorSectorSize);

    app_finalize();

    return 0;
}
