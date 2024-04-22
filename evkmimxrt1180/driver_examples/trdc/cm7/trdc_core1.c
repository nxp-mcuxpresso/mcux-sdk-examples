/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "mcmgr.h"
#include "fsl_common.h"

#include "fsl_cache.h"
#include "fsl_trdc.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_TRDC_INSTANCE          TRDC2
#define EXAMPLE_TRDC_SECURE_DOMAIN     4
#define EXAMPLE_TRDC_NONSECURE_DOMAIN  3
#define EXAMPLE_TRDC_MRC_INDEX         6 /* NETC */
#define EXAMPLE_TRDC_MRC_REGION_INDEX  0
#define EXAMPLE_TRDC_MRC_START_ADDRESS 0x60000000UL
#define EXAMPLE_TRDC_MRC_END_ADDRESS   0x60100000UL
#define EXAMPLE_TRDC_MBC_INDEX         0
#define EXAMPLE_TRDC_MBC_SLAVE_INDEX   0            /* AIPS2 */
#define EXAMPLE_TRDC_MBC_MEMORY_INDEX  0
#define EXAMPLE_TRDC_MBC_START_ADDRESS 0x42000000UL /* AIPS2 start address */
/* These flags are stored in memory for the 2 cores to communicate, should align with the primary core. */
#define EXAMPLE_NONSECURE_MRC_NEED_RESOLVE_FLAG_ADDRESS 0x04000000UL
#define EXAMPLE_NONSECURE_MRC_RESOLVED_FLAG_ADDRESS     0x04000004UL
#define EXAMPLE_NONSECURE_MBC_NEED_RESOLVE_FLAG_ADDRESS 0x04000008UL
#define EXAMPLE_NONSECURE_MBC_RESOLVED_FLAG_ADDRESS     0x0400000CUL
#define EXAMPLE_SECONDARY_CORE_START_FLAG_ADDRESS       0x04000010UL
#define DEMO_SECURE 0
#define DEMO_NONSECURE_MBC 1
#define DEMO_NONSECURE_MRC 2
#define DEMO_OTHER 3
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_SetTrdcGlobalConfig(void);
void APP_SetTrdcAccessControl(void);
void APP_SetTrdcDacConfigSecureDomain(void);
void APP_SetTrdcDacConfigNonsecureDomainMbc(void);
void APP_SetTrdcDacConfigNonsecureDomainMrc(void);
void APP_SetMrcUnaccessible(uint8_t domain, bool nseEnable);
void APP_SetMbcUnaccessible(uint8_t domain, bool nseEnable);
void APP_TouchMrcMemory(void);
void APP_TouchMbcMemory(void);
void APP_CheckAndResolveMrcAccessError(trdc_domain_error_t *error);
void APP_CheckAndResolveMbcAccessError(trdc_domain_error_t *error);

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_INIT(volatile static bool g_hardfaultFlag) = false;
AT_NONCACHEABLE_SECTION_INIT(volatile static uint8_t g_demo) = DEMO_SECURE;

/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_SetTrdcGlobalConfig(void)
{
    /* Has been settled in Core0 APP_SetTrdcGlobalConfig */
}

void APP_SetTrdcAccessControl(void)
{
    uint32_t i, j, m, n;

    trdc_hardware_config_t hwConfig;
    TRDC_GetHardwareConfig(EXAMPLE_TRDC_INSTANCE, &hwConfig);

    /* 1. Set the configuration for MRC. */
    trdc_mrc_region_descriptor_config_t mrcRegionConfig;
    (void)memset(&mrcRegionConfig, 0, sizeof(mrcRegionConfig));
    /* First clear all the memory and control policies for all the resions of the MRC instance to be tested.
       Otherwise if any other region that has the same address but different control policy will affect the test result
       of the region to be tested.  */
    mrcRegionConfig.mrcIdx = EXAMPLE_TRDC_MRC_INDEX;
    for (i = 0; i < hwConfig.domainNumber; i++)
    {
        mrcRegionConfig.domainIdx = i;
        /* Get region count for current MRC instance */
        m = TRDC_GetMrcRegionNumber(EXAMPLE_TRDC_INSTANCE, EXAMPLE_TRDC_MRC_INDEX);
        for (j = 0U; j < m; j++)
        {
            mrcRegionConfig.regionIdx = j;
            TRDC_MrcSetRegionDescriptorConfig(EXAMPLE_TRDC_INSTANCE, &mrcRegionConfig);
        }
    }

    /* Configure the access to EXAMPLE_TRDC_SECURE_DOMAIN to secure access only. */
    mrcRegionConfig.memoryAccessControlSelect = 0U;
    mrcRegionConfig.valid                     = true;
    mrcRegionConfig.nseEnable                 = false; /* Disable nonsecure, meaning enable secure. */
    mrcRegionConfig.mrcIdx                    = EXAMPLE_TRDC_MRC_INDEX;
    mrcRegionConfig.domainIdx                 = EXAMPLE_TRDC_SECURE_DOMAIN;
    mrcRegionConfig.regionIdx                 = EXAMPLE_TRDC_MRC_REGION_INDEX;
    mrcRegionConfig.startAddr                 = EXAMPLE_TRDC_MRC_START_ADDRESS;
    mrcRegionConfig.endAddr                   = EXAMPLE_TRDC_MRC_END_ADDRESS;
    TRDC_MrcSetRegionDescriptorConfig(EXAMPLE_TRDC_INSTANCE, &mrcRegionConfig);

    /* Configure the access to EXAMPLE_TRDC_NONSECURE_DOMAIN to non-secure access only. */
    mrcRegionConfig.mrcIdx    = EXAMPLE_TRDC_MRC_INDEX;
    mrcRegionConfig.startAddr = EXAMPLE_TRDC_MRC_START_ADDRESS;
    mrcRegionConfig.endAddr   = EXAMPLE_TRDC_MRC_END_ADDRESS;
    mrcRegionConfig.nseEnable = true; /* Enable nonsecure, meaning disable secure. */
    mrcRegionConfig.domainIdx = EXAMPLE_TRDC_NONSECURE_DOMAIN;
    TRDC_MrcSetRegionDescriptorConfig(EXAMPLE_TRDC_INSTANCE, &mrcRegionConfig);

    /* Some variables are located in OCRAM1, make sure they can be accessed from secure and non-secure */
    mrcRegionConfig.mrcIdx    = 3U;
    mrcRegionConfig.regionIdx = 0;
    mrcRegionConfig.startAddr = 0x20480000UL;
    mrcRegionConfig.endAddr   = 0x204FFFFFUL;

    mrcRegionConfig.nseEnable = true; /* Enable nonsecure, meaning disable secure. */
    mrcRegionConfig.domainIdx = EXAMPLE_TRDC_NONSECURE_DOMAIN;
    TRDC_MrcSetRegionDescriptorConfig(EXAMPLE_TRDC_INSTANCE, &mrcRegionConfig);

    mrcRegionConfig.nseEnable = false; /* Disable nonsecure, meaning enable secure. */
    mrcRegionConfig.domainIdx = EXAMPLE_TRDC_SECURE_DOMAIN;
    TRDC_MrcSetRegionDescriptorConfig(EXAMPLE_TRDC_INSTANCE, &mrcRegionConfig);

    /* 2. Set the configuration for MBC. */
    trdc_slave_memory_hardware_config_t mbcHwConfig;
    trdc_mbc_memory_block_config_t mbcBlockConfig;
    (void)memset(&mbcBlockConfig, 0, sizeof(mbcBlockConfig));
    mbcBlockConfig.memoryAccessControlSelect = 0U;

    for (i = 0U; i < hwConfig.mbcNumber; i++)
    {
        mbcBlockConfig.mbcIdx = i;
        for (j = 0U; j < hwConfig.domainNumber; j++)
        {
            mbcBlockConfig.domainIdx = j;
            if (j == EXAMPLE_TRDC_NONSECURE_DOMAIN)
            {
                /* Configure the access to EXAMPLE_TRDC_NONSECURE_DOMAIN to non-secure access only. */
                mbcBlockConfig.nseEnable = true;
            }
            else if (j == EXAMPLE_TRDC_SECURE_DOMAIN)
            {
                /* Configure the access to EXAMPLE_TRDC_SECURE_DOMAIN to secure access only. */
                mbcBlockConfig.nseEnable = false;
            }
            else
            {
                /* Do not need to configure the other domains. */
                continue;
            }
            for (m = 0U; m < 4; m++)
            {
                TRDC_GetMbcHardwareConfig(EXAMPLE_TRDC_INSTANCE, &mbcHwConfig, i, m);
                if (mbcHwConfig.blockNum == 0U)
                {
                    break;
                }
                mbcBlockConfig.slaveMemoryIdx = m;
                for (n = 0U; n < mbcHwConfig.blockNum; n++)
                {
                    mbcBlockConfig.memoryBlockIdx = n;
                    TRDC_MbcSetMemoryBlockConfig(EXAMPLE_TRDC_INSTANCE, &mbcBlockConfig);
                }
            }
        }
    }
}

void APP_SetTrdcDacConfigSecureDomain(void)
{
    /* Configure the access control for CM7 AXI and AHBP(master 0&1 for TRDC2) for EXAMPLE_TRDC_SECURE_DOMAIN. */
    trdc_processor_domain_assignment_t domainAssignment0 = {
        .domainId =
            EXAMPLE_TRDC_SECURE_DOMAIN,  /* This configuration is affective only for EXAMPLE_TRDC_SECURE_DOMAIN. */
        .domainIdSelect = kTRDC_DidMda,  /* Use the domian ID in this MDA configuration directly. */
        .pidDomainHitConfig =
            kTRDC_pidDomainHitInclusive, /* When all the bits in PID(ID of the pocessor) is masked by the pidMask,
                                           (In this demo pidMask is 0x7 meaning when PID is ranging from 0~7),
                                           the processor's access right to the resource, is controled by the
                                           configuration of EXAMPLE_TRDC_SECURE_DOMAIN, which is secure access only. */
        .pidMask = 0x7U,
        .secureAttr =
            kTRDC_ForceSecure, /* Force the master with PID that fits into EXAMPLE_TRDC_SECURE_DOMAIN to be secure. */
        .pid  = 3U,
        .lock = false};
    TRDC_SetProcessorDomainAssignment(EXAMPLE_TRDC_INSTANCE, (uint8_t)kTRDC2_MasterCM7AXI, 0U, &domainAssignment0);
    TRDC_SetProcessorDomainAssignment(EXAMPLE_TRDC_INSTANCE, (uint8_t)kTRDC2_MasterCM7AHBP, 0U, &domainAssignment0);
}

void APP_SetTrdcDacConfigNonsecureDomainMbc(void)
{
    /* Configure the access control for CM7 AHBP which controls the MBC(according to system block) used in this example.
     */
    trdc_processor_domain_assignment_t domainAssignment = {
        .domainId =
            EXAMPLE_TRDC_NONSECURE_DOMAIN, /* This configuration is affective only for EXAMPLE_TRDC_NONSECURE_DOMAIN. */
        .domainIdSelect = kTRDC_DidMda,    /* Use the domian ID in this MDA configuration directly. */
        .pidDomainHitConfig =
            kTRDC_pidDomainHitExclusive,   /* When NOT all the bits in PID(ID of the pocessor) is masked by the pidMask,
                                              (In this demo pidMask is 0x7 meaning when PID is larger than 7),
                                              the processor's access right to the resource is controled by the
                                              configuration of EXAMPLE_TRDC_NONSECURE_DOMAIN, which is non-secure access
                                              only. */
        .pidMask = 0x7U,
        .secureAttr =
            kTRDC_ForceNonSecure, /* Force the master that fits into EXAMPLE_TRDC_SECURE_DOMAIN to be non-secure. */
        .pid  = 8U,
        .lock = false};
    TRDC_EnableProcessorDomainAssignment(EXAMPLE_TRDC_INSTANCE, (uint8_t)kTRDC2_MasterCM7AXI, 0U, false);
    TRDC_EnableProcessorDomainAssignment(EXAMPLE_TRDC_INSTANCE, (uint8_t)kTRDC2_MasterCM7AHBP, 0U, false);
    TRDC_SetProcessorDomainAssignment(EXAMPLE_TRDC_INSTANCE, (uint8_t)kTRDC2_MasterCM7AHBP, 1U, &domainAssignment);
}

void APP_SetTrdcDacConfigNonsecureDomainMrc(void)
{
    /* Configure the access control for CM7 AXI which controls the MRC(according to system block) used in this example.
     */
    trdc_processor_domain_assignment_t domainAssignment = {
        .domainId =
            EXAMPLE_TRDC_NONSECURE_DOMAIN, /* This configuration is affective only for EXAMPLE_TRDC_NONSECURE_DOMAIN. */
        .domainIdSelect = kTRDC_DidMda,    /* Use the domian ID in this MDA configuration directly. */
        .pidDomainHitConfig =
            kTRDC_pidDomainHitExclusive,   /* When NOT all the bits in PID(ID of the pocessor) is masked by the pidMask,
                                              (In this demo pidMask is 0x7 meaning when PID is larger than 7), the
                                              processor's access right to the resource is controled by the configuration
                                              of EXAMPLE_TRDC_NONSECURE_DOMAIN, which is non-secure access only. */
        .pidMask = 0x7U,
        .secureAttr =
            kTRDC_ForceNonSecure, /* Force the master that fits into EXAMPLE_TRDC_SECURE_DOMAIN to be non-secure. */
        .pid  = 8U,
        .lock = false};
    TRDC_EnableProcessorDomainAssignment(EXAMPLE_TRDC_INSTANCE, (uint8_t)kTRDC2_MasterCM7AHBP, 1U, false);
    TRDC_SetProcessorDomainAssignment(EXAMPLE_TRDC_INSTANCE, (uint8_t)kTRDC2_MasterCM7AXI, 1U, &domainAssignment);
}

void APP_SetMrcUnaccessible(uint8_t domain, bool nseEnable)
{
    /* Set the MRC region descriptor configuration and select the memory access control of no access for this region. */
    trdc_mrc_region_descriptor_config_t mrcRegionConfig;
    (void)memset(&mrcRegionConfig, 0, sizeof(mrcRegionConfig));
    mrcRegionConfig.memoryAccessControlSelect = 0;
    mrcRegionConfig.valid                     = true;
    mrcRegionConfig.nseEnable                 = nseEnable;
    mrcRegionConfig.startAddr                 = EXAMPLE_TRDC_MRC_START_ADDRESS;
    mrcRegionConfig.endAddr                   = EXAMPLE_TRDC_MRC_END_ADDRESS;
    mrcRegionConfig.mrcIdx                    = EXAMPLE_TRDC_MRC_INDEX;
    mrcRegionConfig.domainIdx                 = domain;
    mrcRegionConfig.regionIdx                 = EXAMPLE_TRDC_MRC_REGION_INDEX;

    TRDC_MrcSetRegionDescriptorConfig(EXAMPLE_TRDC_INSTANCE, &mrcRegionConfig);
}

void APP_TouchMrcMemory(void)
{
    /* Touch the memory. */
    (*(volatile uint32_t *)EXAMPLE_TRDC_MRC_START_ADDRESS);
}

void APP_CheckAndResolveMrcAccessError(trdc_domain_error_t *error)
{
    if (error->controller == kTRDC_MemRegionChecker6)
    {
        PRINTF("Core1 MRC violent access at address: 0x%8X\r\n", error->address);

        /* Set the MRC region descriptor configuration and select the memory access control of all access for this
         * region */
        trdc_mrc_region_descriptor_config_t mrcRegionConfig;
        (void)memset(&mrcRegionConfig, 0, sizeof(mrcRegionConfig));
        mrcRegionConfig.memoryAccessControlSelect = 0U;
        mrcRegionConfig.valid                     = true;
        mrcRegionConfig.startAddr                 = EXAMPLE_TRDC_MRC_START_ADDRESS;
        mrcRegionConfig.endAddr                   = EXAMPLE_TRDC_MRC_END_ADDRESS;
        mrcRegionConfig.mrcIdx                    = EXAMPLE_TRDC_MRC_INDEX;
        mrcRegionConfig.regionIdx                 = EXAMPLE_TRDC_MRC_REGION_INDEX;
        /* Disable nseEnable meaning enable secure access for EXAMPLE_TRDC_SECURE_DOMAIN. */
        mrcRegionConfig.nseEnable = false;
        mrcRegionConfig.domainIdx = EXAMPLE_TRDC_SECURE_DOMAIN;
        TRDC_MrcSetRegionDescriptorConfig(EXAMPLE_TRDC_INSTANCE, &mrcRegionConfig);
    }
}

void APP_SetMbcUnaccessible(uint8_t domain, bool nseEnable)
{
    trdc_mbc_memory_block_config_t mbcBlockConfig;
    (void)memset(&mbcBlockConfig, 0, sizeof(mbcBlockConfig));
    mbcBlockConfig.memoryAccessControlSelect = 0U;
    mbcBlockConfig.nseEnable                 = nseEnable;
    mbcBlockConfig.mbcIdx                    = EXAMPLE_TRDC_MBC_INDEX;
    mbcBlockConfig.domainIdx                 = domain;
    mbcBlockConfig.slaveMemoryIdx            = EXAMPLE_TRDC_MBC_SLAVE_INDEX;
    mbcBlockConfig.memoryBlockIdx            = EXAMPLE_TRDC_MBC_MEMORY_INDEX;

    TRDC_MbcSetMemoryBlockConfig(EXAMPLE_TRDC_INSTANCE, &mbcBlockConfig);
}

void APP_TouchMbcMemory(void)
{
    /* Touch the memory. */
    (*(volatile uint32_t *)EXAMPLE_TRDC_MBC_START_ADDRESS);
}

void APP_CheckAndResolveMbcAccessError(trdc_domain_error_t *error)
{
    if (error->controller == kTRDC_MemBlockController0)
    {
        PRINTF("Core1 MBC violent access at address: 0x%8X\r\n", error->address);

        trdc_mbc_memory_block_config_t mbcBlockConfig;
        (void)memset(&mbcBlockConfig, 0, sizeof(mbcBlockConfig));
        mbcBlockConfig.memoryAccessControlSelect = 0U;
        mbcBlockConfig.mbcIdx                    = EXAMPLE_TRDC_MBC_INDEX;
        mbcBlockConfig.slaveMemoryIdx            = EXAMPLE_TRDC_MBC_SLAVE_INDEX;
        mbcBlockConfig.memoryBlockIdx            = EXAMPLE_TRDC_MBC_MEMORY_INDEX;
        /* Disable nseEnable meaning enable secure access for EXAMPLE_TRDC_SECURE_DOMAIN. */
        mbcBlockConfig.nseEnable = false;
        mbcBlockConfig.domainIdx = EXAMPLE_TRDC_SECURE_DOMAIN;
        TRDC_MbcSetMemoryBlockConfig(EXAMPLE_TRDC_INSTANCE, &mbcBlockConfig);
    }
}
/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
    (void)MCMGR_EarlyInit();
}

void Fault_handler()
{
    if (g_demo == DEMO_SECURE)
    {
        trdc_domain_error_t error;
        while (kStatus_Success == TRDC_GetAndClearFirstDomainError(EXAMPLE_TRDC_INSTANCE, &error))
        {
            APP_CheckAndResolveMrcAccessError(&error);
            APP_CheckAndResolveMbcAccessError(&error);
            g_hardfaultFlag = true;
        }
    }
    else if (g_demo == DEMO_NONSECURE_MRC)
    {
        /* Set the flag to let the primary core to resolve the issue. */
        (*(volatile uint32_t *)EXAMPLE_NONSECURE_MRC_NEED_RESOLVE_FLAG_ADDRESS) = 1U;
        /* Clear the g_demo to prevent setting the flag again by mistake after primary core has resolved it. */
        g_demo = DEMO_OTHER;
    }
    else if (g_demo == DEMO_NONSECURE_MBC)
    {
        /* Set the flag to let the primary core to resolve the issue. */
        (*(volatile uint32_t *)EXAMPLE_NONSECURE_MBC_NEED_RESOLVE_FLAG_ADDRESS) = 1U;
        /* Clear the g_demo to prevent setting the flag again by mistake after primary core has resolved it. */
        g_demo = DEMO_OTHER;
    }
    else
    {
        /* Do nothing. */
    }
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
    uint32_t startupData;
    mcmgr_status_t status;

    /* Init board hardware.*/
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitDebugConsole();
    L1CACHE_DisableDCache();

    /* Initialize MCMGR, install generic event handlers */
    (void)MCMGR_Init();

    /* Get the startup data */
    do
    {
        status = MCMGR_GetStartupData(&startupData);
    } while (status != kStatus_MCMGR_Success);

    /* Wait for the primary core to finished printing. */
    while ((*(volatile uint8_t *)EXAMPLE_SECONDARY_CORE_START_FLAG_ADDRESS) == 0U)
    {
    }

    PRINTF("\r\nIn secondary core demo we use 2 domains, one is for secure access only, and the other is for non-secure access only.\r\n");

    /* 1. Enable all access to all blocks. */
    APP_SetTrdcGlobalConfig();

    /* 2. In secondary core demo we use 2 domains, one is for secure access only, and the other is for non-secure access only. */
    APP_SetTrdcAccessControl();

    PRINTF("Secondary core secure access demo starts.\r\n");
    /* 3. Access demo for secure master. */
    /* 3.1 Configure the domain access control for secure domain. */
    APP_SetTrdcDacConfigSecureDomain();

    /* 3.2 MBC function demonstration. */
    PRINTF("Set the selected MBC block to non-secure for domain that is secure access only\r\n");
    /* Set the non-secure to true for secure domain to disable the memory access. */
    APP_SetMbcUnaccessible(EXAMPLE_TRDC_SECURE_DOMAIN, true);

    /* Touch the MBC, there will be hardfault. */
    g_hardfaultFlag = false;

    __DSB();

    APP_TouchMbcMemory();

    /* Wait for the hardfault occurs. */
    while (!g_hardfaultFlag)
    {
    }

    PRINTF("The MBC selected block is accessiable for secure master now\r\n");

    /* 3.3 MRC function demonstration. */
    PRINTF("Set the selected MRC region to non-secure for domain that is secure access only\r\n");
    /* Set the non-secure to true for secure domain to disable the memory access. */
    APP_SetMrcUnaccessible(EXAMPLE_TRDC_SECURE_DOMAIN, true);

    /* Touch the MRC, there will be hardfault. */
    g_hardfaultFlag = false;

    __DSB();

    APP_TouchMrcMemory();

    /* Wait for the hardfault occurs. */
    while (!g_hardfaultFlag)
    {
    }
    PRINTF("The MRC selected region is accessiable for secure master now\r\n");

    PRINTF("Secondary core non-secure access demo starts.\r\n");
    /* 4. Access demo for non-secure master. */
    /* 4.1 Configure the MBC accessibility. */
    PRINTF("Set the selected MBC block to non-secure for domain that is secure access only\r\n");
    /* Set the non-secure to false for non-secure domain to disable the memory access. */
    APP_SetMbcUnaccessible(EXAMPLE_TRDC_NONSECURE_DOMAIN, false);

    /* 4.2 Configure the MRC accessibility. */
    PRINTF("Set the selected MRC region to non-secure for domain that is secure access only\r\n");
    /* Set the non-secure to false for non-secure domain to disable the memory access. */
    APP_SetMrcUnaccessible(EXAMPLE_TRDC_NONSECURE_DOMAIN, false);

    /* 4.3 Cofigure the domain access control for MBC for non-secure domain.
       Notice once this configuration is done, the processor master is forced to be non-secure,
       thus can no-loner access the TRDC register, because TRDC is secure access only.*/
    g_demo = DEMO_NONSECURE_MBC;
    APP_SetTrdcDacConfigNonsecureDomainMbc();

    /* 4.4 Touch the MBC, there will be hardfault. */
    __DSB();
    APP_TouchMbcMemory();

    /* Wait for the primary core to change TRDC configuration to resolve the access error. */
    while ((*(volatile uint32_t *)EXAMPLE_NONSECURE_MBC_RESOLVED_FLAG_ADDRESS) == 0U)
    {
    }

    PRINTF("The MBC selected block is accessiable for non-secure master now\r\n");

    /* 4.5 Cofigure the domain access control for MRC for non-secure domain.
       Notice once this configuration is done, the processor master is forced to be non-secure,
       thus can no-loner access the TRDC register, because TRDC is secure access only. */
    g_demo = DEMO_NONSECURE_MRC;
    APP_SetTrdcDacConfigNonsecureDomainMrc();

    /* 4.6 Touch the MRC, there will be hardfault. */
    __DSB();
    APP_TouchMrcMemory();

    /* Wait for the primary core to change TRDC configuration to resolve the access error. */
    while ((*(volatile uint32_t *)EXAMPLE_NONSECURE_MRC_RESOLVED_FLAG_ADDRESS) == 0U)
    {
    }
    PRINTF("The MRC selected region is accessiable for non-secure master now\r\n");

    PRINTF("TRDC example succeeds on secondary core\r\n");

    while (1)
    {
    }
}
