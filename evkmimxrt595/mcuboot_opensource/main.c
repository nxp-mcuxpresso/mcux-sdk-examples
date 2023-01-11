/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sbl.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_power.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef SOC_REMAP_ENABLE
#define REMAPADDRSTART  (FLEXSPI0_BASE + 0x420)
#define REMAPADDREND    (FLEXSPI0_BASE + 0x424)
#define REMAPADDROFFSET (FLEXSPI0_BASE + 0x428)
#endif

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
#if (defined(COMPONENT_MCU_ISP))
extern int isp_kboot_main(bool isInfiniteIsp);
#endif

/* Initialize debug console. */

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
#if (defined(COMPONENT_MCU_ISP))
    bool isInfiniteIsp = false;
    (void)isp_kboot_main(isInfiniteIsp);
#endif

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    APP_InitAppDebugConsole();

    /* Define the init structure for the OSPI reset pin*/
    gpio_pin_config_t reset_config = {
        kGPIO_DigitalOutput,
        1,
    };

    /* Init output OSPI reset pin. */
    GPIO_PortInit(GPIO, BOARD_FLASH_RESET_GPIO_PORT);
    GPIO_PinInit(GPIO, BOARD_FLASH_RESET_GPIO_PORT, BOARD_FLASH_RESET_GPIO_PIN, &reset_config);

    /* Make sure casper ram buffer has power up */
    POWER_DisablePD(kPDRUNCFG_PPD_CASPER_SRAM);
    POWER_ApplyPD();

    PRINTF("hello sbl.\r\n");

    (void)sbl_boot_main();

    return 0;
}

void SBL_DisablePeripherals(void)
{
}

#ifdef SOC_REMAP_ENABLE
void SBL_EnableRemap(uint32_t start_addr, uint32_t end_addr, uint32_t off)
{
    uint32_t *remap_start  = (uint32_t *)REMAPADDRSTART;
    uint32_t *remap_end    = (uint32_t *)REMAPADDREND;
    uint32_t *remap_offset = (uint32_t *)REMAPADDROFFSET;

    *remap_start  = start_addr + 1;
    *remap_end    = end_addr;
    *remap_offset = off;
}

void SBL_DisableRemap(void)
{
    uint32_t *remap_start  = (uint32_t *)REMAPADDRSTART;
    uint32_t *remap_end    = (uint32_t *)REMAPADDREND;
    uint32_t *remap_offset = (uint32_t *)REMAPADDROFFSET;

    *remap_start  = 0;
    *remap_end    = 0;
    *remap_offset = 0;
}
#endif
