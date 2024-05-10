/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_trdc.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_TRDC_FLW_BLK_COUNT                       16U
#define EXAMPLE_TRDC_FLW_ARRAY_BASEADDR                  0x0U
#define EXAMPLE_TRDC_PROCESSOR_MASTER_DOMAIN_ID          0
#define EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_ALL_INDEX 0

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_SetTrdcFlwConfig(void);
void APP_TouchMemory(void);
bool APP_CheckMemoryMap(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_hardfaultFlag = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_SetTrdcFlwConfig()
{
    uint32_t i, j, m, n;

    TRDC_Init(TRDC);

    /* 1. Get the hardware configuration of the TRDC module */
    trdc_hardware_config_t hwConfig;
    TRDC_GetHardwareConfig(TRDC, &hwConfig);

    /* 2. Set control policies for MBC and MRC access control configuration registers */
    trdc_memory_access_control_config_t memAccessConfig;
    (void)memset(&memAccessConfig, 0, sizeof(memAccessConfig));

    /* Enable all access modes for MBC and MRC access control configuration register 0. */
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

    for (i = 0U; i < hwConfig.mbcNumber; i++)
    {
        TRDC_MbcSetMemoryAccessConfig(TRDC, &memAccessConfig, i, EXAMPLE_TRDC_MBC_ACCESS_CONTROL_POLICY_ALL_INDEX);
    }

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

    /* 6. Configure flash logical window */
    trdc_flw_config_t flwConfig;
    TRDC_GetDefaultFlashLogicalWindowConfig(&flwConfig);
    flwConfig.arrayBaseAddr = EXAMPLE_TRDC_FLW_ARRAY_BASEADDR;
    flwConfig.blockCount    = EXAMPLE_TRDC_FLW_BLK_COUNT;

    TRDC_SetFlashLogicalWindow(TRDC, &flwConfig);
}

void APP_TouchMemory(void)
{
    /* Touch the memory. */
    (*(volatile uint32_t *)(TRDC_GetFlashLogicalWindowPbase(TRDC) + 32U * 0x400U * EXAMPLE_TRDC_FLW_BLK_COUNT));
}

bool APP_CheckMemoryMap(void)
{
    for (uint32_t i = 0U; i < 8U * 0x400U * EXAMPLE_TRDC_FLW_BLK_COUNT; i++)
    {
        if ((*(volatile uint32_t *)(EXAMPLE_TRDC_FLW_ARRAY_BASEADDR + 4 * i)) !=
            (*(volatile uint32_t *)(TRDC_GetFlashLogicalWindowPbase(TRDC) + 4 * i)))
        {
            return false;
        }
    }
    return true;
}

/*!
 * @brief BusFault_Handler
 */
void BusFault_Handler(void)
{
    /* Enlarge the logical window */
    TRDC_GetSetFlashLogicalWindowSize(TRDC, EXAMPLE_TRDC_FLW_BLK_COUNT + 1U);
    PRINTF("Violent access at address: 0x%8X\r\n", SCB->BFAR);
    g_hardfaultFlag = true;
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
    PRINTF("TRDC flash logical window example start\r\n");

    /* Set the flash logical window. */
    PRINTF("Set the flash logical window\r\n");
    APP_SetTrdcFlwConfig();

    /* Touch memory beyond the window */
    APP_TouchMemory();

    /* Wait for the hardfault occurs. */
    while (!g_hardfaultFlag)
    {
    }

    PRINTF("The flash memory is accessiable now\r\n");

    if (APP_CheckMemoryMap())
    {
        PRINTF("The data between physical address window and programmable flash address are identical.\r\n");
    }
    else
    {
        PRINTF("The data between physical address window and programmable flash address are not identical!\r\n");
        return -1;
    }

    PRINTF("TRDC flash logical window example success\r\n");

    while (1)
    {
    }
}
