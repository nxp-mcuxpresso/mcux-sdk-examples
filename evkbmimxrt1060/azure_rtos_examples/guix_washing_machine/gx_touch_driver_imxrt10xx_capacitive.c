/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** GUIX Component                                                        */
/**                                                                       */
/**   Touch Component    (Touch)                                          */
/**                                                                       */
/**************************************************************************/

#include "tx_api.h"
#include "gx_api.h"

/* Notes

This file contains the hardware-specific functions of the resistive touch
driver. The generic portions of the touch driver are provided by the file
gx_generic_resistive_touch

*/

#include "fsl_lpi2c.h"
#include "fsl_ft5406_rt.h"

#define BOARD_TOUCH_I2C LPI2C1

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)

/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)

#define BOARD_TOUCH_I2C_CLOCK_FREQ ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))
#define BOARD_TOUCH_I2C_BAUDRATE   100000U

/* Define the touch thread control block and stack.  */
TX_THREAD touch_thread;
UCHAR touch_thread_stack[4096];
VOID touch_thread_entry(ULONG thread_input);

#define TOUCH_STATE_TOUCHED  1
#define TOUCH_STATE_RELEASED 2
#define MIN_DRAG_DELTA       10

static int last_pos_x;
static int last_pos_y;
static int curpos_x;
static int curpos_y;
static int touch_state;

/**************************************************************************/
/* called by application to fire off the touch screen driver thread       */
VOID start_touch_thread(void)
{
    /* Create the touch driver thread.  */
    tx_thread_create(&touch_thread, "GUIX Touch Thread", touch_thread_entry, 0, touch_thread_stack,
                     sizeof(touch_thread_stack), GX_SYSTEM_THREAD_PRIORITY - 1, GX_SYSTEM_THREAD_PRIORITY - 1,
                     TX_NO_TIME_SLICE, TX_AUTO_START);
}

/*******************************************************************************
 * Implementation of communication with the touch controller
 ******************************************************************************/

static void gx_touch_init(void)
{
    lpi2c_master_config_t masterConfig = {0};
    LPI2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Hz = BOARD_TOUCH_I2C_BAUDRATE;

    /* Initialize the LPI2C master peripheral */
    LPI2C_MasterInit(BOARD_TOUCH_I2C, &masterConfig, BOARD_TOUCH_I2C_CLOCK_FREQ);

    /*Clock setting for LPI2C*/
    // CLOCK_SetMux(kCLOCK_Lpi2cMux, LPI2C_CLOCK_SOURCE_SELECT);
    // CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LPI2C_CLOCK_SOURCE_DIVIDER);
}

void gx_touch_deinit(void)
{
    LPI2C_MasterDeinit(BOARD_TOUCH_I2C);
}

/**************************************************************************/
VOID gx_send_pen_down_event(VOID)
{
    GX_EVENT event;
    event.gx_event_type                                  = GX_EVENT_PEN_DOWN;
    event.gx_event_payload.gx_event_pointdata.gx_point_x = curpos_x;
    event.gx_event_payload.gx_event_pointdata.gx_point_y = curpos_y;
    event.gx_event_sender                                = 0;
    event.gx_event_target                                = 0;
    event.gx_event_display_handle                        = 0;
    gx_system_event_send(&event);
}

/**************************************************************************/
VOID gx_send_pen_drag_event(VOID)
{
    GX_EVENT event;
    int x_delta = abs(curpos_x - last_pos_x);
    int y_delta = abs(curpos_y - last_pos_y);

    if (x_delta > MIN_DRAG_DELTA || y_delta > MIN_DRAG_DELTA)
    {
        event.gx_event_type                                  = GX_EVENT_PEN_DRAG;
        event.gx_event_payload.gx_event_pointdata.gx_point_x = curpos_x;
        event.gx_event_payload.gx_event_pointdata.gx_point_y = curpos_y;
        event.gx_event_sender                                = 0;
        event.gx_event_target                                = 0;
        event.gx_event_display_handle                        = 0;
        last_pos_x                                           = curpos_x;
        last_pos_y                                           = curpos_y;

        gx_system_event_fold(&event);
    }
}

/**************************************************************************/
VOID gx_send_pen_up_event(VOID)
{
    GX_EVENT event;
    event.gx_event_type                                  = GX_EVENT_PEN_UP;
    event.gx_event_payload.gx_event_pointdata.gx_point_x = curpos_x;
    event.gx_event_payload.gx_event_pointdata.gx_point_y = curpos_y;
    event.gx_event_sender                                = 0;
    event.gx_event_target                                = 0;
    event.gx_event_display_handle                        = 0;
    last_pos_x                                           = curpos_x;
    last_pos_y                                           = curpos_y;
    gx_system_event_send(&event);
}

/**************************************************************************/
VOID touch_thread_entry(ULONG thread_input)
{
    ft5406_rt_handle_t touch_handle;
    touch_event_t driver_state;
    status_t status;

    /* fow now run in polling mode */
    /*
    tx_event_flags_create(&touch_events, "touch_events");
    touch_interrupt_configure();
    */

    gx_touch_init();

    /* Initialize the touch handle. */
    FT5406_RT_Init(&touch_handle, BOARD_TOUCH_I2C);
    touch_state = TOUCH_STATE_RELEASED;

    tx_thread_sleep(30);

    while (1)
    {
        // tx_event_flags_get(&touch_events, 1, TX_AND_CLEAR, &actual_flags, TX_WAIT_FOREVER);
        tx_thread_sleep(2);
        status = FT5406_RT_GetSingleTouch(&touch_handle, &driver_state, &curpos_y, &curpos_x);

        if (status == kStatus_Success)
        {
            if ((driver_state == kTouch_Down) || (driver_state == kTouch_Contact))
            {
                // screen is touched, update coords:

                if (touch_state == TOUCH_STATE_RELEASED)
                {
                    touch_state = TOUCH_STATE_TOUCHED;
                    gx_send_pen_down_event();
                }
                else
                {
                    // test and send pen drag
                    gx_send_pen_drag_event();
                }
            }
            else
            {
                // no touch, check so see if last was touched
                if (touch_state == TOUCH_STATE_TOUCHED)
                {
                    touch_state = TOUCH_STATE_RELEASED;
                    gx_send_pen_up_event();
                }
            }
        }
    }
}
