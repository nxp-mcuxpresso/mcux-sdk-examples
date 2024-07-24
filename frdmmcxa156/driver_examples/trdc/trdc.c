/*
 * Copyright 2021,2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_reset.h"
#include "fsl_trdc.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_TRDC_INSTANCE         MBC0
#define EXAMPLE_TRDC_MBC_SLAVE_INDEX  1 /* IFR0 */
#define EXAMPLE_TRDC_MBC_MEMORY_INDEX 0

/*
 * This platform only supports secure privilege mode.
 * This example uses two global access registers, one has read permission, and
 * the other doesn't have.
 */
#define EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_INDEX           4
#define EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_INDEX_NO_ACCESS 5

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_SetTrdcGlobalConfig(void);
void APP_SetMbcUnaccessible(void);
void APP_TouchMbcMemory(void);
void APP_ResolveMbcAccessError(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static trdc_mbc_memory_block_config_t mbcBlockConfig;
volatile bool g_hardfaultFlag = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_SetTrdcAccessible(void)
{
    /* Configure in GLIKEY to make the TRDC accessiable. */
    *(volatile uint32_t *)0x40091D00 = 0x00060000;
    *(volatile uint32_t *)0x40091D00 = 0x0002000F;
    *(volatile uint32_t *)0x40091D00 = 0x0001000F;
    *(volatile uint32_t *)0x40091D04 = 0x00290000;
    *(volatile uint32_t *)0x40091D00 = 0x0002000F;
    *(volatile uint32_t *)0x40091D04 = 0x00280000;
    *(volatile uint32_t *)0x40091D00 = 0x0000000F;
}


void APP_SetTrdcGlobalConfig(void)
{
    TRDC_Init(EXAMPLE_TRDC_INSTANCE);

    /* Make the all flash region accessiable. */
    *(volatile uint32_t *)0x4008E020 = 0x00007777;
    *(volatile uint32_t *)0x4008E040 = 0x00000000;
    *(volatile uint32_t *)0x4008E044 = 0x00000000;
    *(volatile uint32_t *)0x4008E048 = 0x00000000;
    *(volatile uint32_t *)0x4008E04C = 0x00000000;
    *(volatile uint32_t *)0x4008E050 = 0x00000000;
    *(volatile uint32_t *)0x4008E054 = 0x00000000;
    *(volatile uint32_t *)0x4008E058 = 0x00000000;
    *(volatile uint32_t *)0x4008E05C = 0x00000000;

    /* 1. Set control policies for MBC access control configuration registers */
    trdc_memory_access_control_config_t memAccessConfig;
    (void)memset(&memAccessConfig, 0, sizeof(memAccessConfig));

    memAccessConfig.nonsecureUsrX  = 1U;
    memAccessConfig.nonsecureUsrW  = 1U;
    memAccessConfig.nonsecureUsrR  = 1U;
    memAccessConfig.nonsecurePrivX = 1U;
    memAccessConfig.nonsecurePrivW = 1U;
    memAccessConfig.nonsecurePrivR = 1U;
    memAccessConfig.secureUsrX     = 1U;
    memAccessConfig.secureUsrW     = 1U;
    memAccessConfig.secureUsrR     = 1U;
    memAccessConfig.securePrivX    = 1U;
    memAccessConfig.securePrivW    = 1U;
    memAccessConfig.securePrivR    = 1U;

    TRDC_MbcSetMemoryAccessConfig(EXAMPLE_TRDC_INSTANCE, &memAccessConfig, 0U, EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_INDEX);

    memAccessConfig.securePrivX    = 0U;
    memAccessConfig.securePrivW    = 0U;
    memAccessConfig.securePrivR    = 0U;
    memAccessConfig.nonsecurePrivX = 0U;
    memAccessConfig.nonsecurePrivW = 0U;
    memAccessConfig.nonsecurePrivR = 0U;
    memAccessConfig.secureUsrX     = 0U;
    memAccessConfig.secureUsrW     = 0U;
    memAccessConfig.secureUsrR     = 0U;
    memAccessConfig.nonsecureUsrX  = 0U;
    memAccessConfig.nonsecureUsrW  = 0U;
    memAccessConfig.nonsecureUsrR  = 0U;

    TRDC_MbcSetMemoryAccessConfig(EXAMPLE_TRDC_INSTANCE, &memAccessConfig, 0U, EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_INDEX_NO_ACCESS);

    /* 2. Set the configuration for the MBC slave memory block that is to be tested */

    (void)memset(&mbcBlockConfig, 0, sizeof(mbcBlockConfig));
    mbcBlockConfig.memoryAccessControlSelect = EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_INDEX;
    mbcBlockConfig.nseEnable                 = false;
    mbcBlockConfig.mbcIdx                    = 0U; /* Only have one MBC */
    mbcBlockConfig.domainIdx                 = 0U; /* Only have one domain */
    mbcBlockConfig.slaveMemoryIdx            = EXAMPLE_TRDC_MBC_SLAVE_INDEX;
    mbcBlockConfig.memoryBlockIdx            = EXAMPLE_TRDC_MBC_MEMORY_INDEX;

    TRDC_MbcSetMemoryBlockConfig(EXAMPLE_TRDC_INSTANCE, &mbcBlockConfig);
}

void APP_SetMbcUnaccessible(void)
{
    /* Use policy that can't access the memory region. */
    mbcBlockConfig.memoryAccessControlSelect = EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_INDEX_NO_ACCESS;
    TRDC_MbcSetMemoryBlockConfig(EXAMPLE_TRDC_INSTANCE, &mbcBlockConfig);
}

void APP_TouchMbcMemory(void)
{
    /* Touch the memory. */
    (*(volatile uint32_t *)0x1000000);
}

void APP_ResolveMbcAccessError(void)
{
    PRINTF("Resolve access error\r\n");
    /* Use policy that can access the memory region. */
    mbcBlockConfig.memoryAccessControlSelect = EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_INDEX;
    TRDC_MbcSetMemoryBlockConfig(EXAMPLE_TRDC_INSTANCE, &mbcBlockConfig);
}
void Fault_handler()
{
#if defined(TRDC_DOMAIN_ERROR_OFFSET) && TRDC_DOMAIN_ERROR_OFFSET
    trdc_domain_error_t error;
    while (kStatus_Success == TRDC_GetAndClearFirstDomainError(EXAMPLE_TRDC_INSTANCE, &error))
    {
        APP_CheckAndResolveMrcAccessError(&error);
        APP_CheckAndResolveMbcAccessError(&error);
        g_hardfaultFlag = true;
    }
#else
    g_hardfaultFlag = true;
#if defined(FSL_FEATURE_TRDC_HAS_MRC) && FSL_FEATURE_TRDC_HAS_MRC
    APP_ResolveMrcAccessError();
#endif
#if defined(FSL_FEATURE_TRDC_HAS_MBC) && FSL_FEATURE_TRDC_HAS_MBC
    APP_ResolveMbcAccessError();
#endif
#endif
}
/*!
 * @brief BusFault_Handler
 */
void BusFault_Handler(void)
{
    Fault_handler();
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief HardFault_Handler
 */
void HardFault_Handler(void)
{
    Fault_handler();
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware.*/
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    APP_SetTrdcAccessible();

    /* Print the initial banner */
    PRINTF("TRDC example start\r\n");

    APP_SetTrdcGlobalConfig();
#if defined(FSL_FEATURE_TRDC_HAS_MRC) && FSL_FEATURE_TRDC_HAS_MRC
#if defined(KW45B41Z83_SERIES) || defined(KW45B41Z82_SERIES) || defined(KW45B41Z53_SERIES) || defined(KW45B41Z52_SERIES) || \
    defined(K32W1480_SERIES) || defined(MCXW716A_SERIES) || defined(MCXW716C_SERIES)
    /* For KW45B41Z/K32W1480/MCXW716 soc, The memory 0x48800000-0x48A00000 controlled by MRC0 belongs to the NBU flash memory,
       and CM33 core can only be able to access it if the silicon is NXP Fab or NXP Provisioned.
       Check the CLC bitfield of the LIFECYCLE register in MSCM peripheral to get the silicon revision. */
    if (((SMSCM->LIFECYCLE & SMSCM_LIFECYCLE_CLC_MASK) >> SMSCM_LIFECYCLE_CLC_SHIFT) < 7U)
#endif
    {
        /* Set the MRC unaccessible. */
        PRINTF("Set the MRC selected memory region not accessiable\r\n");
        APP_SetMrcUnaccessible();

        /* Touch the MRC, there will be hardfault. */
        g_hardfaultFlag = false;

        APP_TouchMrcMemory();

        /* Wait for the hardfault occurs. */
        while (!g_hardfaultFlag)
        {
        }
        PRINTF("The MRC selected region is accessiable now\r\n");
    }
#endif

#if defined(FSL_FEATURE_TRDC_HAS_MBC) && FSL_FEATURE_TRDC_HAS_MBC
    /* Set the MBC unaccessible. */
    PRINTF("Set the MBC selected memory block not accessiable\r\n");
    APP_SetMbcUnaccessible();

    /* Touch the MBC, there will be hardfault. */
    g_hardfaultFlag = false;

    APP_TouchMbcMemory();

    /* Wait for the hardfault occurs. */
    while (!g_hardfaultFlag)
    {
    }

    PRINTF("The MBC selected block is accessiable now\r\n");
#endif

    PRINTF("TRDC example Success\r\n");

    while (1)
    {
    }
}
