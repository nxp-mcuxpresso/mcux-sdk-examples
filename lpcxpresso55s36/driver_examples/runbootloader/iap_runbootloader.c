/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_flash.h"
#include "fsl_flash_ffr.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_runbootloader.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define BOOT_ARG_TAG (0xEBu)

/* @brief Boot interface can be selected by user application
 * @note  These interfaces are invalid for ISP boot
 */
enum
{
    kUserAppBootPeripheral_FLASH   = 0u,
    kUserAppBootPeripheral_ISP     = 1u,
    kUserAppBootPeripheral_FLEXSPI = 2u,
    kUserAppBootPeripheral_AUTO    = 3u,
};

/* @brief Boot mode can be selected by user application
 */
enum
{
    kUserAppBootMode_MasterBoot = 0U,
    kUserAppBootMode_IspBoot    = 1U,
};

/* @brief ISP Peripheral definitions
 * @note  For ISP boot, valid boot interfaces for user application are USART I2C SPI USB-HID CAN
 */
//! ISP Peripheral definitions
enum isp_peripheral_constants
{
    kIspPeripheral_Auto     = 0,
    kIspPeripheral_UsbHid   = 1,
    kIspPeripheral_Uart     = 2,
    kIspPeripheral_SpiSlave = 3,
    kIspPeripheral_I2cSlave = 4,
    kIspPeripheral_Can      = 5,
};
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void error_trap();
void app_finalize(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile uint32_t g_systickCounter;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * @brief Gets called when an error occurs.
 *
 * @details Print error message and trap forever.
 */
void error_trap(void)
{
    PRINTF("\r\n\r\n\r\n\t---- HALTED DUE TO ARG CONFIG ERROR! ----");
    while (1)
    {
    }
}

/*
 * @brief Gets called when the app is complete.
 *
 * @details Print finshed message and trap forever.
 */
void app_fail(void)
{
    /* Print finished message. */
    PRINTF("\r\n Fail to run runBootloader Example! \r\n");
    while (1)
    {
    }
}

int main()
{
    /* Init board hardware. */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("Enter boot image...\n");
    user_app_boot_invoke_option_t arg = {.option = {.B = {
                                                        .tag            = BOOT_ARG_TAG,
                                                        .mode           = kUserAppBootMode_IspBoot,
                                                        .boot_interface = kIspPeripheral_Uart,
                                                    }}}; // EB: represents Enter Boot; 12: represents enter ISP mode by
                                                         // UART only,13: represents enter ISP mode by SPI only

    if (arg.option.B.tag != BOOT_ARG_TAG)
    {
        PRINTF("The runBootloader API arg is Invalid...\n");
        error_trap();
    }
    if (arg.option.B.mode == kUserAppBootMode_IspBoot)
    {
        PRINTF("Calling the runBootloader API to force into the ISP mode: %x...\n", arg.option.B.boot_interface);
        PRINTF("The runBootloader ISP interface is choosen from the following one:\n");
        PRINTF("kIspPeripheral_Auto :     0\n");
        PRINTF("kIspPeripheral_UsbHid :   1\n");
        PRINTF("kIspPeripheral_Uart :     2\n");
        PRINTF("kIspPeripheral_SpiSlave : 3\n");
        PRINTF("kIspPeripheral_I2cSlave : 4\n");
        PRINTF("kIspPeripheral_Can :      5\n");
    }
    else
    {
        PRINTF("Not Calling the runBootloader API to force into the ISP mode\n");
    }

    PRINTF("Call the runBootloader API based on the arg : %x...\n", arg);
    bootloader_user_entry(&arg);

    app_fail();

    return 0;
}
