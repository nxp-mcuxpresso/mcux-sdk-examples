/*
 * Copyright 2019 NXP
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
#define APP_MECC                      MECC1
#define APP_MECC_IRQ                  MECC1_INT_IRQn
#define APP_MECC_IRQ_HANDLER          MECC1_INT_IRQHandler
#define APP_MECC_OCRAM_SIZE           512 * 1024 /* 512KB */
#define APP_MECC_OCRAM_START_ADDR     0x20240000 /* OCRAM1 512KB */
#define APP_MECC_OCRAM_END_ADDR       0x202BFFFF
#define APP_MECC_OCRAM_ADDR_OFFSET    0x20                          /* Offset 0x20 from Ocram start address */
#define APP_MECC_OCRAM_ECC_START_ADDR (0x20240000 + 512 * 1024 + 8) /* OCRAM1 ECC 64KB */
#define APP_MECC_OCRAM_SELECTED_BANK  0U                            /* Ocram bank 0 */
#define APP_MECC_SINGLE_BIT_POSTION   2U                            /* 0-base */


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile bool s_mecc_ocram_single_error = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_MECC_IRQ_HANDLER(void)
{
    uint32_t intStatus;

    intStatus = MECC_GetStatusFlags(APP_MECC);
    MECC_ClearStatusFlags(APP_MECC, intStatus);

    if (intStatus & kMECC_SingleError0InterruptFlag)
    {
        s_mecc_ocram_single_error = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    status_t status              = kStatus_Success;
    bool errorFlag               = false;
    uint64_t *ocramAddr          = (uint64_t *)(APP_MECC_OCRAM_START_ADDR + APP_MECC_OCRAM_ADDR_OFFSET);
    uint64_t temp                = 0U;
    uint32_t lowRawData          = 0U; /* Low 32 bits ocram read data*/
    uint32_t highRawData         = 0U; /* High 32 bits ocram read data*/
    uint32_t lowSingleErrorData  = 0U; /* Low 32 bits single error data*/
    uint32_t highSingleErrorData = 0U; /* High 32 bits single error data*/
    /*
     *   Bank0: ocram_base_address+0x20*i
     *   Bank1: ocram_base_address+0x20*i+0x8
     *   Bank2: ocram_base_address+0x20*i+0x10
     *   Bank3: ocram_base_address+0x20*i+0x18
     *   i = 0,1,2,3,4.....
     */
    uint8_t selectedBank          = APP_MECC_OCRAM_SELECTED_BANK;
    uint8_t singleErrorBitPosLow  = APP_MECC_SINGLE_BIT_POSTION;
    uint8_t singleErrorBitPosHigh = 0U;
    uint8_t singleErrorEccData    = 0U;
    mecc_single_error_info_t singleErrorInfo;
    mecc_config_t config;

    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\r\nMECC single error example.\r\n");

    /* Get default configuration */
    MECC_GetDefaultConfig(&config);

    /* Enable MECC */
    config.enableMecc         = true;
    config.Ocram1StartAddress = APP_MECC_OCRAM_START_ADDR;
    config.Ocram1EndAddress   = APP_MECC_OCRAM_END_ADDR;

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

    /* Enable MECC OCRAM single error interrupt */
    MECC_EnableInterrupts(APP_MECC, kMECC_SingleError0InterruptEnable);

    /* Single error injection at ocram bank 0 */
    lowSingleErrorData = 1 << singleErrorBitPosLow; /* Single error data: 0x44332215 */
    status = MECC_ErrorInjection(APP_MECC, lowSingleErrorData, highSingleErrorData, singleErrorEccData, selectedBank);
    if (status != kStatus_Success)
    {
        errorFlag = true;
        PRINTF("\r\nSingle error injection fail.\r\n");
    }
    else
    {
        PRINTF("\r\nSingle error injection success.\r\n");
    }

    /* Flush cache line to make sure data be in cache and be in main memory are latest.*/
    L1CACHE_CleanInvalidateDCacheByRange(APP_MECC_OCRAM_START_ADDR + APP_MECC_OCRAM_ADDR_OFFSET, 32);

    /* Write original data */
    *ocramAddr = 0x1122334444332211;
    __DSB();

    /* Read corrected ocram data with ECC function */
    lowRawData  = 0U;
    highRawData = 0U;

    /* Flush cache line to make sure data be in cache and be in main memory are latest.*/
    L1CACHE_CleanInvalidateDCacheByRange(APP_MECC_OCRAM_START_ADDR + APP_MECC_OCRAM_ADDR_OFFSET, 32);

    temp        = *ocramAddr;
    lowRawData  = temp & 0xFFFFFFFF;
    highRawData = (temp & 0xFFFFFFFF00000000) >> 32;

    /* Wait for normal interrupt */
    while (s_mecc_ocram_single_error == false)
    {
    }

    s_mecc_ocram_single_error = false;

    /* Check corrected ocram data */
    if (lowRawData == 0x44332211 && highRawData == 0x11223344)
    {
        PRINTF("\r\nOriginal ocram data correct.\r\n");
    }
    else
    {
        errorFlag = true;
        PRINTF("\r\nOriginal ocram data incorrect.\r\n");
    }

    /* Read single error information */
    status = MECC_GetSingleErrorInfo(APP_MECC, &singleErrorInfo, selectedBank);
    if (status != kStatus_Success)
    {
        errorFlag = true;
        PRINTF("\r\nSingle error information failed.\r\n");
    }
    else
    {
        PRINTF("\r\nSingle error information success.\r\n");
    }

    /* Check single error address */
    if (singleErrorInfo.singleErrorAddress != APP_MECC_OCRAM_ADDR_OFFSET)
    {
        errorFlag = true;
        PRINTF("\r\nSingle error address failed.\r\n");
    }
    else
    {
        PRINTF("\r\nSingle error address: 0x%x.\r\n", APP_MECC_OCRAM_START_ADDR + singleErrorInfo.singleErrorAddress);
    }

    /* Check single error ecc code */
    PRINTF("\r\nSingle error ecc code: 0x%x.\r\n", singleErrorInfo.singleErrorEccCode);

    /* Check single error low 32 bits data */
    if (singleErrorInfo.singleErrorDataLow != 0x44332215)
    {
        errorFlag = true;
        PRINTF("\r\nSingle error low 32 bits data failed.\r\n");
    }
    else
    {
        PRINTF("\r\nSingle error low 32 bits data: 0x%x.\r\n", singleErrorInfo.singleErrorDataLow);
    }

    /* Check single error high 32 bits data */
    if (singleErrorInfo.singleErrorDataHigh != 0x11223344)
    {
        errorFlag = true;
        PRINTF("\r\nSingle error high 32 bits data failed.\r\n");
    }
    else
    {
        PRINTF("\r\nSingle error high 32 bits data: 0x%x.\r\n", singleErrorInfo.singleErrorDataHigh);
    }

    /* Check single error bit position of low 32 bits */
    if (singleErrorInfo.singleErrorPosLow != singleErrorBitPosLow)
    {
        errorFlag = true;
        PRINTF("\r\nSingle error bit position of low 32 bits failed.\r\n");
    }
    else
    {
        PRINTF("\r\nSingle error bit position of low 32 bits: 0x%x.\r\n", singleErrorInfo.singleErrorPosLow);
    }

    /* Check single error bit position of high 32 bits */
    if (singleErrorInfo.singleErrorPosHigh != singleErrorBitPosHigh)
    {
        errorFlag = true;
        PRINTF("\r\nSingle error bit position of high 32 bits failed.\r\n");
    }
    else
    {
        PRINTF("\r\nSingle error bit position of high 32 bits: 0x%x.\r\n", singleErrorInfo.singleErrorPosHigh);
    }

    if (errorFlag)
    {
        PRINTF("\r\nMECC single error example finished with error.\r\n");
    }
    else
    {
        PRINTF("\r\nMECC single error example finished successfully.\r\n");
    }

    while (1)
    {
    }
}
