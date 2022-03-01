/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "fsl_sd.h"
#include "fsl_debug_console.h"
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
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
#define CARDDETECT_TASK_STACK_SIZE (1024U)
/*! @brief Task stack priority. */
#define CARDDETECT_TASK_PRIORITY (configMAX_PRIORITIES - 1U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief SD card access task 1.
 *
 * @param pvParameters Task parameter.
 */
static void FileAccessTask1(void *pvParameters);

/*!
 * @brief SD card access task 2.
 *
 * @param pvParameters Task parameter.
 */
static void FileAccessTask2(void *pvParameters);

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
/*! @brief SD card detect flag  */
static volatile bool s_cardInserted     = false;
static volatile bool s_cardInsertStatus = false;
/*! @brief Card semaphore  */
static SemaphoreHandle_t s_CardDetectSemaphore = NULL;
/*! @brief file access task handler */
TaskHandle_t fileAccessTaskHandle1;
TaskHandle_t fileAccessTaskHandle2;
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
    s_CardDetectSemaphore = xSemaphoreCreateBinary();

    BOARD_SD_Config(&g_sd, SDCARD_DetectCallBack, BOARD_SDMMC_SD_HOST_IRQ_PRIORITY, NULL);

    /* SD host init function */
    if (SD_HostInit(&g_sd) == kStatus_Success)
    {
        while (true)
        {
            /* take card detect semaphore */
            if (xSemaphoreTake(s_CardDetectSemaphore, portMAX_DELAY) == pdTRUE)
            {
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
                        /* make file system */
                        if (DEMO_MakeFileSystem() != kStatus_Success)
                        {
                            continue;
                        }
                        xTaskNotifyGive(fileAccessTaskHandle1);
                        xTaskNotifyGive(fileAccessTaskHandle2);
                    }
                }

                if (!s_cardInserted)
                {
                    PRINTF("\r\nPlease insert a card into board.\r\n");
                }
            }
        }
    }
    else
    {
        PRINTF("\r\nSD host init fail\r\n");
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

    PRINTF("\r\nSDCARD fatfs freertos example.\r\n");

    if (pdPASS != xTaskCreate(FileAccessTask1, "FileAccessTask1", ACCESSFILE_TASK_STACK_SIZE, NULL,
                              ACCESSFILE_TASK_PRIORITY, &fileAccessTaskHandle1))
    {
        return -1;
    }

    if (pdPASS != xTaskCreate(FileAccessTask2, "FileAccessTask1", ACCESSFILE_TASK_STACK_SIZE, NULL,
                              ACCESSFILE_TASK_PRIORITY, &fileAccessTaskHandle2))
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

static status_t DEMO_MakeFileSystem(void)
{
    FRESULT error;
    const TCHAR driverNumberBuffer[3U] = {SDDISK + '0', ':', '/'};
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

static void FileAccessTask1(void *pvParameters)
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
            writeTimes = 1U;
            xTaskNotifyWait(ULONG_MAX, ULONG_MAX, NULL, portMAX_DELAY);
            continue;
        }
        {
            PRINTF("TASK1: write file successed.\r\n");
        }
    }

    vTaskSuspend(NULL);
}

static void FileAccessTask2(void *pvParameters)
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
            writeTimes = 1U;
            xTaskNotifyWait(ULONG_MAX, ULONG_MAX, NULL, portMAX_DELAY);
            continue;
        }
        {
            PRINTF("TASK2: write file successed.\r\n");
        }
    }

    vTaskSuspend(NULL);
}
