/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_flexram.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_FLEXRAM     FLEXRAM
#define APP_FLEXRAM_IRQ FLEXRAM_IRQn

#define APP_FLEXRAM_IRQ_HANDLER FLEXRAM_IRQHandler

#define APP_FLEXRAM_OCRAM_START_ADDR 0x20360000
#define APP_FLEXRAM_OCRAM_MAGIC_ADDR 0x203600A8

/*__RAM_VECTOR_TABLE_SIZE: 0x400U*/
#define APP_FLEXRAM_DTCM_START_ADDR (0x20000000 + 0x400U)
#define APP_FLEXRAM_DTCM_MAGIC_ADDR (0x20000000 + 0x400U + 0xA0);

#define APP_FLEXRAM_ITCM_START_ADDR 0x0
#define APP_FLEXRAM_ITCM_MAGIC_ADDR 0xA0

/* OCRAM relocate definition */
#define APP_OCRAM_SIZE              (512 * 1024U)
#define APP_OCRAM_ALLOCATE_BANK_NUM 4
#define APP_ITCM_ALLOCATE_BANK_NUM  8
#define APP_DTCM_ALLOCATE_BANK_NUM  4

/*
 * If cache is enabled, this example should maintain the cache to make sure
 * CPU core accesses the memory, not cache only.
 */
#define APP_USING_CACHE 1


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static status_t OCRAM_Reallocate(void);
static void OCRAM_Access(void);
static void DTCM_Access(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Boundary flag */
static volatile bool s_flexram_ocram_access_error_match = false;
static volatile bool s_flexram_dtcm_access_error_match  = false;
/* ECC error flag */
static volatile bool s_flexram_ocram_ecc_single_error = false;
static volatile bool s_flexram_dtcm_ecc_single_error  = false;
static volatile bool s_flexram_dtcm_ecc_multi_error   = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_FLEXRAM_IRQ_HANDLER(void)
{
    if (FLEXRAM_GetInterruptStatus(APP_FLEXRAM) & kFLEXRAM_OCRAMAccessError)
    {
        FLEXRAM_ClearInterruptStatus(APP_FLEXRAM, kFLEXRAM_OCRAMAccessError);
        s_flexram_ocram_access_error_match = true;
    }

    if (FLEXRAM_GetInterruptStatus(APP_FLEXRAM) & kFLEXRAM_DTCMAccessError)
    {
        FLEXRAM_ClearInterruptStatus(APP_FLEXRAM, kFLEXRAM_DTCMAccessError);
        s_flexram_dtcm_access_error_match = true;
    }

    if (FLEXRAM_GetInterruptStatus(APP_FLEXRAM) & kFLEXRAM_D0TCMECCSingleError)
    {
        FLEXRAM_ClearInterruptStatus(APP_FLEXRAM, kFLEXRAM_D0TCMECCSingleError);
        s_flexram_dtcm_ecc_single_error = true;
    }

    if (FLEXRAM_GetInterruptStatus(APP_FLEXRAM) & kFLEXRAM_D0TCMECCMultiError)
    {
        FLEXRAM_ClearInterruptStatus(APP_FLEXRAM, kFLEXRAM_D0TCMECCMultiError);
        s_flexram_dtcm_ecc_multi_error = true;
    }

    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
#if APP_USING_CACHE
#include "fsl_cache.h"
#endif

int main(void)
{
    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\r\nFLEXRAM DTCM ECC example.\r\n");

    /* enable IRQ */
    EnableIRQ(APP_FLEXRAM_IRQ);
    /* reallocate ram */
    OCRAM_Reallocate();
    /* init flexram */
    FLEXRAM_Init(APP_FLEXRAM);
    /*test OCRAM access*/
    OCRAM_Access();
    /*test DTCM access*/
    DTCM_Access();

    PRINTF("\r\nFLEXRAM DTCM ECC example finish.\r\n");

    while (1)
    {
    }
}

static status_t OCRAM_Reallocate(void)
{
    flexram_allocate_ram_t ramAllocate = {
        .ocramBankNum = APP_OCRAM_ALLOCATE_BANK_NUM,
        .dtcmBankNum  = APP_DTCM_ALLOCATE_BANK_NUM,
        .itcmBankNum  = APP_ITCM_ALLOCATE_BANK_NUM,
    };

    PRINTF("\r\nAllocate on-chip ram:\r\n");
    PRINTF("\r\n   OCRAM bank numbers %d\r\n", ramAllocate.ocramBankNum);
    PRINTF("\r\n   DTCM  bank numbers %d\r\n", ramAllocate.dtcmBankNum);
    PRINTF("\r\n   ITCM  bank numbers %d\r\n", ramAllocate.itcmBankNum);

    if (FLEXRAM_AllocateRam(&ramAllocate) != kStatus_Success)
    {
        PRINTF("\r\nAllocate on-chip ram fail\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("\r\nAllocate on-chip ram success\r\n");
    }

    return kStatus_Success;
}

static void OCRAM_Access(void)
{
    uint32_t *ocramAddr      = (uint32_t *)APP_FLEXRAM_OCRAM_START_ADDR;
    uint32_t ocramReadBuffer = 0x00U;
    flexram_ocram_ecc_single_error_info_t info;

    /* Enable OCRAM ECC function. */
    FLEXRAM_EnableECC(APP_FLEXRAM, true, false);
    /* Enable FLEXRAM OCRAM access error interrupt*/
    FLEXRAM_EnableInterruptSignal(APP_FLEXRAM, kFLEXRAM_OCRAMAccessError);

    for (;;)
    {
        *ocramAddr = 0xCCU;
        /* Synchronizes the execution stream with memory accesses */
        __DSB();
        __ISB();

#if APP_USING_CACHE
        DCACHE_CleanByRange((uint32_t)ocramAddr, sizeof(uint32_t));
#endif

        /* check ocram access error event */
        if (s_flexram_ocram_access_error_match)
        {
            s_flexram_ocram_access_error_match = false;
            PRINTF("\r\nOCRAM access to 0x%x boundary.\r\n", ocramAddr);
            break;
        }

        /* If there has ECC error, it will be generate ECC error interrupt. */
        ocramReadBuffer = *ocramAddr;

        if (s_flexram_ocram_ecc_single_error)
        {
            s_flexram_ocram_ecc_single_error = false;

            FLEXRAM_GetOcramSingleErroInfo(APP_FLEXRAM, &info);
#if defined(FLEXRAM_ECC_ERROR_DETAILED_INFO) && FLEXRAM_ECC_ERROR_DETAILED_INFO
            PRINTF("\r\nOCRAM single-bit ECC error corresponding ECC cipher of OCRAM single-bit ECC error: 0x%x.\r\n",
                   info.OcramSingleErrorECCCipher);
            PRINTF("\r\nOCRAM single-bit ECC error corresponding ECC syndrome of OCRAM single-bit ECC error: 0x%x.\r\n",
                   info.OcramSingleErrorECCSyndrome);
#else
            PRINTF("\r\nOCRAM single error information: 0x%x.\r\n", info.OcramSingleErrorInfo);
#endif /*FLEXRAM_ECC_ERROR_DETAILED_INFO*/
            PRINTF("\r\nOCRAM single-bit ECC error address: 0x%x.\r\n", info.OcramSingleErrorAddr);
            PRINTF("\r\nOCRAM single-bit ECC error data LSB: 0x%x.\r\n", info.OcramSingleErrorDataLSB);
            PRINTF("\r\nOCRAM single-bit ECC error data MSB: 0x%x.\r\n", info.OcramSingleErrorDataMSB);

            PRINTF("\r\nOCRAM expected data LSB: 0x%x.\r\n", ocramReadBuffer);
            break;
        }

        ocramAddr++;
    }
}

static void DTCM_Access(void)
{
    uint32_t *dtcmAddr     = (uint32_t *)APP_FLEXRAM_DTCM_START_ADDR;
    uint32_t tcmReadBuffer = 0x00U;
    uint32_t dtcmEndAddr   = APP_FLEXRAM_DTCM_START_ADDR + 0x8000U * APP_DTCM_ALLOCATE_BANK_NUM - 0x400U;
    flexram_dtcm_ecc_single_error_info_t info;

    /* Enable TCM ECC function. */
    FLEXRAM_EnableECC(APP_FLEXRAM, false, true);
    /* Enable FLEXRAM OCRAM access error interrupt*/
    FLEXRAM_EnableInterruptSignal(APP_FLEXRAM, kFLEXRAM_DTCMAccessError | kFLEXRAM_D0TCMECCSingleError);

    for (;;)
    {
        *dtcmAddr = 0xCCU;
        /* Synchronizes the execution stream with memory accesses */
        __DSB();
        __ISB();

        /* If there has ECC error, it will be generate ECC error interrupt. */
        tcmReadBuffer = *dtcmAddr;

        /* check dtcm access error event */
        if (s_flexram_dtcm_ecc_single_error)
        {
            s_flexram_dtcm_ecc_single_error = false;

            FLEXRAM_GetDtcmSingleErroInfo(APP_FLEXRAM, &info, 0x00U);
#if defined(FLEXRAM_ECC_ERROR_DETAILED_INFO) && FLEXRAM_ECC_ERROR_DETAILED_INFO
            PRINTF("\r\nD0TCM single-bit ECC error corresponding tcm_wr value: 0x%x.\r\n",
                   info.DtcmSingleErrorTCMWriteRead);
            PRINTF("\r\nD0TCM single-bit ECC error corresponding tcm access size: 0x%x.\r\n",
                   info.DtcmSingleErrorTCMAccessSize);
            PRINTF("\r\nD0TCM single-bit ECC error corresponding tcm_master: 0x%x.\r\n", info.DtcmSingleErrorTCMMaster);
            PRINTF("\r\nD0TCM single-bit ECC error corresponding tcm_priv: 0x%x.\r\n",
                   info.DtcmSingleErrorTCMPrivilege);
            PRINTF("\r\nD0TCM single-bit ECC error corresponding syndrome: 0x%x.\r\n", info.DtcmSingleErrorBitPostion);
#else
            PRINTF("\r\nD0TCM single error information: 0x%x.\r\n", info.DtcmSingleErrorInfo);
#endif /*FLEXRAM_ECC_ERROR_DETAILED_INFO*/

            PRINTF("\r\nD0TCM single error address: 0x%x.\r\n", info.DtcmSingleErrorAddr);
            PRINTF("\r\nD0TCM single error data: 0x%x.\r\n", info.DtcmSingleErrorData);

            PRINTF("\r\nD0TCM expected data: 0x%x.\r\n", tcmReadBuffer);
            break;
        }

        dtcmAddr++;

        /* Break to avoid hardfault when access to DTCM boundary;
           Otherwise, it will be hardfault and assert DTCM access error IRQ synchronously. */
        if (dtcmEndAddr == (uint32_t)dtcmAddr)
        {
            PRINTF("\r\nDTCM access to 0x%x boundary.\r\n", dtcmAddr);
            break;
        }
    }
}
