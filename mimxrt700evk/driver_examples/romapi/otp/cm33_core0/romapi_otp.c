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
#include "fsl_romapi_otp.h"

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

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t read_otp_fusewords_range(uint32_t idx, uint32_t num, uint32_t *buffer)
{
    for (uint32_t i = 0; i < num; i++, buffer++, idx++)
    {
        if (otp_fuse_read(idx, buffer) != kStatus_Success)
        {
            PRINTF("otp_fuse_read failed for fuse index=[%d]\r\n", idx);
            return kStatus_Fail;
        }
    }
    return kStatus_Success;
}

void test_otp_init()
{
    PRINTF("INFO: Starting Example %s \r\n", __func__);

    otp_init();

    PRINTF("INFO: Finished Example %s\r\n", __func__);
}

void test_otp_fuse_read()
{
    PRINTF("INFO: Starting Example %s \r\n", __func__);
    status_t status          = kStatus_Fail;
    uint32_t fuse_read_index = 52;
    uint32_t fuse_read_data  = 0x12345678;
    status                   = otp_fuse_read(fuse_read_index, &fuse_read_data);
    APP_ASSERT(kStatus_Success, status, "otp_fuse_read returned with code [0x%X]\r\n", status);
    PRINTF("Read fuseword[%d] = [0x%X]\r\n", fuse_read_index, fuse_read_data);

    PRINTF("INFO: Finished Example %s \r\n", __func__);
}

void test_otp_fuse_program()
{
/* Disable example for fuse program */
#if defined(romapi_FUSE_PROGRAM_ENABLE)
    PRINTF("INFO: Starting Example %s \r\n", __func__);
    status_t status           = kStatus_Fail;
    uint32_t fuse_write_index = 135;
    uint32_t fuse_write_data  = 0x1;
    status                    = otp_fuse_program(fuse_write_index, fuse_write_data);
    APP_ASSERT(kStatus_Success, status, "otp_fuse_read returned with code [0x%X]\r\n", status);

    uint32_t read_back = 0xdead1234;
    status             = otp_fuse_read(fuse_write_index, &read_back);
    APP_ASSERT(kStatus_Success, status, "otp_fuse_read returned with code [0x%X]\r\n", status);
    APP_ASSERT(read_back, fuse_write_data,
               "Unexpected programmed data fuse_write_data = [0x%X], read_back = [0x%X]\r\n", fuse_write_data,
               read_back);
    PRINTF("Programming fuseword[%d] = [0x%X] successful\r\n", fuse_write_index, read_back);
    PRINTF("INFO: Finished Example %s \r\n", __func__);
#endif
}

void test_otp_deinit()
{
    PRINTF("INFO: Starting Example %s \r\n", __func__);
    status_t status = kStatus_Fail;

    status = otp_deinit();
    APP_ASSERT(kStatus_Success, status, "otp_deinit returned with code [0x%X]\r\n", status);

    PRINTF("INFO: Finished Example %s \r\n", __func__);
}

int main()
{
    /* Init hardware */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_Els);
    PRINTF("\r\nROM API OTP Driver Version 0x%X\r\n", otp_version());

    test_otp_init();
    test_otp_fuse_read();
    test_otp_fuse_program();
    test_otp_deinit();

    PRINTF("ALL OTP Examples completed successfully!\r\n");
    /* End of example */
    while (1)
    {
    }
}
