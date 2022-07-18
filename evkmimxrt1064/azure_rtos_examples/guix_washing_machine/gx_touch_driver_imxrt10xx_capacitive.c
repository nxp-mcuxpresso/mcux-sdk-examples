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

/* Notes

This file contains the hardware-specific functions of the resistive touch
driver. The generic portions of the touch driver are provided by the file
gx_generic_resistive_touch

*/

#include "fsl_lpi2c.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "display_support.h"

#if defined(DEMO_PANEL) && (DEMO_PANEL == DEMO_PANEL_RK043FN66HS)
#include "fsl_gt911.h"
#else
#include "fsl_ft5406_rt.h"
#endif

#include "tx_api.h"
#include "gx_api.h"

#define BOARD_TOUCH_I2C LPI2C1

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)

/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)

#define BOARD_TOUCH_I2C_CLOCK_FREQ ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

#define BOARD_TOUCH_I2C_BAUDRATE    100000U

#define TOUCH_STATE_TOUCHED         1
#define TOUCH_STATE_RELEASED        2
#define MIN_DRAG_DELTA              10

#define DEMO_STACK_SIZE             (4 * 1024)

/* Define the touch thread control block and stack.  */
static TX_THREAD touch_thread;
static ULONG touch_thread_stack[DEMO_STACK_SIZE / sizeof(ULONG)];

static int last_pos_x;
static int last_pos_y;
static int curpos_x;
static int curpos_y;

#if defined(DEMO_PANEL) && (DEMO_PANEL == DEMO_PANEL_RK043FN66HS)

static void gx_delay(uint32_t ms);
static void BOARD_PullTouchResetPin(bool pullUp);
static void BOARD_ConfigTouchIntPin(gt911_int_pin_mode_t mode);

static gt911_handle_t s_touchHandle;

static const gt911_config_t s_touchConfig = {
    .I2C_SendFunc     = BOARD_Touch_I2C_Send,
    .I2C_ReceiveFunc  = BOARD_Touch_I2C_Receive,
    .pullResetPinFunc = BOARD_PullTouchResetPin,
    .intPinFunc       = BOARD_ConfigTouchIntPin,
    .timeDelayMsFunc  = gx_delay,
    .touchPointNum    = 1,
    .i2cAddrMode      = kGT911_I2cAddrAny,
    .intTrigMode      = kGT911_IntRisingEdge,
};
static int s_touchResolutionX;
static int s_touchResolutionY;
#else

static ft5406_rt_handle_t touch_handle;
static int touch_state;
#endif

static VOID touch_thread_entry(ULONG thread_input);

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

static VOID gx_send_pen_down_event(VOID)
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

static VOID gx_send_pen_drag_event(VOID)
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

static VOID gx_send_pen_up_event(VOID)
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

#if defined(DEMO_PANEL) && (DEMO_PANEL == DEMO_PANEL_RK043FN66HS)

static void gx_delay(uint32_t ms)
{
    ULONG ticks;

    /* translate ms into ticks. */
    ticks = (ULONG)(ms * TX_TIMER_TICKS_PER_SECOND) / 1000;

    if (ticks == 0)
    {
        while (0U != (ms--))
        {
            SDK_DelayAtLeastUs(1000U, SystemCoreClock);
        }
    }
    else
    {
        tx_thread_sleep(ticks);
    }
}

static void BOARD_PullTouchResetPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_TOUCH_RST_GPIO, BOARD_TOUCH_RST_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(BOARD_TOUCH_RST_GPIO, BOARD_TOUCH_RST_PIN, 0);
    }
}

static void BOARD_ConfigTouchIntPin(gt911_int_pin_mode_t mode)
{
    if (mode == kGT911_IntPinInput)
    {
        BOARD_TOUCH_INT_GPIO->GDIR &= ~(1UL << BOARD_TOUCH_INT_PIN);
    }
    else
    {
        if (mode == kGT911_IntPinPullDown)
        {
            GPIO_PinWrite(BOARD_TOUCH_INT_GPIO, BOARD_TOUCH_INT_PIN, 0);
        }
        else
        {
            GPIO_PinWrite(BOARD_TOUCH_INT_GPIO, BOARD_TOUCH_INT_PIN, 1);
        }

        BOARD_TOUCH_INT_GPIO->GDIR |= (1UL << BOARD_TOUCH_INT_PIN);
    }
}

static void gx_touch_init(void)
{
    status_t status;
    gpio_pin_config_t pinConfig = {
                    .direction = kGPIO_DigitalOutput,
                    .outputLogic = 0,
                    .interruptMode = kGPIO_NoIntmode
    };

    GPIO_PinInit(BOARD_TOUCH_INT_GPIO, BOARD_TOUCH_INT_PIN, &pinConfig);
    GPIO_PinInit(BOARD_TOUCH_RST_GPIO, BOARD_TOUCH_RST_PIN, &pinConfig);

    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LPI2C_CLOCK_SOURCE_DIVIDER);

    BOARD_LPI2C_Init(BOARD_TOUCH_I2C, BOARD_TOUCH_I2C_CLOCK_FREQ);

    status = GT911_Init(&s_touchHandle, &s_touchConfig);
    if (kStatus_Success != status)
    {
        PRINTF("Touch IC initialization failed\r\n");
        assert(false);
    }

    GT911_GetResolution(&s_touchHandle, &s_touchResolutionX, &s_touchResolutionY);

    tx_thread_sleep(30);
}

static VOID gx_touch_read(int *x, int *y, int *state)
{
    status_t status;
    static int touch_x = 0;
    static int touch_y = 0;

    status = GT911_GetSingleTouch(&s_touchHandle, &touch_x, &touch_y);
    if (status == kStatus_Success)
    {
        *state = TOUCH_STATE_TOUCHED;
    }
    else
    {
        *state = TOUCH_STATE_RELEASED;
    }

    *x = touch_x * DEMO_PANEL_WIDTH / s_touchResolutionX;
    *y = touch_y * DEMO_PANEL_HEIGHT / s_touchResolutionY;
}

static VOID touch_thread_entry(ULONG thread_input)
{
    int prev_touch_state;
    int cur_touch_state;

    gx_touch_init();

    prev_touch_state = TOUCH_STATE_RELEASED;

    while (1)
    {
        tx_thread_sleep(2);

        gx_touch_read(&curpos_x, &curpos_y, &cur_touch_state);

        if (prev_touch_state == TOUCH_STATE_TOUCHED)
        {
            if (cur_touch_state == TOUCH_STATE_TOUCHED)
            {
                gx_send_pen_drag_event();
            }
            else
            {
                gx_send_pen_up_event();
            }
        }
        else
        {
            if (cur_touch_state == TOUCH_STATE_TOUCHED)
            {
                gx_send_pen_down_event();
            }
        }

        prev_touch_state = cur_touch_state;
    }

}

#else

static void gx_touch_init(void)
{
    lpi2c_master_config_t masterConfig = {0};

    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LPI2C_CLOCK_SOURCE_DIVIDER);

    LPI2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Hz = BOARD_TOUCH_I2C_BAUDRATE;

    /* Initialize the LPI2C master peripheral */
    LPI2C_MasterInit(BOARD_TOUCH_I2C, &masterConfig, BOARD_TOUCH_I2C_CLOCK_FREQ);

    /* Initialize the touch handle. */
    FT5406_RT_Init(&touch_handle, BOARD_TOUCH_I2C);

    tx_thread_sleep(30);
}

static VOID touch_thread_entry(ULONG thread_input)
{
    touch_event_t driver_state;
    status_t status;

    gx_touch_init();

    touch_state = TOUCH_STATE_RELEASED;

    while (1)
    {
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
#endif
