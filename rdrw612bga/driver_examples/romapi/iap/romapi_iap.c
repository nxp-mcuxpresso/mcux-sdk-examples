/*
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_romapi_iap.h"
#include "fsl_iped.h"
#include "fsl_cache.h"

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

enum _ospi_constants
{
    kOspiMem_BaseAddr               = 0x08000000,
    kOspiMem_ConfigBlockOffset      = 0x400,
    kOspiMem_BootImageVersionOffset = 0x600,
    kOspiMem_ImageStartOffset       = 0x1000,
    kOspiMem_MaxSizeInBytes         = 128ul * 1024ul * 1024ul,
    // Formula: For DDR command, delay cell = 1/4 cycle time at max support frequency / interval per cell
    //        : For SDR command, delay cell = 1/2 cycle time at max support frequency / interval per cell
    kOspiDelayCell_FailsafeValueDdr = 30,
    kOspiDelayCell_FailsafeValueSdr = 50,
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/
static uint32_t get_remap_offset()
{
    uint32_t offset = FLEXSPI->HADDROFFSET;
    return offset;
}

static void clear_ahb_buffer()
{
    FLEXSPI->AHBCR |= FLEXSPI_AHBCR_CLRAHBRXBUF_MASK;
}

void test_version()
{
    standard_version_t version = (standard_version_t)iap_api_version();
    PRINTF("INFO: IAP driver version [%c-v%d.%d.%d]\r\n", version.name, version.major, version.minor, version.minor);
}

void test_iap_mem_operation()
{
    PRINTF("INFO: Starting Example %s \r\n", __func__);
    status_t status            = kStatus_Fail;
    const uint32_t fcb_address = kOspiMem_BaseAddr + kOspiMem_ConfigBlockOffset;
    const uint32_t address     = kOspiMem_BaseAddr + 0x10000;
    const uint32_t data        = 0x10203040;
    uint32_t remapped_address  = address + get_remap_offset();
    api_core_context_t apiCoreCtx;

    const kp_api_init_param_t apiInitParam = {.allocStart = 0x30010000, .allocSize = 0x6000};
    status                                 = iap_api_init(&apiCoreCtx, &apiInitParam);
    APP_ASSERT(kStatus_Success, status, "iap_api_init returned with code [0x%X]\r\n", status);

    status = iap_mem_config(&apiCoreCtx, (uint32_t *)fcb_address, kMemoryID_FlexspiNor);
    APP_ASSERT(kStatus_Success, status, "iap_mem_config returned with code [0x%X]\r\n", status);

    status = iap_mem_erase(&apiCoreCtx, remapped_address, sizeof(uint32_t), kMemoryID_FlexspiNor);
    APP_ASSERT(kStatus_Success, status, "iap_mem_erase returned with code [0x%X]\r\n", status);

    status = iap_mem_write(&apiCoreCtx, remapped_address, sizeof(data), (uint8_t *)&data, kMemoryID_FlexspiNor);
    APP_ASSERT(kStatus_Success, status, "iap_mem_write returned with code [0x%X]\r\n", status);

    status = iap_mem_flush(&apiCoreCtx);
    APP_ASSERT(kStatus_Success, status, "iap_mem_flush returned with code [0x%X]\r\n", status);

    clear_ahb_buffer();
    CACHE64_InvalidateCache(CACHE64_CTRL0);

    // The test is pass, if the memory content was programmed.
    APP_ASSERT(data, *((uint32_t *)remapped_address),
               "memory content was not programmed correctly!\
               expected [0x%X], actual [0x%X]\r\n",
               data, *((uint32_t *)remapped_address));

    PRINTF("INFO: Finished Example %s \r\n", __func__);
}

void test_iap_mem_operation_user_managed_encrypted_flash()
{
    PRINTF("INFO: Starting Example %s \r\n", __func__);
    status_t status                = kStatus_Fail;
    const uint32_t fcb_address     = kOspiMem_BaseAddr + kOspiMem_ConfigBlockOffset;
    const uint32_t address         = kOspiMem_BaseAddr + 0x10000;
    const uint32_t data            = 0x10203040;
    uint32_t remapped_address      = address + get_remap_offset();
    uint32_t page_size             = ((flexspi_nor_config_t *)fcb_address)->pageSize;
    uint32_t encrypted_region_size = remapped_address + 12 * page_size;
    api_core_context_t apiCoreCtx;
    const uint8_t iv[8] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    };

    IPED_EncryptEnable(FLEXSPI);

    // Set up an IPED region that contains the address to write to. The region size is a
    // multiple of 4 times the Flash page size.
    status = IPED_SetRegionAddressRange(FLEXSPI, kIPED_Region11, remapped_address, encrypted_region_size);
    APP_ASSERT(status, kStatus_Success, "IPED_SetRegionAddressRange returned with code [0x%08x]\r\n");

    // In a real-world scenario, the IV needs to be carefully chosen to avoid re-using it.
    IPED_SetRegionIV(FLEXSPI, kIPED_Region11, iv);
    APP_ASSERT(status, kStatus_Success, "IPED_SetRegionAddressRange returned with code [0x%08x]\r\n");

    status = IPED_SetRegionEnable(FLEXSPI, kIPED_Region11, true);
    APP_ASSERT(status, kStatus_Success, "IPED_SetRegionEnable  returned with code [0x%08x]\r\n");

    clear_ahb_buffer();
    CACHE64_InvalidateCache(CACHE64_CTRL0);

    const kp_api_init_param_t apiInitParam = {.allocStart = 0x30010000, .allocSize = 0x6000};
    status                                 = iap_api_init(&apiCoreCtx, &apiInitParam);
    APP_ASSERT(kStatus_Success, status, "iap_api_init returned with code [0x%X]\r\n", status);

    status = iap_mem_config(&apiCoreCtx, (uint32_t *)fcb_address, kMemoryID_FlexspiNor);
    APP_ASSERT(kStatus_Success, status, "iap_mem_config returned with code [0x%X]\r\n", status);

    status = iap_mem_erase(&apiCoreCtx, remapped_address, sizeof(uint32_t), kMemoryID_FlexspiNor);
    APP_ASSERT(kStatus_Success, status, "iap_mem_erase returned with code [0x%X]\r\n", status);

    status = iap_mem_write(&apiCoreCtx, remapped_address, sizeof(data), (uint8_t *)&data, kMemoryID_FlexspiNor);
    APP_ASSERT(kStatus_Success, status, "iap_mem_write returned with code [0x%X]\r\n", status);

    status = iap_mem_flush(&apiCoreCtx);
    APP_ASSERT(kStatus_Success, status, "iap_mem_flush returned with code [0x%X]\r\n", status);

    clear_ahb_buffer();
    CACHE64_InvalidateCache(CACHE64_CTRL0);

    // The test is pass, if the memory content was programmed.
    APP_ASSERT(data, *((uint32_t *)remapped_address),
               "memory content was not programmed correctly!\
               expected [0x%X], actual [0x%X]\r\n",
               data, *((uint32_t *)remapped_address));

    status = IPED_SetRegionEnable(FLEXSPI, kIPED_Region11, false);
    APP_ASSERT(status, kStatus_Success, "IPED_SetRegionAddressRange returned with code [0x%08x]\r\n");

    clear_ahb_buffer();
    CACHE64_InvalidateCache(CACHE64_CTRL0);

    // When the region is disabled, the data is not readable.
    if (data == *((uint32_t *)remapped_address))
    {
        PRINTF("Example failed: ");
        PRINTF("memory content was readable after disabling encryption!");
        while (1)
            ;
    }

    PRINTF("INFO: Finished Example %s \r\n", __func__);
}

void test_sb_loader_examples()
{
    PRINTF("INFO: Starting Example %s \r\n", __func__);
    status_t status = kStatus_Fail;
    api_core_context_t apiCoreCtx;

    const kp_api_init_param_t apiInitParam = {.allocStart = 0x30010000, .allocSize = 0x6000};
    status                                 = iap_api_init(&apiCoreCtx, &apiInitParam);
    APP_ASSERT(kStatus_Success, status, "iap_api_init returned with code [0x%X]\r\n", status);

    status = iap_sbloader_init(&apiCoreCtx);
    APP_ASSERT(kStatus_Success, status, "iap_sbloader_init returned with code [0x%X]\r\n", status);

#if 0
    uint8_t user_buf[1024];
    uint32_t size;
    while ((status = get_data_from_sbfile(&buffer, &size)) == kStatus_Success)
    {
        status = iap_sbloader_pump(&apiCoreCtx, &buffer, size);
        if (status == kStatusRomLdrDataUnderrun)
        {
            PRINTF("Warning: Success to execute ROM API erase, but Need more data \n");
            //continue to receive the data from SB file for processing
        }
        else if (status == kStatus_Success)
        {
            PRINTF("Completed: Success to execute ROM API erase\n");
            break; //finish the SB file firewarre update and break
        }
        else if (status != kStatus_Success)
        {
            PRINTF("Error: Failed to run API kb_execute, status is hex(%x) \n", (uint32_t)status);
            return status; //return the fail status or other actions
        }
    }
#endif
    status = iap_sbloader_finalize(&apiCoreCtx);
    APP_ASSERT(kStatus_Success, status, "iap_sbloader_finalize returned with code [0x%X]\r\n", status);

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
    test_version();

    test_iap_mem_operation();
    test_iap_mem_operation_user_managed_encrypted_flash();
    test_sb_loader_examples();
    PRINTF("ALL IAP Examples completed successfully!\r\n");
    /* End of example */
    while (1)
    {
    }
}
