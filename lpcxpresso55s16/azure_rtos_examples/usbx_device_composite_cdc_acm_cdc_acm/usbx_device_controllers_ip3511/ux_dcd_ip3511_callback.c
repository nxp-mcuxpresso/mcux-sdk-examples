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
/** USBX Component                                                        */ 
/**                                                                       */
/**   IP3511 Controller Driver                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE
#define UX_DCD_IP3511_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_dcd_ip3511.h"
#include "ux_device_stack.h"
#include "ux_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_ip3511_callback                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles callback from the USB driver.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    handle                                Pointer to device handle      */
/*    event                                 Event for callback            */
/*    param                                 Parameter for callback        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_device_stack_control_request_process                            */ 
/*                                          Process control request       */ 
/*    USB_DeviceGetStatus                   Get status                    */
/*    _ux_dcd_ip3511_initialize_complete    Complete initialization       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    IP3511 Driver                                                       */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
usb_status_t _ux_dcd_ip3511_callback(usb_device_handle handle, uint32_t event, void *param)
{

usb_status_t status;
uint8_t device_speed;


    if ((uint32_t)kUSB_DeviceEventBusReset == event)
    {

        /* If the device is attached or configured, we need to disconnect it.  */
        if (_ux_system_slave -> ux_system_slave_device.ux_slave_device_state !=  UX_DEVICE_RESET)
        {

            /* Disconnect the device.  */
            _ux_device_stack_disconnect();
        }

        status = USB_DeviceGetStatus(handle, kUSB_DeviceStatusSpeed, &device_speed);

        if (status != kStatus_USB_Success)
        {
            return status;
        }

        /* Set USB Current Speed */
        if (device_speed == USB_SPEED_HIGH)
        {

            /* We are connected at high speed.  */
            _ux_system_slave -> ux_system_slave_speed =  UX_HIGH_SPEED_DEVICE;
        }
        else
        {

            /* We are connected at full speed.  */
            _ux_system_slave -> ux_system_slave_speed =  UX_FULL_SPEED_DEVICE;
        }

        /* Complete the device initialization.  */
        _ux_dcd_ip3511_initialize_complete();

        /* Mark the device as attached now.  */
        _ux_system_slave -> ux_system_slave_device.ux_slave_device_state =  UX_DEVICE_ATTACHED;
    }

    return kStatus_USB_Success;
}


