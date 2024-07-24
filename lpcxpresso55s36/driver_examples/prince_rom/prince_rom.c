/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_prince.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FLASH_OPTION_QSPI_SDR 0xC0403001

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief config API context structure */
api_core_context_t apiCoreCtx;

/*! @brief API initialization data structure
 * The IAP APIs require a dedicated RAM region as the buffer for each operation. The user application needs to prepare
 * an unused RAM region organized as the kp_api_init_param_t structure, and pass this structure to the iap_api_init API.
 * For more info plese see IAP APIs section in documentation
 * Note: Avoid using  NS: 0x20002000-0x20004000 & S:0x30002000-0x30004000 address ranges, since it's PKC RAM.
 */
/*! @brief config API initialization data structure */
static kp_api_init_param_t apiInitParam = {
    .allocStart = 0x20010000U, /* Allocate an area from ram for storing configuration information. */
    .allocSize  = 0x3000       /* Configuration information size. */
};

/*! @brief PRINCE region configuration parameters
 * For more info plese see PRINCE region configuration parameters in documentation
 */
prince_prot_region_arg_t flashConfigOptionPrince = {
    .option = {.tag = PRINCE_TAG, .target_prince_region = 0u}, .start = 0x00010000, .length = 0x2000};

uint8_t flash_data[8192] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}; /* Dummy data with size 0x2000 */
uint8_t flash_read[16]   = {0};                                                     /* Read dummy data*/
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    status_t status;

    /* Init board hardware. */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    /* Do not use other clock, otherwise the Flash driver is not working correctly */
    BOARD_BootClockFRO12M();
    BOARD_InitDebugConsole();

    PRINTF("\r\nPRINCE ROM Example.\r\n");

    /* Clean up API context structure*/
    memset(&apiCoreCtx, 0, sizeof(apiCoreCtx));

    PRINTF("PRINCE Peripheral Driver Example\r\n\r\n");

    /* IAP API Init */
    PRINTF("Calling API_Init\r\n");
    status = API_Init(&apiCoreCtx, &apiInitParam);
    if (status == kStatus_Success)
    {
        PRINTF("API_Init Successfully\r\n");
    }
    else
    {
        PRINTF("API_Init failure\r\n");
    }

    /* Configure PRINCE for on the fly encryption/decryption
     * Note: This function is expected to be called once in the device lifetime,
     * typically during the initial device provisioning, since it is programming the CMPA pages in PFR flash.
     */
    PRINTF(
        "///////////////////////////////////////////  CAUTION!!!  ///////////////////////////////////////////////\r\n"
        "Once the user decides to enable ROM PRINCE feature, ROM does not accept to disable PRINCE for no encrypted "
        "boot,\r\n"
        "i.e., if PRINCE is used via ROM feature, and if you try to boot without PRINCE, that may cause boot fail!!\r\n"
        "//////////////////////////////////////////////////////////////////////////////////////////////////////////"
        "\r\n");
    PRINTF("Press any key to continue\r\n");
    GETCHAR();
    PRINTF("Configure PRINCE enc/dec: start 0x%x size %d\r\n", flashConfigOptionPrince.start,
           flashConfigOptionPrince.length);
    status = PRINCE_Configure(&apiCoreCtx, &flashConfigOptionPrince);
    if (status == kStatus_Success)
    {
        PRINTF("Configure PRINCE  Successfully\r\n");
    }
    else
    {
        PRINTF("Configure PRINCE failure\r\n");
    }

    /* The ENC_ENABLE register enables encryption of write data during flash programming. */
    /* Data written to a subregion is encrypted when ENC_ENABLE[EN] is set, and the corresponding */
    /* bit for the subregion in SR_ENABLE is set. For flash read data, decryption is enabled */
    /* when ENC_ENABLE[EN]=0 and the SR_ENABLE bit of the corresponding sub region is set. */
    PRINCE_EncryptEnable(PRINCE);

    /* Erase memory */
    status = MEM_Erase(&apiCoreCtx, flashConfigOptionPrince.start, flashConfigOptionPrince.length, kMemoryInternal);
    if (status == kStatus_Success)
    {
        PRINTF("Flash erased success\r\n");
    }
    else
    {
        PRINTF("Flash erased failure\r\n");
    }

    /* PRINCE Enable */
    PRINCE_EncryptEnable(PRINCE);

    /* Write data to memory */
    status = MEM_Write(&apiCoreCtx, flashConfigOptionPrince.start, flashConfigOptionPrince.length, flash_data,
                       kMemoryInternal);
    if (status == kStatus_Success)
    {
        PRINTF("Flash encrypted write success\r\n");
    }
    else
    {
        PRINTF("Flash encrypted write failure\r\n");
    }

    /* Flush memory after write in case of non 512 bit aligment */
    status = MEM_Flush(&apiCoreCtx);
    if (status == kStatus_Success)
    {
        PRINTF("Flush memory success\r\n");
    }
    else
    {
        PRINTF("Flush memory failure\r\n");
    }

    /* Test if PRINCE encrypt is disabled, but ENC_ENABLE[EN] bit self-clears at the end of each */
    /* flash program operation. Reading of encrypted flash regions is disabled when ENC_ENABLE[EN] is set. */
    /* When set, reads of the PRINCE-encrypted regions will return invalid data, and cause the error */
    /* and status bit will be set in the ERR register. */
    if (PRINCE_IsEncryptEnable(PRINCE) == true)
    {
        PRINCE_EncryptDisable(PRINCE);
    }

    /* Decrypted read form memory */
    memcpy(flash_read, (void *)flashConfigOptionPrince.start, 16u);

    if (memcmp(flash_read, flash_data, 16u) == 0u)
    {
        PRINTF("Decrypted data read successfully\r\n");
    }
    else
    {
        PRINTF("Decrypted data read FAIL\r\n ");
    }

    /* For testing pourpose we are clearing SR_ENABLE to demonstrate data encryption */
    PRINCE0->SR_ENABLE0 = 0;

    /* Encrypted read form memory */
    memcpy(flash_read, (void *)flashConfigOptionPrince.start, 16u);

    if (memcmp(flash_read, flash_data, 16u) != 0u)
    {
        PRINTF("Encrypted data read successfully\r\n");
    }
    else
    {
        PRINTF("Encrypted data read FAIL\r\n ");
    }

    /* Call PRINCE Reconfigure (should be called after wakeup from the Power Down mode) */
    status = PRINCE_Reconfigure(&apiCoreCtx);
    if (status == kStatus_Success)
    {
        PRINTF("Reconfigure PRINCE  Successfully\r\n");
    }
    else
    {
        PRINTF("Reconfigure PRINCE failure!!\r\n");
    }

    /* Decrypted read from memory */
    memcpy(flash_read, (void *)flashConfigOptionPrince.start, 16u);

    if (memcmp(flash_read, flash_data, 16u) == 0u)
    {
        PRINTF("Decrypted data read successfully\r\n");
    }
    else
    {
        PRINTF("Decrypted data read FAIL\r\n ");
    }

    PRINTF("\r\nExample end.\r\n");

    while (1)
    {
    }
}
