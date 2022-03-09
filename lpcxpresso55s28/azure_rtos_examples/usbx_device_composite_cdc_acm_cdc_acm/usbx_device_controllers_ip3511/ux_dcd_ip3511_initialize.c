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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_ip3511_initialize                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the IP3511 USB device controller.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    controller_id                         Controller ID                 */
/*    handle_ptr                            Pointer to driver handle      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    USB_DeviceInit                        Initialize USB driver         */
/*    USB_DeviceRun                         Enable device                 */
/*    _ux_utility_memory_allocate           Allocate memory               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    USBX Device Stack                                                   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _ux_dcd_ip3511_initialize(ULONG controller_id, VOID** handle_ptr)
{

UX_SLAVE_DCD            *dcd;
UX_DCD_IP3511            *dcd_ip3511;
usb_status_t             status;


    /* Get the pointer to the DCD.  */
    dcd =  &_ux_system_slave -> ux_system_slave_dcd;

    /* The controller initialized here is of IP3511 type.  */
    dcd -> ux_slave_dcd_controller_type =  UX_DCD_IP3511_SLAVE_CONTROLLER;

    /* Allocate memory for this IP3511 DCD instance.  */
    dcd_ip3511 =  _ux_utility_memory_allocate(UX_NO_ALIGN, UX_REGULAR_MEMORY, sizeof(UX_DCD_IP3511));

    /* Check if memory was properly allocated.  */
    if(dcd_ip3511 == UX_NULL)
        return(UX_MEMORY_INSUFFICIENT);

    /* Set the pointer to the IP3511 DCD.  */
    dcd -> ux_slave_dcd_controller_hardware =  (VOID *) dcd_ip3511;

    /* Set the generic DCD owner for the IP3511 DCD.  */
    dcd_ip3511 -> ux_dcd_ip3511_dcd_owner =  dcd;

    /* Initialize the function collector for this DCD.  */
    dcd -> ux_slave_dcd_function =  _ux_dcd_ip3511_function;

    status = USB_DeviceInit((uint8_t)controller_id, _ux_dcd_ip3511_callback, &dcd_ip3511 -> handle);

    *handle_ptr =  dcd_ip3511 -> handle;

    USB_DeviceRun(dcd_ip3511 -> handle);

    /* Set the state of the controller to OPERATIONAL now.  */
    dcd -> ux_slave_dcd_status =  UX_DCD_STATUS_OPERATIONAL;

    /* Return completion status.  */
    return(status);
}

