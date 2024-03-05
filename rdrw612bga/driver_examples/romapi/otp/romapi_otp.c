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
#include "fsl_romapi_otp.h"

#include <string.h>

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEFAULT_SYSTEM_CLOCK            260000000u /* Default System clock value */
#define OTP_OTP_SHARE1_31_0_FUSE_IDX    (76u)
#define OTP_OTP_SHARE2_255_224_FUSE_IDX (91u)
#define OTP_CRC5_FUSE_IDX               (365u)
#define OTP_BOOT_CFG0_FUSE_IDX          (15u)
#define OTP_BOOT_CFG6_FUSE_IDX          (21u)
#define OTP_CRC1_FUSE_IDX               (361u)

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
    status_t status = kStatus_Fail;

    status = otp_init(DEFAULT_SYSTEM_CLOCK);
    APP_ASSERT(kStatus_Success, status, "otp_init returned with code [0x%X]\r\n", status);

    PRINTF("INFO: Finished Example %s \r\n", __func__);
}

void test_otp_fuse_read()
{
    PRINTF("INFO: Starting Example %s \r\n", __func__);
    status_t status          = kStatus_Fail;
    uint32_t fuse_read_index = 45;
    uint32_t fuse_read_data  = 0x12345678;
    status                   = otp_fuse_read(fuse_read_index, &fuse_read_data);
    APP_ASSERT(kStatus_Success, status, "otp_fuse_read returned with code [0x%X]\r\n", status);
    PRINTF("Read fuseword[%d] = [0x%X]\r\n", fuse_read_index, fuse_read_data);

    PRINTF("INFO: Finished Example %s \r\n", __func__);
}

void test_otp_fuse_program()
{
// Disable example for fuse program
#if defined romapi_FUSE_PROGRAM_ENABLE
    PRINTF("INFO: Starting Example %s \r\n", __func__);
    status_t status           = kStatus_Fail;
    uint32_t fuse_write_index = 408;
    uint32_t fuse_write_data  = 0xCBFE;
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

void test_otp_crc_check()
{
// Disable example for Crc Checks
#if defined romapi_CRC_CHECK_ENABLE
    PRINTF("INFO: Starting Example %s \r\n", __func__);
    status_t status = kStatus_Fail;
    status          = otp_crc_check(OTP_OTP_SHARE1_31_0_FUSE_IDX, OTP_OTP_SHARE2_255_224_FUSE_IDX, OTP_CRC5_FUSE_IDX);
    APP_ASSERT(kStatus_OTP_CrcCheckPass, status, "otp_crc_check returned with code [0x%X]\r\n", status);
    PRINTF("INFO: Finished Example %s \r\n", __func__);
#endif
}

void test_otp_crc_check_sw()
{
// Disable example for Crc Checks
#if defined romapi_CRC_CHECK_ENABLE
    PRINTF("INFO: Starting Example %s \r\n", __func__);
    status_t status           = kStatus_Fail;
    const uint32_t start_addr = OTP_BOOT_CFG0_FUSE_IDX, end_addr = OTP_BOOT_CFG6_FUSE_IDX, crc_aadr = OTP_CRC1_FUSE_IDX;
    const uint32_t num_words = end_addr - start_addr;
    uint32_t crc_checksum    = 0;
    uint32_t otp_fusewords_buffer[6]; // otp_fusewords_buffer[num_words]
    status = read_otp_fusewords_range(start_addr, num_words, otp_fusewords_buffer);
    APP_ASSERT(kStatus_Success, status, "read_otp_fusewords_range returned with code [0x%X]\r\n", status);

    status = otp_crc_calc(otp_fusewords_buffer, num_words, &crc_checksum);
    APP_ASSERT(kStatus_Success, status, "otp_crc_calc returned with code [0x%X]\r\n", status);
    PRINTF("CRC checksum = 0x%X\r\n", crc_checksum);

    status = otp_crc_check_sw(otp_fusewords_buffer, num_words, crc_aadr);
    APP_ASSERT(kStatus_Success, status, "otp_crc_check_sw returned with code [0x%X]\r\n", status);
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
    PRINTF("\r\nROM API OTP Driver Version 0x%X\r\n", otp_version());

    test_otp_init();
    test_otp_fuse_read();
    test_otp_fuse_program();
    test_otp_crc_check();
    test_otp_crc_check_sw();
    test_otp_deinit();

    PRINTF("ALL OTP Examples completed successfully!\r\n");
    /* End of example */
    while (1)
    {
    }
}
