
#include <ctype.h>

#include "ux_api.h"
#include "ux_device_class_hid.h"

#include "fsl_debug_console.h"
#include "board_setup.h"

#ifndef USBX_MEMORY_SIZE
#define USBX_MEMORY_SIZE         (1024 * 32)
#endif

static UX_SLAVE_CLASS_HID_PARAMETER hid_parameter;

AT_NONCACHEABLE_SECTION_ALIGN(static ULONG usb_memory[USBX_MEMORY_SIZE / sizeof(ULONG)], 64);

#define DEVICE_FRAMEWORK_LENGTH_FULL_SPEED (sizeof(device_framework_full_speed))
UCHAR device_framework_full_speed[] = {
    /* Device descriptor */
    0x12, 0x01, 0x10, 0x01, 0x00, 0x00, 0x00, 0x40,
    0xc9, 0x1f, 0xa2, 0x00, 0x01, 0x00, 0x01, 0x02,
    0x03, 0x01,

    /* Configuration descriptor */
    0x09, 0x02, 0x29, 0x00, 0x01, 0x01, 0x00, 0xc0,
    0x32,

    /* Interface descriptor */
    0x09, 0x04, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00,
    0x00,

    /* HID descriptor */
    0x09, 0x21, 0x00, 0x01, 0x00, 0x01, 0x22, 0x21,
    0x00,

    /* Endpoint descriptor (Interrupt EP 1 IN) */
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0a,

    /* Endpoint descriptor (Interrupt EP 2 OUT) */
    0x07, 0x05, 0x02, 0x03, 0x08, 0x00, 0x0a
};

#define DEVICE_FRAMEWORK_LENGTH_HIGH_SPEED (sizeof(device_framework_high_speed))
UCHAR device_framework_high_speed[] = {
    /* Device descriptor */
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40,
    0xc9, 0x1f, 0xa2, 0x00, 0x01, 0x01, 0x01, 0x02,
    0x03, 0x01,

    /* Device qualifier descriptor */
    0x0a, 0x06, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40,
    0x01, 0x00,

    /* Configuration descriptor */
    0x09, 0x02, 0x29, 0x00, 0x01, 0x01, 0x00, 0xc0,
    0x32,

    /* Interface descriptor */
    0x09, 0x04, 0x00, 0x00, 0x02, 0x03, 0x00, 0x00,
    0x00,

    /* HID descriptor */
    0x09, 0x21, 0x00, 0x01, 0x00, 0x01, 0x22, 0x21,
    0x00,

    /* Endpoint descriptor (Interrupt EP 1 IN) */
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0a,

    /* Endpoint descriptor (Interrupt EP 2 OUT) */
    0x07, 0x05, 0x02, 0x03, 0x08, 0x00, 0x0a
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
    0x09, 0x04, 0x02, 18U,
    'G', 'E', 'N', 'E', 'R', 'I', 'C', ' ', 'H', 'I', 'D', ' ', 'D', 'E', 'V', 'I', 'C', 'E',

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

#define HID_GENERIC_REPORT_LENGTH   (sizeof(hid_generic_report))    // 33 bytes
UCHAR hid_generic_report[] = {
    0x05U, 0x81U, /* Usage Page (Vendor defined)*/
    0x09U, 0x82U, /* Usage (Vendor defined) */
    0xA1U, 0x01U, /* Collection (Application) */
    0x09U, 0x83U, /* Usage (Vendor defined) */

    0x09U, 0x84U, /* Usage (Vendor defined) */
    0x15U, 0x80U, /* Logical Minimum (-128) */
    0x25U, 0x7FU, /* Logical Maximum (127) */
    0x75U, 0x08U, /* Report Size (8U) */
    0x95U, 0x08U, /* Report Count (8U) */
    0x81U, 0x02U, /* Input(Data, Variable, Absolute) */

    0x09U, 0x84U, /* Usage (Vendor defined) */
    0x15U, 0x80U, /* Logical Minimum (-128) */
    0x25U, 0x7FU, /* Logical Maximum (127) */
    0x75U, 0x08U, /* Report Size (8U) */
    0x95U, 0x08U, /* Report Count (8U) */
    0x91U, 0x02U, /* Output(Data, Variable, Absolute) */

    0xC0U         /* End collection */
};

static VOID demo_print_data(UCHAR *data, ULONG length)
{
    uint8_t byte;

    for (int i = 0; i < length; i++) {

        if (i != 0 && i % 8 == 0) {
            PRINTF("\r\n");
        }
        byte = *data++;
        PRINTF("0x%2x(%c) ", byte, isprint(byte) ? byte : 0x20);
    }
}

static VOID demo_hid_send_event(UX_SLAVE_CLASS_HID *hid, UCHAR *data, ULONG length)
{
    UX_SLAVE_CLASS_HID_EVENT hid_event;
    ULONG left = length;
    ULONG size;
    UINT ret;

    while (left > 0) {
        ux_utility_memory_set(&hid_event, 0, sizeof(hid_event));

        size = UX_MIN(left, UX_DEVICE_CLASS_HID_EVENT_BUFFER_LENGTH);

        hid_event.ux_device_class_hid_event_length = size;
        memcpy(hid_event.ux_device_class_hid_event_buffer, data, size);

        /* When the event buffer is full, retry after a delay. */
        do {
            ret = ux_device_class_hid_event_set(hid, &hid_event);
            if (ret == UX_SUCCESS)
            {
                break;
            }

            tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 100);
        } while (1);

        left -= size;
        data += size;
    }
}

static VOID demo_hid_receiver_event_callback(struct UX_SLAVE_CLASS_HID_STRUCT *hid)
{
    UX_DEVICE_CLASS_HID_RECEIVED_EVENT event;
    ULONG length;
    UCHAR *data;
    UINT ret;

    while (1) {
        ret = ux_device_class_hid_receiver_event_get(hid, &event);
        if (ret == UX_ERROR) {
            return;
        }

        length = event.ux_device_class_hid_received_event_length;
        data = event.ux_device_class_hid_received_event_data;

        /* Send the received data back. */
        demo_hid_send_event(hid, data, length);

        PRINTF("\r\nREV:\r\n");
        demo_print_data(data, length);
        PRINTF("\r\n");

        ux_device_class_hid_receiver_event_free(hid);
    }
}

static VOID demo_hid_instance_activate(VOID *hid_instance)
{
    PRINTF("HID device activate\r\n");
}

int main(void)
{
    /* Initialize the board. */
    board_setup();

    usb_device_hw_setup();

    PRINTF("USBX device HID generic example\r\n");

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
    hid_parameter.ux_device_class_hid_parameter_report_address = hid_generic_report;
    hid_parameter.ux_device_class_hid_parameter_report_length  = HID_GENERIC_REPORT_LENGTH;

    hid_parameter.ux_device_class_hid_parameter_receiver_initialize = ux_device_class_hid_receiver_initialize;
    hid_parameter.ux_device_class_hid_parameter_receiver_event_max_number = 16;
    hid_parameter.ux_device_class_hid_parameter_receiver_event_max_length = 64;
    hid_parameter.ux_device_class_hid_parameter_receiver_event_callback = demo_hid_receiver_event_callback;
    hid_parameter.ux_slave_class_hid_instance_activate = demo_hid_instance_activate;

    /* Initialize the device hid class. The class is connected with interface 0 */
    status = ux_device_stack_class_register(_ux_system_slave_class_hid_name, ux_device_class_hid_entry, 1, 0,
                                            (VOID *)&hid_parameter);

    if (status != UX_SUCCESS)
        goto err;

    return;

err:
    PRINTF("tx_application_define: ERROR 0x%x\r\n", status);
    while (1)
    {
    }
}
