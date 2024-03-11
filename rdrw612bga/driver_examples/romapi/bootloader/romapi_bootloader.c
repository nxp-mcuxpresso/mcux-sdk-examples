/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_romapi.h"

#include <string.h>
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * @brief Helper to assert function return status.
 *
 * @details Print finshed message and trap forever upon failure.
 */
#define APP_ASSERT(expected, actual, ...) \
    do                                    \
    {                                     \
        if (expected != actual)           \
        {                                 \
            PRINTF("Example failed: ");   \
            PRINTF(__VA_ARGS__);          \
            while (1)                     \
                ;                         \
        }                                 \
    } while (0);

//! @brief Boot mode can be selected by user application
//! @note  For master boot, valid boot insterfaces for user application are USART I2C SPI USB-HID USB-DFU SD MMC
//!        For ISP boot, valid boot interfaces for user application are USART I2C SPI
enum
{
    kUserAppBootMode_MasterBoot = 0,
    kUserAppBootMode_IspBoot    = 1,
};

//! @brief Boot interface can be selected by user application
//! @note  For USB-HID QSPI USB-DFU SD MMC, these interfaces are invalid for ISP boot
enum
{
    kUserAppBootPeripheral_UART    = 0u,
    kUserAppBootPeripheral_I2C     = 1u,
    kUserAppBootPeripheral_SPI     = 2u,
    kUserAppBootPeripheral_USB_HID = 3u,
    kUserAppBootPeripheral_FLEXSPI = 4u,
    kUserAppBootPeripheral_DFU     = 5u
};

typedef enum
{
    Test_BootMode_MasterBoot = kUserAppBootMode_MasterBoot,
    Test_BootMode_IspBoot    = kUserAppBootMode_IspBoot,
    Test_BootMode_No_Of_Elems
} test_boot_mode_t;

typedef enum
{
    Test_Boot_Interface_UART    = kUserAppBootPeripheral_UART,
    Test_Boot_Interface_I2C     = kUserAppBootPeripheral_I2C,
    Test_Boot_Interface_SPI     = kUserAppBootPeripheral_SPI,
    Test_Boot_Interface_USB_HID = kUserAppBootPeripheral_USB_HID,
    Test_Boot_Interface_FLEXSPI = kUserAppBootPeripheral_FLEXSPI,
    Test_Boot_Interface_DFU     = kUserAppBootPeripheral_DFU,
    Test_Boot_Interface_No_Of_Elems
} test_boot_interface_t;

typedef enum
{
    Test_Boot_Instance_0 = 0,
    Test_Boot_Instance_1 = 1,
    Test_Boot_Instance_No_Of_Elems
} test_boot_instance_t;

typedef enum
{
    Test_Boot_Image_0 = 0,
    Test_Boot_Image_1 = 1,
    Test_Boot_Image_No_Of_Elems
} test_boot_image_t;

/*
 *!@brief Structure of version property.
 *
 */
typedef union StandardVersion
{
    struct
    {
        uint8_t bugfix; //!< bugfix version [7:0]
        uint8_t minor;  //!< minor version [15:8]
        uint8_t major;  //!< major version [23:16]
        char name;      //!< name [31:24]
    };
    uint32_t version;   //!< combined version numbers
} standard_version_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/
void test_version()
{
    standard_version_t version = (standard_version_t)bootloader_version();
    PRINTF("INFO: Bootloader version [%c-v%d.%d.%d]\r\n", version.name, version.major, version.minor, version.minor);
}

void test_copyright()
{
    PRINTF("INFO: Bootloader Copyright [%s]\r\n", bootloader_copyright());
}

void test_runBootloader(void)
{
#define BOOT_ARG_TAG (0xEBu)
    PRINTF("TC INFO: Starting TC %s", __func__);

    PRINTF("Enter boot image...\n");
    user_app_boot_invoke_option_t arg = {.option = {.B = {
                                                        .tag            = BOOT_ARG_TAG,
                                                        .mode           = kUserAppBootMode_IspBoot,
                                                        .boot_interface = Test_Boot_Interface_UART,
                                                    }}}; // EB: represents Enter Boot; 10: represents enter ISP mode by
                                                         // UART only,12: represents enter ISP mode by SPI only

    if (arg.option.B.tag != BOOT_ARG_TAG)
    {
        PRINTF("The runBootloader API arg is Invalid...\n");
    }
    if (arg.option.B.mode == kUserAppBootMode_IspBoot)
    {
        PRINTF("Calling the runBootloader API to force into the ISP mode: %x\n", arg.option.B.boot_interface);
        PRINTF("The runBootloader ISP interface is choosen from the following one:\n");
        PRINTF("kUserAppBootPeripheral_UART     0\n");
        PRINTF("kUserAppBootPeripheral_I2C      1\n");
        PRINTF("kUserAppBootPeripheral_SPI      2\n");
    }
    else
    {
        PRINTF("Not Calling the runBootloader API to force into the ISP mode\n");
    }

    PRINTF("Call the runBootloader API based on the arg : %x...\n", arg);
    bootloader_user_entry(&arg);
}

int main()
{
    /* Init hardware */
    BOARD_InitBootPins();

    if (BOARD_IS_XIP())
    {
        BOARD_BootClockLPR();
        CLOCK_EnableClock(kCLOCK_Otp);
        CLOCK_EnableClock(kCLOCK_Els);
        CLOCK_EnableClock(kCLOCK_ElsApb);
        RESET_PeripheralReset(kOTP_RST_SHIFT_RSTn);
        RESET_PeripheralReset(kELS_APB_RST_SHIFT_RSTn);
    }
    else
    {
        BOARD_InitBootClocks();
        CLOCK_EnableClock(kCLOCK_Flexspi);
        RESET_ClearPeripheralReset(kFLEXSPI_RST_SHIFT_RSTn);
        /* Use aux0_pll_clk / 2 */
        BOARD_SetFlexspiClock(FLEXSPI, 2U, 2U);
    }
    BOARD_InitDebugConsole();
    test_version();
    test_copyright();
    test_runBootloader();
    PRINTF("ALL Bootloader Examples completed successfully!\r\n");
    /* End of example */
    while (1)
    {
    }
}
