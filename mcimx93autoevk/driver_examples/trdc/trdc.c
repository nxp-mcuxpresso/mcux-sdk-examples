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

#include "fsl_sentinel.h"
#include "fsl_trdc.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_TRDC_INSTANCE TRDC1
/* TRDC1 base address: 4427_0000h  AONMIX */
/* Location  |  MBC   | MBC Instance | Number of | Slave Port   | Slave Memory  *
 * Number    |        | Number       | Slavers   |              |               *
 * -----------------------------------------------------------  ----------------*
 * AONMIX    | MBC_A0 | 0            | 3         | SLV  2       | GPIO1         */

/* Location  |  MRC   | MRC Instance | Number of | Slave Memory | Number of MRC *
 * Number    |        | Number       | Slavers   |              | descriptors   *
 * -----------------------------------------------------------------------------*
 * AONMIX    | MRC_A0 | 0            | 1         | CM33 ROM     | 8             */
#define EXAMPLE_TRDC_DOMAIN_INDEX 2

#define EXAMPLE_TRDC_MRC_INDEX                            0
#define EXAMPLE_TRDC_MRC_REGION_INDEX                     0
#define EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_ALL_INDEX  0
#define EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_NONE_INDEX 2

#define EXAMPLE_TRDC_MBC_INDEX                            0
#define EXAMPLE_TRDC_MBC_SLAVE_INDEX                      2
#define EXAMPLE_TRDC_MBC_MEMORY_INDEX                     0 /* GIPIO1 */
#define EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_ALL_INDEX  0
#define EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_NONE_INDEX 2


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_SetTrdcGlobalConfig(void);
void APP_SetMrcUnaccessible(void);
void APP_SetMbcUnaccessible(void);
void APP_TouchMrcMemory(void);
void APP_TouchMbcMemory(void);
void APP_CheckAndResolveMrcAccessError(trdc_domain_error_t *error);
void APP_CheckAndResolveMbcAccessError(trdc_domain_error_t *error);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_hardfaultFlag = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_SetTrdcGlobalConfig(void)
{
    return;
}

void APP_SetMrcUnaccessible(void)
{
    /* Set the MRC region descriptor configuration and select the memory access control of no access for this region */
    trdc_mrc_region_descriptor_config_t mrcRegionConfig;
    (void)memset(&mrcRegionConfig, 0, sizeof(mrcRegionConfig));
    mrcRegionConfig.memoryAccessControlSelect = EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_NONE_INDEX;
    mrcRegionConfig.startAddr                 = 0;
    mrcRegionConfig.valid                     = true;
    /* CPU is secure mode by default, enable NSE bit to disable secure access. */
    mrcRegionConfig.nseEnable = true;
    mrcRegionConfig.endAddr   = 0x8000;
    mrcRegionConfig.mrcIdx    = EXAMPLE_TRDC_MRC_INDEX;
    mrcRegionConfig.domainIdx = EXAMPLE_TRDC_DOMAIN_INDEX;
    mrcRegionConfig.regionIdx = EXAMPLE_TRDC_MRC_REGION_INDEX;

    TRDC_MrcSetRegionDescriptorConfig(TRDC1, &mrcRegionConfig);
}

void APP_SetMbcUnaccessible(void)
{
    /* Set the MBC slave memory block configuration and select the memory access control of no access for this memory
     * block */
    trdc_mbc_memory_block_config_t mbcBlockConfig;
    (void)memset(&mbcBlockConfig, 0, sizeof(mbcBlockConfig));
    mbcBlockConfig.memoryAccessControlSelect = EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_NONE_INDEX;
    /* CPU is secure mode by default, enable NSE bit to disable secure access. */
    mbcBlockConfig.nseEnable      = true;
    mbcBlockConfig.mbcIdx         = EXAMPLE_TRDC_MBC_INDEX;
    mbcBlockConfig.domainIdx      = EXAMPLE_TRDC_DOMAIN_INDEX;
    mbcBlockConfig.slaveMemoryIdx = EXAMPLE_TRDC_MBC_SLAVE_INDEX;
    mbcBlockConfig.memoryBlockIdx = EXAMPLE_TRDC_MBC_MEMORY_INDEX;

    TRDC_MbcSetMemoryBlockConfig(TRDC1, &mbcBlockConfig);
}

void APP_TouchMrcMemory(void)
{
    /* Touch the memory. */
    (*(volatile uint32_t *)0x100);
}

void APP_TouchMbcMemory(void)
{
    /* Touch the memory. */
    (*(volatile uint32_t *)0x47400000);
}

void APP_CheckAndResolveMrcAccessError(trdc_domain_error_t *error)
{
    if (error->controller == kTRDC_MemRegionChecker0)
    {
        PRINTF("Violent access at address: 0x%8X\r\n", error->address);

        /* Set the MRC region descriptor configuration and select the memory access control of all access for this
         * region */
        trdc_mrc_region_descriptor_config_t mrcRegionConfig;
        (void)memset(&mrcRegionConfig, 0, sizeof(mrcRegionConfig));
        mrcRegionConfig.memoryAccessControlSelect = EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_ALL_INDEX;
        mrcRegionConfig.startAddr                 = 0;
        mrcRegionConfig.valid                     = true;
        /* Disable NSE to enable secure access. */
        mrcRegionConfig.nseEnable = false;
        mrcRegionConfig.endAddr   = 0x80000;
        mrcRegionConfig.mrcIdx    = EXAMPLE_TRDC_MRC_INDEX;
        mrcRegionConfig.domainIdx = EXAMPLE_TRDC_DOMAIN_INDEX;
        mrcRegionConfig.regionIdx = EXAMPLE_TRDC_MRC_REGION_INDEX;

        TRDC_MrcSetRegionDescriptorConfig(TRDC1, &mrcRegionConfig);
    }
}

void APP_CheckAndResolveMbcAccessError(trdc_domain_error_t *error)
{
    if (error->controller == kTRDC_MemBlockController0)
    {
        PRINTF("Violent access at address: 0x%8X\r\n", error->address);

        /* Set the MBC slave memory block configuration and select the memory access control of no access for this
         * memory block */
        trdc_mbc_memory_block_config_t mbcBlockConfig;
        (void)memset(&mbcBlockConfig, 0, sizeof(mbcBlockConfig));
        mbcBlockConfig.memoryAccessControlSelect = EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_ALL_INDEX;
        /* Disable NSE to enable secure access. */
        mbcBlockConfig.nseEnable      = false;
        mbcBlockConfig.mbcIdx         = EXAMPLE_TRDC_MBC_INDEX;
        mbcBlockConfig.domainIdx      = EXAMPLE_TRDC_DOMAIN_INDEX;
        mbcBlockConfig.slaveMemoryIdx = EXAMPLE_TRDC_MBC_SLAVE_INDEX;
        mbcBlockConfig.memoryBlockIdx = EXAMPLE_TRDC_MBC_MEMORY_INDEX;

        TRDC_MbcSetMemoryBlockConfig(TRDC1, &mbcBlockConfig);
    }
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
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    SENTINEL_ReleaseRDC(TRDC_TYPE);

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
