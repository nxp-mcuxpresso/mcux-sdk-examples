/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "board.h"
#include "demo_config.h"
#include "sdmmc_config.h"

#include "fsl_sd.h"
#include "fsl_sd_disk.h"
#include "ff.h"

#include "task.h"
#include "semphr.h"
#include "event_groups.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BUFFER_SIZE 100

/*******************************************************************************
 * Variables
 ******************************************************************************/
sd_card_t g_sdCard;
static FATFS g_fileSystem; /* File system object */
static FIL g_fileObject;   /* File object */
const TCHAR g_driverNumberBuffer[3] = {SDDISK + '0', ':', '/'};

SDK_ALIGN(uint8_t g_bufferWrite[BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);
SDK_ALIGN(uint8_t g_bufferRead[BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

static QueueHandle_t s_CardDetectSemaphore = NULL;
extern EventGroupHandle_t g_errorEvent;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void SDCARD_DetectCallBack(bool isInserted, void *userData);
static status_t sdcardWaitCardInsert(void);
static void sdcard_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/

static void SDCARD_DetectCallBack(bool isInserted, void *userData)
{
    xSemaphoreGiveFromISR(s_CardDetectSemaphore, NULL);
}

void sdcard_init(void)
{
    if (xTaskCreate(sdcard_task, "SDCard Task", 1000UL / sizeof(portSTACK_TYPE), NULL, 4U, NULL) != pdPASS)
    {
        PRINTF("SDCard Task Creation Failed!\r\n");
        while (1)
            ;
    }
}

static status_t sdcardWaitCardInsert(void)
{
    s_CardDetectSemaphore = xSemaphoreCreateBinary();
    BOARD_SD_Config(&g_sd, SDCARD_DetectCallBack, BOARD_SDMMC_SD_HOST_IRQ_PRIORITY, NULL);

    /* SD host init function */
    if (SD_HostInit(&g_sd) != kStatus_Success)
    {
        PRINTF("\r\nSD host init fail\r\n");
        return kStatus_Fail;
    }

    return kStatus_Success;
}

static void sdcard_task(void *pvParameters)
{
    FRESULT error;
    UINT bytesRead;
    UINT bytesWritten;

    if (sdcardWaitCardInsert() != kStatus_Success)
    {
        PRINTF("SDCard Init Failed!\r\n");
        return;
    }
    while (1)
    {
        if (xSemaphoreTake(s_CardDetectSemaphore, portMAX_DELAY) == pdTRUE)
        {
            if (f_mount(&g_fileSystem, g_driverNumberBuffer, 0U))
            {
                PRINTF("SDCARD: Mount volume failed!\r\n");
                continue;
            }

#if (FF_FS_RPATH >= 2U)
            error = f_chdrive((char const *)&g_driverNumberBuffer[0U]);
            if (error)
            {
                PRINTF("Change drive failed.\r\n");
                continue;
            }
#endif
            error = f_open(&g_fileObject, _T("testfile.txt"), (FA_READ | FA_WRITE | FA_CREATE_ALWAYS));
            if (error)
            {
                if (error == FR_EXIST)
                {
                    PRINTF("SDCARD: File exists.\r\n");
                }
                else
                {
                    PRINTF("SDCARD: Failed to open testfile.txt!\r\n");
                    continue;
                }
            }

            memset(g_bufferWrite, 'a', sizeof(g_bufferWrite));
            g_bufferWrite[BUFFER_SIZE - 2U] = '\r';
            g_bufferWrite[BUFFER_SIZE - 1U] = '\n';
            PRINTF("\r\nSDCARD: Write/read file ......\r\n");
            PRINTF("\r\nWrite to testfile.txt.\r\n");
            error = f_write(&g_fileObject, g_bufferWrite, sizeof(g_bufferWrite), &bytesWritten);
            if ((error) || (bytesWritten != sizeof(g_bufferWrite)))
            {
                PRINTF("SDCARD: Write file failed. \r\n");
                continue;
            }

            /* Move the file pointer */
            if (f_lseek(&g_fileObject, 0U))
            {
                PRINTF("SDCARD: Set file pointer position failed. \r\n");
                continue;
            }

            PRINTF("\r\nRead from testfile.txt.\r\n");
            memset(g_bufferRead, 0U, sizeof(g_bufferRead));
            error = f_read(&g_fileObject, g_bufferRead, sizeof(g_bufferRead), &bytesRead);

            if (error || (bytesRead != sizeof(g_bufferRead)))
            {
                PRINTF("SDCARD: Failed to read file!\r\n");
                continue;
            }

            PRINTF("Compare the read/write content......\r\n");
            if (memcmp(g_bufferRead, g_bufferRead, sizeof(g_bufferWrite)) == 0)
            {
                PRINTF("SDCARD Tested OK!\r\n");
                f_close(&g_fileObject);
                break;
            }
            else
            {
                PRINTF("SDCARD: File content not correct!\r\n");
                PRINTF("SDCARD: Expected: %s\r\n", g_bufferWrite);
                PRINTF("SDCARD: Found:    %s\r\n", g_bufferRead);
                xEventGroupSetBits(g_errorEvent, 1U);
                f_close(&g_fileObject);
                break;
            }
        }
    }
    vTaskSuspend(NULL);
}
