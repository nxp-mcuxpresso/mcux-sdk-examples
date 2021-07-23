/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "fsl_sd.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "sdmmc_config.h"
#include "fsl_power.h"
#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Data block count accessed in card */
#define DATA_BLOCK_COUNT (5U)
/*! @brief Start data block number accessed in card */
#define DATA_BLOCK_START (2U)
/*! @brief Data buffer size. */
#define DATA_BUFFER_SIZE (FSL_SDMMC_DEFAULT_BLOCK_SIZE * DATA_BLOCK_COUNT)

/*! @brief Task stack size. */
#define ACCESSCARD_TASK_STACK_SIZE (DATA_BUFFER_SIZE + 1000U)
/*! @brief Task stack priority. */
#define ACCESSCARD_TASK_PRIORITY (configMAX_PRIORITIES - 2U)

/*! @brief Task stack size. */
#define CARDDETECT_TASK_STACK_SIZE (DATA_BUFFER_SIZE + 1000U)
/*! @brief Task stack priority. */
#define CARDDETECT_TASK_PRIORITY (configMAX_PRIORITIES - 1U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief SD card access task.
 *
 * @param pvParameters Task parameter.
 */
static void AccessCardTask(void *pvParameters);

/*!
 * @brief SD card detect task.
 *
 * @param pvParameters Task parameter.
 */
static void CardDetectTask(void *pvParameters);

/*!
 * @brief call back function for SD card detect.
 *
 * @param isInserted  true,  indicate the card is insert.
 *                    false, indicate the card is remove.
 * @param userData
 */
static void SDCARD_DetectCallBack(bool isInserted, void *userData);

/*!
 * @brief card read/write function.
 *
 * @param card card descriptor
 * @param isReadOnly read only flag
 */
static status_t SDCARD_ReadWrite(sd_card_t *card, bool isReadOnly);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Card descriptor. */
sd_card_t g_sd;

/* @brief decription about the read/write buffer
 * The size of the read/write buffer should be a multiple of 512, since SDHC/SDXC card uses 512-byte fixed
 * block length and this driver example is enabled with a SDHC/SDXC card.If you are using a SDSC card, you
 * can define the block length by yourself if the card supports partial access.
 * The address of the read/write buffer should align to the specific DMA data buffer address align value if
 * DMA transfer is used, otherwise the buffer address is not important.
 * At the same time buffer address/size should be aligned to the cache line size if cache is supported.
 */
/*! @brief Data written to the card */
SDK_ALIGN(uint8_t g_dataWrite[DATA_BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);
/*! @brief Data read from the card */
SDK_ALIGN(uint8_t g_dataRead[DATA_BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);
/*! @brief SD card detect flag  */
static volatile bool s_cardInserted     = false;
static volatile bool s_cardInsertStatus = false;
/*! @brief Card semaphore  */
static SemaphoreHandle_t s_CardAccessSemaphore = NULL;
static SemaphoreHandle_t s_CardDetectSemaphore = NULL;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void SDCARD_DetectCallBack(bool isInserted, void *userData)
{
    s_cardInsertStatus = isInserted;
    xSemaphoreGiveFromISR(s_CardDetectSemaphore, NULL);
}

static void CardDetectTask(void *pvParameters)
{
    s_CardAccessSemaphore = xSemaphoreCreateBinary();
    s_CardDetectSemaphore = xSemaphoreCreateBinary();

    BOARD_SD_Config(&g_sd, SDCARD_DetectCallBack, BOARD_SDMMC_SD_HOST_IRQ_PRIORITY, NULL);
    /*
     * Since DEBUG_CONSOLE_TRANSFER_NON_BLOCKING is used in this case, so the debug console port uart's intterrupt
     * should be enabled, but debug console cannot cover the case that on some platform the uart's interrupt is enabled
     * in INTMUX, such as FRDMK32L3A6.
     */
#if defined(DEMO_DEBUG_CONSOLE_IRQ_REDIRECT) && (DEMO_DEBUG_CONSOLE_IRQ_REDIRECT)
    BOARD_DebugConsoleIRQRedirect();
#endif

    /* SD host init function */
    if (SD_HostInit(&g_sd) == kStatus_Success)
    {
        while (true)
        {
            /* take card detect semaphore */
            if (xSemaphoreTake(s_CardDetectSemaphore, portMAX_DELAY) != pdTRUE)
            {
                PRINTF("Failed to take semaphore.\r\n");
            }
            if (s_cardInserted != s_cardInsertStatus)
            {
                s_cardInserted = s_cardInsertStatus;

                if (s_cardInserted)
                {
                    PRINTF("\r\nCard inserted.\r\n");
                    /* power off card */
                    SD_SetCardPower(&g_sd, false);
                    /* power on the card */
                    SD_SetCardPower(&g_sd, true);
                    /* Init card. */
                    if (SD_CardInit(&g_sd))
                    {
                        PRINTF("\r\nSD card init failed.\r\n");
                        return;
                    }
                    xSemaphoreGive(s_CardAccessSemaphore);
                }
            }

            if (!s_cardInserted)
            {
                PRINTF("\r\nPlease insert a card into board.\r\n");
            }
        }
    }
    else
    {
        PRINTF("\r\nSD host init fail\r\n");
    }

    vTaskSuspend(NULL);
}

static void AccessCardTask(void *pvParameters)
{
    sd_card_t *card = &g_sd;
    bool isReadOnly;
    char ch = '0';

    /* take card access semaphore */
    if (xSemaphoreTake(s_CardAccessSemaphore, portMAX_DELAY) != pdTRUE)
    {
        PRINTF("Failed to take semaphore.\r\n");
    }
    while (ch != 'q')
    {
        if (SD_IsCardPresent(card) == false)
        {
            /* make sure host is ready for card re-initialization */
            SD_HostDoReset(card);
            /* take card access semaphore */
            if (xSemaphoreTake(s_CardAccessSemaphore, portMAX_DELAY) != pdTRUE)
            {
                PRINTF("Failed to take semaphore.\r\n");
            }
        }

        PRINTF("\r\nRead/Write/Erase the card continuously until encounter error......\r\n");
        /* Check if card is readonly. */
        isReadOnly = SD_CheckReadOnly(card);

        SDCARD_ReadWrite(card, isReadOnly);

        PRINTF(
            "\r\nInput 'q' to quit card access task.\
            \r\nInput other char to read/write/erase data blocks again.\r\n");
        ch = GETCHAR();
        PUTCHAR(ch);
    }

    PRINTF("\r\nThe card access task will not access card again.\r\n");

    SD_Deinit(card);

    vTaskSuspend(NULL);
}

/*! @brief Main function */
int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\r\nSDCARD freertos example.\r\n");

    if (pdPASS !=
        xTaskCreate(AccessCardTask, "AccessCardTask", ACCESSCARD_TASK_STACK_SIZE, NULL, ACCESSCARD_TASK_PRIORITY, NULL))
    {
        return -1;
    }

    if (pdPASS !=
        xTaskCreate(CardDetectTask, "CardDetectTask", CARDDETECT_TASK_STACK_SIZE, NULL, CARDDETECT_TASK_PRIORITY, NULL))
    {
        return -1;
    }

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* Scheduler should never reach this point. */
    while (true)
    {
    }
}

static status_t SDCARD_ReadWrite(sd_card_t *card, bool isReadOnly)
{
    if (isReadOnly)
    {
        PRINTF("\r\nRead one data block......\r\n");
        if (kStatus_Success != SD_ReadBlocks(card, g_dataRead, DATA_BLOCK_START, 1U))
        {
            PRINTF("Read one data block failed.\r\n");
            return kStatus_Fail;
        }

        PRINTF("Read multiple data blocks......\r\n");
        if (kStatus_Success != SD_ReadBlocks(card, g_dataRead, DATA_BLOCK_START, DATA_BLOCK_COUNT))
        {
            PRINTF("Read multiple data blocks failed.\r\n");
            return kStatus_Fail;
        }
    }
    else
    {
        memset(g_dataWrite, 0x67U, sizeof(g_dataWrite));

        PRINTF("\r\nWrite/read one data block......\r\n");
        if (kStatus_Success != SD_WriteBlocks(card, g_dataWrite, DATA_BLOCK_START, 1U))
        {
            PRINTF("Write one data block failed.\r\n");
            return kStatus_Fail;
        }

        memset(g_dataRead, 0U, sizeof(g_dataRead));
        if (kStatus_Success != SD_ReadBlocks(card, g_dataRead, DATA_BLOCK_START, 1U))
        {
            PRINTF("Read one data block failed.\r\n");
            return kStatus_Fail;
        }

        PRINTF("Compare the read/write content......\r\n");
        if (memcmp(g_dataRead, g_dataWrite, FSL_SDMMC_DEFAULT_BLOCK_SIZE))
        {
            PRINTF("The read/write content isn't consistent.\r\n");
            return kStatus_Fail;
        }
        PRINTF("The read/write content is consistent.\r\n");

        PRINTF("Write/read multiple data blocks......\r\n");
        if (kStatus_Success != SD_WriteBlocks(card, g_dataWrite, DATA_BLOCK_START, DATA_BLOCK_COUNT))
        {
            PRINTF("Write multiple data blocks failed.\r\n");
            return kStatus_Fail;
        }

        memset(g_dataRead, 0U, sizeof(g_dataRead));

        if (kStatus_Success != SD_ReadBlocks(card, g_dataRead, DATA_BLOCK_START, DATA_BLOCK_COUNT))
        {
            PRINTF("Read multiple data blocks failed.\r\n");
            return kStatus_Fail;
        }

        PRINTF("Compare the read/write content......\r\n");
        if (memcmp(g_dataRead, g_dataWrite, FSL_SDMMC_DEFAULT_BLOCK_SIZE))
        {
            PRINTF("The read/write content isn't consistent.\r\n");
            return kStatus_Fail;
        }
        PRINTF("The read/write content is consistent.\r\n");

        PRINTF("Erase multiple data blocks......\r\n");
        if (kStatus_Success != SD_EraseBlocks(card, DATA_BLOCK_START, DATA_BLOCK_COUNT))
        {
            PRINTF("Erase multiple data blocks failed.\r\n");
            return kStatus_Fail;
        }
    }

    return kStatus_Success;
}
