/*
 * Copyright 2021, 2024 NXP
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
#include "clock_config.h"
#include "board.h"

#include "fsl_mem_interface.h"
#include "fsl_iped.h"

#include <string.h>

#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FLASH_OPTION_SEL 0xc0000001
#define CPU_CLK          CLOCK_GetFreq(kCLOCK_CoreSysClk)

#define PLAIN_DATA0_START_ADDR     0x80001000U
#define ENCRYPTED_DATA0_START_ADDR 0x80002000U
#define PLAIN_DATA1_START_ADDR     0x80003000U

#define IPED_REGION_SIZE 0x1000U

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief config API context structure */
static api_core_context_t apiCoreCtx = {0};

/*! @brief config API initialization data structure */
static kp_api_init_param_t apiInitParam = {
    .allocStart = 0x30010000, /* Allocate an area from ram for storing configuration information. */
    .allocSize  = 0x6000      /* Configuration information size. */
};

/*! @brief config API initialization data structure */
static flexspi_iped_region_arg_t flashConfigOptionIped = {
    .option = {.tag = IPED_TAG, .iped_region = 0, .iped_mode = kIPED_ModeCtr},
    .start  = ENCRYPTED_DATA0_START_ADDR,
    .end    = PLAIN_DATA1_START_ADDR};

serial_nor_config_option_t flashConfigOption = {.option0 = {.U = FLASH_OPTION_SEL}};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Compares buffer with specified 32bit word pattern */
static inline bool memcmp_word_patter(uint32_t *buffer, uint32_t pattern, size_t size)
{
    __ISB();
    __DSB();

    register size_t i      = 0;
    register uint32_t *ptr = NULL;
    while (i < ((size / sizeof(uint32_t)) - 1))
    {
        ptr = buffer + i;
        if (memcmp((void *)ptr, &pattern, sizeof(uint32_t)) != 0)
        {
            return false;
        }
        i++;
    }

    return true;
}

/*!
 * @brief Main function.
 */
int main(void)
{
    status_t status = kStatus_Fail;

    /* Init hardware */
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    CLOCK_EnableClock(kCLOCK_Flexspi);
    RESET_ClearPeripheralReset(kFLEXSPI_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    FLEXSPI_Type *FLEXSPI_APP_BASE  = FLEXSPI0;

    const uint8_t iv[8] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    };

    PRINTF("IPED Peripheral Driver Example\r\n\r\n");

    PRINTF("Calling API_Init\r\n");
    status = API_Init(&apiCoreCtx, &apiInitParam);
    if (status == kStatus_Success)
    {
        PRINTF("API_Init Successfully\r\n");
    }
    else
    {
        PRINTF("API_Init failure!!!\r\n");
    }

    IPED_EncryptEnable(FLEXSPI_APP_BASE );

    /* Configure IPED for on the fly encryption/decryption. IPED needs to be configured before External FLASH */
    PRINTF("Configure IPED region %d enc/dec: start 0x%x end 0x%x\r\n", flashConfigOptionIped.option.iped_region,
           flashConfigOptionIped.start, flashConfigOptionIped.end);

    status = IPED_Configure(FLEXSPI_APP_BASE , &apiCoreCtx, &flashConfigOptionIped, kIPED_RegionUnlock, kIPED_SkipCMPA);

    /* In a real-world scenario, the IV needs to be carefully chosen to avoid re-using it. */
    /* Ideally use that one which is calculated from UUID and aditional information. */
    /* So it uses same IV as a ROM*/
    IPED_SetRegionIV(FLEXSPI_APP_BASE , flashConfigOptionIped.option.iped_region, iv);

    if (status == kStatus_Success)
    {
        PRINTF("Configure IPED  Successfully\r\n");
    }
    else
    {
        PRINTF("Configure IPED failure!!!\r\n");
    }

    /* Configure External FLASH */
    status = MEM_Config(&apiCoreCtx, (uint32_t *)&flashConfigOption, kMemoryFlexSpiNor);
    if (status == kStatus_Success)
    {
        PRINTF("External Flash memory configured successfully\r\n");
    }
    else
    {
        PRINTF("External Flash memory configure FAIL!!!\r\n");
    }

    /* Turn-off FLEXSPI CACHE and BUFFERING to be able read directly from ext memory */
    FLEXSPI_APP_BASE ->MCR0 |= FLEXSPI_MCR0_SWRESET_MASK;
    FLEXSPI_APP_BASE ->AHBCR &= ~FLEXSPI_AHBCR_CACHABLEEN_MASK;
    FLEXSPI_APP_BASE ->AHBCR &= ~FLEXSPI_AHBCR_BUFFERABLEEN_MASK;
    FLEXSPI_APP_BASE ->AHBCR &= ~FLEXSPI_AHBCR_PREFETCHEN_MASK;

    /* Erase ext memory used in this example (0x80001000-0x80003FFF) */
    status = MEM_Erase(&apiCoreCtx, PLAIN_DATA0_START_ADDR, IPED_REGION_SIZE, kMemoryFlexSpiNor);
    status = MEM_Erase(&apiCoreCtx, flashConfigOptionIped.start, IPED_REGION_SIZE, kMemoryFlexSpiNor);
    status = MEM_Erase(&apiCoreCtx, PLAIN_DATA1_START_ADDR, IPED_REGION_SIZE, kMemoryFlexSpiNor);

    if (status != kStatus_Success)
    {
        PRINTF("Erase FAIL!!!\r\n");
    }

    /* Fill memory with plaintext data */
    status = MEM_Fill(&apiCoreCtx, PLAIN_DATA0_START_ADDR, IPED_REGION_SIZE, 0x11223344, kMemoryFlexSpiNor);

    /* Fill memory with data to be encrypted */
    status = MEM_Fill(&apiCoreCtx, flashConfigOptionIped.start, IPED_REGION_SIZE, 0xaabbccdd, kMemoryFlexSpiNor);

    /* Fill memory with plaintext data */
    status = MEM_Fill(&apiCoreCtx, PLAIN_DATA1_START_ADDR, IPED_REGION_SIZE, 0x11223344, kMemoryFlexSpiNor);

    /* Wait a while after programming memory */
    SDK_DelayAtLeastUs(100, CPU_CLK);

    if (status != kStatus_Success)
    {
        PRINTF("Memory Fill FAIL!!!\r\n");
    }

    /* Test plain data write */
    if (memcmp_word_patter((void *)PLAIN_DATA0_START_ADDR, 0x11223344, IPED_REGION_SIZE) != true)
    {
        PRINTF("Program plain data FAIL!!!");
    }
    else
    {
        PRINTF("*Success* read plain data from 0x80001000 to 0x80001FFF\r\n");
    }

    /* Test encrypted data write */
    if (memcmp_word_patter((void *)flashConfigOptionIped.start, 0xaabbccdd, IPED_REGION_SIZE) != true)
    {
        PRINTF("Program ecrypted data FAIL!!! \r\n");
    }
    else
    {
        PRINTF("*Success* read programmed & encrypted data from 0x80002000 to 0x80002FFF\r\n");
    }

    /* Test plain data write */
    if (memcmp_word_patter((void *)PLAIN_DATA1_START_ADDR, 0x11223344, IPED_REGION_SIZE) != true)
    {
        PRINTF("Program plain data FAIL!!!\r\n");
    }
    else
    {
        PRINTF("*Success* read plain data from 0x80003000 to 0x80003FFF\r\n");
    }

    /* Call IPED Reconfigure with provided configuration, in  real application should be NULL and use CMPA instead. */
    /* This should be called after wakeup from the Power Down mode */
    PRINTF("Reconfiguring IPED \r\n");
    status = IPED_Reconfigure(FLEXSPI_APP_BASE , &apiCoreCtx, &flashConfigOptionIped);
    if (status == kStatus_Success)
    {
        PRINTF("Reconfigure IPED  Successfully\r\n");
    }
    else
    {
        PRINTF("Reconfigure IPED failure!!\r\n");
    }

    /* Test plain text data read */
    if (memcmp_word_patter((void *)PLAIN_DATA0_START_ADDR, 0x11223344, IPED_REGION_SIZE) != true)
    {
        PRINTF("Read plaintext FAIL!!!\r\n");
    }
    else
    {
        PRINTF("*Success* read plain data from 0x8001000 to 0x8001FFF\r\n");
    }

    /* Test decrypted data read */
    /* Because we do not use IV computed from UUID and aditional information, we will not read same data */
    if (memcmp_word_patter((void *)flashConfigOptionIped.start, 0xaabbccdd, IPED_REGION_SIZE) != false)
    {
        PRINTF("Encryption FAIL!!!\r\n");
    }
    else
    {
        PRINTF("*Success* read decrypted data from 0x8002000 to 0x8002FFF\r\n");
    }

    /* Test plain text data read */
    if (memcmp_word_patter((void *)PLAIN_DATA1_START_ADDR, 0x11223344, IPED_REGION_SIZE) != true)
    {
        PRINTF("Read plaintext FAIL!!!\r\n");
    }
    else
    {
        PRINTF("*Success* read plain data from 0x8003000 to 0x8003FFF\r\n");
    }

    /* Disable IPED */
    PRINTF("Disabling IPED\r\n");
    IPED_EncryptDisable(FLEXSPI_APP_BASE );

    /* Test plain text data read */
    if (memcmp_word_patter((void *)PLAIN_DATA0_START_ADDR, 0x11223344, IPED_REGION_SIZE) != true)
    {
        PRINTF("Read plaintext FAIL!!!\r\n");
    }
    else
    {
        PRINTF("*Success* read plain data from 0x80001000 to 0x80001FFF\r\n");
    }

    /* Test decrypted data read */
    if (memcmp_word_patter((void *)flashConfigOptionIped.start, 0xaabbccdd, IPED_REGION_SIZE) != false)
    {
        PRINTF("Encryption FAIL!!!\r\n");
    }
    else
    {
        PRINTF("*Success* read ciphered data from 0x80002000 to 0x80002FFF\r\n");
    }

    /* Test plain text data read */
    if (memcmp_word_patter((void *)PLAIN_DATA1_START_ADDR, 0x11223344, IPED_REGION_SIZE) != true)
    {
        PRINTF("Read plaintext FAIL!!!\r\n");
    }
    else
    {
        PRINTF("*Success* read plain data from 0x80003000 to 0x80003FFF\r\n");
    }

    PRINTF("End of example\r\n");
    /* End of example */
    while (1)
    {
    }
}
