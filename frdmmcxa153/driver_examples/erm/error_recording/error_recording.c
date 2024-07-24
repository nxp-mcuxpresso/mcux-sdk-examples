/*
 * Copyright 2022 ~ 2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_erm.h"
#include "fsl_eim.h"

#include "fsl_clock.h"
#include "fsl_reset.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_ERM                              ERM0
#define APP_EIM                              EIM0
#define APP_ERM_IRQ                          ERM0_SINGLE_BIT_IRQn
#define APP_ERM_SINGLE_BIT_ERROR_IRQ_HANDLER ERM0_SINGLE_BIT_IRQHandler

#define APP_EIM_MEMORY_CHANNEL    kEIM_MemoryChannelRAMA0
#define APP_EIM_MEMORY_CHANNEL_EN kEIM_MemoryChannelRAMAEnable
#define APP_ERM_MEMORY_CHANNEL    kERM_MemoryChannelRAMA0
#define APP_ERM_RAM_ECC_ENABLE    0x1U   /* Memory channel RAMA0 */
#define APP_ERM_RAM_START_ADDR    0x20000000UL
#define APP_ERM_RAM_SIZE          0x800U /* 8KB */
#define APP_ERM_MAGIC_NUMBER      0xAAAAAAAAU

#define APP_ERM_RAM_CHECK_ADDR 0x20000100UL /* The address of RAM to check, must be in selected RAM block. */

#define APP_ERM_MEMORY_RECODE_OFFSET                \
    (true) /* In MCXA, the offset of error location \
            is provided not the absolute address. */


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_MemoryInit(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile bool s_ram_single_error = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_MemoryInit(void)
{
    SYSCON->RAM_CTRL |= SYSCON_RAM_CTRL_RAMA_ECC_ENABLE_MASK;
    /* RAMA0(8KB): 0x20000000 ~ 0x20001FFF */
    uint32_t *ramAddress = (uint32_t *)APP_ERM_RAM_START_ADDR;
    uint32_t ramSize     = APP_ERM_RAM_SIZE;

    for (uint32_t i = 0x00U; i < ramSize; i++)
    {
        *ramAddress = APP_ERM_MAGIC_NUMBER;
        ramAddress++;
    }
}

void APP_ERM_SINGLE_BIT_ERROR_IRQ_HANDLER(void)
{
    uint32_t status;

    status = ERM_GetInterruptStatus(APP_ERM, APP_ERM_MEMORY_CHANNEL);
    ERM_ClearInterruptStatus(APP_ERM, APP_ERM_MEMORY_CHANNEL, status);

    if (status & kERM_SingleBitCorrectionIntFlag)
    {
        s_ram_single_error = true;
    }

    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    bool errorFlag    = false;
    uint32_t *ramAddr = (uint32_t *)APP_ERM_RAM_CHECK_ADDR;
    uint32_t temp;
    uint32_t errorAddress;

    /* Board pin, clock, debug console init */
    RESET_PeripheralReset(kEIM_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kERM_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    EIM_Init(APP_EIM);
    ERM_Init(APP_ERM);

    /* Initialize the memory to a known value so that the correct corresponding ECC codeword is stored. */
    BOARD_MemoryInit();

    PRINTF("\r\nERM error recording example.\r\n");

    /* Inject databit mask. */
    EIM_InjectDataBitError(APP_EIM, APP_EIM_MEMORY_CHANNEL, 0x01UL);
    /* Enable error injection channel. */
    EIM_EnableErrorInjectionChannels(APP_EIM, APP_EIM_MEMORY_CHANNEL_EN);
    /* Enable global error injection. */
    EIM_EnableGlobalErrorInjection(APP_EIM, true);

    /* Enable ERM interrupt. */
    ERM_EnableInterrupts(APP_ERM, APP_ERM_MEMORY_CHANNEL, kERM_SingleCorrectionIntEnable);
    /* Enable ERM IRQ */
    EnableIRQ(APP_ERM_IRQ);

    /* Read original data to trigger ERM single correction interrupt. */
    temp = *ramAddr;

    /* Check ram data */
    if (APP_ERM_MAGIC_NUMBER == temp)
    {
        PRINTF("\r\nOriginal ram data correct.\r\n");
    }
    else
    {
        errorFlag = true;
        PRINTF("\r\nOriginal ram data incorrect.\r\n");
    }

    /* Wait for error detected */
    while (s_ram_single_error == false)
    {
    }

    s_ram_single_error = false;

#if (defined(APP_ERM_MEMORY_RECODE_OFFSET) && APP_ERM_MEMORY_RECODE_OFFSET)
    errorAddress = ERM_GetMemoryErrorAddr(APP_ERM, APP_ERM_MEMORY_CHANNEL) + APP_ERM_RAM_START_ADDR;
#else
    errorAddress = ERM_GetMemoryErrorAddr(APP_ERM, APP_ERM_MEMORY_CHANNEL);
#endif /* (defined(APP_ERM_MEMORY_RECODE_OFFSET) && APP_ERM_MEMORY_RECODE_OFFSET) */
    if (APP_ERM_RAM_CHECK_ADDR == errorAddress)
    {
        PRINTF("\r\nERM error recording address is 0x%x.\r\n", errorAddress);
    }
    else
    {
        errorFlag = true;
        PRINTF("\r\nError recording address is wrong.\r\n");
    }

    if (errorFlag)
    {
        PRINTF("\r\nERM error recording example finished with error.\r\n");
    }
    else
    {
        PRINTF("\r\nERM error recording example finished successfully.\r\n");
    }

    while (1)
    {
    }
}
