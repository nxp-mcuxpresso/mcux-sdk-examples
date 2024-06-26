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
/*    _ux_dcd_ip3511_function                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function dispatches the DCD function internally to the IP3511  */
/*    controller.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd                                   Pointer to device controller  */
/*    function                              Function requested            */
/*    parameter                             Pointer to function parameters*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_dcd_ip3511_endpoint_create        Create endpoint               */
/*    _ux_dcd_ip3511_endpoint_destroy       Destroy endpoint              */
/*    _ux_dcd_ip3511_endpoint_reset         Reset endpoint                */
/*    _ux_dcd_ip3511_endpoint_stall         Stall endpoint                */
/*    _ux_dcd_ip3511_endpoint_status        Get endpoint status           */
/*    _ux_dcd_ip3511_frame_number_get       Get frame number              */
/*    _ux_dcd_ip3511_transfer_request       Request data transfer         */
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
UINT  _ux_dcd_ip3511_function(UX_SLAVE_DCD *dcd, UINT function, VOID *parameter)
{

UINT                status;
UX_DCD_IP3511       *dcd_ip3511;
uint32_t            address;


    /* Check the status of the controller.  */
    if (dcd -> ux_slave_dcd_status == UX_UNUSED)
    {

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_DCD, UX_CONTROLLER_UNKNOWN);

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_CONTROLLER_UNKNOWN, 0, 0, 0, UX_TRACE_ERRORS, 0, 0)

        return(UX_CONTROLLER_UNKNOWN);
    }

    /* Get the pointer to the IP3511 DCD.  */
    dcd_ip3511 =  (UX_DCD_IP3511 *) dcd -> ux_slave_dcd_controller_hardware;

    /* Look at the function and route it.  */
    switch(function)
    {

    case UX_DCD_GET_FRAME_NUMBER:

        status =  _ux_dcd_ip3511_frame_number_get(dcd_ip3511, (ULONG *) parameter);
        break;

    case UX_DCD_TRANSFER_REQUEST:

        status =  _ux_dcd_ip3511_transfer_request(dcd_ip3511, (UX_SLAVE_TRANSFER *) parameter);
        break;

    case UX_DCD_CREATE_ENDPOINT:

        status =  _ux_dcd_ip3511_endpoint_create(dcd_ip3511, parameter);
        break;

    case UX_DCD_DESTROY_ENDPOINT:

        status =  _ux_dcd_ip3511_endpoint_destroy(dcd_ip3511, parameter);
        break;

    case UX_DCD_RESET_ENDPOINT:

        status =  _ux_dcd_ip3511_endpoint_reset(dcd_ip3511, parameter);
        break;

    case UX_DCD_STALL_ENDPOINT:

        status =  _ux_dcd_ip3511_endpoint_stall(dcd_ip3511, parameter);
        break;

    case UX_DCD_SET_DEVICE_ADDRESS:

        address = (uint32_t)parameter;
        if (USB_DeviceSetStatus(dcd_ip3511 -> handle, kUSB_DeviceStatusAddress, (uint8_t*)&address) == kStatus_USB_Success)
        {
            status = UX_SUCCESS;
        }
        else
        {
            status = UX_ERROR;
        }
        break;

    case UX_DCD_CHANGE_STATE:

        status =  UX_SUCCESS;
        break;

    case UX_DCD_ENDPOINT_STATUS:

        status =  _ux_dcd_ip3511_endpoint_status(dcd_ip3511, (ULONG) parameter);
        break;

    default:

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_DCD, UX_FUNCTION_NOT_SUPPORTED);

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_FUNCTION_NOT_SUPPORTED, 0, 0, 0, UX_TRACE_ERRORS, 0, 0)

        status =  UX_FUNCTION_NOT_SUPPORTED;
        break;
    }

    /* Return completion status.  */
    return(status);
}

