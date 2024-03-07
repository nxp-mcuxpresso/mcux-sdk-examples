/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_flash.h"
#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


#define EXAMPLE_IMAGE_SIZE 0x8000 /* 32KB */
#define LOWER_PFLASH_BASE  FSL_FEATURE_FLASH_PFLASH_START_ADDRESS
#define UPPER_PFLASH_BASE  (FSL_FEATURE_FLASH_PFLASH_BLOCK_SIZE * (FSL_FEATURE_FLASH_PFLASH_BLOCK_COUNT >> 1))
#define FLASH_CONFIG_START 0x400

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void error_trap(void);
void app_finalize(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Flash driver Structure */
static flash_config_t s_flashDriver;
/*! @brief 16 bytes Flash config data */
/* uint32_t flashConfigData[] = {0xffffffffU, 0xffffffffU, 0xffffffffU, 0xfffffffeU}; */
/*! @brief Test data used to validate swap result*/
uint8_t swapTestData[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x5,  0x06, 0x07,
                          0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

uint32_t pflashBaseAddr;

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
    PRINTF("\r\n\r\n\r\n\t---- HALTED DUE TO FLASH ERROR! ----");
    while (1)
    {
    }
}

/*
 * @brief Gets called when the app is complete.
 *
 * @details Print finshed message and trap forever.
 */
void app_finalize(void)
{
    /* Print finished message. */
    PRINTF("\r\n End of PFlash Swap Example \r\n");
    while (1)
    {
    }
}

/*!
 * @brief Use Standard Software Drivers (SSD) to enable flash swap feature.
 *
 * @details This function uses SSD to demonstrate flash mode:
 *            + Check flash information.
 *            + Program swap test data on upper pflash.
 *            + Swap the flash system
 *            + Check lower pflash to see whether swap test data is over there.
 */
int main(void)
{
    ftfx_security_state_t securityStatus = kFTFx_SecurityStateNotSecure; /* Return protection status */
    status_t result; /* Return code from each flash driver function */

    uint32_t pflashTotalSize  = 0;
    uint32_t pflashBlockCount = 0;
    uint32_t pflashSectorSize = 0;

    /* Init hardware */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Clean up structures*/
    memset(&s_flashDriver, 0, sizeof(flash_config_t));

    /* Setup flash driver structure for device and initialize variables. */
    result = FLASH_Init(&s_flashDriver);
    if (kStatus_FTFx_Success != result)
    {
        error_trap();
    }
    /* Get flash properties*/
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflash0TotalSize, &pflashTotalSize);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflash0BlockCount, &pflashBlockCount);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflash0SectorSize, &pflashSectorSize);

    /* Erase all blocks to clear the swap system back to uninitialized */
    /*
    uint32_t flashConfigData[] = {0xffffffffU, 0xffffffffU, 0xffffffffU, 0xfffffffeU};
    result = FLASH_EraseAll(&s_flashDriver, kFLASH_ApiEraseKey);
    result = FLASH_Program(&s_flashDriver, LOWER_PFLASH_BASE + FLASH_CONFIG_START, flashConfigData,
    sizeof(flashConfigData));
    result = FLASH_Program(&s_flashDriver, UPPER_PFLASH_BASE + FLASH_CONFIG_START, flashConfigData,
     sizeof(flashConfigData));
    */

    /* print welcome message */
    PRINTF("\r\n PFlash Swap Example Start \r\n");
    /* Print flash information - PFlash. */
    PRINTF("\r\n PFlash Information: ");
    PRINTF("\r\n Total Program Flash Size:\t%d KB, Hex: (0x%x)", (pflashTotalSize / 1024), pflashTotalSize);
    PRINTF("\r\n Total Program Flash Block Count:\t%d", pflashBlockCount);
    PRINTF("\r\n Program Flash Sector Size:\t%d KB, Hex: (0x%x) ", (pflashSectorSize / 1024), pflashSectorSize);

    /* Check security status. */
    result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
    if (kStatus_FTFx_Success != result)
    {
        error_trap();
    }
    /* Print security status. */
    switch (securityStatus)
    {
        case kFTFx_SecurityStateNotSecure:
            PRINTF("\r\n Flash is UNSECURE!");
            break;
        case kFTFx_SecurityStateBackdoorEnabled:
            PRINTF("\r\n Flash is SECURE, BACKDOOR is ENABLED!");
            break;
        case kFTFx_SecurityStateBackdoorDisabled:
            PRINTF("\r\n Flash is SECURE, BACKDOOR is DISABLED!");
            break;
        default:
            break;
    }
    PRINTF("\r\n");

#if defined(FSL_FEATURE_FLASH_HAS_PFLASH_BLOCK_SWAP) && FSL_FEATURE_FLASH_HAS_PFLASH_BLOCK_SWAP

    /* Debug message for user. */
    /* Test pflash swap feature only if flash is unsecure. */
    if (kFTFx_SecurityStateNotSecure != securityStatus)
    {
        PRINTF("\r\n Flash swap opeation will not be executed, as Flash is SECURE!");
        app_finalize();
    }
    else
    {
        struct _flash_swap_ifr_field_config
        {
            uint16_t swapIndicatorAddress; /*!< A Swap indicator address field.*/
            uint16_t swapEnableWord;       /*!< A Swap enable word field.*/
            uint8_t reserved0[4];          /*!< A reserved field.*/
            uint8_t reserved1[2];          /*!< A reserved field.*/
            uint16_t swapDisableWord;      /*!< A Swap disable word field.*/
            uint8_t reserved2[4];          /*!< A reserved field.*/
        } flashSwapIfrFieldData;
        uint32_t swapIndicatorAddress;
        ftfx_swap_state_config_t returnSwapInfo;
        uint32_t failedAddress;
        uint32_t failedData;
        uint32_t tempData[2];

        /* Clean up structures*/
        memset(&flashSwapIfrFieldData, 0, sizeof(flashSwapIfrFieldData));
        memset(&returnSwapInfo, 0, sizeof(ftfx_swap_state_config_t));

        /* Get original swap indicator address */
        result = FLASH_ReadResource(&s_flashDriver, s_flashDriver.ftfxConfig[0].ifrDesc.resRange.pflashSwapIfrStart,
                                    (uint8_t *)&flashSwapIfrFieldData, sizeof(flashSwapIfrFieldData),
                                    kFTFx_ResourceOptionFlashIfr);

        /* The high bits value of Swap Indicator Address is stored in Program Flash Swap IFR Field,
         * the low severval bit value of Swap Indicator Address is always 1'b0 */
        swapIndicatorAddress = (uint32_t)flashSwapIfrFieldData.swapIndicatorAddress *
                               s_flashDriver.ftfxConfig[0].opsConfig.addrAligment.swapCtrlCmd;
        if (kStatus_FTFx_Success != result)
        {
            error_trap();
        }
        else
        {
            PRINTF("\r\n Original swap indicator address:\t0x%x", swapIndicatorAddress);
        }

        /* It doesn't matter what the provided address is, when option is kFlashSwap_ReportStatus*/
        result = FTFx_CMD_SwapControl(&s_flashDriver.ftfxConfig[0],
                                      FSL_FEATURE_FLASH_PFLASH_SWAP_CONTROL_CMD_ADDRESS_ALIGMENT,
                                      kFTFx_SwapControlOptionReportStatus, &returnSwapInfo);
        if (kStatus_FTFx_Success != result)
        {
            error_trap();
        }
        /* Check current flash swap status*/
        switch (returnSwapInfo.flashSwapState)
        {
            case kFTFx_SwapStateUninitialized:
                PRINTF("\r\n Current swap system status: Uninitialized \r\n");
                swapIndicatorAddress = UPPER_PFLASH_BASE - pflashSectorSize;
                PRINTF("\r\n Set swap indicator address as 0x%x \r\n", swapIndicatorAddress);
                break;
            case kFTFx_SwapStateReady:
                PRINTF("\r\n Current swap system status: Ready \r\n");
                break;
            case kFTFx_SwapStateUpdate:
                PRINTF("\r\n Current swap system status: Update \r\n");
                break;
            case kFTFx_SwapStateUpdateErased:
                PRINTF("\r\n Current swap system status: UpdateErased \r\n");
                break;
            case kFTFx_SwapStateComplete:
                PRINTF("\r\n Current swap system status: Complete \r\n");
                break;
            case kFTFx_SwapStateDisabled:
                PRINTF("\r\n Current swap system status: Disabled \r\n");
                app_finalize();
                break;
            default:
                error_trap();
                break;
        }

        /* Check if the original indicator address is valid */
        if (returnSwapInfo.flashSwapState != kFTFx_SwapStateUninitialized)
        /* if (flashSwapIfrField.swapIndicatorAddress != (uint16_t)(~0)) */
        {
            if ((swapIndicatorAddress < EXAMPLE_IMAGE_SIZE) || (swapIndicatorAddress >= UPPER_PFLASH_BASE))
            {
                PRINTF("\r\n Original swap indicator address couldn't be used for this example \r\n");
                PRINTF(
                    "\r\n Swap system needs to be cleared back to uninitialized, This example cannot clear swap system "
                    "\r\n");
                app_finalize();
            }
        }

        /* Check if backup example image is valid */
        PRINTF("\r\n Validating the backup example image... \r\n");
        /* Workaround here for first programming */
        pflashBaseAddr = LOWER_PFLASH_BASE;
        memcpy((uint8_t *)(&tempData[0]), (uint8_t *)pflashBaseAddr, sizeof(tempData));
        if (memcmp((uint8_t *)(&tempData[0]), (uint8_t *)UPPER_PFLASH_BASE, 8))
        {
            PRINTF("\r\n Backup example image is invalid \r\n");
            PRINTF("\r\n Start to program backup example image \r\n");
            /* Need to program the backup image */
            result = FLASH_Erase(&s_flashDriver, UPPER_PFLASH_BASE, EXAMPLE_IMAGE_SIZE, kFTFx_ApiEraseKey);
            if (kStatus_FTFx_Success != result)
            {
                error_trap();
            }
            result = FLASH_Program(&s_flashDriver, UPPER_PFLASH_BASE, (uint8_t *)tempData, sizeof(tempData));
            if (kStatus_FTFx_Success != result)
            {
                error_trap();
            }
            if (memcmp((uint8_t *)(&tempData[0]), (uint8_t *)UPPER_PFLASH_BASE, sizeof(tempData)))
            {
                error_trap();
            }
            result =
                FLASH_Program(&s_flashDriver, UPPER_PFLASH_BASE + sizeof(tempData),
                              (uint8_t *)(LOWER_PFLASH_BASE + sizeof(tempData)), EXAMPLE_IMAGE_SIZE - sizeof(tempData));
            if (kStatus_FTFx_Success != result)
            {
                error_trap();
            }
            result = FLASH_VerifyProgram(
                &s_flashDriver, UPPER_PFLASH_BASE + sizeof(tempData), EXAMPLE_IMAGE_SIZE - sizeof(tempData),
                (uint8_t *)(LOWER_PFLASH_BASE + sizeof(tempData)), kFTFx_MarginValueUser, &failedAddress, &failedData);
            if (kStatus_FTFx_Success != result)
            {
                error_trap();
            }
            PRINTF("\r\n Finish programming backup example image \r\n");
        }
        else
        {
            PRINTF("\r\n Backup example image is valid \r\n");
        }

        /* Verify if it is first swap for this example*/
        PRINTF("\r\n Checking the system status... \r\n");
        if (!memcmp((uint8_t *)(LOWER_PFLASH_BASE + EXAMPLE_IMAGE_SIZE), swapTestData, sizeof(swapTestData)))
        {
            PRINTF("\r\n The system has been swapped successfully \r\n");
            PRINTF("\r\n Start to erase test data on lower pflash before exiting this example\r\n");
            result = FLASH_Erase(&s_flashDriver, LOWER_PFLASH_BASE + EXAMPLE_IMAGE_SIZE, sizeof(swapTestData),
                                 kFTFx_ApiEraseKey);
            if (kStatus_FTFx_Success != result)
            {
                error_trap();
            }
            PRINTF("\r\n Finish erasing test data \r\n");

            /* app_finalize(); */
            PRINTF("\r\n End of PFlash Swap Example \r\n");

            /* As the last flash sector of lower block is set as swap indicator address,
             *  when the system is swapped, the swap indicator will be last sector of upper block,
             *  the swap indicator is used to store swap info, so it is protected by system,
             *  and it is not allowed to erase/progeam swap indicator*/
            /* Note1: In order to eliminate the impact on other flash case, erasing all the flash is needed here*/
            /* Note2: Don't follow Note1 if we want that this case can pass in auto test system*/
            /*
            #if defined(FSL_FEATURE_FLASH_HAS_ERASE_ALL_BLOCKS_UNSECURE_CMD) &&
            FSL_FEATURE_FLASH_HAS_ERASE_ALL_BLOCKS_UNSECURE_CMD
                        FLASH_EraseAllUnsecure(&s_flashDriver, kFLASH_ApiEraseKey);
            #else
                        FLASH_EraseAll(&s_flashDriver, kFLASH_ApiEraseKey);
            #endif
            */
            while (1)
            {
            }
        }
        else
        {
            PRINTF("\r\n It is first swap for the system \r\n");
            PRINTF("\r\n Start to erase test data on lower pflash before swapping system \r\n");
            result = FLASH_Erase(&s_flashDriver, LOWER_PFLASH_BASE + EXAMPLE_IMAGE_SIZE, sizeof(swapTestData),
                                 kFTFx_ApiEraseKey);
            if (kStatus_FTFx_Success != result)
            {
                error_trap();
            }
            PRINTF("\r\n Finish erasing test data \r\n");
            PRINTF("\r\n Start to program backup test data on upper pflash \r\n");
            result = FLASH_Erase(&s_flashDriver, UPPER_PFLASH_BASE + EXAMPLE_IMAGE_SIZE, sizeof(swapTestData),
                                 kFTFx_ApiEraseKey);
            if (kStatus_FTFx_Success != result)
            {
                error_trap();
            }
            result = FLASH_Program(&s_flashDriver, UPPER_PFLASH_BASE + EXAMPLE_IMAGE_SIZE, &swapTestData[0],
                                   sizeof(swapTestData));
            if (kStatus_FTFx_Success != result)
            {
                error_trap();
            }
            result = FLASH_VerifyProgram(&s_flashDriver, UPPER_PFLASH_BASE + EXAMPLE_IMAGE_SIZE, sizeof(swapTestData),
                                         &swapTestData[0], kFTFx_MarginValueUser, &failedAddress, &failedData);
            if (kStatus_FTFx_Success != result)
            {
                error_trap();
            }
            PRINTF("\r\n Finish programming backup test data \r\n");
        }

        /* Enable swap system*/
        PRINTF("\r\n Start to swap the system \r\n");
        result = FLASH_Swap(&s_flashDriver, swapIndicatorAddress, true);
        if (kStatus_FTFx_Success != result)
        {
            error_trap();
        }

        /* Reset MCU */
        PRINTF("\r\n Perform a system reset \r\n");
        NVIC_SystemReset();
    }

#else
    PRINTF("\r\n Current device doesn't support flash swap feature \r\n");

    app_finalize();
#endif

    return 0;
}
