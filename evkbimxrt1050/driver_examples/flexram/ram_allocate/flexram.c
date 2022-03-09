/*
 * Copyright 2017-2021 NXP
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
#include "fsl_flexram_allocate.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_FLEXRAM     FLEXRAM
#define APP_FLEXRAM_IRQ FLEXRAM_IRQn

#define APP_FLEXRAM_IRQ_HANDLER FLEXRAM_IRQHandler

#define APP_FLEXRAM_OCRAM_START_ADDR 0x20200000
#define APP_FLEXRAM_OCRAM_MAGIC_ADDR 0x202000A0

#define APP_FLEXRAM_DTCM_START_ADDR 0x20000000
#define APP_FLEXRAM_DTCM_MAGIC_ADDR 0x200000A0

/* Start adddress shouldn't be 0x00U, because some toolchain(such as ARMGCC) maybe consider it as null point and assert
 * UDF instruction. */
#define APP_FLEXRAM_ITCM_START_ADDR 0x4
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
/*!
 * @brief OCRAM reallocate function.
 *
 * @param base FLEXRAM base address.
 */
static status_t OCRAM_Reallocate(void);

/*!
 * @brief ocram access function.
 *
 * @param base FLEXRAM base address.
 */
static void OCRAM_Access(void);

/*!
 * @brief DTCM access function.
 *
 * @param base FLEXRAM base address.
 */
static void DTCM_Access(void);

/*!
 * @brief ITCM access function.
 *
 * @param base FLEXRAM base address.
 */
static void ITCM_Access(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static bool s_flexram_ocram_access_error_match = false;

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
    }

    if (FLEXRAM_GetInterruptStatus(APP_FLEXRAM) & kFLEXRAM_ITCMAccessError)
    {
        FLEXRAM_ClearInterruptStatus(APP_FLEXRAM, kFLEXRAM_ITCMAccessError);
    }

    __DSB();
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
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\nFLEXRAM ram allocate example.\r\n");

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
    /*test ITCM access*/
    ITCM_Access();

    PRINTF("\r\nFLEXRAM ram allocate example finish.\r\n");

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
    uint32_t *ocramAddr = (uint32_t *)APP_FLEXRAM_OCRAM_START_ADDR;

    /* enable FLEXRAM OCRAM access error interrupt*/
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

        ocramAddr++;
    }
}

static void DTCM_Access(void)
{
    uint32_t *dtcmAddr = (uint32_t *)APP_FLEXRAM_DTCM_START_ADDR;

    /* enable FLEXRAM DTCM access error interrupt*/
    FLEXRAM_EnableInterruptSignal(APP_FLEXRAM, kFLEXRAM_DTCMAccessError);

    for (;;)
    {
        *dtcmAddr = 0xCCU;
        /* Synchronizes the execution stream with memory accesses */
        __DSB();
        __ISB();

        if ((uint32_t)dtcmAddr == (APP_FLEXRAM_DTCM_START_ADDR +
                                   APP_DTCM_ALLOCATE_BANK_NUM * FSL_FEATURE_FLEXRAM_INTERNAL_RAM_BANK_SIZE - 0x04U))
        {
            PRINTF("\r\nDTCM access to nearly 0x%x boundary.\r\n", ((uint32_t)dtcmAddr + 0x04U));
            break;
        }

        dtcmAddr++;
    }
}

static void ITCM_Access(void)
{
    uint32_t *itcmAddr = (uint32_t *)APP_FLEXRAM_ITCM_START_ADDR;

    /* enable FLEXRAM DTCM access error interrupt*/
    FLEXRAM_EnableInterruptSignal(APP_FLEXRAM, kFLEXRAM_ITCMAccessError);

    for (;;)
    {
        *itcmAddr = 0xCCU;
        /* Synchronizes the execution stream with memory accesses */
        __DSB();
        __ISB();

        if ((uint32_t)itcmAddr == (APP_FLEXRAM_ITCM_START_ADDR +
                                   APP_ITCM_ALLOCATE_BANK_NUM * FSL_FEATURE_FLEXRAM_INTERNAL_RAM_BANK_SIZE - 0x08U))
        {
            PRINTF("\r\nITCM access to nearly 0x%x boundary.\r\n", ((uint32_t)itcmAddr + 0x04U));
            break;
        }

        itcmAddr++;
    }
}
