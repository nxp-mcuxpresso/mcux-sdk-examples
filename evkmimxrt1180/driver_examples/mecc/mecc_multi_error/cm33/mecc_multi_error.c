/*
 * Copyright 2019, 2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_mecc.h"
#include "fsl_cache.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_MECC                      MECC2
#define APP_MECC_IRQ                  MECC2_IRQn
#define APP_MECC_IRQ_HANDLER          MECC2_IRQHandler
#define APP_MECC_OCRAM_SIZE           256 * 1024                    /* 256KB */
#define APP_MECC_OCRAM_START_ADDR     0x20500000                    /* OCRAM2 256KB */
#define APP_MECC_OCRAM_END_ADDR       0x2053FFFF
#define APP_MECC_OCRAM_ADDR_OFFSET    0x20                          /* Offset 0x20 from Ocram start address */
#define APP_MECC_OCRAM_SELECTED_BANK  0U                            /* Ocram bank 0 */
#define APP_MECC_MULTI_BIT_POSTION    2U                            /* 0-base */
#if (APP_MECC_OCRAM_SELECTED_BANK == 0)
#define APP_INTERRUPT_SOURCES     (kMECC_MultiError0InterruptEnable)
#define APP_INTERRUPT_STATUS_FLAGS  (kMECC_MultiError0InterruptFlag)

#elif (APP_MECC_OCRAM_SELECTED_BANK == 1)
#define APP_INTERRUPT_SOURCES     (kMECC_MultiError1InterruptEnable)
#define APP_INTERRUPT_STATUS_FLAGS  (kMECC_MultiError1InterruptFlag)

#elif (APP_MECC_OCRAM_SELECTED_BANK == 2)
#define APP_INTERRUPT_SOURCES     (kMECC_MultiError2InterruptEnable)
#define APP_INTERRUPT_STATUS_FLAGS  (kMECC_MultiError2InterruptFlag)

#elif (APP_MECC_OCRAM_SELECTED_BANK == 3)
#define APP_INTERRUPT_SOURCES     (kMECC_MultiError3InterruptEnable)
#define APP_INTERRUPT_STATUS_FLAGS  (kMECC_MultiError3InterruptFlag)

#else
#error "Wrong Bank Selected"
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile bool s_mecc_ocram_multi_error = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_MECC_IRQ_HANDLER(void)
{
    uint32_t intStatus;

    intStatus = MECC_GetStatusFlags(APP_MECC);
    MECC_ClearStatusFlags(APP_MECC, intStatus);

    if (intStatus & APP_INTERRUPT_STATUS_FLAGS)
    {
        s_mecc_ocram_multi_error = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    status_t status             = kStatus_Success;
    bool errorFlag              = false;
    uint64_t *ocramAddr         = (uint64_t *)(APP_MECC_OCRAM_START_ADDR + APP_MECC_OCRAM_ADDR_OFFSET);
    uint64_t temp               = 0U;
    uint32_t lowRawData         = 0U; /* Low 32 bits ocram read data*/
    uint32_t highRawData        = 0U; /* High 32 bits ocram read data*/
    uint32_t lowMultiErrorData  = 0U; /* Low 32 bits multiple error data*/
    uint32_t highMultiErrorData = 0U; /* High 32 bits multiple error data*/
    /*
     *   Bank0: ocram_base_address+0x20*i
     *   Bank1: ocram_base_address+0x20*i+0x8
     *   Bank2: ocram_base_address+0x20*i+0x10
     *   Bank3: ocram_base_address+0x20*i+0x18
     *   i = 0,1,2,3,4.....
     */
    uint8_t selectedBank         = APP_MECC_OCRAM_SELECTED_BANK;
    uint8_t multiErrorBitPosLow  = APP_MECC_MULTI_BIT_POSTION;
    uint8_t multiErrorBitPosHigh = APP_MECC_MULTI_BIT_POSTION;
    uint8_t multiErrorEccData    = 0U;
    mecc_multi_error_info_t multiErrorInfo;
    mecc_config_t config;

    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\r\nMECC multiple error example.\r\n");

    /* Get default configuration */
    MECC_GetDefaultConfig(&config);

    /* Enable MECC */
    config.enableMecc         = true;
    config.startAddress = APP_MECC_OCRAM_START_ADDR;
    config.endAddress   = APP_MECC_OCRAM_END_ADDR;

    /* Initialize mecc */
    MECC_Init(APP_MECC, &config);
    /* Enable IRQ */
    EnableIRQ(APP_MECC_IRQ);

    /* Write original data */
    *ocramAddr = 0x1122334444332211;
    __DSB();

    /* Read original data */
    temp        = *ocramAddr;
    lowRawData  = temp & 0xFFFFFFFF;
    highRawData = (temp & 0xFFFFFFFF00000000) >> 32;

    /* Check ocram data */
    if (lowRawData == 0x44332211 && highRawData == 0x11223344)
    {
        PRINTF("\r\nOriginal ocram data correct.\r\n");
    }
    else
    {
        errorFlag = true;
        PRINTF("\r\nOriginal ocram data incorrect.\r\n");
    }

    /* Enable MECC OCRAM multiple error interrupt */
    MECC_EnableInterrupts(APP_MECC, APP_INTERRUPT_SOURCES);

    /* Multiple error injection at ocram bank 0 */
    lowMultiErrorData  = 1 << multiErrorBitPosLow;  /* Multiple error data: 0x44332215 */
    highMultiErrorData = 1 << multiErrorBitPosHigh; /* Multiple error data: 0x11223340 */
    status = MECC_ErrorInjection(APP_MECC, lowMultiErrorData, highMultiErrorData, multiErrorEccData, selectedBank);
    if (status != kStatus_Success)
    {
        errorFlag = true;
        PRINTF("\r\nMultiple error injection fail.\r\n");
    }
    else
    {
        PRINTF("\r\nMultiple error injection success.\r\n");
    }

    /* Flush cache line to make sure data be in cache and be in main memory are latest.*/
    DCACHE_CleanInvalidateByRange(APP_MECC_OCRAM_START_ADDR + APP_MECC_OCRAM_ADDR_OFFSET, 32);

    /* Write original data */
    *ocramAddr = 0x1122334444332211;
    __DSB();

    /* Read corrected ocram data with ECC function */
    lowRawData  = 0U;
    highRawData = 0U;

    /* Flush cache line to make sure data be in cache and be in main memory are latest.*/
    DCACHE_CleanInvalidateByRange(APP_MECC_OCRAM_START_ADDR + APP_MECC_OCRAM_ADDR_OFFSET, 32);

    temp        = *ocramAddr;
    lowRawData  = temp & 0xFFFFFFFF;
    highRawData = (temp & 0xFFFFFFFF00000000) >> 32;

    /* Wait for normal interrupt */
    while (s_mecc_ocram_multi_error == false)
    {
    }

    s_mecc_ocram_multi_error = false;

    /* Check multiple error ocram data */
    if (lowRawData == 0x44332211 && highRawData == 0x11223344)
    {
        errorFlag = true;
        PRINTF("\r\nMultiple error injection failed.\r\n");
    }
    else
    {
        PRINTF("\r\nMultiple error injection success.\r\n");
    }

    /* Read multiple error information */
    status = MECC_GetMultiErrorInfo(APP_MECC, &multiErrorInfo, selectedBank);
    if (status != kStatus_Success)
    {
        PRINTF("\r\nMultiple error information failed.\r\n");
    }
    else
    {
        PRINTF("\r\nMultiple error information success.\r\n");
    }

    /* Check multiple error address */
    if (multiErrorInfo.multiErrorAddress != APP_MECC_OCRAM_ADDR_OFFSET)
    {
        errorFlag = true;
        PRINTF("\r\nMultiple error address failed.\r\n");
    }
    else
    {
        PRINTF("\r\nMultiple error address: 0x%x.\r\n", APP_MECC_OCRAM_START_ADDR + multiErrorInfo.multiErrorAddress);
    }

    /* Check multiple error ecc code */
    PRINTF("\r\nMultiple error ecc code: 0x%x.\r\n", multiErrorInfo.multiErrorEccCode);

    /* Check multiple error low 32 bits data */
    if (multiErrorInfo.multiErrorDataLow != 0x44332215)
    {
        errorFlag = true;
        PRINTF("\r\nMultiple error low 32 bits data failed.\r\n");
    }
    else
    {
        PRINTF("\r\nMultiple error low 32 bits data: 0x%x.\r\n", multiErrorInfo.multiErrorDataLow);
    }

    /* Check multiple error high 32 bits data */
    if (multiErrorInfo.multiErrorDataHigh != 0x11223340)
    {
        errorFlag = true;
        PRINTF("\r\nMultiple error high 32 bits data failed.\r\n");
    }
    else
    {
        PRINTF("\r\nMultiple error high 32 bits data: 0x%x.\r\n", multiErrorInfo.multiErrorDataHigh);
    }

    if (errorFlag)
    {
        PRINTF("\r\nMECC Multiple error example finished with error.\r\n");
    }
    else
    {
        PRINTF("\r\nMECC Multiple error example finished successfully.\r\n");
    }

    while (1)
    {
    }
}
