/*
 * Copyright 2022-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "string.h"
#include "stdbool.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "board_init.h"

/* MPP includes */
#include "mpp_api.h"
#include "mpp_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _args_t {
    char camera_name[32];
    char display_name[32];
    mpp_pixel_format_t src_format;
    mpp_pixel_format_t display_format;
} args_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void app_task(void *params);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int main(int argc, char *argv[])
{
    BaseType_t ret = pdFAIL;
    TaskHandle_t handle = NULL;

    /* Init board hardware. */
    BOARD_Init();
    args_t *args = pvPortMalloc(sizeof(args_t));
    if (!args) {
        PRINTF("Allocation failed\n");
        goto err;
    }

    strcpy(args->display_name, APP_DISPLAY_NAME);
    strcpy(args->camera_name, APP_CAMERA_NAME);
    args->src_format = APP_CAMERA_FORMAT;
    args->display_format = APP_DISPLAY_FORMAT;

    /* Create application task */
    ret = xTaskCreate(
          app_task,
          "app_task",
          configMINIMAL_STACK_SIZE + 1000,
          (void *) args,
          MPP_APP_MAX_PRIO,
          &handle);

err:
    if (pdPASS != ret)
    {
        PRINTF("Failed to create app_task task");
        while (1);
    }

    /* MPP API requires RTOS scheduler to be started */
    vTaskStartScheduler();
    for (;;)
        vTaskSuspend(NULL);
    return 0;
}

/* Application task function */
static void app_task(void *params) {
    int ret;
    args_t *args = (args_t *) params;

    PRINTF("[%s]\r\n", mpp_get_version());

    /* init API */
    ret = mpp_api_init(NULL);
    if (ret)
        goto err;

    /* create mpp */
    mpp_t mp;
    mpp_params_t mpp_params;
    memset(&mpp_params, 0, sizeof(mpp_params));
    mpp_params.exec_flag = MPP_EXEC_RC;
    mp = mpp_create(&mpp_params, &ret);
    if (mp == MPP_INVALID)
        goto err;

    /* add camera */
    mpp_camera_params_t cam_params;
    memset(&cam_params, 0 , sizeof(cam_params));
    cam_params.height = APP_CAMERA_HEIGHT;
    cam_params.width  = APP_CAMERA_WIDTH;
    cam_params.format = args->src_format;
    cam_params.fps    = 30;
    ret = mpp_camera_add(mp, args->camera_name, &cam_params);
    if (ret) {
        PRINTF("Failed to add camera %s\n", args->camera_name);
        goto err;
    }

#ifndef APP_SKIP_CONVERT_FOR_DISPLAY
    /* add convert element for color conversion and rotation
       as required by the display */
    mpp_element_params_t elem_params;
    memset(&elem_params, 0, sizeof(elem_params));
    /* pick default device from the first listed and supported by Hw */
    elem_params.convert.dev_name = NULL;
    /* set output buffer dims */
    elem_params.convert.out_buf.width = APP_DISPLAY_WIDTH;
    elem_params.convert.out_buf.height = APP_DISPLAY_HEIGHT;
    elem_params.convert.angle = APP_DISPLAY_LANDSCAPE_ROTATE;
    elem_params.convert.flip = FLIP_HORIZONTAL;
    elem_params.convert.pixel_format = args->display_format;
    elem_params.convert.ops = MPP_CONVERT_COLOR | MPP_CONVERT_ROTATE;
    ret = mpp_element_add(mp, MPP_ELEMENT_CONVERT, &elem_params, NULL);
    if (ret) {
        PRINTF("Failed to add element CONVERT - op COLOR|ROTATE\n");
        goto err;
    }
#endif /* SKIP_CONVERT */

    /* add display */
    mpp_display_params_t disp_params;
    memset(&disp_params, 0 , sizeof(disp_params));
    disp_params.format = args->display_format;
    disp_params.width  = APP_DISPLAY_WIDTH;
    disp_params.height = APP_DISPLAY_HEIGHT;
#ifdef APP_SKIP_CONVERT_FOR_DISPLAY
    disp_params.rotate = APP_DISPLAY_LANDSCAPE_ROTATE;
#endif
    ret = mpp_display_add(mp, args->display_name, &disp_params);
    if (ret) {
        PRINTF("Failed to add display %s\n", args->display_name);
        goto err;
    }

    /* start mpp and run application pipeline */
    ret = mpp_start(mp, 1);
    if (ret) {
        PRINTF("Failed to start pipeline\n");
        goto err;
    }

    /* run for 3 seconds  */
    vTaskDelay(3000/portTICK_PERIOD_MS);

    /* stop the pipeline */
    ret = mpp_stop(mp);
    if (ret) {
        PRINTF("Failed to stop pipeline\n");
        goto err;
    }
    /* wait 3 seconds */
    vTaskDelay(3000/portTICK_PERIOD_MS);

    /* restart the pipeline */
    ret = mpp_start(mp, 0);
    if (ret) {
        PRINTF("Failed to restart pipeline\n");
        goto err;
    }

    /* pause application task */
    vTaskSuspend(NULL);

err:
    for (;;)
    {
        PRINTF("Error building application pipeline : ret %d\r\n", ret);
        vTaskSuspend(NULL);
    }
}

