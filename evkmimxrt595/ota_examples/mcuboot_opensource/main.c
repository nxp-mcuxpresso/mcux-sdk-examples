/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_power.h"
#include "boot.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef CONFIG_MCUBOOT_FLASH_REMAP_ENABLE
void SBL_EnableRemap(uint32_t start_addr, uint32_t end_addr, uint32_t off)
{
      __DMB();
      *((volatile uint32_t*) FLASH_REMAP_END_REG) = end_addr;
      *((volatile uint32_t*) FLASH_REMAP_OFFSET_REG) = off;
      *((volatile uint32_t*) FLASH_REMAP_START_REG) = start_addr | 0x1;
      __DSB();
      __ISB();
}

void SBL_DisableRemap(void)
{
    __DMB();
    /* Disable REMAPEN bit first! */
    *((volatile uint32_t*) FLASH_REMAP_START_REG) = 0;
    *((volatile uint32_t*) FLASH_REMAP_END_REG) = 0;
    *((volatile uint32_t*) FLASH_REMAP_OFFSET_REG) = 0;
    __DSB();
    __ISB();
}
#endif /* CONFIG_MCUBOOT_FLASH_REMAP_ENABLE */

#define APP_DEBUG_UART_TYPE     kSerialPort_Uart
#define APP_DEBUG_UART_INSTANCE 12U
#define APP_DEBUG_UART_CLK_FREQ CLOCK_GetFlexcommClkFreq(12)
#define APP_DEBUG_UART_FRG_CLK \
    (&(const clock_frg_clk_config_t){12U, kCLOCK_FrgPllDiv, 255U, 0U}) /*!< Select FRG0 mux as frg_pll */
#define APP_DEBUG_UART_CLK_ATTACH kFRG_to_FLEXCOMM12
#define APP_DEBUG_UART_BAUDRATE   115200

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Initialize debug console
 */
void APP_InitAppDebugConsole(void)
{
    uint32_t uartClkSrcFreq;

    /* attach FRG0 clock to FLEXCOMM12 (debug console) */
    CLOCK_SetFRGClock(APP_DEBUG_UART_FRG_CLK);
    CLOCK_AttachClk(APP_DEBUG_UART_CLK_ATTACH);

    uartClkSrcFreq = APP_DEBUG_UART_CLK_FREQ;

    DbgConsole_Init(APP_DEBUG_UART_INSTANCE, APP_DEBUG_UART_BAUDRATE, APP_DEBUG_UART_TYPE, uartClkSrcFreq);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    APP_InitAppDebugConsole();

    /* Make sure casper ram buffer has power up */
    POWER_DisablePD(kPDRUNCFG_PPD_CASPER_SRAM);
    POWER_ApplyPD();

    PRINTF("hello sbl.\r\n");
    
#if defined(MCUBOOT_DIRECT_XIP) && defined(CONFIG_MCUBOOT_FLASH_REMAP_ENABLE)
    /* Make sure flash remapping function is disabled before running the
     * bootloader application .
     */
    PRINTF("Disabling flash remapping function\n");
    SBL_DisableRemap();
#endif

    (void)sbl_boot_main();

    return 0;
}

void SBL_DisablePeripherals(void)
{
}
