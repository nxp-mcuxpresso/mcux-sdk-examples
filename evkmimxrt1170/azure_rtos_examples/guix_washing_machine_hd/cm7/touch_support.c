/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_gt911.h"
#include "touch_support.h"
#include "board.h"

#include "tx_api.h"
#include "gx_api.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TOUCH_STATE_TOUCHED     1
#define TOUCH_STATE_RELEASED    2
#define MIN_DRAG_DELTA          10

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void BOARD_PullMIPIPanelTouchResetPin(bool pullUp);

static void BOARD_ConfigMIPIPanelTouchIntPin(gt911_int_pin_mode_t mode);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static gt911_handle_t s_touchHandle;
static const gt911_config_t s_touchConfig = {
    .I2C_SendFunc     = BOARD_MIPIPanelTouch_I2C_Send,
    .I2C_ReceiveFunc  = BOARD_MIPIPanelTouch_I2C_Receive,
    .pullResetPinFunc = BOARD_PullMIPIPanelTouchResetPin,
    .intPinFunc       = BOARD_ConfigMIPIPanelTouchIntPin,
    .timeDelayMsFunc  = VIDEO_DelayMs,
    .touchPointNum    = 1,
    .i2cAddrMode      = kGT911_I2cAddrMode0,
    .intTrigMode      = kGT911_IntRisingEdge,
};
static int s_touchResolutionX;
static int s_touchResolutionY;

static GX_VALUE last_pos_x = 0;
static GX_VALUE last_pos_y = 0;

TX_THREAD touch_thread;
UCHAR touch_thread_stack[2048];

/*******************************************************************************
 * Code
 ******************************************************************************/

static void BOARD_PullMIPIPanelTouchResetPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_MIPI_PANEL_TOUCH_RST_GPIO, BOARD_MIPI_PANEL_TOUCH_RST_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(BOARD_MIPI_PANEL_TOUCH_RST_GPIO, BOARD_MIPI_PANEL_TOUCH_RST_PIN, 0);
    }
}

static void BOARD_ConfigMIPIPanelTouchIntPin(gt911_int_pin_mode_t mode)
{
    if (mode == kGT911_IntPinInput)
    {
        BOARD_MIPI_PANEL_TOUCH_INT_GPIO->GDIR &= ~(1UL << BOARD_MIPI_PANEL_TOUCH_INT_PIN);
    }
    else
    {
        if (mode == kGT911_IntPinPullDown)
        {
            GPIO_PinWrite(BOARD_MIPI_PANEL_TOUCH_INT_GPIO, BOARD_MIPI_PANEL_TOUCH_INT_PIN, 0);
        }
        else
        {
            GPIO_PinWrite(BOARD_MIPI_PANEL_TOUCH_INT_GPIO, BOARD_MIPI_PANEL_TOUCH_INT_PIN, 1);
        }

        BOARD_MIPI_PANEL_TOUCH_INT_GPIO->GDIR |= (1UL << BOARD_MIPI_PANEL_TOUCH_INT_PIN);
    }
}

void BOARD_InitTouchPanel(void)
{
    status_t status;

    const gpio_pin_config_t resetPinConfig = {
        .direction = kGPIO_DigitalOutput, .outputLogic = 0, .interruptMode = kGPIO_NoIntmode};
    GPIO_PinInit(BOARD_MIPI_PANEL_TOUCH_INT_GPIO, BOARD_MIPI_PANEL_TOUCH_INT_PIN, &resetPinConfig);
    GPIO_PinInit(BOARD_MIPI_PANEL_TOUCH_RST_GPIO, BOARD_MIPI_PANEL_TOUCH_RST_PIN, &resetPinConfig);

    status = GT911_Init(&s_touchHandle, &s_touchConfig);

    if (kStatus_Success != status)
    {
        PRINTF("Touch IC initialization failed\r\n");
        assert(false);
    }

    GT911_GetResolution(&s_touchHandle, &s_touchResolutionX, &s_touchResolutionY);
}

static status_t DEMO_ReadTouch(gt911_handle_t *handle, int *p_x, int *p_y)
{
    status_t status;
    int touch_x = 0;
    int touch_y = 0;

    status = GT911_GetSingleTouch(handle, &touch_x, &touch_y);
    if (status == kStatus_Success)
    {
        *p_x = touch_x * DEMO_PANEL_WIDTH / s_touchResolutionX;
        *p_y = touch_y * DEMO_PANEL_HEIGHT / s_touchResolutionY;
        return kStatus_Success;
    }
    else
    {
        return kStatus_Fail;
    }
}

static VOID gx_send_pen_down_event(GX_VALUE x, GX_VALUE y)
{
    GX_EVENT event;

    event.gx_event_type                                  = GX_EVENT_PEN_DOWN;
    event.gx_event_payload.gx_event_pointdata.gx_point_x = x;
    event.gx_event_payload.gx_event_pointdata.gx_point_y = y;
    event.gx_event_sender                                = 0;
    event.gx_event_target                                = 0;
    event.gx_event_display_handle                        = 0;
    gx_system_event_send(&event);

    last_pos_x = x;
    last_pos_y = y;
}

static VOID gx_send_pen_up_event(VOID)
{
    GX_EVENT event;

    event.gx_event_type                                  = GX_EVENT_PEN_UP;
    event.gx_event_payload.gx_event_pointdata.gx_point_x = last_pos_x;
    event.gx_event_payload.gx_event_pointdata.gx_point_y = last_pos_y;
    event.gx_event_sender                                = 0;
    event.gx_event_target                                = 0;
    event.gx_event_display_handle                        = 0;
    gx_system_event_send(&event);
}

static VOID gx_send_pen_drag_event(GX_VALUE x, GX_VALUE y)
{
    GX_EVENT event;

    event.gx_event_type                                  = GX_EVENT_PEN_DRAG;
    event.gx_event_payload.gx_event_pointdata.gx_point_x = x;
    event.gx_event_payload.gx_event_pointdata.gx_point_y = y;
    event.gx_event_sender                                = 0;
    event.gx_event_target                                = 0;
    event.gx_event_display_handle                        = 0;
    gx_system_event_fold(&event);

    last_pos_x = x;
    last_pos_y = y;
}

static VOID touch_thread_entry(ULONG thread_input)
{
    static int touch_state = TOUCH_STATE_RELEASED;
    status_t status;
    int x, y;
    int x_delta, y_delta;

    (void)thread_input;

    while (1)
    {
        tx_thread_sleep(5);
        status = DEMO_ReadTouch(&s_touchHandle, &x, &y);
        if (status == kStatus_Success)
        {
            if (touch_state == TOUCH_STATE_RELEASED)
            {
                touch_state = TOUCH_STATE_TOUCHED;
                gx_send_pen_down_event(x, y);
            }
            else
            {
                x_delta = abs(x - last_pos_x);
                y_delta = abs(y - last_pos_y);
                if (x_delta > MIN_DRAG_DELTA || y_delta > MIN_DRAG_DELTA)
                {
                    gx_send_pen_drag_event(x, y);
                }
            }
        }
        else
        {
            if (touch_state == TOUCH_STATE_TOUCHED)
            {
                touch_state = TOUCH_STATE_RELEASED;
                gx_send_pen_up_event();
            }
        }
    }
}

void start_touch_thread(void)
{
    /* Create the touch driver thread.  */
    tx_thread_create(&touch_thread, "GUIX Touch Thread", touch_thread_entry, 0,
                     touch_thread_stack, sizeof(touch_thread_stack),
                     GX_SYSTEM_THREAD_PRIORITY - 1,GX_SYSTEM_THREAD_PRIORITY - 1,
                     TX_NO_TIME_SLICE, TX_AUTO_START);
}
