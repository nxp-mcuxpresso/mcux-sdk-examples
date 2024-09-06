/*
 * Copyright 2024 NXP
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

/*! @brief Boot mode can be selected by user application
 * @note  For prime boot, valid boot insterfaces for user application are XSPI, LPSPI, DFU, eMMC
 *        For ISP boot, valid boot interfaces for user application are USART I2C SPI, USB-HID, eUSB
 */
enum
{
    kUserAppBootMode_PrimeBoot = 0,
    kUserAppBootMode_IspBoot   = 1,
};

/*!  @brief Boot interface can be selected by user application
 * @note  For XSPI LPSPI DFU eMMC, these interfaces are invalid for ISP boot
 */
enum
{
    kUserAppBootPeripheral_UART    = 0u,
    kUserAppBootPeripheral_I2C     = 1u,
    kUserAppBootPeripheral_SPI     = 2u,
    kUserAppBootPeripheral_USB_HID = 3u,
    kUserAppBootPeripheral_XSPI    = 4u, /*!< XSPI Nor */
    kUserAppBootPeripheral_LPSPI   = 5u, /*!< LPSPI Nor */
    kUserAppBootPeripheral_DFU     = 6u,
    kUserAppBootPeripheral_EMMC    = 7u,
    kUserAppBootPeripheral_EUSB    = 8u,
    kUserAppBootPeripheral_AUTO    = 15u,
};

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
    /* EB: Fixed value(Enter Boot); */
#define BOOT_ARG_TAG (0xEBu)
    PRINTF("TC INFO: Starting TC %s", __func__);

    PRINTF("Enter boot image...\n");
    user_app_boot_invoke_option_t arg = {.option = {.B = {
                                                        .tag            = BOOT_ARG_TAG,
                                                        .mode           = kUserAppBootMode_IspBoot,
                                                        .boot_interface = kUserAppBootPeripheral_UART,
                                                    }}};
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
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_Els);
    test_version();
    test_copyright();
    test_runBootloader();
    PRINTF("ALL Bootloader Examples completed successfully!\r\n");
    /* End of example */
    while (1)
    {
    }
}
