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

#include "fsl_fusion.h"
#include "fsl_reset.h"
#include "fsl_sentinel.h"
#include "fsl_trdc.h"
#include "fsl_upower.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_TRDC_INSTANCE TRDC

#define EXAMPLE_TRDC_PROCESSOR_MASTER_DOMAIN_ID 0
#define EXAMPLE_TRDC_DOMAIN_INDEX               0

#define EXAMPLE_TRDC_MRC_INDEX                            1
#define EXAMPLE_TRDC_MRC_REGION_INDEX                     0
#define EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_ALL_INDEX  0
#define EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_NONE_INDEX 1

#define EXAMPLE_TRDC_MBC_INDEX                            0
#define EXAMPLE_TRDC_MBC_SLAVE_INDEX                      3
#define EXAMPLE_TRDC_MBC_MEMORY_INDEX                     0 /* GIPIO A */
#define EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_ALL_INDEX  0
#define EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_NONE_INDEX 1

#define FSL_FEATURE_TRDC_HAS_MBC 1
#define FSL_FEATURE_TRDC_HAS_MRC 1

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
void APP_ResolveMbcAccessError();
void APP_ResolveMrcAccessError();

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint32_t mrc_start_addr[2] = {0x4000000, 0x40000000};
uint32_t mrc_end_addr[2]   = {0xC000000, 0x50000000};
volatile bool g_hardfaultFlag = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

void TRDC_InitGlobalConfig(void)
{
    uint32_t i, j, m, n;

    /* Ungate TRDC MRC, MBC and DAC PCC */
    TRDC_Init(TRDC);

    /* 1. Get the hardware configuration of the TRDC module */
    trdc_hardware_config_t hwConfig;
    TRDC_GetHardwareConfig(TRDC, &hwConfig);

    /* 2. Set control policies for MRC and MBC access control configuration registers */
    trdc_memory_access_control_config_t memAccessConfig;
    (void)memset(&memAccessConfig, 0, sizeof(memAccessConfig));

    /* Disable all access modes for MBC and MRC access control configuration register 1-7. */
    for (i = 0U; i < hwConfig.mbcNumber; i++)
    {
        for (j = 1U; j < 8U; j++)
        {
            TRDC_MbcSetMemoryAccessConfig(TRDC, &memAccessConfig, i, j);
        }
    }

    for (i = 0U; i < hwConfig.mrcNumber; i++)
    {
        for (j = 1U; j < 8U; j++)
        {
            TRDC_MrcSetMemoryAccessConfig(TRDC, &memAccessConfig, i, j);
        }
    }

    /* Enable all access modes for MRC access control configuration register 0. */
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

    for (i = 0U; i < hwConfig.mrcNumber; i++)
    {
        TRDC_MrcSetMemoryAccessConfig(TRDC, &memAccessConfig, i, EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_ALL_INDEX);
    }

    for (i = 0U; i < hwConfig.mbcNumber; i++)
    {
        TRDC_MbcSetMemoryAccessConfig(TRDC, &memAccessConfig, i, EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_ALL_INDEX);
    }

    /* 3. Set the configuration for all MRC regions */
    trdc_mrc_region_descriptor_config_t mrcRegionConfig;
    (void)memset(&mrcRegionConfig, 0, sizeof(mrcRegionConfig));
    mrcRegionConfig.memoryAccessControlSelect = EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_ALL_INDEX;
    mrcRegionConfig.valid                     = true;
    mrcRegionConfig.nseEnable                 = false;

    for (i = 0; i < hwConfig.mrcNumber; i++)
    {
        mrcRegionConfig.mrcIdx = i;
        for (j = 0; j < hwConfig.domainNumber; j++)
        {
            mrcRegionConfig.domainIdx = j;
            /* Get region number for current MRC instance */
            n = TRDC_GetMrcRegionNumber(TRDC, i);

            /*__IO uint32_t MRC_DOM0_RGD_W[8][2]; n do not more than 8 (coverity check)*/
            if (n > 8)
            {
                return;
            }

            for (m = 0U; m < n; m++)
            {
                mrcRegionConfig.regionIdx = m;
                mrcRegionConfig.startAddr = mrc_start_addr[i] + (mrc_end_addr[i] - mrc_start_addr[i]) / n * m;
                mrcRegionConfig.endAddr   = mrc_start_addr[i] + (mrc_end_addr[i] - mrc_start_addr[i]) / n * (m + 1U);

                TRDC_MrcSetRegionDescriptorConfig(TRDC, &mrcRegionConfig);
            }
        }
    }

    TRDC_SetMrcGlobalValid(TRDC);

    /* 4. Set the configuration for all MBC slave memory blocks */
    trdc_slave_memory_hardware_config_t mbcHwConfig;
    trdc_mbc_memory_block_config_t mbcBlockConfig;
    (void)memset(&mbcBlockConfig, 0, sizeof(mbcBlockConfig));
    mbcBlockConfig.memoryAccessControlSelect = EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_ALL_INDEX;
    mbcBlockConfig.nseEnable                 = false;

    for (i = 0U; i < hwConfig.mbcNumber; i++)
    {
        mbcBlockConfig.mbcIdx = i;
        for (j = 0U; j < hwConfig.domainNumber; j++)
        {
            mbcBlockConfig.domainIdx = j;
            for (m = 0U; m < 4; m++)
            {
                TRDC_GetMbcHardwareConfig(TRDC, &mbcHwConfig, i, m);
                if (mbcHwConfig.blockNum == 0U)
                {
                    break;
                }
                mbcBlockConfig.slaveMemoryIdx = m;
                for (n = 0U; n < mbcHwConfig.blockNum; n++)
                {
                    mbcBlockConfig.memoryBlockIdx = n;

                    TRDC_MbcSetMemoryBlockConfig(TRDC, &mbcBlockConfig);
                }
            }
        }
    }

    TRDC_SetMbcGlobalValid(TRDC);

    /* 5. Set master domain ID for processor master */
    trdc_processor_domain_assignment_t pDomainAssignment;
    TRDC_GetDefaultProcessorDomainAssignment(&pDomainAssignment);
    pDomainAssignment.domainId = EXAMPLE_TRDC_PROCESSOR_MASTER_DOMAIN_ID;

    TRDC_SetProcessorDomainAssignment(TRDC, &pDomainAssignment);

    TRDC_SetDacGlobalValid(TRDC);
}

void APP_SetTrdcGlobalConfig(void)
{
    uint16_t Soc_id  = 0;
    uint16_t Soc_rev = 0;
    uint32_t ret;

    ret = SENTINEL_GetSocInfo(&Soc_id, &Soc_rev);

    /* imx8ulp A0(soc_id:0x84d rev:0xA000) need to init trdc.
     * Others SOC need to change owner of trdc from Sentinel to M33
     * ret = BASELINE_FAILURE_IND The request failed */
    if ((IMX8ULP_SOC_ID == Soc_id && IMX8ULP_SOC_REV_A0 == Soc_rev) || (ret == BASELINE_FAILURE_IND))
    {
        TRDC_InitGlobalConfig();
    }
    else
    {
        BOARD_SetReleaseFlagOfTrdc(false);
        BOARD_ReleaseTRDC();
    }
}

void APP_SetMrcUnaccessible(void)
{
    /* Set the MRC region descriptor configuration and select the memory access control of no access for this region */
    trdc_mrc_region_descriptor_config_t mrcRegionConfig;
    (void)memset(&mrcRegionConfig, 0, sizeof(mrcRegionConfig));
    mrcRegionConfig.memoryAccessControlSelect = EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_NONE_INDEX;
    mrcRegionConfig.startAddr                 = 0x40000000;
    mrcRegionConfig.valid                     = true;
    /* CPU is secure mode by default, enable NSE bit to disable secure access. */
    mrcRegionConfig.nseEnable = true;
    mrcRegionConfig.endAddr   = 0x42000000;
    mrcRegionConfig.mrcIdx    = EXAMPLE_TRDC_MRC_INDEX;
    mrcRegionConfig.domainIdx = TRDC_GetCurrentMasterDomainId(TRDC);
    mrcRegionConfig.regionIdx = EXAMPLE_TRDC_MRC_REGION_INDEX;

    TRDC_MrcSetRegionDescriptorConfig(TRDC, &mrcRegionConfig);
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
    mbcBlockConfig.domainIdx      = TRDC_GetCurrentMasterDomainId(TRDC);
    mbcBlockConfig.slaveMemoryIdx = EXAMPLE_TRDC_MBC_SLAVE_INDEX;
    mbcBlockConfig.memoryBlockIdx = EXAMPLE_TRDC_MBC_MEMORY_INDEX;

    TRDC_MbcSetMemoryBlockConfig(TRDC, &mbcBlockConfig);
}

void APP_TouchMrcMemory(void)
{
    /* Touch the memory. */
    (*(volatile uint32_t *)0x40000000);
}

void APP_TouchMbcMemory(void)
{
    /* Touch the memory. */
    (*(volatile uint32_t *)0x28800000);
}

void APP_CheckAndResolveMrcAccessError(trdc_domain_error_t *error)
{
    if (error->controller == kTRDC_MemRegionChecker1)
    {
        PRINTF("Violent access at address: 0x%8X\r\n", error->address);

        /* Set the MRC region descriptor configuration and select the memory access control of all access for this
         * region */
        trdc_mrc_region_descriptor_config_t mrcRegionConfig;
        (void)memset(&mrcRegionConfig, 0, sizeof(mrcRegionConfig));
        mrcRegionConfig.memoryAccessControlSelect = EXAMPLE_TRDC_MRC_ACCESS_CONTROL_POLICY_ALL_INDEX;
        mrcRegionConfig.startAddr                 = 0x40000000;
        mrcRegionConfig.valid                     = true;
        /* Disable NSE to enable secure access. */
        mrcRegionConfig.nseEnable = false;
        mrcRegionConfig.endAddr   = 0x42000000;
        mrcRegionConfig.mrcIdx    = EXAMPLE_TRDC_MRC_INDEX;
        mrcRegionConfig.domainIdx = TRDC_GetCurrentMasterDomainId(TRDC);
        mrcRegionConfig.regionIdx = EXAMPLE_TRDC_MRC_REGION_INDEX;

        TRDC_MrcSetRegionDescriptorConfig(TRDC, &mrcRegionConfig);
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
        mbcBlockConfig.domainIdx      = TRDC_GetCurrentMasterDomainId(TRDC);
        mbcBlockConfig.slaveMemoryIdx = EXAMPLE_TRDC_MBC_SLAVE_INDEX;
        mbcBlockConfig.memoryBlockIdx = EXAMPLE_TRDC_MBC_MEMORY_INDEX;

        TRDC_MbcSetMemoryBlockConfig(TRDC, &mbcBlockConfig);
    }
}
static int mrc_tested = 0;
static int mbc_tested = 0;
void APP_ResolveMrcAccessError()
{
    trdc_domain_error_t trdc_err;

    if (mrc_tested != 0)
        return;
    if (kStatus_Success == TRDC_GetAndClearFirstDomainError(EXAMPLE_TRDC_INSTANCE, &trdc_err))
    {
        mrc_tested++;
        APP_CheckAndResolveMrcAccessError(&trdc_err);
    }


}

void APP_ResolveMbcAccessError()
{
    trdc_domain_error_t trdc_err;

    if (mbc_tested != 0)
        return;
    if (kStatus_Success == TRDC_GetAndClearFirstDomainError(EXAMPLE_TRDC_INSTANCE, &trdc_err))
    {
        mbc_tested++;
        APP_CheckAndResolveMbcAccessError(&trdc_err);
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
