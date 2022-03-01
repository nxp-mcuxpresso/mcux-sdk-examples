/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "fsl_mmc.h"
#include "fsl_debug_console.h"
#include "ff.h"
#include "diskio.h"
#include "fsl_mmc_disk.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "limits.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "sdmmc_config.h"

#include "fsl_power.h"
#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_TASK_GET_SEM_BLOCK_TICKS 1U
#define DEMO_TASK_ACCESS_SDCARD_TIMES 2U
/*! @brief Task stack size. */
#define ACCESSFILE_TASK_STACK_SIZE (1024U)
/*! @brief Task stack priority. */
#define ACCESSFILE_TASK_PRIORITY (configMAX_PRIORITIES - 2U)

/*! @brief Task stack size. */
#define CARDFORMAT_TASK_STACK_SIZE (1024U)
/*! @brief Task stack priority. */
#define CARDFORMAT_TASK_PRIORITY (configMAX_PRIORITIES - 1U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief SD card access task 1.
 *
 * @param pvParameters Task parameter.
 */
static void CardAccessTask1(void *pvParameters);

/*!
 * @brief SD card access task 2.
 *
 * @param pvParameters Task parameter.
 */
static void CardAccessTask2(void *pvParameters);

/*!
 * @brief SD card detect task.
 *
 * @param pvParameters Task parameter.
 */
static void CardFormatTask(void *pvParameters);

/*!
 * @brief make filesystem.
 */
static status_t DEMO_MakeFileSystem(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static FATFS g_fileSystem; /* File system object */
static FIL g_fileObject1;  /* File object */
static FIL g_fileObject2;  /* File object */
static const uint8_t s_buffer1[] = {'T', 'A', 'S', 'K', '1', '\r', '\n'};
static const uint8_t s_buffer2[] = {'T', 'A', 'S', 'K', '2', '\r', '\n'};
/*! @brief Card semaphore  */
extern mmc_card_t g_mmc;
/*! @brief file access task handler */
TaskHandle_t fileAccessTaskHandle1;
TaskHandle_t fileAccessTaskHandle2;
/*******************************************************************************
 * Code
 ******************************************************************************/
static void CardFormatTask(void *pvParameters)
{
    BOARD_MMC_Config(&g_mmc, BOARD_SDMMC_SD_HOST_IRQ_PRIORITY);

    /* SD host init function */
    if (MMC_HostInit(&g_mmc) == kStatus_Success)
    {
        while (true)
        {
            if (DEMO_MakeFileSystem() != kStatus_Success)
            {
                continue;
            }
            xTaskNotifyGive(fileAccessTaskHandle1);
            xTaskNotifyGive(fileAccessTaskHandle2);

            break;
        }
    }
    else
    {
        PRINTF("\r\nMMC host init fail\r\n");
    }

    vTaskSuspend(NULL);
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\r\nMMCCARD fatfs freertos example.\r\n");

    if (pdPASS != xTaskCreate(CardAccessTask1, "CardAccessTask1", ACCESSFILE_TASK_STACK_SIZE, NULL,
                              ACCESSFILE_TASK_PRIORITY, &fileAccessTaskHandle1))
    {
        return -1;
    }

    if (pdPASS != xTaskCreate(CardAccessTask2, "CardAccessTask2", ACCESSFILE_TASK_STACK_SIZE, NULL,
                              ACCESSFILE_TASK_PRIORITY, &fileAccessTaskHandle2))
    {
        return -1;
    }

    if (pdPASS !=
        xTaskCreate(CardFormatTask, "CardFormatTask", CARDFORMAT_TASK_STACK_SIZE, NULL, CARDFORMAT_TASK_PRIORITY, NULL))
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

static status_t DEMO_MakeFileSystem(void)
{
    FRESULT error;
    const TCHAR driverNumberBuffer[3U] = {MMCDISK + '0', ':', '/'};
    BYTE work[FF_MAX_SS];

    if (f_mount(&g_fileSystem, driverNumberBuffer, 0U))
    {
        PRINTF("Mount volume failed.\r\n");
        return kStatus_Fail;
    }

#if (FF_FS_RPATH >= 2U)
    error = f_chdrive((char const *)&driverNumberBuffer[0U]);
    if (error)
    {
        PRINTF("Change drive failed.\r\n");
        return kStatus_Fail;
    }
#endif

#if FF_USE_MKFS
    PRINTF("\r\nMake file system......The time may be long if the card capacity is big.\r\n");
    if (f_mkfs(driverNumberBuffer, 0, work, sizeof work))
    {
        PRINTF("Make file system failed.\r\n");
        return kStatus_Fail;
    }
#endif /* FF_USE_MKFS */

    PRINTF("\r\nCreate directory......\r\n");
    error = f_mkdir(_T("/dir_1"));
    if (error)
    {
        if (error == FR_EXIST)
        {
            PRINTF("Directory exists.\r\n");
        }
        else
        {
            PRINTF("Make directory failed.\r\n");
            return kStatus_Fail;
        }
    }

    return kStatus_Success;
}

static void CardAccessTask1(void *pvParameters)
{
    UINT bytesWritten   = 0U;
    uint32_t writeTimes = 1U;
    FRESULT error;

    xTaskNotifyWait(ULONG_MAX, ULONG_MAX, NULL, portMAX_DELAY);

    while (1)
    {
        error = f_open(&g_fileObject1, _T("/dir_1/test1.txt"), FA_WRITE);
        if (error)
        {
            if (error == FR_EXIST)
            {
                PRINTF("File exists.\r\n");
            }
            /* if file not exist, creat a new file */
            else if (error == FR_NO_FILE)
            {
                if (f_open(&g_fileObject1, _T("/dir_1/test1.txt"), (FA_WRITE | FA_CREATE_NEW)) != FR_OK)
                {
                    PRINTF("Create file failed.\r\n");
                    break;
                }
            }
            else
            {
                PRINTF("Open file failed.\r\n");
                break;
            }
        }
        /* write append */
        if (f_lseek(&g_fileObject1, g_fileObject1.obj.objsize) != FR_OK)
        {
            PRINTF("lseek file failed.\r\n");
            break;
        }

        error = f_write(&g_fileObject1, s_buffer1, sizeof(s_buffer1), &bytesWritten);
        if ((error) || (bytesWritten != sizeof(s_buffer1)))
        {
            PRINTF("Write file failed.\r\n");
            break;
        }
        f_close(&g_fileObject1);

        if (++writeTimes > DEMO_TASK_ACCESS_SDCARD_TIMES)
        {
            PRINTF("TASK1: finished.\r\n");
            break;
        }
        {
            PRINTF("TASK1: write file successed.\r\n");
        }
    }
    vTaskSuspend(NULL);
}

static void CardAccessTask2(void *pvParameters)
{
    UINT bytesWritten   = 0U;
    uint32_t writeTimes = 1U;
    FRESULT error;

    xTaskNotifyWait(ULONG_MAX, ULONG_MAX, NULL, portMAX_DELAY);

    while (1)
    {
        error = f_open(&g_fileObject2, _T("/dir_1/test2.txt"), FA_WRITE);
        if (error)
        {
            if (error == FR_EXIST)
            {
                PRINTF("File exists.\r\n");
            }
            /* if file not exist, creat a new file */
            else if (error == FR_NO_FILE)
            {
                if (f_open(&g_fileObject2, _T("/dir_1/test2.txt"), (FA_WRITE | FA_CREATE_NEW)) != FR_OK)
                {
                    PRINTF("Create file failed.\r\n");
                    break;
                }
            }
            else
            {
                PRINTF("Open file failed.\r\n");
                break;
            }
        }
        /* write append */
        if (f_lseek(&g_fileObject2, g_fileObject2.obj.objsize) != FR_OK)
        {
            PRINTF("lseek file failed.\r\n");
            break;
        }

        error = f_write(&g_fileObject2, s_buffer2, sizeof(s_buffer2), &bytesWritten);
        if ((error) || (bytesWritten != sizeof(s_buffer2)))
        {
            PRINTF("Write file failed. \r\n");
            break;
        }
        f_close(&g_fileObject2);

        if (++writeTimes > DEMO_TASK_ACCESS_SDCARD_TIMES)
        {
            PRINTF("TASK2: finished.\r\n");
            break;
        }
        {
            PRINTF("TASK2: write file successed.\r\n");
        }
    }
    vTaskSuspend(NULL);
}
