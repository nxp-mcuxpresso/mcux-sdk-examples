/**************************************************************************/
/**                                                                       */
/** USBX Device CDC-ACM Example                                           */
/**                                                                       */
/** This demo will show the device side of USBX. It will make the device  */
/** appear as a serial port device to a host.                             */
/**                                                                       */
/**************************************************************************/

#include "ux_api.h"
#include "ux_device_class_cdc_acm.h"

#include "fsl_debug_console.h"
#include "board_setup.h"

#define UX_DEMO_STACK_SIZE      (1024 * 2)

#ifndef USBX_MEMORY_SIZE
#define USBX_MEMORY_SIZE         (1024 * 64)
#endif

UX_SLAVE_CLASS_CDC_ACM_PARAMETER parameter;
UX_SLAVE_CLASS_CDC_ACM *cdc;
TX_THREAD demo_thread;

AT_NONCACHEABLE_SECTION_ALIGN(static ULONG usb_memory[USBX_MEMORY_SIZE / sizeof(ULONG)], 64);

static ULONG demo_thread_stack[UX_DEMO_STACK_SIZE / sizeof(ULONG)];

#define DEVICE_FRAMEWORK_LENGTH_FULL_SPEED      (sizeof(device_framework_full_speed))
unsigned char device_framework_full_speed[] = {

    /* Device descriptor     18 bytes
       0x02 bDeviceClass:    CDC class code
       0x00 bDeviceSubclass: CDC class sub code
       0x00 bDeviceProtocol: CDC Device protocol

       idVendor & idProduct - http://www.linux-usb.org/usb.ids
    */
    0x12, 0x01, 0x10, 0x01, 0xEF, 0x02, 0x01, 0x40, 0xC9, 0x1F,
    0xB4, 0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x01,

    /* Configuration 1 descriptor 9 bytes */
    0x09, 0x02, 0x4b, 0x00, 0x02, 0x01, 0x00, 0x40, 0x00,

    /* Interface association descriptor. 8 bytes.  */
    0x08, 0x0b, 0x00, 0x02, 0x02, 0x02, 0x00, 0x00,

    /* Communication Class Interface Descriptor Requirement. 9 bytes.   */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,

    /* Header Functional Descriptor 5 bytes */
    0x05, 0x24, 0x00, 0x10, 0x01,

    /* ACM Functional Descriptor 4 bytes */
    0x04, 0x24, 0x02, 0x0f,

    /* Union Functional Descriptor 5 bytes */
    0x05, 0x24, 0x06, 0x00, 0x01,

    /* Call Management Functional Descriptor 5 bytes */
    0x05, 0x24, 0x01, 0x03, 0x01, /* Data interface   */

    /* Endpoint 1 descriptor 7 bytes */
    0x07, 0x05, 0x83, 0x03, 0x08, 0x00, 0x08,

    /* Data Class Interface Descriptor Requirement 9 bytes */
    0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,

    /* First alternate setting Endpoint 1 descriptor 7 bytes*/
    0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00,

    /* Endpoint 2 descriptor 7 bytes */
    0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00,
};

#define DEVICE_FRAMEWORK_LENGTH_HIGH_SPEED      (sizeof(device_framework_high_speed))
unsigned char device_framework_high_speed[] = {

    /* Device descriptor
       0x02 bDeviceClass:    CDC class code
       0x00 bDeviceSubclass: CDC class sub code
       0x00 bDeviceProtocol: CDC Device protocol

       idVendor & idProduct - http://www.linux-usb.org/usb.ids
    */
    0x12, 0x01, 0x00, 0x02, 0xEF, 0x02, 0x01, 0x40, 0xC9, 0x1F,
    0xB4, 0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x01,

    /* Device qualifier descriptor */
    0x0a, 0x06, 0x00, 0x02, 0x02, 0x00, 0x00, 0x40, 0x01, 0x00,

    /* Configuration 1 descriptor */
    0x09, 0x02, 0x4b, 0x00, 0x02, 0x01, 0x00, 0x40, 0x00,

    /* Interface association descriptor. */
    0x08, 0x0b, 0x00, 0x02, 0x02, 0x02, 0x00, 0x00,

    /* Communication Class Interface Descriptor Requirement */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,

    /* Header Functional Descriptor */
    0x05, 0x24, 0x00, 0x10, 0x01,

    /* ACM Functional Descriptor */
    0x04, 0x24, 0x02, 0x0f,

    /* Union Functional Descriptor */
    0x05, 0x24, 0x06, 0x00, 0x01,

    /* Call Management Functional Descriptor */
    0x05, 0x24, 0x01, 0x00, 0x01,

    /* Endpoint 1 descriptor */
    0x07, 0x05, 0x83, 0x03, 0x08, 0x00, 0x08,

    /* Data Class Interface Descriptor Requirement */
    0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,

    /* First alternate setting Endpoint 1 descriptor */
    0x07, 0x05, 0x02, 0x02, 0x00, 0x02, 0x00,

    /* Endpoint 2 descriptor */
    0x07, 0x05, 0x81, 0x02, 0x00, 0x02, 0x00,
};

#define STRING_FRAMEWORK_LENGTH             (sizeof(string_framework))
unsigned char string_framework[] = {
    /* Manufacturer string descriptor : Index 1 */
    0x09, 0x04, 0x01, 18U,
    'N', 'X', 'P', ' ',
    'S', 'E', 'M', 'I', 'C', 'O', 'N', 'D', 'U', 'C', 'T', 'O', 'R', 'S',

    /* Product string descriptor : Index 2 */
    0x09, 0x04, 0x02, 16U,
    'U', 'S', 'B', ' ',
    'C', 'D', 'C', ' ', 'A', 'C', 'M', ' ', 'D', 'E', 'M', 'O',

    /* Serial Number string descriptor : Index 3 */
    0x09, 0x04, 0x03, 0x04, 0x30, 0x30, 0x30, 0x31
};


/*
 * Multiple languages are supported on the device, to add
 * a language besides english, the unicode language code must
 * be appended to the language_id_framework array and the length
 * adjusted accordingly.
 */
#define LANGUAGE_ID_FRAMEWORK_LENGTH        (sizeof(language_id_framework))
unsigned char language_id_framework[] = {

    /* English. */
    0x09, 0x04
};

static VOID demo_cdc_instance_activate(VOID *cdc_instance)
{
    PRINTF("CDC device activate\r\n");

    /* Save the CDC instance.  */
    cdc = (UX_SLAVE_CLASS_CDC_ACM *)cdc_instance;
}

static VOID demo_cdc_instance_deactivate(VOID *cdc_instance)
{
    /* Reset the CDC instance.  */
    cdc = UX_NULL;
}

static void demo_thread_entry(ULONG arg)
{
    ULONG status;
    ULONG actual_length;
    ULONG requested_length;
    UCHAR buffer[256];

    while (1)
    {
        /* Ensure the CDC class is mounted.  */
        while (cdc != UX_NULL)
        {
            /* Read from the CDC class.  */
            status = ux_device_class_cdc_acm_read(cdc, buffer, sizeof(buffer), &actual_length);

            if (status != UX_SUCCESS)
                break;

            PRINTF("SEND: ");

            /* The actual length becomes the requested length.  */
            requested_length = actual_length;

            /* Check the status.  If OK, we will write to the CDC instance.  */
            status = ux_device_class_cdc_acm_write(cdc, buffer, requested_length, &actual_length);
            if (status != UX_SUCCESS)
                break;

            buffer[actual_length] = 0;
            PRINTF("%s", buffer);

            /* Check for CR/LF.  */
            if (buffer[requested_length - 1] == '\r')
            {
                /* Copy LF value into user buffer.  */
                ux_utility_memory_copy(buffer, "\n", 1);

                /* And send it again.  */
                status = ux_device_class_cdc_acm_write(cdc, buffer, 1, &actual_length);

                if (status != UX_SUCCESS)
                    break;

                buffer[actual_length] = 0;
                PRINTF("%s", buffer);
            }

            /* Fill buffer.  */
            buffer[0] = 'a';
            buffer[1] = 'b';
            buffer[2] = 'c';
            buffer[3] = 'd';
            buffer[4] = 'e';
            buffer[5] = 'f';
            buffer[6] = '\r';
            buffer[7] = '\n';

            /* And send 8 bytes.  */
            status = ux_device_class_cdc_acm_write(cdc, buffer, 8, &actual_length);

            if (status != UX_SUCCESS)
                break;

            buffer[actual_length] = 0;
            PRINTF("%s", buffer);

            /* And send 0 byte packet. Forced ZLP.  */
            status = ux_device_class_cdc_acm_write(cdc, buffer, 0, &actual_length);

            if (status != UX_SUCCESS)
                break;

            PRINTF("\r\n");
        }
    }
}

int main(void)
{
    /* Initialize the board. */
    board_setup();

    usb_device_hw_setup();

    PRINTF("Start USBX device CDC ACM example...\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}

void tx_application_define(void *first_unused_memory)
{
    UINT status;

    /* Initialize USBX Memory */
    ux_system_initialize((VOID *)usb_memory, USBX_MEMORY_SIZE, UX_NULL, 0);

    usb_device_setup();

    /* The code below is required for installing the device portion of USBX. No call back for
       device status change in this example. */
    status = ux_device_stack_initialize(device_framework_high_speed, DEVICE_FRAMEWORK_LENGTH_HIGH_SPEED,
                                        device_framework_full_speed, DEVICE_FRAMEWORK_LENGTH_FULL_SPEED,
                                        string_framework, STRING_FRAMEWORK_LENGTH, language_id_framework,
                                        LANGUAGE_ID_FRAMEWORK_LENGTH, UX_NULL);
    if (status != UX_SUCCESS)
        return;

    /* Set the parameters for callback when insertion/extraction of a CDC device.  */
    parameter.ux_slave_class_cdc_acm_instance_activate   = demo_cdc_instance_activate;
    parameter.ux_slave_class_cdc_acm_instance_deactivate = demo_cdc_instance_deactivate;

    /* Initialize the device CDC class. This class owns both interfaces starting with 0. */
    status = ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name, ux_device_class_cdc_acm_entry, 1, 0,
                                            &parameter);

    if (status != UX_SUCCESS)
        return;

    /* Create the main demo thread.  */
    tx_thread_create(&demo_thread, "USBX demo", demo_thread_entry, 0, demo_thread_stack, UX_DEMO_STACK_SIZE, 20, 20, 1,
                     TX_AUTO_START);

    return;
}
