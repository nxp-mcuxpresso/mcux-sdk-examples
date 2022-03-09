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
/*    _ux_dcd_ip3511_uninitialize                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function uninitializes the IP3511 USB device controller        */
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
/*    USB_DeviceStop                        Disable device                */
/*    USB_DeviceDeinit                      Uninitialize device           */
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
UINT  _ux_dcd_ip3511_uninitialize(ULONG controller_id, VOID** handle_ptr)
{

UX_SLAVE_DCD            *dcd;
UX_DCD_IP3511            *dcd_ip3511;


    UX_PARAMETER_NOT_USED(controller_id);

    /* Get the pointer to the DCD.  */
    dcd =  &_ux_system_slave -> ux_system_slave_dcd;

    /* Set the state of the controller to HALTED now.  */
    dcd -> ux_slave_dcd_status =  UX_DCD_STATUS_HALTED;

    /* Get controller driver.  */
    dcd_ip3511 = (UX_DCD_IP3511 *)dcd -> ux_slave_dcd_controller_hardware;

    /* Check parameter.  */
    if (dcd_ip3511 -> handle == *handle_ptr)
    {

        /* Stop the device.  */
        USB_DeviceStop(dcd_ip3511 -> handle);

        /* Uninitialize the device.  */
        USB_DeviceDeinit(dcd_ip3511 -> handle);

        _ux_utility_memory_free(dcd_ip3511);
        dcd -> ux_slave_dcd_controller_hardware = UX_NULL;
        return(UX_SUCCESS);
    }

    /* Parameter not correct.  */
    return(UX_ERROR);
}
