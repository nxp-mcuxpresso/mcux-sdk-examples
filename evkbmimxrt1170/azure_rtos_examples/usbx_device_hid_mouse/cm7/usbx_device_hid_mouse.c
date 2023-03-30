/* This example illustrates USBX Device HID Mouse */

#include "ux_api.h"
#include "ux_device_class_hid.h"

#include "fsl_debug_console.h"
#include "board_setup.h"

#define HID_NUM_LOCK_MASK  1
#define HID_CAPS_LOCK_MASK 2

#define UX_DEMO_STACK_SIZE      (1024 * 4)

#ifndef USBX_MEMORY_SIZE
#define USBX_MEMORY_SIZE         (1024 * 32)
#endif

/* Define global data structures.  */
TX_THREAD demo_thread;
ULONG num_lock_flag  = UX_FALSE;
ULONG caps_lock_flag = UX_FALSE;

static UX_SLAVE_CLASS_HID_PARAMETER hid_parameter;

AT_NONCACHEABLE_SECTION_ALIGN(static ULONG usb_memory[USBX_MEMORY_SIZE / sizeof(ULONG)], 64);

static ULONG demo_thread_stack[UX_DEMO_STACK_SIZE / sizeof(ULONG)];

#define DEVICE_FRAMEWORK_LENGTH_FULL_SPEED (sizeof(device_framework_full_speed))
UCHAR device_framework_full_speed[] = {
    /* Device descriptor */
    0x12, 0x01, 0x10, 0x01, 0x00, 0x00, 0x00, 0x40,
    0xc9, 0x1f, 0xb7, 0x00, 0x01, 0x00, 0x01, 0x02,
    0x00, 0x01,

    /* Configuration descriptor */
    0x09, 0x02, 0x22, 0x00, 0x01, 0x01, 0x00, 0xc0,
    0x32,

    /* Interface descriptor */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01, 0x02,
    0x00,

    /* HID descriptor */
    0x09, 0x21, 0x10, 0x01, 0x21, 0x01, 0x22, 0x34,
    0x00,

    /* Endpoint descriptor (Interrupt) */
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x06
};

#define DEVICE_FRAMEWORK_LENGTH_HIGH_SPEED (sizeof(device_framework_high_speed))
UCHAR device_framework_high_speed[] = {
    /* Device descriptor */
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40,
    0xc9, 0x1f, 0xb7, 0x00, 0x01, 0x01, 0x01, 0x02,
    0x00, 0x01,

    /* Device qualifier descriptor */
    0x0a, 0x06, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40,
    0x01, 0x00,

    /* Configuration descriptor */
    0x09, 0x02, 0x22, 0x00, 0x01, 0x01, 0x00, 0xc0,
    0x32,

    /* Interface descriptor */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01, 0x02,
    0x00,

    /* HID descriptor */
    0x09, 0x21, 0x10, 0x01, 0x00, 0x01, 0x22, 0x34,
    0x00,

    /* Endpoint descriptor (Interrupt) */
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x06
};

/* String Device Framework :
 Byte 0 and 1 : Word containing the language ID : 0x0904 for US
 Byte 2       : Byte containing the index of the descriptor
 Byte 3       : Byte containing the length of the descriptor string
*/

#define STRING_FRAMEWORK_LENGTH (sizeof(string_framework))
UCHAR string_framework[] = {
    /* Manufacturer string descriptor : Index 1 */
    0x09, 0x04, 0x01, 18U,
    'N', 'X', 'P', ' ', 'S', 'E', 'M', 'I', 'C', 'O', 'N', 'D', 'U', 'C', 'T', 'O', 'R', 'S',

    /* Product string descriptor : Index 2 */
    0x09, 0x04, 0x02, 16U,
    'H', 'I', 'D', ' ', 'M', 'O', 'U', 'S', 'E', ' ', 'D', 'E', 'V', 'I', 'C', 'E',

    /* Serial Number string descriptor : Index 3 */
    0x09, 0x04, 0x03, 0x04, 0x30, 0x30, 0x30, 0x31
};

/* Multiple languages are supported on the device, to add
   a language besides english, the unicode language code must
   be appended to the language_id_framework array and the length
   adjusted accordingly. */
#define LANGUAGE_ID_FRAMEWORK_LENGTH (sizeof(language_id_framework))
UCHAR language_id_framework[] = {
    /* English. */
    0x09, 0x04
};

#define HID_MOUSE_REPORT_LENGTH 52
UCHAR hid_mouse_report[] = {
    0x05U, 0x01U, /* Usage Page (Generic Desktop)*/
    0x09U, 0x02U, /* Usage (Mouse) */
    0xA1U, 0x01U, /* Collection (Application) */
    0x09U, 0x01U, /* Usage (Pointer) */

    0xA1U, 0x00U, /* Collection (Physical) */
    0x05U, 0x09U, /* Usage Page (Buttons) */
    0x19U, 0x01U, /* Usage Minimum (01U) */
    0x29U, 0x03U, /* Usage Maximum (03U) */

    0x15U, 0x00U, /* Logical Minimum (0U) */
    0x25U, 0x01U, /* Logical Maximum (1U) */
    0x95U, 0x03U, /* Report Count (3U) */
    0x75U, 0x01U, /* Report Size (1U) */

    0x81U, 0x02U, /* Input(Data, Variable, Absolute) 3U button bit fields */
    0x95U, 0x01U, /* Report Count (1U) */
    0x75U, 0x05U, /* Report Size (5U) */
    0x81U, 0x01U, /* Input (Constant), 5U constant field */

    0x05U, 0x01U, /* Usage Page (Generic Desktop) */
    0x09U, 0x30U, /* Usage (X) */
    0x09U, 0x31U, /* Usage (Y) */
    0x09U, 0x38U, /* Usage (Z) */

    0x15U, 0x81U, /* Logical Minimum (-127) */
    0x25U, 0x7FU, /* Logical Maximum (127) */
    0x75U, 0x08U, /* Report Size (8U) */
    0x95U, 0x03U, /* Report Count (3U) */

    0x81U, 0x06U, /* Input(Data, Variable, Relative), Three position bytes (X & Y & Z)*/
    0xC0U, 0xC0U  /* End collection, Close Pointer collection*/
};

static UINT demo_thread_hid_callback(UX_SLAVE_CLASS_HID *hid, UX_SLAVE_CLASS_HID_EVENT *hid_event)
{
    PRINTF("demo_thread_hid_callback %x %x %x \r\n",
        hid_event->ux_device_class_hid_event_buffer[0],
        hid_event->ux_device_class_hid_event_buffer[1],
        hid_event->ux_device_class_hid_event_buffer[2]);
    return UX_SUCCESS;
}

static VOID demo_hid_instance_activate(VOID *hid_instance)
{
    PRINTF("HID device activate\r\n");
}

static void demo_thread_entry(ULONG arg)
{
    UX_SLAVE_DEVICE *device;
    UX_SLAVE_INTERFACE *interface;
    UX_SLAVE_CLASS_HID *hid;
    UX_SLAVE_CLASS_HID_EVENT hid_event;

    UCHAR x = 0U;
    UCHAR y = 0U;
    enum
    {
        RIGHT,
        DOWN,
        LEFT,
        UP
    };
    UCHAR dir = RIGHT;
    UINT ret;

    /* Get the pointer to the device.  */
    device = &_ux_system_slave->ux_system_slave_device;

    /* reset the HID event structure.  */
    ux_utility_memory_set(&hid_event, 0, sizeof(UX_SLAVE_CLASS_HID_EVENT));

    while (1)
    {
        /* Is the device configured ? */
        while (device->ux_slave_device_state != UX_DEVICE_CONFIGURED) {
            /* Then wait.  */
            tx_thread_sleep(10);
        }

        /* Until the device stays configured.  */
        while (device->ux_slave_device_state == UX_DEVICE_CONFIGURED)
        {
            /* Get the interface.  We use the first interface, this is a simple device.  */
            interface = device->ux_slave_device_first_interface;

            /* Length is fixed to 4. */
            hid_event.ux_device_class_hid_event_length = 4;

            /* First byte is a modifier byte.  */
            hid_event.ux_device_class_hid_event_buffer[0] = 0;

            switch (dir)
            {
                case RIGHT:
                    /* Move right. Increase X value. */
                    hid_event.ux_device_class_hid_event_buffer[1] = 2U;
                    hid_event.ux_device_class_hid_event_buffer[2] = 0U;
                    x++;
                    if (x > 199U)
                    {
                        dir = DOWN;
                    }
                    break;
                case DOWN:
                    /* Move down. Increase Y value. */
                    hid_event.ux_device_class_hid_event_buffer[1] = 0U;
                    hid_event.ux_device_class_hid_event_buffer[2] = 2U;
                    y++;
                    if (y > 199U)
                    {
                        dir = LEFT;
                    }
                    break;
                case LEFT:
                    /* Move left. Discrease X value. */
                    hid_event.ux_device_class_hid_event_buffer[1] = (UCHAR)(-2);
                    hid_event.ux_device_class_hid_event_buffer[2] = 0U;
                    x--;
                    if (x < 2U)
                    {
                        dir = UP;
                    }
                    break;
                case UP:
                    /* Move up. Discrease Y value. */
                    hid_event.ux_device_class_hid_event_buffer[1] = 0U;
                    hid_event.ux_device_class_hid_event_buffer[2] = (UCHAR)(-2);
                    y--;
                    if (y < 2U)
                    {
                        dir = RIGHT;
                    }
                    break;
                default:
                    break;
            }

            /* From that interface, derive the HID owner. */
            hid = interface->ux_slave_interface_class_instance;

            /* When the event buffer is full, add some delay. */
            do {
                /* Set the mouse event.  */
                ret = ux_device_class_hid_event_set(hid, &hid_event);
                if (ret == UX_SUCCESS)
                {
                    break;
                }

                tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 100);
            } while (1);
        }
    }
}

int main(void)
{
    /* Initialize the board. */
    board_setup();

    usb_device_hw_setup();

PRINTF("USBX device HID Mouse example\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}

void tx_application_define(void *first_unused_memory)
{
    UINT status;

    /* Initialize USBX Memory */
    status = ux_system_initialize((VOID *)usb_memory, USBX_MEMORY_SIZE, UX_NULL, 0); // usb_memory
    if (status != UX_SUCCESS)
        goto err;

    usb_device_setup();

   /* The code below is required for installing the device portion of USBX. No call back for
       device status change in this example. */
    status = ux_device_stack_initialize(device_framework_high_speed, DEVICE_FRAMEWORK_LENGTH_HIGH_SPEED,
                                        device_framework_full_speed, DEVICE_FRAMEWORK_LENGTH_FULL_SPEED,
                                        string_framework, STRING_FRAMEWORK_LENGTH, language_id_framework,
                                        LANGUAGE_ID_FRAMEWORK_LENGTH, UX_NULL);
    if (status != UX_SUCCESS)
        goto err;

    /* Initialize the hid class parameters for a mouse.  */
    hid_parameter.ux_device_class_hid_parameter_report_address = hid_mouse_report;
    hid_parameter.ux_device_class_hid_parameter_report_length  = HID_MOUSE_REPORT_LENGTH;
    hid_parameter.ux_device_class_hid_parameter_callback       = demo_thread_hid_callback;
    hid_parameter.ux_slave_class_hid_instance_activate         = demo_hid_instance_activate;

    /* Initialize the device hid class. The class is connected with interface 0 */
    status = ux_device_stack_class_register(_ux_system_slave_class_hid_name, ux_device_class_hid_entry, 1, 0,
                                            (VOID *)&hid_parameter);

    if (status != UX_SUCCESS)
        goto err;

    /* Create the main demo thread.  */
    tx_thread_create(&demo_thread, "USBX demo", demo_thread_entry, 0,
                     (VOID *)demo_thread_stack, UX_DEMO_STACK_SIZE,
                     20, 20, 1, TX_AUTO_START);

    return;

err:
    PRINTF("tx_application_define: ERROR 0x%x\r\n", status);
    while (1)
    {
    }
}
