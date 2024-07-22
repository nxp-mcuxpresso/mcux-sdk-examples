/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_mu.h"
#include "tee_fault_common.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_trdc.h"
#include "fsl_soc_src.h"
#include "resource_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_MU                     MU1_MUA
#define APP_MU_IRQn                 MU1_IRQn
/* In TEE of the config tools, has set the code TCM region (0x0ffff000 - 0x0fffffff)
 * as secure for Domain 5 (non-secure domain)
 */
#define DEMO_INVALID_DATA_ADDR      (0x0ffff000UL)  /* in M7 core ITCM */

/* TRDC related definitions. */
#define EXAMPLE_TRDC_INSTANCE       TRDC1

#define EXAMPLE_TRDC_SECURE_DOMAIN           2
#define EXAMPLE_TRDC_NONSECURE_DOMAIN        5

#define EXAMPLE_SECURE_PID          (1U)
#define EXAMPLE_NONSECURE_PID       (10U)

/* Address of memory, from which the secondary core will boot */
#define CORE1_BOOT_ADDRESS          (0x303C0000UL)
#define CORE1_KICKOFF_ADDRESS       (0x0U)

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base

#elif defined(__ICCARM__)
#pragma section = "__core1_image"
#define CORE1_IMAGE_START __section_begin("__core1_image")

#elif (defined(__GNUC__)) && (!defined(__MCUXPRESSO))
extern const char core1_image_start[];
extern const char *core1_image_end;
extern int core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif


/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8_t userOption = 0U;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif
void DEMO_SwitchToUntrustedDomain(void);
void DEMO_BootCore1(void);
static void DEMO_WaitCore1Stat(uint32_t stat);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    Prepare_CM7(CORE1_KICKOFF_ADDRESS);
}

void DEMO_BootCore1(void)
{
    /* RT1180 Specific CM7 Kick Off operation */
    __attribute__((unused)) volatile uint32_t result1, result2;

    /* Trigger S401 */
    while ((MU_RT_S3MUA->TSR & MU_TSR_TE0_MASK) == 0)
        ; /*Wait TR empty*/
    MU_RT_S3MUA->TR[0] = 0x17d20106;
    while ((MU_RT_S3MUA->RSR & MU_RSR_RF0_MASK) == 0)
        ; /*Wait RR Full*/
    while ((MU_RT_S3MUA->RSR & MU_RSR_RF1_MASK) == 0)
        ; /*Wait RR Full*/

    /* Response from ELE must be always read */
    result1 = MU_RT_S3MUA->RR[0];
    result2 = MU_RT_S3MUA->RR[1];

    /* Deassert Wait */
    BLK_CTRL_S_AONMIX->M7_CFG =
        (BLK_CTRL_S_AONMIX->M7_CFG & (~BLK_CTRL_S_AONMIX_M7_CFG_WAIT_MASK)) | BLK_CTRL_S_AONMIX_M7_CFG_WAIT(0);
}

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void)
{
    uint32_t image_size;
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    image_size = (uint32_t)&Image$$CORE1_REGION$$Length;
#elif defined(__ICCARM__)
    image_size = (uint32_t)__section_end("__core1_image") - (uint32_t)__section_begin("__core1_image");
#elif defined(__GNUC__)
    image_size = (uint32_t)core1_image_size;
#endif
    return image_size;
}
#endif

static void APP_SetPid(uint8_t pid)
{
    const trdc_pid_config_t pidConfig =
        {
            .pid = pid,
            .lock = kTRDC_PidUnlocked0
        };

    TRDC_SetPid(EXAMPLE_TRDC_INSTANCE, kTRDC1_MasterCM33, &pidConfig);
}

void DEMO_SwitchToUntrustedDomain(void)
{
    APP_SetPid(EXAMPLE_NONSECURE_PID);
}

static void release_trdc(void)
{
    uint32_t result;
    /*
     * Check ELE FW status
     */
    do
    {
        /*Wait TR empty*/
        while ((MU_APPS_S3MUA->TSR & MU_TSR_TE0_MASK) == 0)
            ;
        /* Send Get FW Status command(0xc5), message size 0x01 */
        MU_APPS_S3MUA->TR[0] = 0x17c50106;
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF0_MASK) == 0)
            ;
        (void)MU_APPS_S3MUA->RR[0];
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF1_MASK) == 0)
            ;
        /* Get response code, only proceed when result is 0xD6 which is S400_SUCCESS_IND. */
        result = MU_APPS_S3MUA->RR[1];
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF2_MASK) == 0)
            ;
        (void)MU_APPS_S3MUA->RR[2];
    } while (result != 0xD6);

    /*
     * Send Release TRDC command
     */
    do
    {
        /*Wait TR empty*/
        while ((MU_APPS_S3MUA->TSR & MU_TSR_TE0_MASK) == 0)
            ;
        /* Send release RDC command(0xc4), message size 2 */
        MU_APPS_S3MUA->TR[0] = 0x17c40206;
        /*Wait TR empty*/
        while ((MU_APPS_S3MUA->TSR & MU_TSR_TE1_MASK) == 0)
            ;
        /* Release TRDC A(TRDC1, 0x74) to the RTD core(cm33, 0x1) */
        MU_APPS_S3MUA->TR[1] = 0x7401;
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF0_MASK) == 0)
            ;
        (void)MU_APPS_S3MUA->RR[0];
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF1_MASK) == 0)
            ;
        result = MU_APPS_S3MUA->RR[1];
    } while (result != 0xD6);
    do
    {
        /*Wait TR empty*/
        while ((MU_APPS_S3MUA->TSR & MU_TSR_TE0_MASK) == 0)
            ;
        /* Send release RDC command(0xc4), message size 2 */
        MU_APPS_S3MUA->TR[0] = 0x17c40206;
        /*Wait TR empty*/
        while ((MU_APPS_S3MUA->TSR & MU_TSR_TE1_MASK) == 0)
            ;
        /* Release TRDC W(TRDC2, 0x78) to the RTD core(cm33, 0x1) */
        MU_APPS_S3MUA->TR[1] = 0x7801;
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF0_MASK) == 0)
            ;
        (void)MU_APPS_S3MUA->RR[0];
        /*Wait RR Full*/
        while ((MU_APPS_S3MUA->RSR & MU_RSR_RF1_MASK) == 0)
            ;
        result = MU_APPS_S3MUA->RR[1];
    } while (result != 0xD6);
}

static void APP_SetTrdcGlobalConfig(void)
{
    uint32_t i, j;

    /* 1. Get the hardware configuration of the EXAMPLE_TRDC_INSTANCE module. */
    trdc_hardware_config_t hwConfig;
    TRDC_GetHardwareConfig(TRDC1, &hwConfig);

    /* 2. Set control policies for MRC and MBC access control configuration registers. */
    trdc_memory_access_control_config_t memAccessConfig;
    (void)memset(&memAccessConfig, 0, sizeof(memAccessConfig));

    /* 3. Enable all read/write/execute access for MRC/MBC access control. */
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

    for (j = 0U; j < 8U; j++)
    {
        for (i = 0U; i < hwConfig.mrcNumber; i++)
        {
            TRDC_MrcSetMemoryAccessConfig(TRDC1, &memAccessConfig, i, j);
        }

        for (i = 0U; i < hwConfig.mbcNumber; i++)
        {
            TRDC_MbcSetMemoryAccessConfig(TRDC1, &memAccessConfig, i, j);
        }
    }

    /* 4. Enable all read/write/execute access for MRC/MBC access control for the instance used in secondary core. */
    trdc_hardware_config_t hwConfig1;
    TRDC_GetHardwareConfig(TRDC2, &hwConfig1);

    for (j = 0U; j < 8U; j++)
    {
        for (i = 0U; i < hwConfig1.mrcNumber; i++)
        {
            TRDC_MrcSetMemoryAccessConfig(TRDC2, &memAccessConfig, i, j);
        }

        for (i = 0U; i < hwConfig1.mbcNumber; i++)
        {
            TRDC_MbcSetMemoryAccessConfig(TRDC2, &memAccessConfig, i, j);
        }
    }
}

static void APP_SetTrdcDacConfigSecureDomain(void)
{
    /* Configure the access control for CM33(master 1 for TRDC1) for EXAMPLE_TRDC_SECURE_DOMAIN. */
    trdc_processor_domain_assignment_t domainAssignment0 = {
        /* This configuration is affective only for EXAMPLE_TRDC_SECURE_DOMAIN. */
        .domainId = EXAMPLE_TRDC_SECURE_DOMAIN,
        /* Use the domian ID in this MDA configuration directly. */
        .domainIdSelect = kTRDC_DidMda,
        /* When all the bits in PID(ID of the pocessor) is masked by the pidMask,
           (In this demo pidMask is 0x7 meaning when PID is ranging from 0~7),
           the processor's access right to the resource is controled by the
           configuration of EXAMPLE_TRDC_SECURE_DOMAIN, which is secure access only. */
        .pidDomainHitConfig = kTRDC_pidDomainHitInclusive,
        .pidMask = 0x7U,
        /* Force the master with PID that fits into EXAMPLE_TRDC_SECURE_DOMAIN to be secure. */
        .secureAttr = kTRDC_MasterSecure,
        .pid  = 0U,            /* Not used. */
        .lock = false
    };
    TRDC_SetProcessorDomainAssignment(EXAMPLE_TRDC_INSTANCE, (uint8_t)kTRDC1_MasterCM33,
                                      0U, &domainAssignment0);
}

static void APP_SetTrdcDacConfigNonsecureDomain(void)
{
    /* Configure the access control for the EXAMPLE_TRDC_NONSECURE_DOMAIN of CM33. */
    trdc_processor_domain_assignment_t domainAssignment = {
        .domainId = EXAMPLE_TRDC_NONSECURE_DOMAIN,
        /* Use the domian ID in this MDA field directly. */
        .domainIdSelect = kTRDC_DidMda,
        /* When NOT all the bits in PID(ID of the pocessor) is masked by the pidMask,
           (In this demo pidMask is 0x7 meaning when PID is larger than 7), the
           processor's access right to the resource is controled by the configuration of
           EXAMPLE_TRDC_NONSECURE_DOMAIN, which is non-secure access only. */
        .pidDomainHitConfig = kTRDC_pidDomainHitExclusive,
        .pidMask = 0x7U,
        /* Force the master that fits into EXAMPLE_TRDC_SECURE_DOMAIN to be non-secure. */
        .secureAttr = kTRDC_ForceNonSecure,
        .pid  = 0U,               /* Not used. */
        .lock = false
    };

    TRDC_SetProcessorDomainAssignment(EXAMPLE_TRDC_INSTANCE, (uint8_t)kTRDC1_MasterCM33,
                                      1U, &domainAssignment);
}


static void Fault_Handler(void)
{
    switch (userOption)
    {
        case '0':
            PRINTF("Hardfault triggered by invalid data access\r\n");
            break;

        case '1':
            PRINTF("Hardfault triggered by invalid parameters\r\n");
            break;

        default:
            PRINTF("ERROR: HardFault happens with unknown reason\r\n");
            break;
    }

    PRINTF("Reset and rerun\r\n");

    while (1)
        ;
}

void HardFault_Handler(void)
{
    Fault_Handler();
}

void BusFault_Handler(void)
{
    Fault_Handler();
}

static void DEMO_WaitCore1Stat(uint32_t stat)
{
    while (stat != MU_ReceiveMsg(DEMO_MU, DEMO_MU_CH))
    {
    }
}

/*!
 * @brief Function to copy core1 image to execution address.
 */
static void DEMO_CopyCore1Image(void)
{
#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image  - not required on MCUXpresso IDE. MCUXpresso copies the secondary core
       image to the target memory during startup automatically */
    uint32_t core1_image_size = get_core1_image_size();

    PRINTF("Copy Secondary core image to address: 0x%x, size: %d\r\n", CORE1_BOOT_ADDRESS, core1_image_size);

    /* Copy Secondary core application from FLASH to the target memory. */
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache_by_Addr((void *)CORE1_BOOT_ADDRESS, core1_image_size);
#endif
    memcpy((void *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache_by_Addr((void *)CORE1_BOOT_ADDRESS, core1_image_size);
#endif
#endif
}

static void DEMO_PrimaryCoreInvalidDataAccess(void)
{
    /*
     * The DEMO_INVALID_DATA_ADDR is inaccessible for untrusted domain,
     * so the access results to hardfault.
     */
    (*(volatile uint32_t *)DEMO_INVALID_DATA_ADDR)++;

    /* Should never get here */
    while (1)
        ;
}

static void DEMO_PrimaryCoreInvalidParameters(void)
{
    /*
     * The DEMO_INVALID_DATA_ADDR is inaccessible for untrusted domain,
     * so the access results to hardfault.
     */
    PRINTF("%s\r\n", (char *)DEMO_INVALID_DATA_ADDR);

    /* Should never get here */
    while (1)
        ;
}

static void DEMO_SecondaryCoreInvalidDataAccess(void)
{
    MU_SendMsg(DEMO_MU, DEMO_MU_CH, DEMO_MU_MSG_INVALID_DATA_ACCESS);

    DEMO_WaitCore1Stat(DEMO_MU_MSG_INVALID_DATA_ACCESS_DONE);

    PRINTF("Secondary core hardfault triggered by invalid data access\r\n");
    PRINTF("Reset and rerun\r\n");

    while (1)
        ;
}

static void DEMO_SecondaryCoreInvalidParameters(void)
{
    MU_SendMsg(DEMO_MU, DEMO_MU_CH, DEMO_MU_MSG_INVALID_PARAM);

    DEMO_WaitCore1Stat(DEMO_MU_MSG_INVALID_PARAM_DONE);

    PRINTF("Secondary core hardfault triggered by invalid parameters\r\n");
    PRINTF("Reset and rerun\r\n");

    while (1)
        ;
}

int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    release_trdc();
    APP_SetTrdcGlobalConfig();
    BOARD_InitTEE();

    APP_SetPid(EXAMPLE_SECURE_PID);
    APP_SetTrdcDacConfigSecureDomain();
    APP_SetTrdcDacConfigNonsecureDomain();

    BOARD_InitDebugConsole();
    NVIC_EnableIRQ(APP_MU_IRQn);

    DEMO_CopyCore1Image();

    PRINTF("TEE fault example start\r\n");

    MU_Init(DEMO_MU);

    DEMO_BootCore1();

    /* Wait for core 1 boot up. */
    DEMO_WaitCore1Stat(DEMO_MU_MSG_CORE1_READY);

    while (1)
    {
        PRINTF("Select the option:\r\n");
        PRINTF("0: Primary core invalid data access\r\n");
        PRINTF("1: Primary core invalid input parameters\r\n");
        PRINTF("2: Secondary core invalid data access\r\n");
        PRINTF("3: Secondary core invalid input parameters\r\n");

        userOption = 0U;
        userOption = GETCHAR();

        switch (userOption)
        {
            case '0':
                /* Switch to untrusted domain. */
                DEMO_SwitchToUntrustedDomain();
                DEMO_PrimaryCoreInvalidDataAccess();
                break;

            case '1':
                /* Switch to untrusted domain. */
                DEMO_SwitchToUntrustedDomain();
                DEMO_PrimaryCoreInvalidParameters();
                break;

            case '2':
                DEMO_SecondaryCoreInvalidDataAccess();
                break;

            case '3':
                DEMO_SecondaryCoreInvalidParameters();
                break;

            default:
                PRINTF("Invalid input, select again\r\n");
                break;
        }
    }
}
